#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
using namespace std;

#include <queue>
#include <vector>
#include <chrono>
#include "Tetris.h"
#include <ncurses.h>
#include <thread>
#include <map>
#include <mutex>
#include <condition_variable>


char saved_key = 0;
int tty_cbreak(int fd);	/* put terminal into cbreak mode */
int tty_reset(int fd);	/* restore terminal's mode */

/* Read 1 character - echo defines echo mode */
char getChar() {
  char ch;
  int n;
  while (1) {
    tty_cbreak(0);
    n = read(0, &ch, 1);
    tty_reset(0);
    if (n > 0)
      break;
    else if (n < 0) {
      if (errno == EINTR) {
	if (saved_key != 0) {
	  ch = saved_key;
	  saved_key = 0;
	  break;
	}
      }
    }
  }
  return ch;
}

#define MAX_BLK_TYPES 7
#define MAX_BLK_DEGREES 4

int T3D0[] = { 1, 1, 1, 1, -1 }; //O
int T3D1[] = { 1, 1, 1, 1, -1 };
int T3D2[] = { 1, 1, 1, 1, -1 };
int T3D3[] = { 1, 1, 1, 1, -1 };

int T5D0[] = { 0, 1, 0, 1, 1, 1, 0, 0, 0, -1 }; //T
int T5D1[] = { 0, 1, 0, 0, 1, 1, 0, 1, 0, -1 };
int T5D2[] = { 0, 0, 0, 1, 1, 1, 0, 1, 0, -1 };
int T5D3[] = { 0, 1, 0, 1, 1, 0, 0, 1, 0, -1 };

int T1D0[] = { 1, 0, 0, 1, 1, 1, 0, 0, 0, -1 }; //J
int T1D1[] = { 0, 1, 1, 0, 1, 0, 0, 1, 0, -1 };
int T1D2[] = { 0, 0, 0, 1, 1, 1, 0, 0, 1, -1 };
int T1D3[] = { 0, 1, 0, 0, 1, 0, 1, 1, 0, -1 };

int T2D0[] = { 0, 0, 1, 1, 1, 1, 0, 0, 0, -1 }; //L
int T2D1[] = { 0, 1, 0, 0, 1, 0, 0, 1, 1, -1 };
int T2D2[] = { 0, 0, 0, 1, 1, 1, 1, 0, 0, -1 };
int T2D3[] = { 1, 1, 0, 0, 1, 0, 0, 1, 0, -1 };

int T6D0[] = { 1, 1, 0, 0, 1, 1, 0, 0, 0, -1 }; //Z
int T6D1[] = { 0, 0, 1, 0, 1, 1, 0, 1, 0, -1 };
int T6D2[] = { 0, 0, 0, 1, 1, 0, 0, 1, 1, -1 };
int T6D3[] = { 0, 1, 0, 1, 1, 0, 1, 0, 0, -1 };

int T4D0[] = { 0, 1, 1, 1, 1, 0, 0, 0, 0, -1 }; //S
int T4D1[] = { 0, 1, 0, 0, 1, 1, 0, 0, 1, -1 };
int T4D2[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, -1 };
int T4D3[] = { 1, 0, 0, 1, 1, 0, 0, 1, 0, -1 };

int T0D0[] = { 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, -1 }; //I
int T0D1[] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, -1 };
int T0D2[] = { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1 };
int T0D3[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, -1 };

int *setOfBlockArrays[] = {
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};
//*****************Model,View제외 depth1과 모두 동일)*****************//
bool isGameDone=false; //게임이 끝나야하는지 판단하는 변수
std::mutex mu; //창 그리는데 사용할 뮤텍스 mu

