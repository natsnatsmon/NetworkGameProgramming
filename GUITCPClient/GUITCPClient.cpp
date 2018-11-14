#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#include <iostream>


#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...);

void err_quit(const char * msg);
void err_display(const char * msg);

int recvn(SOCKET s, char * buf, int len, int flags);

// ���� ��� ������ �Լ�
DWORD WINAPI ClientMain(LPVOID arg);

SOCKET sock;
char buf[BUFSIZE + 1];
HANDLE hReadEvent, hWriteEvent; // �̺�Ʈ
HWND hSendButton; // ������ ��ư
HWND hEdit1, hEdit2;
HWND hProgress;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// �̺�Ʈ ����
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return 1;


	// ���� ��� ������ ����
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);


	// ��ȭ���� ����
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);


	// �̺�Ʈ ����
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);

	closesocket(sock);
	WSACleanup();
	return 0;
}

char fileName[256];

// ��ȭ���� ���ν���
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static char buf[BUFSIZE + 1];

	switch (uMsg) {
	case WM_INITDIALOG:
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1);
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2);
		hProgress = GetDlgItem(hDlg, IDC_PROGRESS1);
		hSendButton = GetDlgItem(hDlg, IDOK);
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EnableWindow(hSendButton, FALSE); // ������ ��ư ��Ȱ��ȭ
			WaitForSingleObject(hReadEvent, INFINITE); // �б� �Ϸ� ��ٸ���
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent); // ���� �Ϸ� ��ٸ���
			SetFocus(hEdit1);
			ZeroMemory(fileName, 256);
			strcpy(fileName, buf);
//			DisplayText((char*)"%s \r\n", buf);
//			DisplayText((char*)"%s \r\n", fileName);
			SendMessage(hEdit1, EM_SETSEL, 0, -1);
			return TRUE;

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// ���� ��Ʈ�� ��� �Լ�
void DisplayText(char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE + 256];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(hEdit2);
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength);
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
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

DWORD WINAPI ClientMain(LPVOID arg) {
	

	int retVal;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return 1; }

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) { err_quit("socket()"); }

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = ntohs(SERVERPORT);

	retVal = connect(sock, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
	if (retVal == SOCKET_ERROR) { err_quit("connect()"); }

	// ������ ������ ���
	while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE); // ���� �Ϸ� ��ٸ���

		DisplayText((char*)"���� �̸� : %s\r\n", fileName);


		// ���ڿ� ���̰� 0�̸� ������ ����
		if (strlen(fileName) == 0) {
			EnableWindow(hSendButton, TRUE); // ������ ��ư Ȱ��ȭ
			SetEvent(hReadEvent); // �б� �Ϸ� �˸���
			continue;
		}

		//// ������ ������
		//retVal = send(sock, buf, strlen(buf), 0);
		//if (retVal == SOCKET_ERROR) {
		//	err_display("send()");
		//	break;
		//}
		//DisplayText((char*)"[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\r\n", retVal);


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
		
		DisplayText((char*)"[TCP Ŭ���̾�Ʈ] ���� �̸� (%d +) %d ����Ʈ�� ���½��ϴ�. \r\n", sizeof(int), retVal);
		
		// 4. ���� ũ����
		unsigned int fileSize = 0;
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		
		fseek(fp, 0, SEEK_SET); // ���� ������ ���� ���� �����͸� �� ������ �̵�
		char fileBuf[BUFSIZE];
		
		// 5. ���� ũ�⸦ len�� �����Ͽ� ����
		// ������ ������ (���� ����)
		retVal = send(sock, (char*)&fileSize, sizeof(int), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("send()");
		}
		
		DisplayText((char*)"[TCP Ŭ���̾�Ʈ] ���� ũ�� %dByte�� ���� %d����Ʈ�� ���½��ϴ�.\r\n", fileSize, sizeof(int));

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
			ZeroMemory(fileBuf, BUFSIZE);
		
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
			readFileSize = fread(fileBuf, 1, sendFileSize, fp);
			if (readFileSize > 0) {
				retVal = send(sock, fileBuf, readFileSize, 0);
				if (retVal == SOCKET_ERROR) {
					err_display("send()");
				}
				uploadFileSize += readFileSize;
				if (leftFileSize >= BUFSIZE) {
					leftFileSize -= readFileSize;
				}
		
			}
			else if (readFileSize == 0 && uploadFileSize == fileSize) {
				DisplayText((char*)"���� ���� ����!\r\n");
				break;
			}
			else {
				DisplayText((char*)"���� ����� ����!\r\n");
			}
			//DisplayText((char*)"[TCP Ŭ���̾�Ʈ] ���� ���ε� ���� : %lf %, %d + %d Byte ����, �� %d Byte ���� \r\n"
			//	, ((double)uploadFileSize / (double)fileSize) * 100, sizeof(int), retVal, uploadFileSize);
		}
		
		DisplayText((char*)"[TCP Ŭ���̾�Ʈ] ���� ���ε� �Ϸ�! ���� ���� ũ�� : %d Byte\r\n", uploadFileSize);
		
		fclose(fp);

		
	}

	return 0;
}