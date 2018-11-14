#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <iostream>
#include <fstream>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 1024

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
	std::ifstream in(fileName, std::ios::ate|std::ios::binary); // ������ �����ϸ鼭 ���� �����͸� ���κ����� �ű�

	if (!in) {
		std::cout << "�ش� ������ �������� �ʽ��ϴ�!" << std::endl;
		err_quit("send()");
	}

	// 3. ���� �̸� �����ϱ� (�ߺ��� ������ ������ ����� �ϱ� ����)
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
	int fileSize = 0;
	fileSize = in.tellg();

	in.seekg(0, std::ios::beg); // ���� ������ ���� ���� �����͸� �� ������ �̵�
	char buf[BUFSIZE];


	// 5. ���� ũ�⸦ len�� �����Ͽ� ����
	// ������ ������ (���� ����)
	retVal = send(sock, (char*)&fileSize, sizeof(int), 0);
	if (retVal == SOCKET_ERROR) {
		err_display("send()");
	}

	std::cout << "[TCP Ŭ���̾�Ʈ] " << "���� ũ�� " << fileSize << "�� ���� " << sizeof(int) << "����Ʈ�� ���½��ϴ�. \n";
	
	// 6. ���� ����
	// ������ ������ (���� ����)
	// ������ ũ�� < BUFSIZE -> �ٷ� ����
	// ������ ũ�� > BUFSIZE -> ������ ����
	while (1) {
		if (fileSize <= BUFSIZE) {
			in.read(buf, fileSize);
			retVal = send(sock, buf, fileSize, 0);
			if (retVal == SOCKET_ERROR) {
				err_display("send()");
			}
			std::cout << "[TCP Ŭ���̾�Ʈ] " << retVal << "����Ʈ�� ���½��ϴ�. \n";
			std::cout << "���� ���� ����!" << std::endl;
			break;
		}

		else {
			in.read(buf, BUFSIZE);
			std::cout << buf << std::endl;
			retVal = send(sock, buf, BUFSIZE, 0);
			if (retVal == SOCKET_ERROR) {
				err_display("send()");
			}
			
			in.seekg(BUFSIZE);
			fileSize -= BUFSIZE;
		}


		std::cout << "[TCP Ŭ���̾�Ʈ] " << retVal << "����Ʈ�� ���½��ϴ�. \n";
	}




	closesocket(sock);

	WSACleanup();
	in.close();

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
