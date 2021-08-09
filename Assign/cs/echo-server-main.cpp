#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
using namespace std;

void usage(char *argv){ //인자가 2개(프로그램 실행과 포트 넘버)가 아닐 시, 아래의 내용을 출력할 함수
    cout << "Usage : " << argv << " [port]" << endl;
    cout << "Example) " << argv << " 1234" << endl;
}

int main(int argc, char *argv[]){
    if(argc != 2){ // 인자가 2개가 아니면 usage 출력
        usage(argv[0]); 
        return -1;
    }

    char buff[256]; // 읽기, 쓰기용 버퍼 선언

    struct sockaddr_in addr_server = {}; // server용 주소체계 구조체 선언
    struct sockaddr_in addr_client = {}; // client용 주소체계 구조체 선언
    socklen_t addr_client_len = sizeof(addr_client_len); // 길이 계산

    memset(&addr_server, 0, sizeof(addr_server)); // 초기화
    addr_server.sin_family = AF_INET; // IPv4 인터넷 프로토콜
    addr_server.sin_port = htons(atoi(argv[1])); // 첫번째 인자 PORT 지정
    addr_server.sin_addr.s_addr = htonl(INADDR_ANY); // Anybody
 

    int sock_server = socket(AF_INET, SOCK_STREAM, 0); // 소켓 생성
    if(sock_server == -1){
        cout << "socket error" << endl;
        close(sock_server);
        exit(1);
    }

    if(bind(sock_server, (sockaddr*) &addr_server, sizeof(addr_server)) == -1){ // 주소 지정
        cout << "bind error" << endl;
        close(sock_server);
        exit(1);
    }

    if(listen(sock_server, 3) == -1){ // 연결 대기
        cout << "listen error" << endl;
        close(sock_server);
        exit(1);
    }

    int sock_client = accept(sock_server, (sockaddr*) &addr_client, &addr_client_len); // 연결 수락
    if(sock_client == -1){
        cout << "accept error" << endl;
        close(sock_server);
        exit(1);
    }

    while(1){
        memset(buff, 0, 256); // 버퍼 초기화
        int read_chk = read(sock_client, buff, sizeof(buff)-1); // 버퍼크기-1만큼 read(읽기)
        if(read_chk == -1){
            cout << "read error" << endl;
            break;
        }
        buff[strlen(buff)] = '\n';
        cout << buff; // 버퍼 출력
        int write_chk = write(sock_client, buff, strlen(buff)); // 버퍼 사이즈만큼 write(전송)
        if(write_chk == -1){
            cout << "write error" << endl;
            break;
        }
    }
    close(sock_server); // 연결 종료
    return 0;
}

