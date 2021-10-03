#include "SendRecv.h"

SendController::SendController(string Name, bool isserver){
    name=Name;
    isServer=isserver;
}
void SendController::updateKey(char key){ 
    std::unique_lock<std::mutex> lk(m);
    keys.push(key);
    cv.notify_all(); 
    lk.unlock(); 
}
char SendController::read(){
    std::unique_lock<std::mutex> lk(m);
    while(keys.size()<1){ 
        cv.wait(lk);
    }
    char key=keys.front();
    keys.pop(); 
    lk.unlock();
    return key;
}
void SendController::setClient(int client1, int client2){ //각 소켓디스크립터를 설정해주는 함수
    sock_client1=client1; //전달받은 소켓디스크립터 client1을 객체의 sock_client1으로 설정
    sock_client2=client2; //전달받은 소켓디스크립터 client2를 객체의 sock_client2로 설정
}
void SendController::run(){
    while(!isGameDone){
        char key=read();
        if(!key) break;
        char w_buff[256];
        w_buff[0]=key;
        if(key=='q'){
            if(isServer){
                w_buff[1]='W';
                int write_chk=write(sock_client1, w_buff, 2);
                w_buff[1]='L';
                write_chk=write(sock_client2, w_buff, 2);
            }
            else int write_chk=write(sock_client1,w_buff,1);
        }
        else int write_chk=write(sock_client1,w_buff,1);
        if(isServer){
            cout<<name<<" recv : "<<key<<endl;
        }
    }
    sleep(1);
    if(isServer)
        cout<<name<< "is terminated"<<endl;
}

RecvController::RecvController(string Name, int Sock_client, bool isserver){
    name=Name;
    sock_client=Sock_client;
    isServer=isserver;
}
void RecvController::addObserverKey(KeyObserver* model){ 
    Models.push_back(model); 
}
void RecvController::notifyObserversKey(char key){
    for(int i=0;i<Models.size();i++){
        Models[i]->updateKey(key);
    }
}
void RecvController::run(){
    char r_buff[256];
    while(!isGameDone){
        int read_chk=read(sock_client,r_buff,2);
        if(read_chk == -1){ 
            cout << "read error" << endl;
            break;
        }
        char key=r_buff[0];
        if(isServer){
            cout<<name<<" recv : "<<key<<endl;
        }
        if(!key) break;
        notifyObserversKey(key);
        if(key=='q') {
            notifyObserversKey(r_buff[1]);
            break;
        }
    }
    sleep(1);
    notifyObserversKey(0);
    if(isServer)
        cout<<name<< "is terminated"<<endl;
}