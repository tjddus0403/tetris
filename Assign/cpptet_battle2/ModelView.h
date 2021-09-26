#pragma once
#include "Interface.h"
#include <ncurses.h>
#include <map>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

#define MAX_BLK_TYPES 7
#define MAX_BLK_DEGREES 4

extern bool isGameDone;
extern std::mutex mu;

class View:public OutScreenObserver{
    public:
        string name;
        std::queue<Matrix*> Screens;
        WINDOW* win;
        std::condition_variable cv; 
        std::mutex m;

        View(string Name);
        void addWindow(WINDOW* window); 

        void updateView(Matrix* screen);
       
        Matrix* read();
        void printWindow(WINDOW *window, Matrix& screen);
        void run();
};

class Model:public KeyObserver,public OutScreenPublisher,public delRectObserver,public delRectPublisher{ //Model 클래스
    public:
        string name;
        std::vector<View*> observers_view; 
        std::vector<KeyObserver*> observers_send;
        std::vector<Model*> models;
        std::queue<Obj> objs; 
        std::map<char,char> Keypad;
        std::condition_variable cv;
        std::mutex m; 
        bool isMine;
        bool isServer; //해당 객체가 서버의 객체인지 클라이언트의 객체인지 구분하기 위한 변수
        int sock_client1; //(서버일 경우 필요) 클라이언트1의 소켓디스크립터
        int sock_client2; //(서버일 경우 필요) 클라이언트2의 소켓디스크립터
        int sock_server; //(클라이언트일 경우 필요) 서버의 소켓디스크립터
        
        Model(string Name, bool ismine, bool isServer);
        void addKeypad(map<char,char>& keypad);

        void addObserverView(View* view);
        void notifyObserversView(Matrix* screen);
        
        void updateKey(char key);
        void addObserverKey(KeyObserver* send);
        void notifyObserversKey(char key);
        
        void updateDel(Matrix delRect);
        void addObserverDel(Model* model);
        void notifyObserversDel(Matrix delRect);

        Obj Read();
        TetrisState processKey(CTetris* board, Obj obj);
        void printMsg(string msg);
        void setClient(int server, int client1, int client2); //각 소켓디스크립터를 설정해주는 함수
        void sendQ(); //(서버일 경우 사용) 클라이언트들에게 승패의 결과를 전달해주는 함수
        void recvQ(); //(클라이언트일 경우 사용) 서버로부터 승패의 결과를 전달받는 함수
        void run();
};
