#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <iostream>


#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

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



	// 1. ���� �̸� �Է¹ޱ�
	char fileName[256];
	ZeroMemory(fileName, 256);
	std::cout << "Ȯ���ڸ� ������ ���� �̸��� �Է����ּ���. > ";
	std::cin >> fileName;

	// 2. ���� �о����
	FILE *fp = fopen(fileName, "rb");

	if (fp == NULL) {
		std::cout << "�ش� ������ �������� �ʽ��ϴ�!" << std::endl;
		err_quit("send()");
	}

	// 3. ���� �̸� �����ϱ�
	int fileNameLen = strlen(fileName);

	// ������ ������ (���� ����)
	retVal = send(sock, (char*)&fileNameLen, sizeof(int), 0);
	if (retVal == SOCKET_ERROR) {
		err_display("send()");
	}

	// ������ ������ (���� ����)
	retVal = send(sock, fileName, fileNameLen, 0);
	if (retVal == SOCKET_ERROR) {
		err_display("send()");
	}

	std::cout << "[TCP Ŭ���̾�Ʈ] " << sizeof(int) << " + " << retVal << "(���� �̸��� ũ��)" << "����Ʈ�� ���½��ϴ�. \n";


	// 4. ���� ũ����
	unsigned int fileSize = 0;
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);

	fseek(fp, 0, SEEK_SET); // ���� ������ ���� ���� �����͸� �� ������ �̵�
	char buf[BUFSIZE];

	// 5. ���� ũ�⸦ len�� �����Ͽ� ����
	// ������ ������ (���� ����)
	retVal = send(sock, (char*)&fileSize, sizeof(int), 0);
	if (retVal == SOCKET_ERROR) {
		err_display("send()");
	}

	std::cout << "[TCP Ŭ���̾�Ʈ] " << "���� ũ�� " << fileSize << "(����Ʈ)�� ���� " << sizeof(int) << "����Ʈ�� ���½��ϴ�. \n";

	// 6. ���� ����
	// ������ ������ (���� ����)
	// ������ ũ�� < BUFSIZE -> �ٷ� ����
	// ������ ũ�� > BUFSIZE -> ������ ����

	unsigned int sendFileSize = 0;
	unsigned int uploadFileSize = 0;
	unsigned int readFileSize = 0;
	unsigned int leftFileSize = 0;
	leftFileSize = fileSize;


	while (1) {
		ZeroMemory(buf, BUFSIZE);

		// ������ �� ���� ����� ������ ũ�⺸�� ũ�ٸ� �߶��ֱ�
		if (leftFileSize >= BUFSIZE) {
			sendFileSize = BUFSIZE - 1;
		}
		else {
			sendFileSize = leftFileSize;
		}

		// ���� ����
		retVal = send(sock, (char*)&sendFileSize, sizeof(int), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("send()");
		}

		// ���� ����
		readFileSize = fread(buf, 1, sendFileSize, fp);
		if (readFileSize > 0) {
			retVal = send(sock, buf, readFileSize, 0);
			if (retVal == SOCKET_ERROR) {
				err_display("send()");
			}
			uploadFileSize += readFileSize;
			if (leftFileSize >= BUFSIZE) {
				leftFileSize -= readFileSize;
			}

		}
		else if (readFileSize == 0 && uploadFileSize == fileSize) {
			std::cout << "���� ���� ����!" << std::endl;
			break;
		}
		else {
			std::cout << "���� ����� ����!" << std::endl;
		}
		std::cout << "[TCP Ŭ���̾�Ʈ] ���� ���ε� ���� : " << ((double)uploadFileSize / (double)fileSize) * 100 << "%, "
			<< sizeof(int) << " + " << retVal << "����Ʈ ����, �� " << uploadFileSize << "����Ʈ ����. \n";
	}

	std::cout << "[TCP Ŭ���̾�Ʈ] ���� ���ε� �Ϸ�! ���� ���� ũ�� : " << uploadFileSize << "����Ʈ" << std::endl;

	fclose(fp);
	closesocket(sock);

	WSACleanup();

	Sleep(50000);


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
