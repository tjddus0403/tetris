#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <ncurses.h>
#include <thread>
#include "ModelView.h"
#include "SendRecv.h"
using namespace std;

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
std::mutex mu;
TetrisState state;

void usage(char *argv){ 
    cout << "Usage : " << argv << " [port]" << endl;
    cout << "Example) " << argv << " 1234" << endl;
}
void signal_handler(int signal)
{
  cout<<"but ok!"<<endl;
}
int main(int argc, char *argv[]){
    if(argc != 2){ 
        usage(argv[0]); 
        return -1;
    }
    char buff[256];
    struct sockaddr_in addr_server = {}; 
    struct sockaddr_in addr_client1 = {}; 
    struct sockaddr_in addr_client2 = {}; 
    socklen_t addr_client_len = sizeof(addr_client_len); 
    signal(SIGPIPE, signal_handler);
    signal(SIGTSTP, signal_handler);
    bool option=true;
 
    int sock_server = socket(AF_INET, SOCK_STREAM, 0); 
    if(sock_server == -1){ 
        cout << "socket error" << endl;
        close(sock_server); 
        exit(1); 
    }
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET; 
    addr_server.sin_port = htons(atoi(argv[1])); 
    addr_server.sin_addr.s_addr = htonl(INADDR_ANY); 
    setsockopt(sock_server,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
    if(bind(sock_server, (sockaddr*) &addr_server, sizeof(addr_server)) == -1){
        cout << "bind error" << endl; 
        close(sock_server);
        exit(1); 
    }

    if(listen(sock_server, 3) == -1){ 
        cout << "listen error" << endl; 
        close(sock_server); 
        exit(1);
    }

    setlocale(LC_ALL,""); //□, ■ 출력할 수 있게 함
    initscr(); //curses모드 시작 (curses를 사용하려면 반드시 초기화 해주어야 함)
    clear(); //창 지우기
    start_color(); //color사용할 수 있게 함
    init_pair(1,COLOR_RED,COLOR_BLACK);
    init_pair(2,COLOR_GREEN,COLOR_BLACK);
    init_pair(3,COLOR_YELLOW,COLOR_BLACK);
    init_pair(4,COLOR_BLUE,COLOR_BLACK);
    init_pair(5,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(6,COLOR_CYAN,COLOR_BLACK);
    init_pair(7,COLOR_WHITE,COLOR_BLACK);
     
    WINDOW *win1,*win2;
    win1=newwin(20, 30, 0, 0); 
    win2=newwin(20, 30, 0, 60); 

    map<char,char> keypad1={{'q','q'},{'w','w'},{'a','a'},{'s','y'},{'d','d'},{' ',' '},{'y','y'},{-1,-1},{'0','0'},{'1','1'},{'2','2'},{'3','3'},{'4','4'},{'5','5'},{'6','6'}};
    View th_view1("view1"); 
    th_view1.addWindow(win1);
    View th_view2("view2"); 
    th_view2.addWindow(win2);

    while(1){ 
        clear();
        refresh();
        int sock_client1 = accept(sock_server, (sockaddr*) &addr_client1, &addr_client_len); 
        int sock_client2 = accept(sock_server, (sockaddr*) &addr_client2, &addr_client_len); 
        if(sock_client1 == -1 || sock_client2 == -1){ 
            cout << "accept error" << endl; 
            close(sock_server); 
            exit(1);
        }
        CTetris::init(setOfCBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES);
        isGameDone=false;
        
        SendController scont1("scont1",sock_client1,false);
        SendController scont2("scont2",sock_client2,false);
        
        Model th_model1("model1",false); 
        th_model1.addKeypad(keypad1);
        th_model1.addObserverView(&th_view1); 
        th_model1.addObserverKey(&scont2);
        Model th_model2("model2",false); 
        th_model2.addKeypad(keypad1);
        th_model2.addObserverView(&th_view2); 
        th_model2.addObserverKey(&scont1);
        
        RecvController rcont1("rcont1",sock_client1,false);
        rcont1.addObserverKey(&th_model1);
        RecvController rcont2("rcont2",sock_client2,false);
        rcont2.addObserverKey(&th_model2);

        std::vector<std::thread> threads;
        threads.push_back(std::thread(&View::run, &th_view1)); 
        threads.push_back(std::thread(&View::run, &th_view2)); 
        threads.push_back(std::thread(&Model::run, &th_model1));
        threads.push_back(std::thread(&Model::run, &th_model2));
        threads.push_back(std::thread(&SendController::run, &scont1));
        threads.push_back(std::thread(&SendController::run, &scont2));
        threads.push_back(std::thread(&RecvController::run, &rcont1));
        threads.push_back(std::thread(&RecvController::run, &rcont2));
        for(int i=0;i<threads.size();i++){
            threads[i].join();
        }
    }
    close(sock_server); 
    return 0;
}
