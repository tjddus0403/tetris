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

bool isGameDone=false; 
std::mutex mu;
TetrisState state;

void printWindow(WINDOW *window, Matrix& screen){ 
  int **array=screen.get_array();
  int dw=CTetris::iScreenDw; 
  int dy=screen.get_dy(); 
  int dx=screen.get_dx();
  wclear(window);
  for(int y=0;y<(dy-dw);y++){
    for(int x=dw;x<dx-dw;x++){ 
        wattron(window,COLOR_PAIR(7));
        if(array[y][x]==0)
            waddstr(window,"□");
        else{
            wattron(window, COLOR_PAIR(array[y][x]));
            waddstr(window,"■");
        }
    }
    waddstr(window,"\n");
  }
  mu.lock(); 
  wrefresh(window); 
  mu.unlock(); 
}

void printMsg(string msg){ 
  WINDOW *win0; 
  win0=newwin(3, 70, 21, 0);
  wclear(win0);
  const char* c=msg.c_str();
  wprintw(win0,c);
  mu.lock(); 
  wrefresh(win0); 
  mu.unlock();
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

class View:public OutScreenObserver{
    public:
        string name;
        std::queue<Matrix*> Screens;
        WINDOW* win;
        std::condition_variable cv; 
        std::mutex m;
        View(string Name){ 
            name=Name;
        }
        virtual void update(Matrix* screen){ 
            std::unique_lock<std::mutex> lk(m);
            Screens.push(screen);
            cv.notify_all();
            lk.unlock();
        }
        Matrix* read(){ 
            std::unique_lock<std::mutex> lk(m); 
            
            while(Screens.size()<1){
                cv.wait(lk);
            }
            Matrix* obj=Screens.front();
            Screens.pop();
            lk.unlock();
            return obj;
        }
        void addWindow(WINDOW* window){
            win=window; 
        } 
        void run(){
            while(!isGameDone){ 
                Matrix* screen=read(); 
                if(screen==nullptr) break;
                if(isGameDone) break;
                printWindow(win, *screen);
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1); 
        }
};
class SendController:public KeyObserver{
    public:
        std::queue<char> keys;
        std::condition_variable cv;
        std::mutex m;
        string name;
        int sock_client;
        
        SendController(string Name, int Sock_client){
            name=Name;
            sock_client=Sock_client;
        }
        virtual void update(char key){ 
            std::unique_lock<std::mutex> lk(m);
            keys.push(key);
            cv.notify_all(); 
            lk.unlock(); 
        }
        char read(){
            std::unique_lock<std::mutex> lk(m);
            while(keys.size()<1){ 
                cv.wait(lk);
            }
            char key=keys.front();
            keys.pop(); 
            lk.unlock();
            return key;
        }
        void run(){
            while(!isGameDone){
                char key=read();
                if(!key) break;
                if(state==Killed) {
                    key='q';
                    isGameDone=true;
                }
                else if(state==Aborted) isGameDone=true;
                char w_buff[256];
                w_buff[0]=key;
                int write_chk=write(sock_client,w_buff,1);
                if(write_chk==-1){
                    break;
                }
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1);
        }
};

class InputController:public KeyPublisher, public KeyObserver{
    public:
        std::queue<char> keys;
        std::vector<SendController*> obs;
        std::condition_variable cv;
        std::mutex m;
        string name;
        InputController(string Name){
            name=Name;
        }
        virtual void addObserver(SendController* ob){ 
            obs.push_back(ob);
        }
        virtual void notifyObservers(char key){
            for(int i=0;i<obs.size();i++){
                obs[i]->update(key);
            }
        }
        virtual void update(char key){ 
            std::unique_lock<std::mutex> lk(m);
            keys.push(key);
            cv.notify_all();
            lk.unlock();
        }
        char read(){
            std::unique_lock<std::mutex> lk(m);
            while(keys.size()<1){ 
                cv.wait(lk);
            }
            char key=keys.front();
            keys.pop(); 
            lk.unlock();
            return key;
        }
        void run(){
            while(!isGameDone){
                char key=read();
                if(!key) break;
                notifyObservers(key);
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1); 
            notifyObservers(0);
        }
};
class Model:public KeyObserver,public OutScreenPublisher,public delRectObserver,public delRectPublisher{ //Model 클래스
    public:
        string name;
        std::vector<View*> observers_view; 
        std::vector<InputController*> observers_input;
        std::vector<Model*> models;
        std::queue<Obj> objs; 
        std::map<char,char> Keypad;
        std::condition_variable cv;
        std::mutex m; 
        bool isMine;
        
        Model(string Name, bool ismine){
            name=Name; 
            isMine=ismine;
        }
        virtual void addObserverView(View* view){
            observers_view.push_back(view); 
        }
        virtual void notifyObserversView(Matrix* screen){ 
            for(int i=0;i<observers_view.size();i++){ 
                observers_view[i]->update(screen); 
            }
        }
        virtual void update(char key){ 
            std::unique_lock<std::mutex> lk(m);
            Obj obj(key);
            objs.push(obj);
            cv.notify_all();
            lk.unlock();
        }
        void addKeypad(map<char,char>& keypad){ 
            Keypad=keypad; 
        }
        virtual void addObserverInput(InputController* input){
            observers_input.push_back(input);
        }
        virtual void notifyObserversInput(char key){
            for(int i=0;i<observers_input.size();i++){
                observers_input[i]->update(key); 
            }
        }
        virtual void addObserver(Model* model){
            models.push_back(model); 
        }
        virtual void update(Matrix delRect){
            std::unique_lock<std::mutex> lk(m);
            Obj obj(delRect);
            objs.push(obj); 
            cv.notify_all();
            lk.unlock();
        }
        virtual void notifyObservers(Matrix delRect){ 
            for(int i=0;i<models.size();i++){
                models[i]->update(delRect);
            }
        }
        Obj read(){ 
            std::unique_lock<std::mutex> lk(m);
            while(objs.size()<1){ 
                cv.wait(lk);
            }
            Obj obj=objs.front();
            objs.pop(); 
            lk.unlock();
            return obj; 
        }
        TetrisState processKey(CTetris* board, Obj obj){ 
            TetrisState state=board->accept(obj);
            if(isMine) notifyObserversInput(obj.key);
            notifyObserversView(&(board->oCScreen)); 
            if((state!=NewBlockWDel)&&(state!=NewBlockWODel)) return state; 
            if(isMine){
                srand((unsigned int)time(NULL)); 
                char Key = (char)('0' + rand() % MAX_BLK_TYPES);
                obj.key=Key;
                notifyObserversInput(obj.key);
            }
            else{
                obj=read();
            }
            state=board->accept(obj); 
            if(state==NewBlockWDel){ 
                notifyObservers(board->getDelRect());
            }
            notifyObserversView(&(board->oCScreen)); 
            return state; 
        }
        void run(){
            CTetris *board=new CTetris(20,15); 
            Obj obj;

            if(isMine){
                srand((unsigned int)time(NULL)); 
                char Key = (char)('0' + rand() % MAX_BLK_TYPES);
                obj.key=Key;
                notifyObserversInput(obj.key);
            }
            else{
                obj=read();
            }
            state=board->accept(obj); 
            notifyObserversView(&(board->oCScreen));
            
            while(!isGameDone){ 
                obj=read();
                if(!obj.key) break;
                if(Keypad.find(obj.key)==Keypad.end()) continue;
                obj.key=Keypad[obj.key];
                if(obj.key=='q') {
                    state=Aborted;
                    if(isMine) notifyObserversInput(obj.key);
                }
                else state=processKey(board, obj);
            }
            delete board;
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1);
            notifyObserversView(nullptr);
            notifyObserversInput(0);
        }
};
class RecvController:public KeyPublisher{
    public:
        string name;
        std::vector<Model*> Models;
        int sock_client;
        RecvController(string Name, int Sock_client){
            name=Name;
            sock_client=Sock_client;
        }
        virtual void addObserver(Model* model){ 
            Models.push_back(model); 
        }
        virtual void notifyObservers(char key){
            for(int i=0;i<Models.size();i++){
                Models[i]->update(key);
            }
        }
        void run(){
            char r_buff[256];
            while(!isGameDone){
                int read_chk=read(sock_client,r_buff,1);
                if(read_chk == -1){ 
                    cout << "read error" << endl;
                    break;
                }
                char key=r_buff[0];
                printMsg(to_string(key));
                notifyObservers(key);
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1);
            notifyObservers(0);
        }

};

