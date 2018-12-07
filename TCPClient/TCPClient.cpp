#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <WinSock2.h>
#include <iostream>

using namespace std;

#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE	   512

void err_quit(const char *msg);
void err_display(const char *msg);
int recvn(SOCKET s, char *buf, int len, int flags);

int main(int argc, char *argv[]) {
	
	int retVal;

	// init winsock
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return 1; }


	// socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET) { err_quit("socket()"); }


	// connect()
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = htons(SERVERPORT);

	retVal = connect(sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (retVal == SOCKET_ERROR) { err_quit("connect()"); }


	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE + 1];
	int len;


	// 서버와 데이터 통신
	while (1) {
		// 데이터 입력
		cout << "\n[보낼 데이터] ";

		if (fgets(buf, BUFSIZE + 1, stdin) == NULL) { break; }


		// '\n' 문자 제거
		if (strlen(buf) == 0) {
			break;
		}
		else {
			len = strlen(buf);
			if (buf[len - 1] == '\n') {
				buf[len - 1] = '\0';
			}

		}


		// 데이터 보내기
		retVal = send(sock, buf, strlen(buf), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		cout << "[TCP 클라이언트] " << retVal << "바이트를 보냈습니다." << endl;


		// 데이터 받기
		retVal = recvn(sock, buf, retVal, 0);
		if (retVal == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retVal == 0) {
			break;
		}


		// 받은 데이터 출력
		buf[retVal] = '\0';
		cout << "[TCP 클라이언트] " << retVal << "바이트를 받았습니다." << endl;
		cout << "[받은 데이터] " << buf << endl;
	}


	// closesocket()
	closesocket(sock);


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

int recvn(SOCKET s, char *buf, int len, int flags) {
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		
		if (received == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
		else if (received == 0) {
			break;
		}

		left -= received;
		ptr += received;
	}

	return (len - left);
}
