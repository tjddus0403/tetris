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

#include <ncurses.h>
#include <stdarg.h>
#include <thread>
#include <map>

#define color_normal "\x1b[0m"
#define color_red "\x1b[31m"
#define color_green "\x1b[32m"
#define color_yellow "\x1b[33m"
#define color_blue "\x1b[34m"
#define color_magenta "\x1b[35m" //purple
#define color_cyan "\x1b[36m"
#define color_white "\x1b[37m"
#define color_black "\x1b[30m"
#define color_pink "\x1b[95m"
#define b_color_black "\x1b[40m"

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

bool isGameDone=false;
pthread_mutex_t m;
pthread_cond_t cv;
std::queue<char> queuel;
std::vector<std::thread*> threads;
void printWindow(WINDOW *window, Matrix& screen){
  int **array=screen.get_array();
  int dw=CTetris::iScreenDw;
  int dy=screen.get_dy();
  int dx=screen.get_dx();
  //pthread_mutex_lock(&m);
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
    //wprintw(window,"\n");
  }
  pthread_mutex_lock(&m);
  wrefresh(window);
  pthread_mutex_unlock(&m);
}
void sigint_handler(int signo) {
  cout << "SIGINT received!" << endl;
  //do nothing;
}
void printMsg(string msg){
  const char* c=msg.c_str();
  WINDOW *win0;
  win0=newwin(3, 70, 21, 0);
  wclear(win0);
  wprintw(win0,c);
  pthread_mutex_lock(&m);
  wrefresh(win0);
  pthread_mutex_unlock(&m);
}
class Observer{
  public:
    virtual void update(char key){
      printMsg("update\n");
    }
    virtual void update(Matrix& Screen){
      printMsg("update\n");
    }
};
class Publisher{
  public:
    virtual void addObserver(Observer& observer){
      printMsg("addObserver\n");
    }
    virtual void notifyObservers(char key){
      printMsg("notifyObserver\n");
    }
    virtual void notifyObservers(Matrix& Screen){
      printMsg("notifyObserver\n");
    }
};
class View:public Observer{
  public:
    string name;
    std::vector<Observer> observers;
    std::queue<Matrix> que;
    pthread_mutex_t mu;
    pthread_cond_t cva;
    map<char,char> Keypad;
    WINDOW *win;
    View(string args){
      /*va_list ap;
      va_start(ap, args);
      name=va_arg(ap, string);*/
      name=args;
    }
    virtual void update(char key){
      pthread_mutex_lock(&mu);
      //que.push(key);
      pthread_cond_signal(&cva);
      pthread_mutex_unlock(&mu);
    }
    virtual void update(Matrix& Screen){
      pthread_mutex_lock(&mu);
      que.push(Screen);
      pthread_cond_signal(&cva);
      pthread_mutex_unlock(&mu);
    }
    Matrix read(){
      pthread_mutex_lock(&mu);
      while(que.size()<1){
        pthread_cond_wait(&cva,&mu);
      }
      Matrix obj=que.front();
      que.pop();
      pthread_mutex_unlock(&mu);
      return obj;
    }
    void addWindow(WINDOW *window){
      win=window;
    }
    void run(){
      while(!isGameDone){
        Matrix obj=read();
        Matrix* pob=&obj;
        if(pob==nullptr) break;
        printWindow(win, obj);
      }
      string str=name+"terminated... Press any key to continue";
      printMsg(str);
      sleep(1);
    }
    static void *callThread(void *arg);
    void callme(std::thread *t/*pthread_t t*/){
      *t=std::thread(&View::run,this);
      //pthread_create(&t,nullptr,callThread,nullptr);
      threads.push_back(t);
    }
};
void *View::callThread(void *arg){
  View* tp;
  tp=(View*) arg;
  tp->run();
}

