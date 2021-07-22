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
#include <pthread.h>
#include "CTetris.h"
#include <ncursesw/curses.h>
#include <mutex>
#include <condition_variable>
#include <map>
#include <thread>

/**************************************************************/
/**************** Linux System Functions **********************/
/**************************************************************/

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

bool isGameDone=false;
std::vector<std::thread> threads;
std::mutex mu;
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

class Observer{
    public:
    virtual void update(char key){
        printMsg("Observer update\n");
    } 
};
class Publisher{
    public:
    virtual void addObserver(Observer& observer){
        printMsg("Publisher addObserver\n");
    }
    virtual void notifyObservers(char key){
        printMsg("Publisher notifyObservers\n");
    }
};
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
void printWindow(WINDOW *win, Tetris *board){
    int dy=board->oScreen.get_dy();
    int dx=board->oScreen.get_dx();
    int dw=board->iScreenDw;
    int **array=board->oScreen.get_array();
    wclear(win);
    for(int y=0;y<dy-dw;y++){
        for(int x=dw;x<dx-dw;x++){
            if (array[y][x]==0){
                waddstr(win,"□");
            }
            else if (array[y][x]==1){
                waddstr(win,"■");
            }
            else{
                waddstr(win,"XX");
            }
        }
        waddstr(win,"\n");
    }
    mu.lock();
    wrefresh(win);
    mu.unlock();
}
TetrisState processKey(Tetris *board, char key, WINDOW* win){
  TetrisState state=board->accept(key);
  printWindow(win,board);
  if(state!=NewBlock) return state;
  srand((unsigned int)time(NULL));
  key = (char)('0' + rand() % MAX_BLK_TYPES);
  state=board->accept(key);
  printWindow(win,board);
  if(state!=Finished) return state;
  return state;
}
class ModelView:public Observer{
    public:
        std::condition_variable cv;
        std::mutex m;
        string name;
        std::queue<char> que;
        map<char,char> Keypad;
        WINDOW *win;
        ModelView(string arg){
            name=arg;
        }
        void update(char key){
            std::unique_lock<std::mutex> lk(m);
            que.push(key);
            cv.notify_one();
            lk.unlock();
        } 
        char read(){
            std::unique_lock<std::mutex> lk(m);
            while(que.size()<1){
                cv.wait(lk);
            }
            char key=que.front();
            que.pop();
            lk.unlock();
            return key;
        }
        void addKeypad(map<char,char>& keypad){
            Keypad=keypad;
        }
        void addWindow(WINDOW *window){
            win=window;
        }
        void run(){
            Tetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
            Tetris *board = new Tetris(20,15);
            TetrisState state;
            char key;

            srand((unsigned int)time(NULL));
            key = (char)('0' + rand() % MAX_BLK_TYPES);
            state=board->accept(key);
            printWindow(win, board);

            while(!isGameDone){
                key=read();
                if(!key) break;
                if(Keypad.find(key)==Keypad.end()) printMsg("continue");
                key=Keypad[key];
                if(key=='q')
                    state=Finished;
                else
                    state=processKey(board,key,win);
                if(state==Finished){
                    isGameDone=true;
                    string str=name+" IS DEAD!!!";
                    printMsg(str);
                    sleep(2);
                    break;
                }
            }
            string str=name+" terminated... Press any key to continue";
            printMsg(str);
            sleep(1);
            delete board;
        }
        void callme(){
            threads.push_back(std::thread(&ModelView::run,this));
        }
};
class KeyController:public Publisher{
    public:
        string name;
        std::vector<ModelView*> observers;

        KeyController(string arg){
            name=arg;
        }
        void addObserver(ModelView* observer){
            observers.push_back(observer);
        }
        void notifyObservers(char key){
            for(int i=0;i<observers.size();i++){
                observers[i]->update(key);
            }
        }
        void run(){
            while(!isGameDone){
                try{
                    char key=getChar();
                    string str=key+"!!!\n";
                    printMsg(str);
                    notifyObservers(key);
                    if(!key) throw runtime_error("key interrupt");
                } catch (runtime_error err){
                cout<<"pass\n";
                }
            } 
            string str=name+"terminated... Press any key to continue";
            printMsg(str);
            sleep(1);
            notifyObservers(0);    
        }
        void callme(){
            threads.push_back(std::thread(&KeyController::run,this));
        }
};
class TimeController:public Publisher{
    public:
        string name;
        std::vector<ModelView*> observers;

        TimeController(string arg){
            name=arg;
        }
        void addObserver(ModelView* observer){
            observers.push_back(observer);
        }
        void notifyObservers(char key){
            for(int i=0;i<observers.size();i++){
                observers[i]->update(key);
            }
        }
        void run(){
            while(!isGameDone){
                sleep(1);
                notifyObservers('y');
            }
            string str=name+"terminated... Press any key to continue";
            printMsg(str);
            sleep(1);
            notifyObservers(0);
        }
        void callme(){
            threads.push_back(std::thread(&TimeController::run,this));
        }
};


/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

int main() {
  setlocale(LC_ALL,"");
  initscr();
  clear();
  echo();
  start_color();
  use_default_colors(); 
  WINDOW *win1,*win2;
  win1=newwin(20, 30, 0, 0);
  win2=newwin(20, 30, 0, 60);

  map<char,char> keypad1={{'q','q'},{'w','w'},{'a','a'},{'s','y'},{'d','d'},{' ',' '},{'y','y'}};
  map<char,char> keypad2={{'u','q'},{'i','w'},{'j','a'},{'k','y'},{'l','d'},{'\r',' '},{'y','y'}};

  ModelView th_model1("model1");
  th_model1.addKeypad(keypad1);
  th_model1.addWindow(win1);

  ModelView th_model2("model2");
  th_model2.addKeypad(keypad2);
  th_model2.addWindow(win2);

  KeyController th_cont1("kcont");
  th_cont1.addObserver(&th_model1);
  th_cont1.addObserver(&th_model2);

  TimeController th_cont2("tcont");
  th_cont2.addObserver(&th_model1);
  th_cont2.addObserver(&th_model2);

  th_model1.callme();
  th_model2.callme();
  th_cont1.callme();
  th_cont2.callme();
  for(int i=0;i<threads.size();i++){
    threads[i].join();
  }
  printMsg("Program terminated!\n");
  endwin();
  return 0;
}
