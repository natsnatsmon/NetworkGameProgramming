#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <WinSock2.h>
#include <iostream>

using namespace std;

#define SERVERPORT 9000
#define BUFSIZE		512

void err_quit(const char *msg);
void err_display(const char *msg);

int main(int argc, char *argv[]) {
	int retVal;

	// init winsock
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return 1; }

	
	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) { err_quit("socket()"); }

	
	// bind()
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);

	retVal = bind(listen_sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));

	if (retVal == SOCKET_ERROR) { err_quit("bind()"); }


	// listen()
	retVal = listen(listen_sock, SOMAXCONN);
	if (retVal == SOCKET_ERROR) { err_quit("listen()"); }

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrLen;
	char buf[BUFSIZE + 1];

	while (1) {

		// accept()
		addrLen = sizeof(clientAddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientAddr, &addrLen);

		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}


		// 접속한 클라이언트 정보 출력
		cout << "\n[TCP 서버] 클라이언트 접속 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr) 
			<< ", 포트 번호 = " << ntohs(clientAddr.sin_port) << endl;

		
		// 클라이언트와 데이터 통신
		while (1) {
			
			//데이터 받기
			retVal = recv(client_sock, buf, BUFSIZE, 0);

			if (retVal == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retVal == 0) {
				break;
			}


			// 받은 데이터 출력
			buf[retVal] = '\0';
			cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << ":" 
				<< ntohs(clientAddr.sin_port) << "]" << buf << endl;


			// 데이터 보내기
			retVal = send(client_sock, buf, retVal, 0);

			if (retVal == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		
		//closesocket()
		closesocket(client_sock);
		cout << "[TCP 서버] 클라이언트 종료 : IP 주소 = " << inet_ntoa(clientAddr.sin_addr)
			<< ", 포트 번호 = " << ntohs(clientAddr.sin_port) << endl;
	}


	// closesocket()
	closesocket(listen_sock);


	// 윈속 종료
	WSACleanup();
	return 0;

}

void err_quit(const char *msg) {
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char *msg) {
	LPVOID lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	
	cout << "[" << msg << "] " << (char *)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}