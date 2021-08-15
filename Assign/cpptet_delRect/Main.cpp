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

//#include <queue>

#include <vector>
#include <chrono>
#include "CTetris.h"
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

int *setOfCBlockArrays[] = {
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
  int dw=CTetris::iScreenDw; //테트리스의 dw길이 받기
  int dy=screen.get_dy(); //screen으로부터 y(세로)길이 받기
  int dx=screen.get_dx(); //screen으로부터 x(가로)길이 받기
  wclear(window); //전달받은 창 지우기
  for(int y=0;y<(dy-dw);y++){ //반복문 통해 array의 각 원소들 값 확인
    //echo(); 
    for(int x=dw;x<dx-dw;x++){ 
        wattron(window,COLOR_PAIR(7));
        if(array[y][x]==0)
            waddstr(window,"□");
        else{
            wattron(window, COLOR_PAIR(array[y][x]));
            waddstr(window,"■");
        }
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
class KeyObserver{
    public:
        virtual void update(char key){}
};
class KeyPublisher{
    public:
        virtual void addObserver(KeyObserver* observer){}
        virtual void notifyObservers(char key){}
};
class OutScreenObserver{
    public:
        virtual void update(Matrix* screen){}
};
class OutScreenPublisher{
    public:
        virtual void addObserver(OutScreenObserver* observer){}
        virtual void notifyObservers(Matrix* screen){}
};
class delRectObserver{
    public:
        virtual void update(Matrix delRect){}
};
class delRectPublisher{
    public:
        virtual void addObserver(delRectObserver* observer){}
        virtual void notifyObservers(Matrix delRect){}
};

class View:public OutScreenObserver{ //View 클래스
    public:
        string name; //객체 이름
        std::queue<Matrix*> Screens; //Screen 포인터를 저장할 큐 Screens 생성
        WINDOW* win; //객체가 사용할 창 지정하기 위한 win포인터 생성
        std::condition_variable cv; 
	//Screen포인터 읽어오는 과정에서 실행 순서를 지정해줄 조건변수 생성
        std::mutex m; //조건변수 보호할 뮤텍스 생성
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
                printWindow(win, *screen); //아니면 printWindow로 해당 screen출력
            }
            string str=name+" is terminated...";
            printMsg(str); //printMsg통해 종료 메시지 출력
            sleep(1); //1초 쉬기
        }
};
class Model:public KeyObserver,public OutScreenPublisher,public delRectObserver,public delRectPublisher{ //Model 클래스
    public:
        string name; //객체 이름
        std::vector<View*> observers; //객체의 OutScreenObserver를 저장할 벡터 observers
        std::vector<Model*> models; //객체의 delRectObserver를 저장할 벡터 models
        std::queue<Obj> objs; //테트리스 객체에 전달할 값(키값 or delRect) 저장하는 큐 objs
        std::map<char,char> Keypad; //객체가 사용할 키패드
        std::condition_variable cv; //조건변수
        std::mutex m; //뮤텍스
        
        Model(string Name){ //Model 객체 생성자
            name=Name; 
        }
        virtual void addObserver(View* view){ //객체의 OutScreenObserver를 추가하는 함수
            observers.push_back(view); 
        }
        virtual void notifyObservers(Matrix* screen){ //객체의 OutScreenObserver들에게 출력할 screen을 알리는 notifyObservers
            for(int i=0;i<observers.size();i++){ //각 observer들의 update함수를 호출하여 출력할 screen전달
                observers[i]->update(screen); 
            }
        }
        virtual void update(char key){ //객체의 KeyPublisher로부터 테트리스 객체에 전달할 값(키 값)을 받아오는 함수
            std::unique_lock<std::mutex> lk(m);
            Obj obj(key); //전달받은 키 값을 이용하여 Obj 객체 obj 생성
            objs.push(obj); //obj을 objs에 추가
            cv.notify_all(); 
            lk.unlock(); 
        }
        void addKeypad(map<char,char>& keypad){ //객체의 키패드를 설정하는 함수
            Keypad=keypad; 
        }
        virtual void addObserver(Model* model){ //객체의 delRectObserver를 추가하는 함수
            models.push_back(model); 
        }
        virtual void update(Matrix delRect){ //객체의 delRectPublisher로부터 테트리스 객체에 전달할 값(delRect)을 받아오는 함수
            std::unique_lock<std::mutex> lk(m);
            Obj obj(delRect); //전달받은 delRect 이용하여 Obj 객체 obj 생성
            objs.push(obj); //obj을 objs에 추가
            cv.notify_all();
            lk.unlock();
        }
        virtual void notifyObservers(Matrix delRect){ //객체의 delRectObserver들에게 테트리스 객체에 전달할 값(delRect)을 알리는 notifyObservers
            for(int i=0;i<models.size();i++){ //각 observer들의 update함수를 호출하여 테트리스 객체에 전달할 값 delRect전달 
                models[i]->update(delRect);
            }
        }
        Obj read(){ //테트리스 객체에 전달할 값(Obj객체) 반환해주는 함수
            std::unique_lock<std::mutex> lk(m);
            while(objs.size()<1){ 
                cv.wait(lk);
            }
            Obj obj=objs.front(); //objs에서 맨 앞에 있는 obj 뽑아서
            objs.pop(); //(가져온 후 삭제)
            lk.unlock();
            return obj; //해당 obj 반환
        }
        TetrisState processKey(CTetris* board, Obj obj){ //테트리스 객체에 obj가 적용되는 과정을 담은 함수 
            TetrisState state=board->accept(obj); //obj를 Tetris::accept에 넘겨 상태 확인
            notifyObservers(&(board->oCScreen)); //이에 따른 테트리스 상태를 출력하기 위해 OutScreenObserver들에게 출력할 screen 전달
            if((state!=NewBlockWDel)&&(state!=NewBlockWODel)) return state; //만약 obj 적용 후 상태가 Finished / Running 이라면 해당 상태 반환
            //아니면 NewBlockWDel / NewBlockWODel 상태이므로 일단 새 블록 생성
            srand((unsigned int)time(NULL)); 
            char Key = (char)('0' + rand() % MAX_BLK_TYPES);
            obj.key=Key; //현재 obj의 키 값을 새 블록의 키 값으로 바꾸고
            state=board->accept(obj); //다시 한 번 obj를 Tetris::accept에 넘겨 상태 확인
            if(state==NewBlockWDel){ //만약 상태가 NewBlockWDel이면
                notifyObservers(board->getDelRect()); //getDelRect로 테트리스 객체의 delRect를 얻어와서 delRectObserver에 전달
            }
            notifyObservers(&(board->oCScreen)); //마지막으로 다시 한 번 현재 화면 출력하기 위해 OutScreenObserver들에게 출력할 screen 전달

            return state; //현재 상태 반환
        }
        void run(){ //스레드 돌릴 함수
            CTetris *board=new CTetris(20,15); //테트리스 객체 생성(board)
            TetrisState state; //테트리스 상태 확인 (Running, NewBlockWDel, NewBlockWODel, Finished)
            Obj obj; //테트리스 객체에 전달할 값을 담은 Obj 객체 obj

            srand((unsigned int)time(NULL)); //처음엔 랜덤 키로 새 블록 생성해주고 시작
            char Key = (char)('0' + rand() % MAX_BLK_TYPES); 
            obj.key=Key;
            state=board->accept(obj); 
            notifyObservers(&(board->oCScreen));
		
            while(!isGameDone){ //반복문을 돌며 테트리스 진행 (크게 전달할 값 저장-> 테트리스에 적용-> 상태 확인 순서로 진행됨)
                obj=read(); //테트리스 객체에 전달할 값 obj 받아오기
                if(!obj.key) break; //obj의 키 값이 없으면 반복문 탈출 후 게임 종료
                if(Keypad.find(obj.key)==Keypad.end()) continue; //객체의 키패드에 obj의 키 값이 없으면 다음 턴으로 넘기기
                obj.key=Keypad[obj.key]; //있으면 해당하는 값으로 obj의 키 값 재설정
                //if(models.size()==0) printMsg(name+" "+obj.delRect.print()+" "+to_string(obj.key));
                if(obj.key=='q') state=Finished; //obj의 키 값이 q이면 상태를 Finished로 설정
                else state=processKey(board, obj); //아니면 processKey통해 obj를 테트리스에 적용
                if(state==Finished){ //만약 상태가 Finished라면
                    isGameDone=true; //isGameDone을 true로 설정
                    string str=name+" is dead!!"; 
                    printMsg(str); 
                    sleep(2);
                    break; //반복문 탈출 후 게임 종료
                }
            }
            delete board;
            string str=name+" is terminated...";
            printMsg(str); //printMsg통해 종료 메시지 출력
            sleep(1); //1초 쉬기
            notifyObservers(nullptr); //객체의 observer(View)들에게 빈 포인터값 전달
        }
};
class KeyController:public KeyPublisher{ //KeyController 클래스 (depth1과 동일)
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
};
class TimeController:public KeyPublisher{ //TimeController 클래스 (depth1과 동일)
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
};
int main(){
    setlocale(LC_ALL,""); //□, ■ 출력할 수 있게 함
    initscr(); //curses모드 시작 (curses를 사용하려면 반드시 초기화 해주어야 함)
    clear(); //창 지우기
    //echo(); //echo모드 사용 (사용자로부터 입력받은 문자를 출력할 수 있도록 설정)
    //getch를 사용하는 경우를 말함 (이번 코드에서 사용하지 않아도 될 듯 함)
    start_color(); //color사용할 수 있게 함
    //use_default_colors(); //기본 색상 사용할 수 있게 함
    init_pair(1,COLOR_RED,COLOR_BLACK);
    init_pair(2,COLOR_GREEN,COLOR_BLACK);
    init_pair(3,COLOR_YELLOW,COLOR_BLACK);
    init_pair(4,COLOR_BLUE,COLOR_BLACK);
    init_pair(5,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(6,COLOR_CYAN,COLOR_BLACK);
    init_pair(7,COLOR_WHITE,COLOR_BLACK);
    CTetris::init(setOfCBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES); //테트리스 초기화 (블록 설정)
    //스레드 안에서 실행하면 setOfBlockArrays(전역변수)에 동시접근 가능해서 segmentaion fault가 뜨는 것 같습니다. 
    WINDOW *win1,*win2; //테트리스가 진행될 창 win1, win2 생성
    win1=newwin(20, 30, 0, 0); //win1은 20*30크기로 (0,0)좌표에 생성
    win2=newwin(20, 30, 0, 60); //win2는 20*30크기로 (60,0)좌표에 생성

    map<char,char> keypad1={{'q','q'},{'w','w'},{'a','a'},{'s','y'},{'d','d'},{' ',' '},{'y','y'},{-1,-1}};
    map<char,char> keypad2={{'u','q'},{'i','w'},{'j','a'},{'k','y'},{'l','d'},{'\r',' '},{'y','y'},{-1,-1}};

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

    th_model1.addObserver(&th_model2);
    th_model2.addObserver(&th_model1);

    KeyController th_cont1("kcont"); //keycontrol객체 생성
    th_cont1.addObserver(&th_model1); //keycontrol객체의 observer로 model1등록
    th_cont1.addObserver(&th_model2); //keycontrol객체의 observer로 model2등록

    TimeController th_cont2("tcont"); //timecontrol객체 생성
    th_cont2.addObserver(&th_model1); //timecontrol객체의 observer로 model1등록
    th_cont2.addObserver(&th_model2); //timecontrol객체의 observer로 model2등록
	
    std::vector<std::thread> threads; //스레드들을 담을 벡터 threads 생성
    //threads에 각 스레드 추가하기
    threads.push_back(std::thread(&View::run, &th_view1)); 
    threads.push_back(std::thread(&View::run, &th_view2));
    threads.push_back(std::thread(&Model::run, &th_model1));
    threads.push_back(std::thread(&Model::run, &th_model2));
    threads.push_back(std::thread(&KeyController::run, &th_cont1));
    threads.push_back(std::thread(&TimeController::run, &th_cont2));

    for(int i=0;i<threads.size();i++){ //반복문 통해 각 스레드 종료시키기
        threads[i].join();
    }
    printMsg("Program terminated!\n"); //printMsg통해 프로그램 종료 메시지 출력
    endwin(); //curses모드 종료
    return 0; 
}
