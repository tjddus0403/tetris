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

#include "CTetris.h"
#include<fstream>
#include"keylog.cpp"
using namespace std;

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
  // cout << "SIGINT received!" << endl;
  // do nothing;
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

char readKey(){
  char ch1;
  ch1=getch();
  if(ch1!=27) return ch1;
  char ch2;
  ch2=getch();
  if(ch2!=91) return ch1;
  char ch3;
  ch3=getch();
  return char(16+ch3-65);
}
char readKeyWithTimeOut(){
  registerAlarm();
  char key;
  try{
    key=readKey();
    unregisterAlarm();
    return key;
    if(!key) throw runtime_error("readkey interrupt\n");
  }catch(runtime_error err) 
    {cout<<"readkey interrupt\n";
    exit(1);}
  return key;
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

bool is_log_mode;
bool is_replay_mode;
//char* keys;
int key_idx;

char get_key_from_log(){
  char key;
  key=keys[key_idx];
  key_idx+=1;
  cout<<key;
  return key;
}
void log_start(){
  ofstream writefile;
  writefile.open("keylog.cpp");
  if(writefile.is_open())    //파일이 열렸는지 확인
    writefile<<"char keys[]={";    //파일에 문자열 쓰기
  writefile.close();
}
void log_end(){
  ofstream writefile;
  writefile.open("keylog.cpp",ios::app);
  if(writefile.is_open())
    writefile<<"};\n";
  writefile.close();
}
void log_key(char key){
  ofstream writefile;
  writefile.open("keylog.cpp",ios::app);
  if(writefile.is_open())
  {
    writefile<<"'";
    writefile<<key;
    writefile<<"'";
    writefile<<",";
  }
  writefile.close();
}
char getKey(bool is_keystroke_needed,CTetris *board){
  char key;
  if(is_replay_mode) key=get_key_from_log();
  else if(is_keystroke_needed){
    key=readKeyWithTimeOut();
    if(!key) key='s';
  } 
  else{
    srand((unsigned int)time(NULL));
    key=(char)('0'+rand()%MAX_BLK_TYPES);
  }
  if(is_log_mode) log_key(key);
  return key;
}

void drawScreen(CTetris *board)
{
  int dy = board->oCScreen.get_dy();
  int dx = board->oCScreen.get_dx();
  int dw = board->iScreenDw;
  int **array = board->oCScreen.get_array();
  system("clear");
  for (int y = 0; y < dy - dw; y++) {
    for (int x = dw; x < dx - dw; x++) {
      if (array[y][x] == 0)
	cout << color_white << "□" << color_normal;
      else if (array[y][x] == 1)
	cout << color_red<< "■" << color_normal;
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
      else 
	cout << b_color_black << "XX" << color_normal;
    }
    cout << endl;
  }
}
TetrisState processKey(CTetris *board, char key){
  TetrisState state=board->accept(key); //주어진 key로 state받기
  drawScreen(board); //printScreen(board)
  if(state!=NewBlock) return state; //만약 state=NewBlock 아니면 state반환
  key=getKey(false,board); //NewBlock이면 key에 새로운 블록 받기(false)
  state=board->accept(key); //새로받은 블록으로 state반환
  drawScreen(board); //printScreen(board)
  if(state!=Finished) return state; //state반환
  return state;
}

/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/
int main(int argc, char *argv[]) {
  int dy, dx;
  char key = 0;
  is_log_mode=false;
  if(argc==4&&(atoi(argv[3]))==1){
    is_log_mode=true;
    log_start();
  }
  is_replay_mode=false;
  if(argc==4&&(atoi(argv[3]))==2){
    is_replay_mode=true;
    key_idx=0;
  }
  if (argc != 3&&argc!=4) {
    cout << "usage: " << argv[0] << " dy dx" << endl;
    exit(1);
  }
  if ((dy = atoi(argv[1])) <= 0 || (dx = atoi(argv[2])) <= 0) {
    cout << "dy and dx should be greater than 0" << endl;
    exit(1);
  }

  CTetris::init(setOfBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);//클래스 초기화
  CTetris *board = new CTetris(dy, dx);//객체 생성
  TetrisState state;
  key=getKey(false,board);//key받기(false->NewBlock)
  //registerAlarm();
  state=board->accept(key);//state설정
  drawScreen(board);//printScreen(board)

  while(true){ //반복문 시작
    key=getKey(true,board); //key받기(True->keystroke)
    if(key=='q'){  //key=q면 종료
      state=Finished;
      cout<<"Game aborted\n";
      break;
    }
    state=processKey(board,key); //아니면 state=processKey
    if(state==Finished){  //만약 state=Finished면 종료
      cout<<"Game over\n";
      break;
    }
  }
  unregisterAlarm();
  cout<<"program terminated\n";
  if(is_log_mode) log_end();
  delete board;
  return 0;
}
