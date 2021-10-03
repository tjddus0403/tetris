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
        w_buff[0]=key; //읽어온 key값을 w_buff[0]에 저장 
        if(key=='q'){ //만약 읽어온 key값이 'q'라면,
            if(isServer){ //서버의 SendController 객체인 경우,
                w_buff[1]='W'; //w_buff[1]을 'W'로 설정하여
                int write_chk=write(sock_client1, w_buff, 2);
                //상대 클라이언트에게 'qW' 전송 (승리 의미)
                w_buff[1]='L'; //w_buff[1]을 'L'로 설정하여
                write_chk=write(sock_client2, w_buff, 2);
                //내가 담당한 클라이언트에게 'qL' 전송 (패배 의미)
            } //서버가 아닌 경우, write를 이용해 서버에 'q'전달
            else int write_chk=write(sock_client1,w_buff,1);
        }//key값이 'q'가 아닌 경우, write를 이용해 서버에 key값 전달
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
            if(!isServer) notifyObserversKey(r_buff[1]); 
            //서버로부터 받아온 Key값이 'q'라면, r_buff[1]값('L'/'W')도 객체의 KeyObserver에 전달
            break; 
        }
    }
    sleep(1);
    notifyObserversKey(0);
    if(isServer)
        cout<<name<< "is terminated"<<endl;
}