class Model:public Observer, public Publisher{
  public:
    string name;
    std::vector<Observer> observers;
    std::queue<char> que;
    pthread_mutex_t mu;
    pthread_cond_t cva;
    map<char,char> Keypad;
    Model(string args){
      /*va_list ap;
      va_start(ap, args);
      name=va_arg(ap, string);*/
      name=args;
    }
    virtual void addObserver(Observer& observer){
      observers.push_back(observer);
    }
    virtual void update(char key){
      pthread_mutex_lock(&mu);
      que.push(key);
      pthread_cond_signal(&cva);
      pthread_mutex_unlock(&mu);
    }
    virtual void update(Matrix& Screen){
      pthread_mutex_lock(&mu);
      que.push('z');
      pthread_cond_signal(&cva);
      pthread_mutex_unlock(&mu);
    }
    virtual void notifyObservers(char key){
      for(int i=0;i<observers.size();i++){
        observers[i].update(key);
      }
    }
    virtual void notifyObservers(Matrix& Screen){
      for(int i=0;i<observers.size();i++){
        Matrix obj=Screen;
        observers[i].update(obj);
      }
    }
    char read(){
      pthread_mutex_lock(&mu);
      while(que.size()<1){
        pthread_cond_wait(&cva,&mu);
        char key=que.front();
        que.pop();
        pthread_mutex_unlock(&mu);
        return key;
      }
    }
    void addKeypad(map<char,char>&keypad){
      Keypad=keypad;
    }
    TetrisState processKey(CTetris *board, char key){
      TetrisState state=board->accept(key);
      this->notifyObservers(board->oScreen);
      
      if(state!=NewBlock) return state;

      srand((unsigned int)time(NULL));
      key = (char)('0' + rand() % MAX_BLK_TYPES);
      state=board->accept(key);
      this->notifyObservers(board->oScreen);
      
      if(state!=Finished) return state;
      return state;
    }
    void run(/*void *arg*/){
      /*char **argv;
      argv=(char **)arg;*/
      CTetris::init(setOfCBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
      //CTetris *board=new CTetris(atoi(argv[1]), atoi(argv[2]));
      CTetris *board=new CTetris(20,15);
      TetrisState state;
      char key;

      srand((unsigned int)time(NULL));
      key = (char)('0' + rand() % MAX_BLK_TYPES);
      state=board->accept(key);
      this->notifyObservers(board->oScreen);

      while(!isGameDone){
        key=read();
        if(key==0) break;
        if(Keypad.find(key)==Keypad.end()) cout<<"continue\n";
        key=Keypad[key];
        
        if(key=='q') state=Finished;
        else state=this->processKey(board,key);
        if(state==Finished){
          isGameDone=true;
          string str=name+"terminated... Press any key to continue";
          printMsg(str);
          sleep(2);
          break;
        }
      }
      //printMsg
      sleep(1);
      this->notifyObservers(0);
    }
    static void *callThread(void *arg);
    void callme(std::thread *t/*pthread_t t*/){
      *t=std::thread(&Model::run,this);
      //pthread_create(&t,nullptr,callThread,nullptr);
      threads.push_back(t);
    }
};
void *Model::callThread(void *arg){
  Model* tp;
  tp=(Model*) arg;
  tp->run();
}

class KeyController:public Publisher, public std::thread{
  public:
    string name;
    std::vector<Observer> observers;
    KeyController(string args){
      //va_list ap;
      //va_start(ap, args);
      //name=va_arg(ap, string);
      name=args;
    }
    virtual void addObserver(Observer& observer){
      observers.push_back(observer);
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
          this->notifyObservers(key);
          if(!key) throw runtime_error("key interrupt\n");
        } catch (runtime_error err){
          cout<<"pass\n";
        }
      }
      string str=name+"terminated... Press any key to continue";
      printMsg(str);
      sleep(1);
      this->notifyObservers(0);
    }
    static void *callThread(void *arg);
    void callme(std::thread *t/*pthread_t t*/){
      *t=std::thread(&KeyController::run,this);
      //pthread_create(&t,nullptr,callThread,nullptr);
      threads.push_back(t);
    }
};
void *KeyController::callThread(void *arg){
  KeyController* tp;
  tp=(KeyController*) arg;
  tp->run();
}

