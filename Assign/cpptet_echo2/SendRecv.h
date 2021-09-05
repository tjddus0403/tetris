#pragma once
#include "Interface.h"
#include <mutex>
#include <condition_variable>
#include <unistd.h>

extern bool isGameDone;
extern TetrisState state;

class SendController:public KeyObserver{
    public:
        std::queue<char> keys;
        std::condition_variable cv;
        std::mutex m;
        string name;
        int sock_client;
        
        SendController(string Name, int Sock_client);
        void updateKey(char key);
        char read();
        void run();
};

class RecvController:public KeyPublisher{
    public:
        string name;
        std::vector<KeyObserver*> Models;
        int sock_client;
        
        RecvController(string Name, int Sock_client);
        void addObserverKey(KeyObserver* model);
        void notifyObserversKey(char key);
        void run();
};

