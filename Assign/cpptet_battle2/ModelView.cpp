#include "ModelView.h"

View::View(string Name){ 
    name=Name;
}
void View::addWindow(WINDOW* window){
    win=window; 
}

void View::updateView(Matrix* screen){ 
    std::unique_lock<std::mutex> lk(m);
    Screens.push(screen);
    cv.notify_all();
    lk.unlock();
}

Matrix* View::read(){ 
    std::unique_lock<std::mutex> lk(m); 
    
    while(Screens.size()<1){
        cv.wait(lk);
    }
    Matrix* obj=Screens.front();
    Screens.pop();
    lk.unlock();
    return obj;
}
void View::printWindow(WINDOW *window, Matrix& screen){ 
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
void View::run(){
    while(!isGameDone){ 
        Matrix* screen=read(); 
        if(screen==nullptr) break;
        if(isGameDone) break;
        printWindow(win, *screen);
    }
    sleep(1); 
}

Model::Model(string Name, bool ismine, bool isserver){
    name=Name; 
    isMine=ismine;
    isServer=isserver;
}
void Model::addKeypad(map<char,char>& keypad){ 
    Keypad=keypad; 
}

void Model::addObserverView(View* view){
    observers_view.push_back(view); 
}
void Model::notifyObserversView(Matrix* screen){ 
    for(int i=0;i<observers_view.size();i++){ 
        observers_view[i]->updateView(screen); 
    }
}

void Model::updateKey(char key){ 
    std::unique_lock<std::mutex> lk(m);
    Obj obj(key);
    objs.push(obj);
    cv.notify_all();
    lk.unlock();
}
void Model::addObserverKey(KeyObserver* send){
    observers_send.push_back(send);
}
void Model::notifyObserversKey(char key){
    for(int i=0;i<observers_send.size();i++){
        observers_send[i]->updateKey(key); 
    }
}

void Model::updateDel(Matrix delRect){
    std::unique_lock<std::mutex> lk(m);
    Obj obj(delRect);
    objs.push(obj); 
    cv.notify_all();
    lk.unlock();
}
void Model::addObserverDel(Model* model){
    models.push_back(model); 
}
void Model::notifyObserversDel(Matrix delRect){ 
    for(int i=0;i<models.size();i++){
        models[i]->updateDel(delRect);
    }
}

Obj Model::Read(){ 
    std::unique_lock<std::mutex> lk(m);
    while(objs.size()<1){ 
        cv.wait(lk);
    }
    Obj obj=objs.front();
    objs.pop(); 
    lk.unlock();
    return obj; 
}

void Model::printMsg(string msg){ //전달받은 문자열을 win0에 출력해주는 함수
  const char* c=msg.c_str(); //wprintw함수는 인자로 문자열을 받을 수 없고 char배열 형태로 주어야 함
  //따라서 전달받은 문자열을 char배열 형태로 바꿔줌
  WINDOW *win0; 
  win0=newwin(3, 70, 21, 0); //70*3 크기의 win0을 (0,21)좌표에 생성
  wclear(win0); //win0창 지우기
  wprintw(win0,c); //win0에 문자열 출력(사실상 char* 형태로 바꾼 것을 출력)
  mu.lock(); //락 걸기
  wrefresh(win0); //win0 갱신 (갱신을 해야 지금까지 해당 창에 적용한 것들이 실제로 보임)
  mu.unlock(); //락 해제
}
void Model::setClient(int server, int client1, int client2){
    sock_server=server;
    sock_client1=client1;
    sock_client2=client2;
}
void Model::sendQ(){
    char w_buff[256];
    w_buff[0]='q';
    w_buff[1]='L';
    int write_chk1=write(sock_client1,w_buff,2);
    cout<<"Send Lose message\n";
    w_buff[1]='W';
    int write_chk2=write(sock_client2,w_buff,2);
    cout<<"Send Win message\n";
    if(write_chk1==-1||write_chk2==-1){
        cout << "write error" << endl;
    }
}
void Model::recvQ(){
    char r_buff[256];
    int read_chk=read(sock_server,r_buff,2);
    if(read_chk == -1){ 
        cout << "read error" << endl;
    }
    if(r_buff[1]=='W'){
        printMsg("You Win\n");
    }
    else if(r_buff[1]=='L'){
        printMsg("You Lose\n");
    }
}
TetrisState Model::processKey(CTetris* board, Obj obj){ 
    TetrisState state=board->accept(obj);
    if(state!=Finished) notifyObserversKey(obj.key);
    notifyObserversView(&(board->oCScreen)); 

    if((state!=NewBlockWDel)&&(state!=NewBlockWODel)) return state; 
    if(isServer) {
        if(state==Running) cout<<name <<" : Running"<<endl;
        else if(state==NewBlockWDel) cout<<name <<" : NewBlockWDel"<<endl;
        else if(state==NewBlockWODel) cout<<name <<" : NewBlockWODel"<<endl;
        else if(state==Finished) cout<<name <<" : Finished"<<endl;
    }
    if(isMine){
        srand((unsigned int)time(NULL)); 
        char Key = (char)('0' + rand() % MAX_BLK_TYPES);
        obj.key=Key;
    }
    else{
        obj=Read();
    }
    state=board->accept(obj);
    if(isServer) cout<<name<<" is applied "<<obj.key<<endl;
    
    if(state!=Finished) notifyObserversKey(obj.key);
    
    if(state==NewBlockWDel){ 
        notifyObserversDel(board->getDelRect());
    }
    notifyObserversView(&(board->oCScreen)); 
    return state; 
}
void Model::run(){
    CTetris *board=new CTetris(20,15); 
    TetrisState state;
    Obj obj;
    int qcount=0;
    if(isMine){
        srand((unsigned int)time(NULL));
        char Key = (char)('0' + rand() % MAX_BLK_TYPES);
        obj.key=Key;
        notifyObserversKey(obj.key);
    }
    else{
        obj=Read();
        notifyObserversKey(obj.key);
    }
    state=board->accept(obj); 
    notifyObserversView(&(board->oCScreen));
    
    while(!isGameDone){ 
        obj=Read();
        if(!obj.key) break;
        if(Keypad.find(obj.key)==Keypad.end()) continue;
        obj.key=Keypad[obj.key];
        if(obj.key=='q') state=Finished;
        else state=processKey(board, obj);
        if(state==Finished){
            notifyObserversKey('q');
            if(isServer) sendQ();
            else recvQ();
            isGameDone=true;
        }
    }
    delete board;
    sleep(1);
    notifyObserversView(nullptr);
    notifyObserversKey(0);
    if(isServer)
        cout<<name<< "is terminated"<<endl;
}