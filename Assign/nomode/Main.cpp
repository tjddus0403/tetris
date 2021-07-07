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
  char ch; //사용자로부터 입력받아 반환할 문자
  int n; //읽어들인 문자열 길이
  while (1) {
    tty_cbreak(0); //tty_cbreak(int fd) : 표준입력 cbreak모드로 전환
    n = read(0, &ch, 1);
/*ssize_t read(int fd, void *buf, size_t nbytes);
int fd : 읽을 파일의 파일 디스크립터 (0 : 표준 입력 (stdin), 1 : 표준 출력 (stdout), 2 : 표준 오류 (stderr))
void *buf : 읽어들인 데이터를 저장할 버퍼(배열)
size_t nbytes : 읽어들일 데이터의 최대 길이 (buf의 길이보다 길어선 안됨)
반환값 : 읽어들인 데이터의 길이
무조건 nbytes 가 리턴되는 것은 아님. 중간에 파일의 끝을 만난다면 거기까지만 읽기 때문*/
    tty_reset(0); //tty_reset(int fd) : 표준입력 원래 모드로 돌려놓기
    if (n > 0) //만약 읽어들인 데이터 길이가 0보다 크다면 반복문 탈출해서 ch값 반환 
      break; 
    else if (n < 0) { //0보다 짧으면 
      if (errno == EINTR) { //EINTR 에러이면
	if (saved_key != 0) { //saved_key값이 0이 아니면
	  ch = saved_key; //saved_key값으로 ch설정
	  saved_key = 0; //saved_key값은 0으로 돌려놓고
	  break; //반복문 탈출해서 ch값 반환
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
  alarm(1); //1초 후에 프로세스에 SIGARLM 전달 , SIGARLM을 sigalrm_handler로 처리하는데 처리기 안에
	//alarm(1)이 있기 때문에 1초마다 계속 SIGARLM 전달되게 
  saved_key = 's'; //saved_key값 's'로 설정
}

void unregisterAlarm() {
	alarm(0); //0초 후에 프로세스에 SIGALRM 전달 (SIGARLM 기본행동 : 프로세스 종료)
}

void registerAlarm() {
  struct sigaction act, oact; //sigaction 구조체 타입의 객체 act, oact생성
  act.sa_handler = sigalrm_handler; //act객체의 시그널을 처리하기 위한 핸들러를 sigalarm_handler로 설정
  sigemptyset(&act.sa_mask); // 시그널 처리 중 블록될 시그널은 없음
#ifdef SA_INTERRUPT //만약 SA_INTERRUPT가 정의되어있다면
  act.sa_flags = SA_INTERRUPT; //이게 무슨 용도인지...
#else
  act.sa_flags = 0;
#endif
  if (sigaction(SIGALRM, &act, &oact) < 0) { 
//act : 설정할 행동(새롭게 지정할 처리 행동), oact : 이전 행동(이 함수를 호출하기 전에 지정된 행동 정보 입력됨)
//반환값 : 성공 시, 0 / 실패 시, -1
    cerr << "sigaction error" << endl; //실패 시, 에러문구 출력
    exit(1); //강제종료
  }
  alarm(1); //성공 시, 1초 후 프로세스에 SIGARLM 
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
  system("clear");

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

/**************************************************************/
/******************** Tetris Main Loop ************************/
/**************************************************************/

int main(int argc, char *argv[]) {
  int dy, dx;
  char key = 0;

  if (argc != 3) {
    cout << "usage: " << argv[0] << " dy dx" << endl;
    exit(1);
  }
  if ((dy = atoi(argv[1])) <= 0 || (dx = atoi(argv[2])) <= 0) {
    cout << "dy and dx should be greater than 0" << endl;
    exit(1);
  }

#if 1
  CTetris::init(setOfCBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
  CTetris *board = new CTetris(dy, dx);
  TetrisState state;

  srand((unsigned int)time(NULL));
  key = (char)('0' + rand() % MAX_BLK_TYPES);
#endif

  registerAlarm();
  while (key != 'q') {
#if 1
    state = board->accept(key);
    if (state == NewBlock) {
      key = (char)('0' + rand() % MAX_BLK_TYPES);
      state = board->accept(key);
      if (state == Finished) {
	drawScreen(board);
	cout << endl;
	break;
      }
    }
    drawScreen(board);
    cout << endl;
#endif
    key = getch();
    cout << key << endl;
  }
#if 1
  delete board;
#endif

  cout << "Program terminated!" << endl;
  return 0;
}