void printWindow(WINDOW *window, Matrix& screen){ //전달받은 창에 전달받은 화면(행렬)을 출력해주는 함수
  int **array=screen.get_array(); //screen을 2차원 배열로 받아서 array에 저장
  int dw=Tetris::iScreenDw; //테트리스의 dw길이 받기
  int dy=screen.get_dy(); //screen으로부터 y(세로)길이 받기
  int dx=screen.get_dx(); //screen으로부터 x(가로)길이 받기
  wclear(window); //전달받은 창 지우기
  for(int y=0;y<(dy-dw);y++){ //반복문 통해 array의 각 원소들 값 확인
    //echo(); 
    for(int x=dw;x<dx-dw;x++){ 
      if(array[y][x]==0) //원소의 값이 0이면 창에 빈 칸 출력
        waddstr(window,"□");
      else if(array[y][x]==1) //원소의 값이 1이면 창에 채워진 칸 출력
        waddstr(window,"■");
      else waddstr(window,"XX"); //둘다 아니면 창에 XX출력
    }
    waddstr(window,"\n"); //한 줄 다 그리면 줄바꿈
  }
  mu.lock(); //락 걸기
  wrefresh(window); //창 갱신
  mu.unlock(); //락 해제
}
void printMsg(string msg){ //전달받은 문자열을 win0에 출력해주는 함수
  const char* c=msg.c_str(); //wprintw함수는 인자로 문자열을 받을 수 없고 char배열 형태로 주어야 함
  //따라서 전달받은 문자열을 char배열 형태로 바꿔줌
  WINDOW *win0; 
  win0=newwin(3, 70, 21, 0); //70*3 크기의 win0을 (0,21)좌표에 생성
  wclear(win0); //win0창 지우기
  wprintw(win0,c); //win0에 문자열 출력(사실상 char* 형태로 바꾼 것을 출력)
  mu.lock(); //락 걸기
  wrefresh(win0); //win0 갱신 (갱신을 해야 지금까지 해당 창에 적용한 것들이 실제로 보임)
  mu.unlock(); //락 해제
}

class Observer{ //Observer 클래스 (depth1과 동일)
    public:
        virtual void update(char key){ //Controllers(pub)-Model(obs) 관계에서 사용하는 update
            printMsg("Observer's update(key)");
        }
        virtual void update(Matrix* screen){ //Model(pub)-View(obs) 관계에서 사용하는 update
            printMsg("Observer's update(screen)");
        }
};
class Publisher{ //Publisher 클래스 (depth1과 동일)
    public:
        virtual void addObserver(Observer* observer){ //Observer를 추가하는 함수
            printMsg("Publisher's addObserver");
        }
        virtual void notifyObservers(char key){ //Controllers(pub)-Model(obs) 관계에서 사용하는 notifyObservers
            printMsg("Publisher's notifyObservers(key)");
        }
        virtual void notifyObservers(Matrix* screen){ //Model(pub)-View(obs) 관계에서 사용하는 notifyObservers
            printMsg("Publisher's notifyObservers(screen)");
        }
};
class View:public Observer{ //View 클래스
    //Observer 클래스를 상속하며 observer 역할을 함
    public:
        string name; //객체 이름
        std::queue<Matrix*> Screens; //Screen 포인터를 저장할 큐 Screens 생성
        WINDOW* win; //객체가 사용할 창 지정하기 위한 win포인터 생성
        std::condition_variable cv; 
	//Screen포인터 읽어오는 과정에서 실행 순서를 지정해줄 조건변수 생성
        std::mutex m; //조건변수 보호할 뮤텍스 생성
        Tetris* board; //객체의 publisher가 사용하고 있는 테트리스 객체(나중에 delete 위함)
        View(string Name){ //View 객체 생성자
            name=Name; //인자로 받은 문자열을 이름으로 설정
        }
        virtual void update(Matrix* screen){ 
	//Model(pub)에서 받은 screen포인터 값을 Screens에 저장하는 함수
            std::unique_lock<std::mutex> lk(m); //락 걸기
            Screens.push(screen); //Model(pub)로부터 전달받은 screen포인터 값 Screens에 추가
            cv.notify_all(); //notify 이용해 잠들어 있던 스레드 깨우기
            lk.unlock(); //락 해제
        }
        Matrix* read(){ //Screens에서 screen포인터 값 가져와 반환하는 함수
            std::unique_lock<std::mutex> lk(m); //락 걸기
            
            while(Screens.size()<1){ //Screens 길이가 1보다 작다면
                cv.wait(lk); //wait 이용해 스레드 잠들게 하기
            }
            Matrix* obj=Screens.front(); //Screens의 제일 앞에 있는 값 obj에 저장
            Screens.pop(); //obj에 저장한 값 Screens에서 삭제
            lk.unlock(); //락 해제
            return obj; //obj값 반환
        }
        void addWindow(WINDOW* window){ //창 설정하는 함수
            win=window; //전달받은 창을 객체의 창으로 설정
        } 
        void run(){ //스레드 돌릴 함수
            while(!isGameDone){ //isGameDone이 false면
                Matrix* screen=read(); //read함수를 통해 screen의 포인터 값 받아옴
                if(screen==nullptr) break; //읽어온 포인터 값이 nullptr이면 반복문 탈출 후 종료
                printWindow(win,*screen); //아니면 printWindow로 해당 screen출력
            }
            delete board; //객체의 publisher의 테트리스 객체 delete
            string str=name+" is terminated...";
            printMsg(str); //printMsg통해 종료 메시지 출력
            sleep(1); //1초 쉬기
        }
        std::thread callme(){ //스레드 시작해주는 함수
            return std::thread(&View::run,this);
	    //객체의 run함수를 이용해서 스레드 시작하고 해당 스레드 반환
        }
};
class Model:public Observer,public Publisher{ //Model 클래스
    //Publisher클래스와 Observer클래스를 상속하며 
    //controllers-model에서는 observer역할을, model-view에서는 publisher역할을 함
    public:
        string name; //객체 이름
        std::vector<View*> observers; //객체의 observer(View)를 관리하기 위해 벡터 observers 생성
        std::queue<char> Keys; //key값을 저장할 큐 Keys 생성
        std::map<char,char> Keypad; //객체가 사용할 키패드 지정하기 위한 Keypad 생성
        std::condition_variable cv; 
	//key값을 읽어오는 과정에서 실행 순서를 지정해줄 조건변수 생성 
        std::mutex m; //조건변수 보호할 뮤텍스 생성
        
