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
using namespace std;

void usage(char *argv){ //인자가 2개(프로그램 실행과 포트 넘버)가 아닐 시, 아래의 내용을 출력할 함수
    cout << "Usage : " << argv << " [port]" << endl;
    cout << "Example) " << argv << " 1234" << endl;
}
void signal_handler(int signal)
{
  cout<<"but ok!"<<endl;
}
int main(int argc, char *argv[]){
    if(argc != 2){ // 인자가 2개가 아니면 usage 출력
        usage(argv[0]); 
        return -1;
    }
    char buff[256]; // 읽기, 쓰기용 버퍼 선언
    //char buff
    //TCP/IP 상에서의 통신이므로 IPv4용 구조체인 sockaddr_in 사용
    struct sockaddr_in addr_server = {}; // 서버용 주소체계 구조체 선언
    struct sockaddr_in addr_client = {}; // 클라이언트용 주소체계 구조체 선언
    socklen_t addr_client_len = sizeof(addr_client_len); // 길이 계산
    signal(SIGPIPE, signal_handler);
    signal(SIGTSTP, signal_handler);
    bool option=true;
 
    int sock_server = socket(AF_INET, SOCK_STREAM, 0); 
    // socket함수 통해 프로토콜 체계와 소켓 타입을 지정해서 TCP 서버 소켓 생성 (실패 시, -1 반환)
    //프로토콜 체계 : AF_INET (정수 2) (IPv4) / 소켓 생성 시, 환경을 고려해 프로토콜 체계를 지정해주면 그 환경에 사용 가능한 소켓 생성됨
    //소켓 타입 : SOCK_STREAM (정수 1) (스트림 / 연결 지향형 소켓, TCP 프로토콜의 전송 방식)
    //프로토콜 : 0 (기본 프로토콜)
    //(AF_INET)-(SOCK_STREAM) = TCP 소켓
    //따라서 1,2번째 인자로 인해 프로토콜이 정해질 수 있기 때문에 3번재 인자값에 0을 입력해도 소캣을 생성됨 (3번째 : 프로토콜을 조금더 구체화)
    ////서버 소켓 디스크립터 sock_server (소켓 번호라고 함)
    if(sock_server == -1){ //소켓 생성 실패 시, 
        cout << "socket error" << endl; //에러 메시지 출력 후
        close(sock_server); //소켓 닫기
        exit(1); //프로그램 종료
    }
    //setsockopt( sock_server, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &option, sizeof(option) );
    memset(&addr_server, 0, sizeof(addr_server)); // addr_server 구조체 초기화
    addr_server.sin_family = AF_INET; // IPv4 인터넷 프로토콜
    addr_server.sin_port = htons(atoi(argv[1])); // 첫번째 인자 PORT 지정 (서버가 사용할 포트 번호)
    //unsigned long nAddr = inet_addr("192.168.59.111");
    addr_server.sin_addr.s_addr = htonl(INADDR_ANY); 
    //htonl : 서버의 주소와 포트 번호는 IP헤더에 저장되어 전송되지만 이를 중계하는 라우터들은 네트워크 바이트 방식으로 처리
    //INADDR_ANY : 서버에 연결된 네트워크 인터페이스를 목적지로 하여 들어오는 모든 자료를 수신 가능
    setsockopt(sock_server,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
    if(bind(sock_server, (sockaddr*) &addr_server, sizeof(addr_server)) == -1){
        //bind함수 통해 socket_server가 가리키는 서버 소켓에 addr_server가 가리키는 주소 정보가 할당됨 (실패 시, -1 반환)
        //따라서 해당 addr_server는 서버 소켓을 통해 다른 클라이언트로부터 연결을 받아들일 수 있음
        cout << "bind error" << endl; //bind 실패 시, 에러 메시지 출력 후
        close(sock_server); //소켓 닫기
        exit(1); //프로그램 종료
    }
    //setsockopt(sock_server,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
    //setsockopt(sock_server,SOL_SOCKET,SO_REUSEPORT,&option,sizeof(option));

    if(listen(sock_server, 3) == -1){ 
        //listen함수 통해 서버 소켓과 연결 요청 대기 큐가 완전히 준비되어 클라이언트의 연결 요청을 받을 수 있는 상태로 만들기 (실패 시, -1 반환)
        //성공 시, 클라이언트들이 연결 요청을 할 것임. 모든 연결 요청들은 미리 만들어 놓은 연결 요청 대기 큐로 들어가 순서대로 수락될 때까지 기다림
        //listen함수 호출 전 : 서버 소켓 상태=CLOSE, 호출 후 : 서버 소켓 상태=LISTEN
        cout << "listen error" << endl; //listen 실패 시, 에러 메시지 출력 후 
        close(sock_server); //소켓 닫기
        exit(1); //프로그램 종료
    }


    while(1){ 
        int sock_client = accept(sock_server, (sockaddr*) &addr_client, &addr_client_len); 
        //accept함수 통해 연결 요청 대기 큐에 있는 클라이언트의 데이터 입,출력을 위해 사용될 소켓을 생성하고 해당 소켓의 파일 디스크립터 반환 받음 (실패 시, -1 반환)
        //클라이언트 소켓 디스크립터 sock_client (소켓 번호라고 함)
        if(sock_client == -1){ //accept 실패 시,
            cout << "accept error" << endl; //에러 메시지 출력 후
            close(sock_server); //소켓 닫기
            exit(1); //프로그램 종료
        }//반복문 통해 클라이언트와 계속하여 데이터 주고 받음
        while(1){
            memset(buff, 0, 256); // 버퍼 초기화
            int read_chk = read(sock_client, buff, sizeof(buff)-1);
            //read함수 통해 클라이언트 소켓으로부터 데이터를 버퍼크기-1 크기 만큼 읽어와 버퍼에 읽어온 데이터 저장 (실패 시, -1 반환 / 성공 시, 읽어온 데이터 크기 반환)
            if(read_chk == -1){ //read 실패 시,
                cout << "read error" << endl; //에러 메시지 출력 후 
                break; //반복문 탈출
            }
            buff[strlen(buff)] = '\n';
            cout << buff; // 버퍼 출력 (클라이언트로부터 읽어온 데이터 출력)
            int write_chk = write(sock_client, buff, strlen(buff)); // 버퍼 사이즈만큼 write(전송)
            //write함수 통해 버퍼에 있는 데이터를 클라이언트 소켓에 전송 (실패 시, -1 반환 / 성공 시, 전송한 데이터 크기 반환)
            if(write_chk == -1){ //write 실패 시,
                cout << "write error" << endl; //에러 메시지 출력 후
                break; //반복문 탈출
            }
        }
    }
    close(sock_server); // 소켓 닫기(연결 종료)
    return 0;
}
