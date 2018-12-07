#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <WinSock2.h>
#include <iostream>

using namespace std;

void err_quit(const char* msg);
void err_display(const char* msg);

// ip, 포트번호 ~ 포트번호 입력하고 connect()로 열려있는지 확인하기~
int main(int argc, char* argv[]) {
	//init winsock
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return 1; }

	char* serverIp {};
	int serverPortMin;
	int serverPortMax;
	
	if (argc >= 5) { 
		cout << "인자의 갯수가 너무 많습니다." << endl; 
		return 0;
	}
	else if (argc <= 3) { 
		cout << "인자의 갯수가 너무 적습니다." << endl;
		return 0;
	}
	else {
		serverIp = argv[1];
		serverPortMin = atoi(argv[2]);
		serverPortMax = atoi(argv[3]);
	} 

	int retVal;

	for (int serverPort = serverPortMin; serverPort <= serverPortMax; ++serverPort) {
		// socket()
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) { err_quit("socket()"); }

		// connect()
		SOCKADDR_IN serverAddr;
		ZeroMemory(&serverAddr, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.S_un.S_addr = inet_addr(serverIp);
		serverAddr.sin_port = htons(serverPort);
		retVal = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (retVal == SOCKET_ERROR) { 
			cout << "포트번호 : " << serverPort << " 연결 실패!" << endl << "사유 : ";
			err_display("connect()"); 
		}
		else if (retVal == 0) {
			cout << "포트번호 : " << serverPort << " 연결 성공!" << endl << endl;
		}

		// closesocket()
		closesocket(sock);
	}

	WSACleanup();
	return 0;

}


void err_quit(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	cout << "[" << msg << "] " << (char*)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}