        Model(string Name){ //Model 객체 생성자
            name=Name; //인자로 받은 문자열을 이름으로 설정
        }
        virtual void addObserver(View* view){ //객체의 observer(View*)를 추가하는 함수
            observers.push_back(view); //observers에 전달받은 observer(View*) 추가
        }
        virtual void notifyObservers(Matrix* screen){ 
	//객체의 observer들에게 screen포인터를 전해주는 함수
            for(int i=0;i<observers.size();i++){ //observers에 있는 각 observer 객체들은
                observers[i]->update(screen);
		//각자 자신의 Screens에 전달받은 screen포인터 값을 update해줌
            }
        }
        virtual void update(char key){ //Controllers(pub)에서 받은 key값을 Keys에 저장하는 함수
            std::unique_lock<std::mutex> lk(m); //락 걸기
            Keys.push(key); //Controllers(pub)로 부터 전달받은 key값 Keys에 추가
            cv.notify_all(); //notify 이용해 잠들어 있던 스레드 깨우기
            lk.unlock(); //락 해제
        }
        void addKeypad(map<char,char>& keypad){ //키패드 설정하는 함수
            Keypad=keypad; //인자로 받은 키패드를 객체의 키패드로 설정
        }
        char read(){ //Keys에서 key값 가져와 반환하는 함수
            std::unique_lock<std::mutex> lk(m); //락 걸기
            while(Keys.size()<1){ //Keys길이가 1보다 작으면
                cv.wait(lk); //wait 이용해 스레드 잠들게 하기
            }
            char key=Keys.front(); //Keys의 제일 앞에 있는 값 key에 저장
            Keys.pop(); //key에 저장한 값 Keys에서 삭제
            lk.unlock(); //락 해제
            return key; //key값 반환
        }
        TetrisState processKey(Tetris* board, char Key){
	//기존 cpptet의 processKey와 동일
	//depth1에서는 drawScreen->printWindow로 바뀌었는데 (+창도 지정해줘야 했음)
	//depth2에서는 그 printWindow->notifyObservers로 바뀐 것 뿐임
	//depth2에서는 printWindow역할을 View가 하고 있기 때문
	//따라서 창도 View에서 지정되어 있음
            TetrisState state=board->accept(Key);
            notifyObservers(&(board->oScreen));
            if(state!=NewBlock) return state;

            srand((unsigned int)time(NULL));
            char key = (char)('0' + rand() % MAX_BLK_TYPES);
            state=board->accept(key);
            notifyObservers(&(board->oScreen));

            if(state!=Finished) return state;
            return state;
        }
        void run(){ //스레드 돌릴 함수
            Tetris *board=new Tetris(20,15); //동적할당으로 테트리스 객체 생성
            for(int i=0;i<observers.size();i++){ //객체의 observer들의 테트리스 객체를
                observers[i]->board=board; //객체의 테트리스 객체로 설정
            }
            TetrisState state;
            char key;

            srand((unsigned int)time(NULL));
            key = (char)('0' + rand() % MAX_BLK_TYPES); //랜덤으로 key값 하나 가져옴(블록종류)
            state=board->accept(key); //accept에 key값을 넣어 테트리스 상태 반환 받기
            notifyObservers(&(board->oScreen)); 
	    //객체의 observer들에게 현재 테트리스의 oScreen 전달
	    //그러면 observer(View객체)들이 창에 현재 테트리스 화면 출력해줌
            while(!isGameDone){ //isGameDone이 false라면 
                key=read(); //read함수를 통해 key값 받아옴
                if(!key) break; //읽어온 key값이 빈값이면 반복문 탈출 후 종료
                if(Keypad.find(key)==Keypad.end()) continue; //읽어온 key값이 객체의 키패드에 없다면 그냥 넘기기
                key=Keypad[key]; //있으면 키패드에 해당하는 값으로 key값 재설정
                if(key=='q') state=Finished; //만약 key값이 'q'이면 state를 Finished로 설정
                else state=processKey(board, key); //아니라면 processKey통해 반환받은 테트리스 게임상태를 state에 저장
                if(state==Finished){ //만약 state가 Finished면
                    isGameDone=true; //isGameDone을 true로 바꿈
                    string str=name+" is dead!!"; 
                    printMsg(str); //printMsg통해 진 객체 알리기
                    sleep(2); //2초 쉬기
                    break; //반복문 탈출 후 종료
                }
            }
            string str=name+" is terminated...";
            printMsg(str); //printMsg통해 종료 메시지 출력
            sleep(1); //1초 쉬기
            notifyObservers(nullptr); //객체의 observer(View)들에게 빈 포인터값 전달
        }
        