class TimeController{
  public:
    string name;
    std::vector<Observer> observers;
    TimeController(string args){
      /*va_list ap;
      va_start(ap, args);
      name=va_arg(ap, string);*/
      name=args;
    }
    virtual void addObserver(Observer& observer){
      observers.push_back(observer);
    }
    virtual void notifyObservers(char key){
      for(int i=0;i<observers.size();i++){
        observers[i].update(key);
      }
    }
    void run(){
      while(!isGameDone){
        sleep(1);
        this->notifyObservers('y');
      }
      string str=name+"terminated... Press any key to continue";
      printMsg(str);
      sleep(1);
      this->notifyObservers(0);
    }
    static void *callThread(void *arg);
    void callme(std::thread *t/*pthread_t t*/){
      *t=std::thread(&TimeController::run,this);
      //pthread_create(&t,nullptr,callThread,nullptr);
      threads.push_back(t);
    }
};
void *TimeController::callThread(void *arg){
  TimeController* tp;
  tp=(TimeController*) arg;
  tp->run();
}

void sigalrm_handler(int signo) {
  alarm(1);
  saved_key = 's';
}

void unregisterAlarm() {
	alarm(0);
}

void registerAlarm() {
  struct sigaction act, oact;
  act.sa_handler = sigalrm_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
  alarm(1);
}

/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

int main(int argc, char **argv) {
  char key = 0;
  //std::vector<pthread_t> threads;
  if (argc != 3) {
    cout << "usage: " << argv[0] << " dy dx" << endl;
    exit(1);
  }
  if ((atoi(argv[1])) <= 0 || (atoi(argv[2])) <= 0) {
    cout << "dy and dx should be greater than 0" << endl;
    exit(1);
  }
  struct sigaction act, oact;
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGINT, &act, &oact) < 0) {
    cerr << "sigaction error" << endl;
    exit(1);
  }
  void *arg=(void *)argv;
  WINDOW *win1,*win2;
  initscr();
  clear();
  echo();
  start_color();
  use_default_colors();
  win1=newwin(20, 30, 0, 0);
  win2=newwin(20, 30, 0, 40);
  //pthread_t th1,th2,th3,th4,th5,th6;

  View th_view1("view1");
  th_view1.addWindow(win1);
  View th_view2("view2");
  th_view2.addWindow(win2);
  
  map<char,char> keypad1={{'q','q'},{'w','w'},{'a','a'},{'s','y'},{'d','d'},{' ',' '},{'y','y'}};
  map<char,char> keypad2={{'u','q'},{'i','w'},{'j','a'},{'k','y'},{'l','d'},{'\r',' '},{'y','y'}};
  
  Model th_model1("model1");
  th_model1.addKeypad(keypad1);
  th_model1.addObserver(th_view1);

  Model th_model2("model2");
  th_model2.addKeypad(keypad2);
  th_model2.addObserver(th_view2);

  KeyController th_cont1("kcont");
  th_cont1.addObserver(th_model1);
  th_cont1.addObserver(th_model2);

  TimeController th_cont2("tcont");
  th_cont2.addObserver(th_model1);
  th_cont2.addObserver(th_model2);
  
  std::thread th1,th2,th3,th4,th5,th6;
  th_view1.callme(&th1);
  th_view1.callme(&th2);
  th_model1.callme(&th3);
  th_model2.callme(&th4);
  th_cont1.callme(&th5);
  th_cont2.callme(&th6);
  th1.join();
  th2.join();
  th3.join();
  th4.join();
  th5.join();
  th6.join();
  cout << "Program terminated!" << endl;
  
  endwin();
  return 0;
}