#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

// 연습문제 3 : 도메인 이름을 명령행 인자로 입력받아,
// 해당 호스트의 **모든 별명**과 **IP 주소**를 출력하는 프로그램을 작성하시오.
using namespace std;

void err_display(const char *msg);
BOOL GetIPAddrAliases(char *name);

int main(int argc, char *argv[]) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return 1; }

	char *domainName{};
	if (argc <= 1) {
		cout << "도메인 이름을 입력해주세요." << endl;
		return 0;
	}
	else if (argc >= 3) {
		cout << "하나의 도메인 이름만 입력해주세요." << endl;
		return 0;
	}
	else {
		domainName = argv[1];
	}

	GetIPAddrAliases(domainName);
	
	getchar();

	return 0;
}


void err_display(const char *msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL
	);

	cout << msg << (char *)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

BOOL GetIPAddrAliases(char *name) {
	HOSTENT *ptr = gethostbyname(name);

	// 없다면 err_diplay 호출 후 리턴
	if (ptr == NULL) {
		err_display("gethostbyname()");
		return FALSE;
	}

	// IPv4가 아닐 경우에도 리턴
	if (ptr->h_addrtype != AF_INET) {
		return FALSE;
	}



	cout << endl << "** 요청하신 도메인 네임의 모든 별명입니다. **" << endl << endl;

	for (int index = 0; ptr->h_aliases[index] != NULL; ++index) {
		cout << index + 1 << ". " << ptr->h_aliases[index] << endl;
	}
	
	IN_ADDR addr;
	cout << endl << endl << "** 요청하신 도메인 네임의 모든 IP주소입니다. **" << endl;
	
	for (int index = 0; ptr->h_addr_list[index] != NULL; ++index) {
		addr.S_un.S_addr = *(ULONG *)(ptr->h_addr_list[index]);
		cout << index + 1 << ". " << inet_ntoa(addr) << endl;
	}

	
	
	return TRUE;
}