        std::thread callme(){ //스레드 시작해주는 함수
            return std::thread(&Model::run,this);
	    //객체의 run함수를 이용해서 스레드 시작하고 해당 스레드 반환
        }
};
class KeyController:public Publisher{ //KeyController 클래스 (depth1과 동일)
    //Publisher 클래스를 상속하며 publisher 역할을 함
    public:
        string name; //객체 이름
        std::vector<Model*> observers; //객체의 observer를 관리하기 위해 벡터 observers 생성
        //복사해서 갖고있는건 의미가 없기에 포인터로 주소를 받는다.
	
	KeyController(string Name){ //KeyController 객체 생성자
            name=Name; //인자로 받은 문자열을 이름으로 저장
        }
        virtual void addObserver(Model* model){ //객체의 observer(Model*)를 추가하는 함수
            observers.push_back(model); //observers에 전달받은 observer(Model*) 추가
        }
        virtual void notifyObservers(char key){ //객체의 observer들에게 key값을 전해주는 함수
            for(int i=0;i<observers.size();i++){ //observers에 있는 각 observer객체들은 
                observers[i]->update(key);
		//각자 자신의 Keys에 전달받은 key값을 update해줌
            }
        }
        void run(){ //스레드 돌릴 함수
            while(!isGameDone){ //isGameDone이 false라면
                try{ //에러가 발생할 수 있는 코드(key값이 입력되지 않는 경우)
                char key=getChar(); //getChar을 통해 사용자로부터 key값 받아오기
                notifyObservers(key); //받아온 key값을 notifyObservers통해 객체의 observer들에게 전달
                if(!key) throw runtime_error("key interrupt");
		//만약 key값이 입력되지 않았으면(런타임 에러)
                } catch (runtime_error err){
                //printMsg("pass");
		//그냥 넘어가기
                }
            }
            string str=name+" is terminated...";
            printMsg(str); //printMsg통해 종료 메시지 출력
            sleep(1); //1초 쉬기
            notifyObservers(0); //객체의 observer들에게 빈 key값 전달
        }
        std::thread callme(){ //스레드 시작해주는 함수
            return std::thread(&KeyController::run,this);
	    //객체의 run함수를 이용해서 스레드 시작하고 해당 스레드 반환
        }
};
class TimeController:public Publisher{ //TimeController 클래스 (depth1과 동일)
    //Publisehr 클래스를 상속하며 publisher 역할을 함
    public: 
        string name; //객체 이름
        std::vector<Model*> observers; //객체의 observer를 관리하기 위해 벡터 observers 생성
        
