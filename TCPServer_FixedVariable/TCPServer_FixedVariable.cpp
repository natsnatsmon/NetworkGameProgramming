#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <iostream>
#include <fstream>

#define SERVERPORT 9000
#define BUFSIZE 1024

void err_quit(const char * msg);
void err_display(const char * msg);
int recvn(SOCKET s, char * buf, int len, int flags);

int main(int argc, char * argv[]) {
	int retVal;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return 1; }

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) { err_quit("socket()"); }


	// connect()
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = ntohs(SERVERPORT);

	retVal = bind(listen_sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (retVal == SOCKET_ERROR) { err_quit("bind()"); }

	
	// listen()
	retVal = listen(listen_sock, SOMAXCONN);
	if (retVal == SOCKET_ERROR) { err_quit("listen()"); }


	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrLen;
	char buf[BUFSIZE + 1];
	int fileSize = 0;
	int fileNameLen = 0;
	int downLoadSize = 0;
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
			<< " ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl;

		// 1. ���� �̸� �ޱ�
		char fileName[256];
		ZeroMemory(fileName, 256);

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
		std::ofstream out(fileName, std::ios::trunc|std::ios::binary);
		if (!out) {
			std::cout << "���� ���� ����" << std::endl;
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


		while (downLoadSize == fileSize) {
			// ������ �ޱ�(���� ����)
			// fileSize < BUFSIZE
			if (fileSize < BUFSIZE) {
				retVal = recvn(client_sock, buf, fileSize, 0);
				if (retVal == SOCKET_ERROR) {
					err_display("recv()");
					break;
				}
				else if (retVal == 0) { break; }
				for (char c = 0; c < fileSize; ++c) {
					out << buf[c];
				}
				downLoadSize += fileSize;
			}

			else {
				retVal = recvn(client_sock, buf, BUFSIZE, 0);
				std::cout << buf << std::endl;
				if (retVal == SOCKET_ERROR) {
					err_display("recv()");
					break;
				}
				else if (retVal == 0) { break; }
				for (char c = 0; c < BUFSIZE - 1; ++c) {
					out << buf[c];
				}

				std::cout << fileSize << std::endl;

				fileSize -= BUFSIZE;
				downLoadSize += BUFSIZE;
			}

			std::cout << " ���� ������ ũ�� " << out.tellp() << std::endl;

			// ���� ���� ���൵ ���
			std::cout << "[TCP/" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "] " 
				<< "���� ���� ���൵ : " << ((double)downLoadSize / (double)fileSize) * 100 << std::endl;
		}


		// closesocket()
		closesocket(client_sock);
		std::cout << "[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = " << inet_ntoa(clientAddr.sin_addr)
			<< " ��Ʈ ��ȣ = " << ntohs(clientAddr.sin_port) << std::endl;
	}
	

	// closesocket()
	closesocket(listen_sock);

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