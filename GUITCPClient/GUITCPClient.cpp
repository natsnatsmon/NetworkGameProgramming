#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>
#include "resource.h"
#include <iostream>
#include <commctrl.h>


#define SERVERIP "127.0.0.1"
#define SERVERPORT 9000
#define BUFSIZE 512

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);

void err_quit(const char * msg);
void err_display(const char * msg);

int recvn(SOCKET s, char * buf, int len, int flags);

// 소켓 통신 스레드 함수
DWORD WINAPI ClientMain(LPVOID arg);

SOCKET sock;
char buf[BUFSIZE + 1];
HANDLE hReadEvent, hWriteEvent; // 이벤트
HWND hSendButton; // 보내기 버튼
HWND hEdit1, hEdit2;
HWND hProgress;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// 이벤트 생성
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) return 1;
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) return 1;


	// 소켓 통신 스레드 생성
	CreateThread(NULL, 0, ClientMain, NULL, 0, NULL);


	// 대화상자 생성
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);


	// 이벤트 제거
	CloseHandle(hReadEvent);
	CloseHandle(hWriteEvent);

	WSACleanup();
	return 0;
}

char fileName[256];

// 대화상자 프로시저
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
			EnableWindow(hSendButton, FALSE); // 보내기 버튼 비활성화
			WaitForSingleObject(hReadEvent, INFINITE); // 읽기 완료 기다리기
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1);
			SetEvent(hWriteEvent); // 쓰기 완료 기다리기
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

// 편집 컨트롤 출력 함수
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

	unsigned int fileSize = 0;
	unsigned int fileNameLen = 0;
	unsigned int downloadFileSize = 0;
	unsigned int recvFileSize = 0;

	// 서버와 데이터 통신
	while (1) {
		WaitForSingleObject(hWriteEvent, INFINITE); // 쓰기 완료 기다리기

		// 1. 문자열 길이가 0이면 보내지 않음
		if (strlen(fileName) == 0) {
			EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
			SetEvent(hReadEvent); // 읽기 완료 알리기
			continue;
		}

		// 2. 파일 생성
		FILE *fp = fopen(fileName, "wb");
		if (fp == NULL) {
			DisplayText((char*)"파일 생성 실패");
			exit(1);
		}

		// 3. 파일 이름 전송하기
		fileNameLen = strlen(fileName);
		
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
		
		DisplayText((char*)"[TCP 클라이언트] 파일 이름 %d byte와 고정크기 %d byte를 보냈습니다. \r\n", retVal, sizeof(int));
		
		// 3. 파일 저장하기
		// 클라이언트와 데이터 통신
		// 데이터 받기(고정 길이)
		retVal = recvn(sock, (char *)&fileSize, sizeof(int), 0);
		if (retVal == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retVal == 0) { break; }
		DisplayText((char*)"[TCP 클라이언트] 파일크기 : %d \r\n", fileSize);


		// 데이터 받기(가변 길이)
		while (1) {

			//DisplayText((char*)"%d", currentThreadNum);

			ZeroMemory(buf, BUFSIZE);

			retVal = recvn(sock, (char *)&recvFileSize, sizeof(int), 0);
			if (retVal == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retVal == 0) { break; }

			retVal = recvn(sock, buf, recvFileSize, 0);
			if (retVal == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retVal == 0) { break; }

			fwrite(buf, 1, retVal, fp);
			downloadFileSize += retVal;


			// 파일 전송 진행도 출력
			SendMessage(hProgress, PBM_SETPOS, ((double)downloadFileSize / (double)fileSize * 100.f), 0);
	/*		DisplayText((char*)"[TCP/%s:%d] 파일 이름 : %s \n\t 받은 파일 크기 : %d바이트, 파일 다운로드 진행 %f \n",
				inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), fileName, downloadFileSize, (((double)downloadFileSize / (double)fileSize) * 100));*/
		}

		DisplayText((char*)"[TCP 클라이언트] 파일 다운로드 완료! 받은 파일 크기 : %d byte\r\n", downloadFileSize);

		fclose(fp);

		closesocket(sock);

		//// 2. 파일 읽어오기
		//FILE *fp = fopen(fileName, "rb");
		//if (fp == NULL) {
		//	std::cout << "해당 파일이 존재하지 않습니다!" << std::endl;
		//	err_quit("send()");
		//}
		//// 4. 파일 크기얻기
		//unsigned int fileSize = 0;
		//fseek(fp, 0, SEEK_END);
		//fileSize = ftell(fp);
		//
		//fseek(fp, 0, SEEK_SET); // 파일 전송을 위해 파일 포인터를 맨 앞으로 이동
		//char fileBuf[BUFSIZE];
		//
		//// 5. 파일 크기를 len에 저장하여 전송
		//// 데이터 보내기 (고정 길이)
		//retVal = send(sock, (char*)&fileSize, sizeof(int), 0);
		//if (retVal == SOCKET_ERROR) {
		//	err_display("send()");
		//}
		//
		//DisplayText((char*)"[TCP 클라이언트] 파일 크기 %d byte를 담은 %d바이트를 보냈습니다.\r\n", fileSize, sizeof(int));
		// 6. 파일 전송
		// 데이터 보내기 (가변 길이)
		// 파일의 크기 < BUFSIZE -> 바로 전송
		// 파일의 크기 > BUFSIZE -> 나눠서 전송
		//unsigned int sendFileSize = 0;
		//unsigned int uploadFileSize = 0;
		//unsigned int readFileSize = 0;
		//unsigned int leftFileSize = 0;
		//leftFileSize = fileSize;
		//
		//while (1) {
		//	ZeroMemory(fileBuf, BUFSIZE);
		//
		//	// 보내야 할 파일 사이즈가 버퍼의 크기보다 크다면 잘라주기
		//	if (leftFileSize >= BUFSIZE) {
		//		sendFileSize = BUFSIZE - 1;
		//	}
		//	else {
		//		sendFileSize = leftFileSize;
		//	}
		//
		//	// 고정 길이
		//	retVal = send(sock, (char*)&sendFileSize, sizeof(int), 0);
		//	if (retVal == SOCKET_ERROR) {
		//		err_display("send()");
		//	}
		//
		//	// 가변 길이
		//	readFileSize = fread(fileBuf, 1, sendFileSize, fp);
		//	if (readFileSize > 0) {
		//		retVal = send(sock, fileBuf, readFileSize, 0);
		//		if (retVal == SOCKET_ERROR) {
		//			err_display("send()");
		//		}
		//		uploadFileSize += readFileSize;
		//		if (leftFileSize >= BUFSIZE) {
		//			leftFileSize -= readFileSize;
		//		}
		//
		//	}
		//	else if (readFileSize == 0 && uploadFileSize == fileSize) {
		//		DisplayText((char*)"파일 전송 성공!\r\n");
		//		break;
		//	}
		//	else {
		//		DisplayText((char*)"파일 입출력 오류!\r\n");
		//	}


		//	SendMessage(hProgress, PBM_SETPOS, ((double)uploadFileSize / (double)fileSize * 100.f), 0);
		//	//DisplayText((char*)"[TCP 클라이언트] 파일 업로드 진행 : %lf %, %d + %d Byte 보냄, 총 %d Byte 보냄 \r\n"
		//	//	, ((double)uploadFileSize / (double)fileSize) * 100, sizeof(int), retVal, uploadFileSize);
		//}
		//
		// EnableWindow(hSendButton, TRUE); // 보내기 버튼 활성화
		// SetEvent(hReadEvent);
		
	}

	return 0;
}