	TimeController(string Name){ //TimeController 객체 생성자
            name=Name; //인자로 받은 문자열을 이름으로 저장
        }
        virtual void addObserver(Model* model){ //객체의 observer(Model*)를 추가하는 함수
            observers.push_back(model); //observers에 전달받은 observer(Model*) 추가
        }
        virtual void notifyObservers(char key){ //객체의 observer들에게 key값을 전해주는 함수
            for(int i=0;i<observers.size();i++){ //observers에 있는 각 observer 객체들은 
                observers[i]->update(key);
		//각자 자신의 Keys에 전달받은 key값을 update해줌
            }
        }
        void run(){ //스레드 돌릴 함수
            while(!isGameDone){ //isGameDone이 false라면
                sleep(1); //1초 쉬기
                notifyObservers('y'); //notifyObservers통해 객체의 observer들에게 key값 'y'를 전달
            }
            string str=name+" is terminated...";
            printMsg(str); //printMsg통해 종료 메시지 출력
            sleep(1); //1초 쉬기
            notifyObservers(0); //객체의 observer들에게 빈 key값 전달
        }
        std::thread callme(){ //스레드 시작해주는 함수
            return std::thread(&TimeController::run,this);
	    //객체의 run함수를 이용해서 스레드 시작하고 해당 스레드 반환
        }
};
int main(){
    setlocale(LC_ALL,""); //□, ■ 출력할 수 있게 함
    initscr(); //curses모드 시작 (curses를 사용하려면 반드시 초기화 해주어야 함)
    clear(); //창 지우기
    //echo(); //echo모드 사용 (사용자로부터 입력받은 문자를 출력할 수 있도록 설정)
    //getch를 사용하는 경우를 말함 (이번 코드에서 사용하지 않아도 될 듯 함)
    start_color(); //color사용할 수 있게 함
    use_default_colors(); //기본 색상 사용할 수 있게 함
    Tetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES); //테트리스 초기화 (블록 설정)
    //스레드 안에서 실행하면 setOfBlockArrays(전역변수)에 동시접근 가능해서 segmentaion fault가 뜨는 것 같습니다. 
    WINDOW *win1,*win2; //테트리스가 진행될 창 win1, win2 생성
    win1=newwin(20, 30, 0, 0); //win1은 20*30크기로 (0,0)좌표에 생성
    win2=newwin(20, 30, 0, 60); //win2는 20*30크기로 (60,0)좌표에 생성

    map<char,char> keypad1={{'q','q'},{'w','w'},{'a','a'},{'s','y'},{'d','d'},{' ',' '},{'y','y'}};
    map<char,char> keypad2={{'u','q'},{'i','w'},{'j','a'},{'k','y'},{'l','d'},{'\r',' '},{'y','y'}};

    View th_view1("view1"); //view1객체 생성
    th_view1.addWindow(win1); //view1객체의 창을 win1로 설정
    View th_view2("view2"); //view2객체 생성
    th_view2.addWindow(win2); //view2객체의 창을 win2로 설정
    
    Model th_model1("model1"); //model1객체 생성
    th_model1.addKeypad(keypad1); //model1객체의 키패드를 keypad1으로 설정
    th_model1.addObserver(&th_view1); //model1객체의 observer로 view1등록

    Model th_model2("model2"); //model2객체 생성
    th_model2.addKeypad(keypad2); //model2객체의 키패드를 keypad2로 설정
    th_model2.addObserver(&th_view2); //model2객체의 observer로 view2등록

    KeyController th_cont1("kcont"); //keycontrol객체 생성
    th_cont1.addObserver(&th_model1); //keycontrol객체의 observer로 model1등록
    th_cont1.addObserver(&th_model2); //keycontrol객체의 observer로 model2등록

    TimeController th_cont2("tcont"); //timecontrol객체 생성
    th_cont2.addObserver(&th_model1); //timecontrol객체의 observer로 model1등록
    th_cont2.addObserver(&th_model2); //timecontrol객체의 observer로 model2등록
	
    std::vector<std::thread> threads; //스레드들을 담을 벡터 threads 생성
    //각 객체들의 callme함수 호출하여 스레드 반환 받고 threads에 각 스레드 추가하기
    threads.push_back(th_view1.callme()); 
    threads.push_back(th_view2.callme());
    threads.push_back(th_model1.callme());
    threads.push_back(th_model2.callme());
    threads.push_back(th_cont1.callme());
    threads.push_back(th_cont2.callme());

    for(int i=0;i<threads.size();i++){ //반복문 통해 각 스레드 종료시키기
        threads[i].join();
    }
    Tetris::kinit(); //테트리스 전역변수 중 동적할당된 setOfBlockObject를 delete해줌
    //이것을 각 객체가 소멸할 때 delete board에서 함께 실행되도록 했더니 setOfBlockObject이
    //두 번 delete되어 invalid pointer 에러가 생기기 때문에 한번만 delete되도록 마지막에 해줌
    printMsg("Program terminated!\n"); //printMsg통해 프로그램 종료 메시지 출력
    endwin(); //curses모드 종료
    return 0; 
}
