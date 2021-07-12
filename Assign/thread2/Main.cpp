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

#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <chrono>
//#include <pthread.h>
#include "CTetris.h"
using std::this_thread::sleep_for;

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
char getch() {
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
int fd;
int *old_settings;
bool isGameDone=false;
pthread_mutex_t m;
pthread_cond_t cv;
std::queue<char> queuel;
void KeyProducer(){
  char key;
  while(!isGameDone){
    try{
      key=getch();
      if(!key) throw runtime_error("key interrupt\n");
    } catch(runtime_error err){
      isGameDone=true;
      cout<<"getch() wakes up!!\n";
      break;
    }
    pthread_mutex_lock(&m);
    cout<<"KeyProducer Lock!!!\n";
    queuel.push(key);
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
    cout<<"KeyProducer Unlock!!!\n";
    if(key=='q'){
      isGameDone=true;
      break;
    }
  }
  return ;
}
void TimeOutProducer(){
  while(!isGameDone){
    sleep_for(std::chrono::milliseconds(1000));
    pthread_mutex_lock(&m);
    cout<<"TimeOutProducer Lock!!!\n";
    queuel.push('s');
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
    cout<<"TimeOutProducer Unlock!!!\n";
  }
  return ;
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

int *setOfCBlockArrays[] = {
  T0D0, T0D1, T0D2, T0D3,
  T1D0, T1D1, T1D2, T1D3,
  T2D0, T2D1, T2D2, T2D3,
  T3D0, T3D1, T3D2, T3D3,
  T4D0, T4D1, T4D2, T4D3,
  T5D0, T5D1, T5D2, T5D3,
  T6D0, T6D1, T6D2, T6D3,
};

#if 1
void drawScreen(CTetris *board)
{
  int dy = board->oCScreen.get_dy();
  int dx = board->oCScreen.get_dx();
  int dw = board->iScreenDw;
  int **array = board->oCScreen.get_array();
  //system("clear");

  for (int y = 0; y < dy - dw ; y++) {
    for (int x = dw ; x < dx - dw ; x++) {
      if (array[y][x] == 0)
	cout << color_white << "□" << color_normal;
      else if (array[y][x] == 1)
	cout << color_red << "■" << color_normal;
      else if (array[y][x] == 2)
	cout << color_green << "■" << color_normal;
      else if (array[y][x] == 3)
	cout << color_yellow << "■" << color_normal;
      else if (array[y][x] == 4)
	cout << color_blue << "■" << color_normal;
      else if (array[y][x] == 5)
	cout << color_magenta << "■" << color_normal;
      else if (array[y][x] == 6)
	cout << color_cyan << "■" << color_normal;
      else if (array[y][x] == 7)
	cout << color_pink << "■" << color_normal;
      else // array[y][x] == 1 // wall
	cout << b_color_black << "■" << color_normal;
    }
    cout << endl;
  }
}
#endif
TetrisState processKey(CTetris *board, char key){
  TetrisState state=board->accept(key);
  drawScreen(board);
  if(state!=NewBlock) return state;
  srand((unsigned int)time(NULL));
  key = (char)('0' + rand() % MAX_BLK_TYPES);
  state=board->accept(key);
  drawScreen(board);
  if(state!=Finished) return state;
  return state;
}
void Consumer(char *argv[]){
  CTetris::init(setOfCBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
  CTetris *board = new CTetris(20, 15);
  TetrisState state;
  char key;

  srand((unsigned int)time(NULL));
  key = (char)('0' + rand() % MAX_BLK_TYPES);
  state=board->accept(key);
  drawScreen(board);

  while(!isGameDone){
    pthread_mutex_lock(&m);
    cout<<"Consumer Lock!!!\n";
    while(queuel.size()<1) 
      pthread_cond_wait(&cv,&m);
    key=queuel.front();
    queuel.pop();
    pthread_mutex_unlock(&m);
    cout<<"Consumer Unlock!!!\n";

    if(key=='q'){
      state=Finished;
      cout<<"Game aborted...\n";
      break;
    }
    state=processKey(board,key);
    if(state==Finished){
      isGameDone=true;
      cout<<"Game over!!!\n";
      kill(atoi(argv[0]), SIGINT);
      break;
    }
  }
  delete board;
  return ;
}
/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

int main(int argc, char *argv[]) {
  int dy, dx;
  char key = 0;
  std::vector<std::thread> threads;
  if (argc != 3) {
    cout << "usage: " << argv[0] << " dy dx" << endl;
    exit(1);
  }
  if ((dy = atoi(argv[1])) <= 0 || (dx = atoi(argv[2])) <= 0) {
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
  threads.push_back(std::thread(Consumer,argv));
  threads.push_back(std::thread(KeyProducer));
  threads.push_back(std::thread(TimeOutProducer));

  for(int i=0;i<threads.size();i++)
  {
    threads[i].join();
  }
  cout << "Program terminated!" << endl;
  return 0;
}
