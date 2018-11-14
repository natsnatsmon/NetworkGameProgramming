#pragma comment(lib, "ws2_32")
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <WinSock2.h>
#include <iostream>

using namespace std;

void err_quit(const char* msg);
void err_display(const char* msg);

// ip, ��Ʈ��ȣ ~ ��Ʈ��ȣ �Է��ϰ� connect()�� �����ִ��� Ȯ���ϱ�~
int main(int argc, char* argv[]) {
	//init winsock
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return 1; }

	char* serverIp {};
	int serverPortMin;
	int serverPortMax;
	
	if (argc >= 5) { 
		cout << "������ ������ �ʹ� �����ϴ�." << endl; 
		return 0;
	}
	else if (argc <= 3) { 
		cout << "������ ������ �ʹ� �����ϴ�." << endl;
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
			cout << "��Ʈ��ȣ : " << serverPort << " ���� ����!" << endl << "���� : ";
			err_display("connect()"); 
		}
		else if (retVal == 0) {
			cout << "��Ʈ��ȣ : " << serverPort << " ���� ����!" << endl << endl;
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
