#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
using namespace std;

void usage(char *argv){  //인자가 3개(프로그램 실항과 IP주소, 포트 넘버)가 아닐 시, 아래의 내용을 출력할 함수
    cout << "Usage : " << argv << " [ip] [port]" << endl;
    cout << "Example) " << argv << " 192.168.0.12 1234" << endl;
}

int main(int argc, char *argv[]){ 
    if(argc != 3){ //인자가 3개가 아니면 usage를 출력
        usage(argv[0]);
        return -1;
    }

    struct sockaddr_in addr_server = {}; // 서버용 주소체계 구조체 선언

    memset(&addr_server, 0, sizeof(addr_server)); //addr_server 구조체 초기화
    addr_server.sin_family = AF_INET; // IPv4 통신, 기본 인터넷 프로토콜 지정
    addr_server.sin_addr.s_addr = inet_addr(argv[1]); // 첫번째 인자 IP 지정 (서버가 사용할 IP주소)
    addr_server.sin_port = htons(atoi(argv[2])); // 두번째 인자 PORT 지정 (서버가 사용할 포트 번호)

    char w_buff[256]; // 쓰기용 버퍼 선언
    char r_buff[256]; // 읽기용 버퍼 선언

    int sock_client = socket(AF_INET, SOCK_STREAM, 0); // 소켓 생성(IPv4, TCP, 기본프로토콜)
    // socket함수 통해 프로토콜 체계와 소켓 타입을 지정해서 TCP 클라이언트 소켓 생성 (실패 시, -1 반환)
    if(sock_client == -1){ //소켓 생성 실패 시,
        cout << "socket error" << endl; //에러 메시지 출력 후
        close(sock_client); //소켓 닫기
        exit(1); //프로그램 종료
    }

    if(connect(sock_client, (struct sockaddr*) &addr_server, sizeof(addr_server)) == -1){ 
        //connect함수 통해 클라이언트 소켓을 addr_server가 가리키는 서버에 연결 요청 (실패 시, -1 반환)
        //네트워크 통해 데이터를 주고 받기 위해 소켓이 필요 + 소켓은 IP주소와 포트 넘버가 반드시 할당되어야 함
        //클라이언트의 주소 할당은 connect함수 호출 시, 커널이 자동으로 해주기 때문에 bind가 필요 없음
        //값이 리턴되는 시점은 연결 요청이 수락되거나, 오류가 발생해 요청이 중단되는 경우임
        //연결 요청이 이루어지지 않고 서버의 대기 큐에서 기다리고 있는 상태이면 connect함수는 리턴되지 않고 블로킹 상태에 있게됨
        cout << "connect error" << endl; //연결 요청 실패 시, 에러 메시지 출력 후
        close(sock_client); //소켓 닫기
        exit(1); //프로그램 종료
    }

    while(1){ // 연결 수락시 반복문
        memset(r_buff, 0, 256); // 읽기 버퍼 초기화
        cin >> w_buff; // 쓰기 버퍼에 문자열 입력
        if(strlen(w_buff)>255) break; // 버퍼 오버플로그 방지
        int write_chk = write(sock_client, w_buff, strlen(w_buff)); // 작성 길이만큼 write(전송)
        //write함수 통해 쓰기 버퍼에 있는 데이터를 클라이언트 소켓에 전송 
        if(write_chk == -1){ //write 실패 시,
            cout << "write error" << endl; //에러 메시지 출력 후 
            break; //반복문 탈출
        }
        int read_chk = read(sock_client, r_buff, sizeof(r_buff)-1); // 읽기 버퍼사이즈-1 만큼 read(읽기)
        //read함수 통해 클라이언트 소켓으로부터 데이터를 읽기 버퍼크기-1 크기 만큼 읽어와 읽기 버퍼에 저장
        if(read_chk == -1){ //read 실패 시,
            cout << "read error" << endl; //에러 메시지 출력 후
            break; //반복문 탈출
        }else{
            r_buff[strlen(r_buff)] = '\n'; //성공 시, 
            cout << r_buff; // 읽기 버퍼 출력
        }
    }
    close(sock_client); // 소켓 닫기(연결 종료)
    return 0;
}

