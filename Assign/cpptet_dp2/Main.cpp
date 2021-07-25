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
bool isGameDone=false;
std::mutex mu;

void printWindow(WINDOW *window, Matrix& screen){
  int **array=screen.get_array();
  int dw=Tetris::iScreenDw;
  int dy=screen.get_dy();
  int dx=screen.get_dx();
  wclear(window);
  for(int y=0;y<(dy-dw);y++){
    echo();
    for(int x=dw;x<dx-dw;x++){
      if(array[y][x]==0)
        waddstr(window,"□");
      else if(array[y][x]==1)
        waddstr(window,"■");
      else waddstr(window,"XX");
    }
    waddstr(window,"\n");
  }
  mu.lock();
  wrefresh(window);
  mu.unlock();
}
void printMsg(string msg){
  const char* c=msg.c_str();
  WINDOW *win0;
  win0=newwin(3, 70, 21, 0);
  wclear(win0);
  wprintw(win0,c);
  mu.lock();
  wrefresh(win0);
  mu.unlock();
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
        virtual void addObserver(Observer* observer){
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
    public:
        string name; //객체 이름
        std::queue<Matrix*> Screens; //Screen 포인터를 저장할 큐 Screens 생성
        WINDOW* win; //객체가 사용할 창 지정하기 위한 win포인터 생성
        std::condition_variable cv; 
	//Screen포인터 읽어오는 과정에서 실행 순서를 지정해줄 조건변수 생성
        std::mutex m; //조건변수 보호할 뮤텍스 생성
        Tetris* board;
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
                printWindow(win,*screen); //아니면 printWindow로 해당 screen
            }
            delete board;
            string str=name+" is terminated...";
            printMsg(str); //printMsg통해 종료 메시지 출력
            sleep(1); //1초 쉬기
        }
        void callme(std::vector<std::thread> *threads){ //스레드 시작해주는 함수
            threads->push_back(std::thread(&View::run,this));
	    //객체의 run함수를 이용해서 스레드 시작하고 threads에 해당 스레드 추가
        }
};
class Model:public Observer,public Publisher{ //Model 클래스
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
        virtual void addObserver(View* view){ //객체의 observer(View)를 추가하는 함수
            observers.push_back(view); //observers에 전달받은 observer(View포인터) 추가
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
	//depth1의 processKey와 동일
	//(printWindow->notifyObservers로 바뀌고 더이상 창을 인자로 받지 않을 뿐임)
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
	//depth1의 ModelView::run과 동일 
	//(printWindow->notifyObservers로 바뀐 것 뿐임)
            Tetris *board=new Tetris(20,15);
            for(int i=0;i<observers.size();i++){
                observers[i]->board=board;
            }
            TetrisState state;
            char key;

            srand((unsigned int)time(NULL));
            key = (char)('0' + rand() % MAX_BLK_TYPES);
            state=board->accept(key);
            notifyObservers(&(board->oScreen));

            while(!isGameDone){
                key=read();
                if(!key) break;
                if(Keypad.find(key)==Keypad.end()) continue;
                key=Keypad[key];
                if(key=='q') state=Finished;
                else state=processKey(board, key);
                if(state==Finished){
                    isGameDone=true;
                    string str=name+" is dead!!";
                    printMsg(str);
                    sleep(2);
                    break;
                }
            }
            string str=name+" is terminated...";
            printMsg(str); //printMsg통해 종료 메시지 출력
            sleep(1); //1초 쉬기
            notifyObservers(nullptr); //객체의 observer(View)들에게 빈 포인터값 전달
        }
        
        void callme(std::vector<std::thread> *threads){ //스레드 시작해주는 함수
            threads->push_back(std::thread(&Model::run,this));
	    //객체의 run함수를 이용해서 스레드 시작하고 threads에 해당 스레드 
        }
};
class KeyController:public Publisher{ //KeyController 클래스 (depth1과 동일)
    public:
        string name;
        std::vector<Model*> observers;
        KeyController(string Name){
            name=Name;
        }
        virtual void addObserver(Model* model){
            observers.push_back(model);
        }
        virtual void notifyObservers(char key){
            for(int i=0;i<observers.size();i++){
                observers[i]->update(key);
            }
        }
        void run(){
            while(!isGameDone){
                try{
                char key=getChar();
                notifyObservers(key);
                if(!key) throw runtime_error("key interrupt\n");
                } catch (runtime_error err){
                //printMsg("pass");
                }
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1);
            notifyObservers(0);
        }
        void callme(std::vector<std::thread> *threads){
            threads->push_back(std::thread(&KeyController::run,this));
        }
};
class TimeController:public Publisher{ //TimeController 클래스 (depth1과 동일)
    public:
        string name;
        std::vector<Model*> observers;
        TimeController(string Name){
            name=Name;
        }
        virtual void addObserver(Model* model){
            observers.push_back(model);
        }
        virtual void notifyObservers(char key){
            for(int i=0;i<observers.size();i++){
                observers[i]->update(key);
            }
        }
        void run(){
            while(!isGameDone){
                sleep(1);
                notifyObservers('y');
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1);
            notifyObservers(0);
        }
        void callme(std::vector<std::thread> *threads){
            threads->push_back(std::thread(&TimeController::run,this));
        }
};
int main(){
    setlocale(LC_ALL,"");
    initscr();
    clear();
    //echo();
    start_color();
    use_default_colors(); 
    Tetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
    WINDOW *win1,*win2;
    win1=newwin(20, 30, 0, 0);
    win2=newwin(20, 30, 0, 60);

    map<char,char> keypad1={{'q','q'},{'w','w'},{'a','a'},{'s','y'},{'d','d'},{' ',' '},{'y','y'}};
    map<char,char> keypad2={{'u','q'},{'i','w'},{'j','a'},{'k','y'},{'l','d'},{'\r',' '},{'y','y'}};

    View th_view1("view1");
    th_view1.addWindow(win1);
    View th_view2("view2");
    th_view2.addWindow(win2);
    
    Model th_model1("model1");
    th_model1.addKeypad(keypad1);
    th_model1.addObserver(&th_view1);

    Model th_model2("model2");
    th_model2.addKeypad(keypad2);
    th_model2.addObserver(&th_view2);

    KeyController th_cont1("kcont");
    th_cont1.addObserver(&th_model1);
    th_cont1.addObserver(&th_model2);

    TimeController th_cont2("tcont");
    th_cont2.addObserver(&th_model1);
    th_cont2.addObserver(&th_model2);
    std::vector<std::thread> threads;
    th_view1.callme(&threads);
    th_view2.callme(&threads);
    th_model1.callme(&threads);
    th_model2.callme(&threads);
    th_cont1.callme(&threads);
    th_cont2.callme(&threads);

    for(int i=0;i<threads.size();i++){
        threads[i].join();
    }
    Tetris::kinit();
    printMsg("Program terminated!\n");
    endwin();
    return 0;
}
