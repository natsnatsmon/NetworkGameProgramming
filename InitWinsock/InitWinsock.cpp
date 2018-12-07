#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <iostream>
#include <bitset>

using namespace std;

void err_quit(const char *msg);

int main(int argc, char *argv[]) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0) {
		return 1;
	}
//	MessageBox(NULL, "윈속 초기화 성공", "알림", MB_OK);

	// 연습문제 1
	// wsa 변수의 필드 중 처음 네 개를 화면에 출력하시오. 윈속 1.1 버전을 사용하도록 예제를 수정할 경우 이 값들이 어떻게 바뀌는가?

	//  MAKEWORD(majorVer, minorVer);
	// majorVer = Low byte, minorVer = High byte
	// DWORD_PTR = unsigned _int64
	int major_version = 0x01 & 0xff;
	int minor_version = 0x01 & 0xff;
	int version = major_version | (minor_version<<8);
	cout << "MAKEWORD(1,1)" << endl <<
		"= (" << bitset<16>(minor_version) << " << 8) | " << bitset<16>(major_version) << endl <<
		"= " << bitset<16>(minor_version << 8) << " | " << bitset<16>(major_version) << endl <<
		"= " << bitset<16>(version) << endl <<
		"= " << version << endl << endl;


	cout << "wsa.wVersion : " << "(int)((BYTE)(wsa.wVersion)).(wsa.wVersion >> 8)" << " = " << (int)((BYTE)(wsa.wVersion)) << "." << (wsa.wVersion >> 8) << endl;
	cout << "wsa.wHighVersion : " << "(int)((BYTE)wsa.wHighVersion).(wsa.wHighVersion >> 8)" << " = " << (int)((BYTE)(wsa.wHighVersion)) << "." << (wsa.wHighVersion>>8) << endl;
	cout << "wsa.szDescription : " << wsa.szDescription << endl;
	cout << "wsa.szSystemStatus : " << wsa.szSystemStatus << endl;
	//// socket()
	//SOCKET tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	//if (tcp_sock == INVALID_SOCKET) err_quit("socket()");
	//MessageBox(NULL, "TCP 소켓 생성 성공", "알림", MB_OK);

	//// closesocket()
	//closesocket(tcp_sock);

	int temp;
	cin >> temp;
	// 윈속 종료
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
