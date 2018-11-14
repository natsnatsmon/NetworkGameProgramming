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


	// ������ ��ſ� ����� ����
	char buf[BUFSIZE + 1];
	int len;


	// ������ ������ ���
	while (1) {
		// ������ �Է�
		cout << "\n[���� ������] ";

		if (fgets(buf, BUFSIZE + 1, stdin) == NULL) { break; }


		// '\n' ���� ����
		if (strlen(buf) == 0) {
			break;
		}
		else {
			len = strlen(buf);
			if (buf[len - 1] == '\n') {
				buf[len - 1] = '\0';
			}

		}


		// ������ ������
		retVal = send(sock, buf, strlen(buf), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		cout << "[TCP Ŭ���̾�Ʈ] " << retVal << "����Ʈ�� ���½��ϴ�." << endl;


		// ������ �ޱ�
		retVal = recvn(sock, buf, retVal, 0);
		if (retVal == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retVal == 0) {
			break;
		}


		// ���� ������ ���
		buf[retVal] = '\0';
		cout << "[TCP Ŭ���̾�Ʈ] " << retVal << "����Ʈ�� �޾ҽ��ϴ�." << endl;
		cout << "[���� ������] " << buf << endl;
	}


	// closesocket()
	closesocket(sock);


	// ���� ����
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