class KeyController:public KeyPublisher{
    public:
        string name; 
        std::vector<Model*> observers;
	
        KeyController(string Name){
                name=Name; 
            }
            virtual void addObserver(Model* input){
                observers.push_back(input);
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
                if(!key) throw runtime_error("key interrupt");
                } catch (runtime_error err){
                //printMsg("pass");
                }
            }
            string str=name+" is terminated...";
            printMsg(str);
            sleep(1);
            notifyObservers(0);
        }
};
class TimeController:public KeyPublisher{ 
    public: 
        string name;
        std::vector<Model*> observers; 
        
	TimeController(string Name){
            name=Name; 
        }
        virtual void addObserver(Model* input){ 
            observers.push_back(input); 
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
};

void usage(char *argv){ 
    cout << "Usage : " << argv << " [ip] [port]" << endl;
    cout << "Example) " << argv << " 192.168.0.12 1234" << endl;
}
void signal_handler(int signal)
{
  cout<<" "<<endl;
}
int main(int argc, char *argv[]){
    if(argc!=3){
        usage(argv[0]);
        return -1;
    }
    signal(SIGTSTP,signal_handler);
    struct sockaddr_in addr_server = {};
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET; 
    addr_server.sin_addr.s_addr = inet_addr(argv[1]); 
    addr_server.sin_port = htons(atoi(argv[2])); 
    int sock_client = socket(AF_INET, SOCK_STREAM, 0);
    
    if(sock_client == -1){ 
        cout << "socket error" << endl; 
        close(sock_client); 
        exit(1); 
    }
    bool option=1;
    setsockopt( sock_client, SOL_SOCKET, SO_REUSEADDR||SO_REUSEPORT, (const char*)&option, sizeof(option) );
    if(connect(sock_client, (struct sockaddr*) &addr_server, sizeof(addr_server)) == -1){ 
        cout << "connect error" << endl;
        close(sock_client);
        exit(1); 
    }

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
    CTetris::init(setOfCBlockArrays, MAX_BLK_TYPES, MAX_BLK_DEGREES); 
    WINDOW *win1,*win2;
    win1=newwin(20, 30, 0, 0); 
    win2=newwin(20, 30, 0, 60); 

    map<char,char> keypad1={{'q','q'},{'w','w'},{'a','a'},{'s','y'},{'d','d'},{' ',' '},{'y','y'},{-1,-1},{'0','0'},{'1','1'},{'2','2'},{'3','3'},{'4','4'},{'5','5'},{'6','6'}};
    //map<char,char> keypad2={{'u','q'},{'i','w'},{'j','a'},{'k','y'},{'l','d'},{'\r',' '},{'y','y'},{-1,-1}};
    
    View th_view1("view1"); 
    th_view1.addWindow(win1);
    View th_view2("view2"); 
    th_view2.addWindow(win2); 
    
    SendController scont("scont",sock_client);

    InputController icont("icont");
    icont.addObserver(&scont);

    Model th_model1("model1",true); 
    th_model1.addKeypad(keypad1);
    th_model1.addObserverView(&th_view1); 
    th_model1.addObserverInput(&icont);

    Model th_model2("model2",false); 
    th_model2.addKeypad(keypad1); 
    th_model2.addObserverView(&th_view2); 

    RecvController rcont("rcont",sock_client);
    rcont.addObserver(&th_model2);

    KeyController th_cont1("kcont"); 
    th_cont1.addObserver(&th_model1);

    TimeController th_cont2("tcont");
    th_cont2.addObserver(&th_model1);

	
    std::vector<std::thread> threads;
    threads.push_back(std::thread(&View::run, &th_view1)); 
    threads.push_back(std::thread(&View::run, &th_view2));
    threads.push_back(std::thread(&Model::run, &th_model1));
    threads.push_back(std::thread(&Model::run, &th_model2));
    threads.push_back(std::thread(&SendController::run, &scont));
    threads.push_back(std::thread(&InputController::run, &icont));
    threads.push_back(std::thread(&RecvController::run, &rcont));
    threads.push_back(std::thread(&KeyController::run, &th_cont1));
    threads.push_back(std::thread(&TimeController::run, &th_cont2));

    for(int i=0;i<threads.size();i++){
        threads[i].join();
    }
    printMsg("Program terminated!\n");
    endwin();
    return 0; 
}
