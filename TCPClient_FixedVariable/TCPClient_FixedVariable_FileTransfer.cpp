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



	// 1. 파일 이름 입력받기
	char fileName[256];
	ZeroMemory(fileName, 256);
	std::cout << "확장자를 포함한 파일 이름을 입력해주세요. > ";
	std::cin >> fileName;

	// 2. 파일 읽어오기
	std::ifstream in(fileName, std::ios::ate|std::ios::binary); // 파일을 오픈하면서 파일 포인터를 끝부분으로 옮김

	if (!in) {
		std::cout << "해당 파일이 존재하지 않습니다!" << std::endl;
		err_quit("send()");
	}

	// 3. 파일 이름 전송하기 (중복된 파일이 있으면 덮어쓰게 하기 위함)
	int fileNameLen = strlen(fileName);

	// 데이터 보내기 (고정 길이)
	retVal = send(sock, (char*)&fileNameLen, sizeof(int), 0);
	if (retVal == SOCKET_ERROR) {
		err_display("send()");
	}

	// 데이터 보내기 (가변 길이)
	retVal = send(sock, fileName, fileNameLen, 0);
	if (retVal == SOCKET_ERROR) {
		err_display("send()");
	}

	std::cout << "[TCP 클라이언트] " << sizeof(int) << " + " << retVal << "(파일 이름의 크기)" << "바이트를 보냈습니다. \n";


	// 4. 파일 크기얻기
	int fileSize = 0;
	fileSize = in.tellg();

	in.seekg(0, std::ios::beg); // 파일 전송을 위해 파일 포인터를 맨 앞으로 이동
	char buf[BUFSIZE];


	// 5. 파일 크기를 len에 저장하여 전송
	// 데이터 보내기 (고정 길이)
	retVal = send(sock, (char*)&fileSize, sizeof(int), 0);
	if (retVal == SOCKET_ERROR) {
		err_display("send()");
	}

	std::cout << "[TCP 클라이언트] " << "파일 크기 " << fileSize << "를 담은 " << sizeof(int) << "바이트를 보냈습니다. \n";
	
	// 6. 파일 전송
	// 데이터 보내기 (가변 길이)
	// 파일의 크기 < BUFSIZE -> 바로 전송
	// 파일의 크기 > BUFSIZE -> 나눠서 전송
	while (1) {
		if (fileSize <= BUFSIZE) {
			in.read(buf, fileSize);
			retVal = send(sock, buf, fileSize, 0);
			if (retVal == SOCKET_ERROR) {
				err_display("send()");
			}
			std::cout << "[TCP 클라이언트] " << retVal << "바이트를 보냈습니다. \n";
			std::cout << "파일 전송 성공!" << std::endl;
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


		std::cout << "[TCP 클라이언트] " << retVal << "바이트를 보냈습니다. \n";
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
