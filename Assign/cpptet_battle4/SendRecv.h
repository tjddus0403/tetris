#pragma once
#include "Interface.h"
#include <mutex>
#include <condition_variable>
#include <unistd.h>

extern bool isGameDone;

class SendController:public KeyObserver{
    public:
        std::queue<char> keys;
        std::condition_variable cv;
        std::mutex m;
        string name;
        bool isServer;
        int sock_client1; 
        //(서버일 경우) 내가 담당하지 않는 클라이언트의 소켓디스크립터
        //(클라이언트일 경우) 서버의 소켓디스크립터
        int sock_client2; //(서버일 경우 필요) 내가 담당한 클라이언트의 소켓디스크립터
        
        SendController(string Name, bool isserver);
        void updateKey(char key);
        char read();
        void setClient(int client1, int client2); //각 소켓디스크립터를 설정해주는 함수
        void run();
};

class RecvController:public KeyPublisher{
    public:
        string name;
        std::vector<KeyObserver*> Models;
        int sock_client;
        bool isServer;
        
        RecvController(string Name, int Sock_client, bool isserver);
        void addObserverKey(KeyObserver* model);
        void notifyObserversKey(char key);
        void run();
};

