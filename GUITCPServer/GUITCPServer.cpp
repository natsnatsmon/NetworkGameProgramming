#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment (lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <iostream>

#define SERVERPORT 9000
#define BUFSIZE 512

// ������ ���ν���
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...);
// ���� ��� �Լ�
void err_quit(const char *msg);
void err_display(const char *msg);

int recvn(SOCKET s, char * buf, int len, int flags);

// ���� ��� ������ �Լ�
DWORD WINAPI ServerMain(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);

HINSTANCE hInst;
HWND hEdit;
CRITICAL_SECTION cs;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	hInst = hInstance;
	InitializeCriticalSection(&cs);

	// ������ Ŭ���� ���
	WNDCLASS wndClass;
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "MyWndClass";

	if (!RegisterClass(&wndClass)) return 1;


	// ������ ����
	HWND hWnd = CreateWindow("MyWndClass", "WinApp", WS_OVERLAPPEDWINDOW, 100, 100, 600, 400, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// ���� ��� ������ ����
	CreateThread(NULL, 0, ServerMain, NULL, 0, NULL);

	// �޼��� ����
	MSG msg;
	while (GetMessage(&msg, 0, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DeleteCriticalSection(&cs);

	return (int)msg.wParam;
}


// ������ ���ν���
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
			ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_READONLY,
			0, 0, 0, 0, hWnd, (HMENU)100, hInst, NULL);
		//DisplayText("������ GUI ���� ���α׷��Դϴ�. \r\n");
		//DisplayText("�ν��Ͻ� �ڵ� ���� %#x�Դϴ�. \r\n", hInst);
		return 0;

	case WM_SIZE:
		MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;

	case WM_SETFOCUS:
		SetFocus(hEdit);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE+256];
	vsprintf(cbuf, fmt, arg);

	EnterCriticalSection(&cs);
	int nLength = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
	LeaveCriticalSection(&cs);

	va_end(arg);
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

	DisplayText((char*) "[%s] \r\n", (char*)lpMsgBuf);
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

// TCP ���� ���ۺκ�
DWORD WINAPI ServerMain(LPVOID arg) {
	int retVal;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVERPORT);
	retVal = bind(listen_sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (retVal == SOCKET_ERROR) err_quit("bind()");

	retVal = listen(listen_sock, SOMAXCONN);
	if (retVal == SOCKET_ERROR) err_quit("listen()");

	SOCKET client_sock;
	SOCKADDR_IN clientAddr;
	int addrLen;
	HANDLE hThread;

	while (1) {
		addrLen = sizeof(clientAddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientAddr, &addrLen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}


		// ������ Ŭ���̾�Ʈ ���� ���
		DisplayText((char*)"\r\n[TCP ����] Ŭ���̾�Ʈ ���� : IP�ּ� = %s, ��Ʈ ��ȣ = %d \r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));


		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }
	}

	closesocket(listen_sock);

	WSACleanup();
	return 0;
}

