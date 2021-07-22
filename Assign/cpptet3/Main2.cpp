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

void sigint_handler(int signo) {
  cout << "SIGINT received!" << endl;
  //do nothing;
}
pthread_mutex_t mu;
void printMsg(string msg){
  const char* c=msg.c_str();
  WINDOW *win0;
  win0=newwin(3, 70, 21, 0);
  wclear(win0);
  wprintw(win0,c);
  pthread_mutex_lock(&mu);
  wrefresh(win0);
  pthread_mutex_unlock(&mu);
}
bool isGameDone=false;
pthread_mutex_t m;
pthread_cond_t cv;
std::queue<char> queuel;

void *KeyProducer(void *arg){
  char key;
  while(!isGameDone){
    try{
      key=getChar();
      if(!key) throw runtime_error("key interrupt\n");
    } catch(runtime_error err){
      isGameDone=true;
      printMsg("getChar() wakes up!!\n");
      break;
    }
    pthread_mutex_lock(&m);
    printMsg("KeyProducer Lock!!!\n");
    queuel.push(key);
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
    printMsg("KeyProducer Unlock!!!\n");
    if(key=='q'){
      isGameDone=true;
      break;
    }
  }
}

void *TimeOutProducer(void *arg){
  while(!isGameDone){
    //sleep_for(std::chrono::milliseconds(1000));
    sleep(1);
    pthread_mutex_lock(&m);
    printMsg("TimeOutProducer Lock!!!\n");
    queuel.push('y');
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
    printMsg("TimeOutProducer Unlock!!!\n");
  }
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
    printMsg("sigaction error\n");
    exit(1);
  }
  alarm(1);
}

/**************************************************************/
/**************** Tetris Blocks Definitions *******************/
/**************************************************************/
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
//WINDOW *win1,*win2;
//win1=newwin(20,30,0,0);
//win2=newwin(20,30,0,40);
void printWindow(WINDOW *win, Tetris *board){
    int dy=board->oScreen.get_dy();
    int dx=board->oScreen.get_dx();
    int dw=board->iScreenDw;
    int **array=board->oScreen.get_array();
    //WINDOW *win1,*win2;
    //win1=newwin(20,30,0,0);
    //win2=newwin(20,30,0,40);
    wclear(win);
    //wclear(win2);
    for(int y=0;y<dy-dw;y++){
        echo();
        for(int x=dw;x<dx-dw;x++){
            if (array[y][x]==0){
                waddstr(win,"AA");
                //waddstr(win2,"AA");
            }
            else if (array[y][x]==1){
                waddstr(win,"BB");
                //waddstr(win2,"BB");
            }
            else{
                waddstr(win,"XX");
                //waddstr(win2,"XX");
            }
        }
    }
    wrefresh(win);
    //wrefresh(win2);
}
TetrisState processKey(Tetris *board, char key, WINDOW* win){
  TetrisState state=board->accept(key);
  //drawScreen(board);
  printWindow(win,board);
  if(state!=NewBlock) return state;
  srand((unsigned int)time(NULL));
  key = (char)('0' + rand() % MAX_BLK_TYPES);
  state=board->accept(key);
  //drawScreen(board);
  printWindow(win,board);
  if(state!=Finished) return state;
  return state;
}

void *Consumer(void *arg){
  char **argv;
  argv=(char **)arg;
  
  Tetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
  Tetris *board = new Tetris(atoi(argv[1]), atoi(argv[2]));
  TetrisState state;
  char key;
  WINDOW *win1;
  win1=newwin(20,30,0,0);
  srand((unsigned int)time(NULL));
  key = (char)('0' + rand() % MAX_BLK_TYPES);
  state=board->accept(key);
  //drawScreen(board);
  printWindow(win1, board);
  while(!isGameDone){
    pthread_mutex_lock(&m);
    printMsg("Consumer Lock!!!\n");
    while(queuel.size()<1) 
      pthread_cond_wait(&cv,&m);
    key=queuel.front();
    queuel.pop();
    pthread_mutex_unlock(&m);
    printMsg("Consumer Unlock!!!\n");

    if(key=='q'){
      state=Finished;
      printMsg("Game aborted...\n");
      break;
    }
    state=processKey(board,key,win1);
    if(state==Finished){
      isGameDone=true;
      printMsg("Game over!!!\n");
      kill(atoi(argv[0]), SIGINT);
      break;
    }
  }
  delete board;
}
void *Consumerr(void *arg){
  char **argv;
  argv=(char **)arg;
  
  Tetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
  Tetris *board = new Tetris(atoi(argv[1]), atoi(argv[2]));
  TetrisState state;
  char key;
  WINDOW *win2;
  win2=newwin(20,30,0,40);
  srand((unsigned int)time(NULL));
  key = (char)('0' + rand() % MAX_BLK_TYPES);
  state=board->accept(key);
  //drawScreen(board);
  printWindow(win2, board);
  while(!isGameDone){
    pthread_mutex_lock(&m);
    printMsg("Consumer Lock!!!\n");
    while(queuel.size()<1) 
      pthread_cond_wait(&cv,&m);
    key=queuel.front();
    queuel.pop();
    pthread_mutex_unlock(&m);
    printMsg("Consumer Unlock!!!\n");

    if(key=='q'){
      state=Finished;
      printMsg("Game aborted...\n");
      break;
    }
    state=processKey(board,key,win2);
    if(state==Finished){
      isGameDone=true;
      printMsg("Game over!!!\n");
      kill(atoi(argv[0]), SIGINT);
      break;
    }
  }
  delete board;
}
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

int main(int argc, char **argv) {
  char key = 0;
  std::vector<pthread_t> threads;
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
    cerr<< "sigaction error" << endl;
    exit(1);
  }
  initscr();
  clear();
  echo();
  start_color();
  use_default_colors();
  pthread_t th1,th2,th3,th4; 

  pthread_create(&th1, nullptr, Consumer, (void *)argv);
  pthread_create(&th2, nullptr, Consumerr, (void *)argv);
  pthread_create(&th3, nullptr, KeyProducer, nullptr);
  pthread_create(&th4, nullptr, TimeOutProducer, nullptr);

  threads.push_back(th1);
  threads.push_back(th2);
  threads.push_back(th3);
  threads.push_back(th4);
  
  for(int i=0;i<threads.size();i++)
  {
    pthread_join(threads[i],nullptr);
  }
  printMsg("Program terminated!\n");
  //clear();
  //refresh();
  endwin();
  return 0;
}
