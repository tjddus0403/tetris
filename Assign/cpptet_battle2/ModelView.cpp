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

void Model::printMsg(string msg){ 
  const char* c=msg.c_str(); 
  WINDOW *win0; 
  win0=newwin(3, 70, 21, 0);
  wclear(win0);
  wprintw(win0,c);
  mu.lock();
  wrefresh(win0); 
  mu.unlock();
}
void Model::setClient(int server, int client1, int client2){ //각 소켓디스크립터를 설정해주는 함수
    sock_server=server; //전달받은 소켓디스크립터 server를 객체의 sock_server로 설정
    sock_client1=client1; //전달받은 소켓디스크립터 client1을 객체의 sock_client1으로 설정
    sock_client2=client2; //전달받은 소켓디스크립터 client2를 객체의 sock_client2로 설정
}
void Model::sendQ(){ //(서버일 경우 사용) 클라이언트들에게 승패의 결과를 전달해주는 함수
    char w_buff[256]; //클라이언트에 전달할 메시지를 담는 버퍼
    w_buff[0]='q'; //w_buff[0]의 값을 'q'로 설정
    w_buff[1]='L'; //w_buff[1]의 값을 'L'로 설정 (Lose의 의미)
    int write_chk1=write(sock_client1,w_buff,2); //write함수 이용하여 클라이언트1에 'qL'전달
    cout<<"Send Lose message\n";
    w_buff[1]='W'; //w_buff[1]의 값을 'W'로 설정 (Win의 의미)
    int write_chk2=write(sock_client2,w_buff,2); //write함수 이용하여 클라이언트2에 'qW'전달
    cout<<"Send Win message\n";
    if(write_chk1==-1||write_chk2==-1){
        cout << "write error" << endl;
    }
    
    char r_buff[256];
    int read_chk1=read(sock_client1,r_buff,1); //클라이언트1이 결과를 인정했는지 확인
    if(r_buff[0]=='O') cout<<"Accept\n"; //만약 O라면 결과를 인정한 것
    int read_chk2=read(sock_client2,r_buff,1); //클라이언트2가 결과를 인정했는지 확인
    if(r_buff[0]=='O') cout<<"Accept\n"; //만약 O라면 결과를 인정한 것
}
void Model::recvQ(){ //(클라이언트일 경우 사용) 서버로부터 승패의 결과를 전달받는 함수
    char r_buff[256]; //서버로부터 메시지를 받아올 버퍼
    int read_chk=read(sock_server,r_buff,2); //read함수 이용하여 r_buff에 서버로부터의 메시지 읽어오기 
    if(read_chk == -1){ 
        cout << "read error" << endl;
    }
    if(r_buff[1]=='W'){ //만약 r_buff[1]의 값이 'W'면, 화면에 승리 문구 출력
        printMsg("You Win\n");
    }
    else if(r_buff[1]=='L'){ //만약 r_buff[1]의 값이 'L'이면, 화면에  문구 출력
        printMsg("You Lose\n");
    }
    char w_buff[256]; 
    w_buff[0]='O';
    int write_chk=write(sock_server,w_buff,1); //서버의 결과에 동의한다는 의미로 다시 O전송
}
TetrisState Model::processKey(CTetris* board, Obj obj){ //테트리스 객체에 obj가 적용되는 과정을 담은 함수
    TetrisState state=board->accept(obj); //Tetris::accept함수 이용해 obj 적용 후 게임 상태 반환받기 
    if(state!=Finished) notifyObserversKey(obj.key); //반환받은 state가 Finished가 아니면 KeyObserver들에게 key값 전달 
    notifyObserversView(&(board->oCScreen)); //테트리스 상태를 출력하기 위해 OutScreenObserver들에게 출력할 screen 전달

    if((state!=NewBlockWDel)&&(state!=NewBlockWODel)) return state; //state가 Finished / Running 이라면 해당 상태 반환
    //아니면 NewBlockWDel / NewBlockWODel 상태이므로 일단 새 블록 생성
    
    if(isMine){ //만약 사용자의 키입력을 받는 객체라면
        srand((unsigned int)time(NULL));  //랜덤으로 블록 키 값 받아서
        char Key = (char)('0' + rand() % MAX_BLK_TYPES);
        obj.key=Key; //obj의 key값으로 설정
    }
    else{ //아니면(Recv객체를 통해 키입력을 받는 객체라면)
        obj=Read(); //Read함수를 통해 obj 받아오기
    }
    state=board->accept(obj); //Tetris::accept함수 이용해 재설정된 obj 적용 후 게임 상태 반환받기 
    
    if(state!=Finished) notifyObserversKey(obj.key); //반환받은 state가 Finished가 아니면 KeyObserver들에게 key값 전달 
    if(state==NewBlockWDel){ //만약 state가 NewBlockWDel이면
        notifyObserversDel(board->getDelRect()); //getDelRect로 테트리스 객체의 delRect를 얻어와서 delRectObserver에 전달
    }
    notifyObserversView(&(board->oCScreen)); //마지막으로 다시 한 번 현재 화면 출력하기 위해 OutScreenObserver들에게 출력할 screen 전달
    return state; //현재 상태 반환
}
void Model::run(){
    CTetris *board=new CTetris(20,15); 
    TetrisState state;
    Obj obj;
    
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
        if(state==Finished){ //만약 state가 Finished라면,
            notifyObserversKey('q'); //KeyObserver에게 'q'전송
            if(isServer) { //만약 해당 객체가 서버의 객체라면,
                sendQ(); //sendQ함수를 이용하여 클라이언트들에게 게임결과(승패) 전달
            }
            else { //아니면(클라이언트의 객체라면),
                recvQ(); //recvQ함수를 이용하여 서버로부터 게임결과(승패) 받기
            }
            isGameDone=true; //isGameDone을 true로 설정 후 게임 종료
        }
    }
    delete board;
    sleep(1);
    notifyObserversView(nullptr);
    notifyObserversKey(0);
    if(isServer)
        cout<<name<< "is terminated"<<endl;
}
