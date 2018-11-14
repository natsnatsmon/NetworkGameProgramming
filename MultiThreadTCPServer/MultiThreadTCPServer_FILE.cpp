#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <iostream>
#include <Windows.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE 4096

CRITICAL_SECTION hCriticalSection; // critical section ����

void err_quit(const char * msg);
void err_display(const char * msg);
int recvn(SOCKET s, char * buf, int len, int flags);
void gotoxy(int x, int y);


int threadNum = 0;

DWORD WINAPI ProcessClient(LPVOID arg) {
	SOCKET client_sock = (SOCKET)arg;
	const int currentThreadNum = threadNum++;
	int retVal;

	SOCKADDR_IN clientAddr;
	int addrLen;
	char buf[BUFSIZE + 1];
	
	system("cls");

	gotoxy(0, currentThreadNum);


	// Ŭ���̾�Ʈ ���� �ޱ�
	addrLen = sizeof(clientAddr);
	getpeername(client_sock, (SOCKADDR *)&clientAddr, &addrLen);

	unsigned int fileSize = 0;
	unsigned int fileNameLen = 0;
	unsigned int downloadFileSize = 0;
	unsigned int recvFileSize = 0;

	while (1) {

		// 1. ���� �̸� �ޱ�
		char fileName[256];
		ZeroMemory(fileName, 256);

		// ������ �ޱ�(���� ����)
		retVal = recvn(client_sock, (char *)&fileNameLen, sizeof(int), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retVal == 0) { break; }


		// ������ �ޱ�(���� ����)
		retVal = recvn(client_sock, buf, fileNameLen, 0);
		if (retVal == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retVal == 0) { break; }

		// ���� ������ ���
		buf[retVal] = '\0';
		std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr)
			<< ":" << ntohs(clientAddr.sin_port) << "] " << buf << " ���� ����" << std::endl;

		strcpy(fileName, buf);

		// 2. ���� ����
		FILE *fp = fopen(fileName, "wb");
		if (fp == NULL) {
			std::cout << "���� ���� ����" << std::endl;
			exit(1);
		}

		// 3. ���� �����ϱ�
		// Ŭ���̾�Ʈ�� ������ ���
		// ������ �ޱ�(���� ����)
		retVal = recvn(client_sock, (char *)&fileSize, sizeof(int), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retVal == 0) { break; }
		std::cout << "����ũ�� " << fileSize << std::endl;

		// ������ �ޱ�(���� ����)
		while (1) {

			std::cout << currentThreadNum << std::endl;

			ZeroMemory(buf, BUFSIZE);

			retVal = recvn(client_sock, (char *)&recvFileSize, sizeof(int), 0);
			if (retVal == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retVal == 0) { break; }

			retVal = recvn(client_sock, buf, recvFileSize, 0);
			if (retVal == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retVal == 0) { break; }

			fwrite(buf, 1, retVal, fp);
			downloadFileSize += retVal;


			// ���� ���� ���൵ ���
			printf("[TCP/%s:%d] ���� �̸� : %s \n\t ���� ���� ũ�� : %d����Ʈ, ���� �ٿ�ε� ���� %f \% \n",
				inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fileName, downloadFileSize, (((double)downloadFileSize / (double)fileSize) * 100));
		}

		std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "] \""
			<< fileName << "\" �ٿ�ε� �Ϸ�! ���� ���� ũ�� : " << ftell(fp) << std::endl;
		fclose(fp);
	}


	// closesocket()
	closesocket(client_sock);

	std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr)
		<< ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl;

	return 0;
}

int main(int argc, char* argv[]) {
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


	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrLen;
	HANDLE hThread;

	while (1) {
		// accept()
		addrLen = sizeof(clientAddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientAddr, &addrLen);

		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}


		// ������ Ŭ���̾�Ʈ ���� ���
		std::cout << "\n[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr)
			<< ", ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl;


		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);

		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }
	}


	// closesocket()
	closesocket(listen_sock);


	// quit winsock
	WSACleanup();
	return 0;
}


void err_quit(const char * msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);

	exit(1);
}

void err_display(const char * msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

	std::cout << "[" << msg << "] " << (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
}

int recvn(SOCKET s, char * buf, int len, int flags) {
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);

		if (received == SOCKET_ERROR) { return SOCKET_ERROR; }
		else if (received == 0) { break; }

		left -= received;
		ptr += received;
	}

	return (len - left);
}

void gotoxy(int x, int y){

	COORD Pos = { x - 1, y - 1 };

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);

}