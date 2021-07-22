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
void sigint_handler(int signo) {
  cout << "SIGINT received!" << endl;
  //do nothing;
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

std::mutex m;
std::mutex mu;
std::condition_variable cv;
std::vector<std::thread*> threads;

void printWindow(WINDOW *window, Matrix& screen){
  int **array=screen.get_array();
  int dw=CTetris::iScreenDw;
  int dy=screen.get_dy();
  int dx=screen.get_dx();
  wclear(window);
  for(int y=0;y<(dy-dw);y++){
    echo();
    for(int x=dw;x<dx-dw;x++){
      if(array[y][x]==0)
        waddstr(window,"AA");
      else if(array[y][x]==1)
        waddstr(window,"BB");
      else waddstr(window,"XX");
    }
  }
  m.lock();
  wrefresh(window);
  m.unlock();
}
void printMsg(string msg){
  const char* c=msg.c_str();
  WINDOW *win0;
  win0=newwin(3, 70, 21, 0);
  wclear(win0);
  wprintw(win0,c);
  m.lock();
  wrefresh(win0);
  m.unlock();
}

bool isGameDone=false;

class Observer{
    public:
        virtual void update(char key){
            printMsg("Observer's update(key)");
        }
        virtual void update(Matrix& screen){
            printMsg("Observer's update(screen)");
        }
};
class Publisher{
    public:
        virtual void addObserver(Observer& observer){
            printMsg("Publisher's addObserver");
        }
        virtual void notifyObservers(char key){
            printMsg("Publisher's notifyObservers(key)");
        }
        virtual void notifyObservers(Matrix& screen){
            printMsg("Publisher's notifyObservers(screen)");
        }
};
class View:public Observer{
    public:
        string name;
        std::queue<Matrix> Screens;
        WINDOW* win;
        //std::condition_variable cv;
        //std::mutex m;
        View(string Name){
            name=Name;
        }
        virtual void update(Matrix& screen){
            std::unique_lock<std::mutex> lk(m);
            Screens.push(screen);
            cv.notify_all();
            lk.unlock();
        }
        Matrix read(){
            std::unique_lock<std::mutex> lk(m);
            
            while(Screens.size()<1){
                cv.wait(lk);
            }
            Matrix obj=Screens.front();
            Screens.pop();
            lk.unlock();
            return obj;
        }
        void addWindow(WINDOW* window){
            win=window;
        }
        void run(){
            while(!isGameDone){
                Matrix screen=read();
                Matrix* obj=&screen;
                if(obj==nullptr) break;
                printWindow(win,screen);
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1);
        }
        void callme(std::thread *t){
            *t=std::thread(&View::run,this);
            threads.push_back(t);
        }
};
class Model:public Observer,public Publisher{
    public:
        string name;
        std::vector<View> observers;
        std::queue<char> Keys;
        std::map<char,char> Keypad;
        //std::condition_variable cv;
        //std::mutex m;
        
        Model(string Name){
            name=Name;
        }
        virtual void addObserver(View& view){
            observers.push_back(view);
        }
        virtual void notifyObservers(Matrix& screen){
            for(int i=0;i<observers.size();i++){
                observers[i].update(screen);
            }
        }
        virtual void update(char key){
            std::unique_lock<std::mutex> lk(m);
            
            Keys.push(key);
            cv.notify_all();
            lk.unlock();
        }
        void addKeypad(map<char,char>& keypad){
            Keypad=keypad;
        }
        char read(){
            std::unique_lock<std::mutex> lk(m);
            
            while(Keys.size()<1){
                cv.wait(lk);
            }
            char key=Keys.front();
            Keys.pop();
            lk.unlock();
            return key;
        }
        TetrisState processKey(Tetris* board, char Key){
            TetrisState state=board->accept(Key);
            notifyObservers(board->oScreen);
            if(state!=NewBlock) return state;

            srand((unsigned int)time(NULL));
            char key = (char)('0' + rand() % MAX_BLK_TYPES);
            state=board->accept(key);
            notifyObservers(board->oScreen);

            if(state!=Finished) return state;
            return state;
        }
        void run(){
            while(!isGameDone){
                Tetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
                Tetris *board=new Tetris(20,15);

                TetrisState state;
                char key;

                srand((unsigned int)time(NULL));
                key = (char)('0' + rand() % MAX_BLK_TYPES);
                state=board->accept(key);
                notifyObservers(board->oScreen);

                while(!isGameDone){
                    key=read();
                    if(!key) break;
                    if(Keypad.find(key)==Keypad.end()) printMsg("Wrong key!!");
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
                printMsg(str);
                sleep(1);
                //notifyObservers(obj);
            }
        }
        void callme(std::thread *t){
            *t=std::thread(&Model::run,this);
            threads.push_back(t);
        }
};
class KeyController:public Publisher{
    public:
        string name;
        std::vector<Model> observers;
        KeyController(string Name){
            name=Name;
        }
        virtual void addObserver(Model& model){
            observers.push_back(model);
        }
        virtual void notifyObservers(char key){
            for(int i=0;i<observers.size();i++){
                observers[i].update(key);
            }
        }
        void run(){
            while(!isGameDone){
                try{
                char key=getChar();
                notifyObservers(key);
                if(!key) throw runtime_error("key interrupt\n");
                } catch (runtime_error err){
                printMsg("pass");
                }
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1);
            //notifyObservers();
        }
        void callme(std::thread *t){
            *t=std::thread(&KeyController::run,this);
            threads.push_back(t);
        }
};
class TimeController:public Publisher{
    public:
        string name;
        std::vector<Model> observers;
        TimeController(string Name){
            name=Name;
        }
        virtual void addObserver(Model& model){
            observers.push_back(model);
        }
        virtual void notifyObservers(char key){
            for(int i=0;i<observers.size();i++){
                observers[i].update(key);
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
            //notifyObservers();
        }
        void callme(std::thread *t){
            *t=std::thread(&TimeController::run,this);
            threads.push_back(t);
        }
};
int main(){
    initscr();
    clear();
    echo();
    start_color();
    use_default_colors();

    WINDOW *win1, *win2;
    win1=newwin(20, 30, 0, 0);
    win2=newwin(20, 30, 0, 40);

    View th_view1("view1");
    th_view1.addWindow(win1);
    //View th_view2("view2");
    //th_view2.addWindow(win2);
    
    map<char,char> keypad1={{'q','q'},{'w','w'},{'a','a'},{'s','y'},{'d','d'},{' ',' '},{'y','y'}};
    //map<char,char> keypad2={{'u','q'},{'i','w'},{'j','a'},{'k','y'},{'l','d'},{'\r',' '},{'y','y'}};
    
    Model th_model1("model1");
    th_model1.addKeypad(keypad1);
    th_model1.addObserver(th_view1);

    //Model th_model2("model2");
    //th_model2.addKeypad(keypad2);
    //th_model2.addObserver(th_view2);

    KeyController th_cont1("kcont");
    th_cont1.addObserver(th_model1);
    //th_cont1.addObserver(th_model2);

    TimeController th_cont2("tcont");
    th_cont2.addObserver(th_model1);
    //th_cont2.addObserver(th_model2);
    
    std::thread th1,th2,th3,th4,th5,th6;
    th_view1.callme(&th1);
    //th_view2.callme(&th2);
    th_model1.callme(&th3);
    //th_model2.callme(&th4);
    th_cont1.callme(&th5);
    th_cont2.callme(&th6);
    th1.join();
    //th2.join();
    th3.join();
    //th4.join();
    th5.join();
    th6.join();
    cout << "Program terminated!" << endl;
    
    endwin();
    return 0;
}