short threadNum = 0;
DWORD WINAPI ProcessClient(LPVOID arg) {
	SOCKET client_sock = (SOCKET)arg;
	const short currentThreadNum = threadNum++;
	int retVal;

	SOCKADDR_IN clientAddr;
	int addrLen;
	char buf[BUFSIZE + 1];


	// Ŭ���̾�Ʈ ���� ���
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
		DisplayText((char*)"[TCP/%s:%d] %s ���� ��û�޾ҽ��ϴ�. \r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buf);

		strcpy(fileName, buf);

		// 2. ���� �о����
		FILE *fp = fopen(fileName, "rb");

		if (fp == NULL) {
			std::cout << "�ش� ������ �������� �ʽ��ϴ�!" << std::endl;
			err_quit("send()");
		}

		// 3. ���� ũ����
		unsigned int fileSize = 0;
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);

		fseek(fp, 0, SEEK_SET); // ���� ������ ���� ���� �����͸� �� ������ �̵�
		char fileBuf[BUFSIZE];

		DisplayText((char*)"���� ũ�� : %d\r\n", fileSize);
		// 4. ���� ũ�⸦ len�� �����Ͽ� ����
		// ������ ������ (���� ����)
		retVal = send(client_sock, (char*)&fileSize, sizeof(int), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("send()");
		}

		DisplayText((char*)"[TCP/%s:%d] ���� ũ�� %d byte�� ���� %d����Ʈ�� ���½��ϴ�. \r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fileSize, sizeof(int));


		// 5. ���� ����
		// ������ ������ (���� ����)
		// ������ ũ�� < BUFSIZE -> �ٷ� ����
		// ������ ũ�� > BUFSIZE -> ������ ����

		unsigned int sendFileSize = 0;
		unsigned int uploadFileSize = 0;
		unsigned int readFileSize = 0;
		unsigned int leftFileSize = 0;
		leftFileSize = fileSize;


		while (1) {
			ZeroMemory(fileBuf, BUFSIZE);

			// ������ �� ���� ����� ������ ũ�⺸�� ũ�ٸ� �߶��ֱ�
			if (leftFileSize >= BUFSIZE) {
				sendFileSize = BUFSIZE - 1;
			}
			else {
				sendFileSize = leftFileSize;
			}

			// ���� ����
			retVal = send(client_sock, (char*)&sendFileSize, sizeof(int), 0);
			if (retVal == SOCKET_ERROR) {
				err_display("send()");
			}

			// ���� ����
			readFileSize = fread(fileBuf, 1, sendFileSize, fp);
			if (readFileSize > 0) {
				retVal = send(client_sock, fileBuf, readFileSize, 0);
				if (retVal == SOCKET_ERROR) {
					err_display("send()");
				}
				uploadFileSize += readFileSize;

			}
			else if (readFileSize == 0 && uploadFileSize == fileSize) {
				DisplayText((char*)"���� ���� ����!\r\n");
				break;
			}
			else {
				DisplayText((char*)"���� ����� ����!\r\n");
			}
		}

		// ������ �޴� ���
		//// 2. ���� ����
		//FILE *fp = fopen(fileName, "wb");
		//if (fp == NULL) {
		//	DisplayText((char*)"���� ���� ����");
		//	exit(1);
		//}
		//DisplayText((char*)"[TCP Ŭ���̾�Ʈ] \r\n", fileSize, sizeof(int));
		//// 3. ���� �����ϱ�
		//// Ŭ���̾�Ʈ�� ������ ���
		//// ������ �ޱ�(���� ����)
		//retVal = recvn(client_sock, (char *)&fileSize, sizeof(int), 0);
		//if (retVal == SOCKET_ERROR) {
		//	err_display("recv()");
		//	break;
		//}
		//else if (retVal == 0) { break; }
		//DisplayText((char*)"����ũ�� : %d \r\n", fileSize);
		//// ������ �ޱ�(���� ����)
		//while (1) {
		//	//DisplayText((char*)"%d", currentThreadNum);
		//	ZeroMemory(buf, BUFSIZE);
		//	retVal = recvn(client_sock, (char *)&recvFileSize, sizeof(int), 0);
		//	if (retVal == SOCKET_ERROR) {
		//		err_display("recv()");
		//		break;
		//	}
		//	else if (retVal == 0) { break; }
		//	retVal = recvn(client_sock, buf, recvFileSize, 0);
		//	if (retVal == SOCKET_ERROR) {
		//		err_display("recv()");
		//		break;
		//	}
		//	else if (retVal == 0) { break; }
		//	fwrite(buf, 1, retVal, fp);
		//	downloadFileSize += retVal;
		//	// ���� ���� ���൵ ���
		//	DisplayText((char*)"[TCP/%s:%d] ���� �̸� : %s \n\t ���� ���� ũ�� : %d����Ʈ, ���� �ٿ�ε� ���� %f \n",
		//		inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fileName, downloadFileSize, (((double)downloadFileSize / (double)fileSize) * 100));
		//}
		//DisplayText((char*)"[TCP/%s:%d] %s �ٿ�ε� �Ϸ�! ���� ���� ũ�� : %ld \r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fileName, ftell(fp));
		//fclose(fp);

	}

	// closesocket()
	//closesocket(client_sock);
	//DisplayText((char*)"[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = %s, ��Ʈ ��ȣ = %d \r\n",
	//	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));


	return 0;
}
