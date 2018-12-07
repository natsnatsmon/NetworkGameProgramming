#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <iostream>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 50

void err_quit(const char * msg);
void err_display(const char * msg);

int main(int argc, char * argv[]) {
	int retVal;

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
	serverAddr.sin_port = ntohs(SERVERPORT);

	retVal = connect(sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (retVal == SOCKET_ERROR) { err_quit("connect()"); }


	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	const char *testData[] = {
		"안뇽안뇽안뇽",
		"Nice to meet you!",
		"저 영어 못하는데요",
		"Sorry"
	};
	int len;


	// 서버와 데이터 통신
	for (int i = 0; testData[i] != NULL; ++i) {
		// 데이터 입력 (시뮬레이션)
		len = strlen(testData[i]);
		strncpy(buf, testData[i], len);


		// 데이터 보내기 (고정 길이)
		retVal = send(sock, (char*)&len, sizeof(int), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		
		// 데이터 보내기 (가변 길이)
		retVal = send(sock, buf, len, 0);
		if (retVal == SOCKET_ERROR) { break; }



		std::cout << "[TCP 클라이언트] " << sizeof(int) << " + " << retVal << "바이트를 보냈습니다. \n";
	}


	closesocket(sock);

	WSACleanup();
	return 0;
}


void err_quit(const char * msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char * msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	std::cout << "[" << msg << "] " << (char*)lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}
