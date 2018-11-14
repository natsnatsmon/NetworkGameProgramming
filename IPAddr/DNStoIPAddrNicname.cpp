#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

// �������� 3 : ������ �̸��� ����� ���ڷ� �Է¹޾�,
// �ش� ȣ��Ʈ�� **��� ����**�� **IP �ּ�**�� ����ϴ� ���α׷��� �ۼ��Ͻÿ�.
using namespace std;

void err_display(const char *msg);
BOOL GetIPAddrAliases(char *name);

int main(int argc, char *argv[]) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { return 1; }

	char *domainName{};
	if (argc <= 1) {
		cout << "������ �̸��� �Է����ּ���." << endl;
		return 0;
	}
	else if (argc >= 3) {
		cout << "�ϳ��� ������ �̸��� �Է����ּ���." << endl;
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

	// ���ٸ� err_diplay ȣ�� �� ����
	if (ptr == NULL) {
		err_display("gethostbyname()");
		return FALSE;
	}

	// IPv4�� �ƴ� ��쿡�� ����
	if (ptr->h_addrtype != AF_INET) {
		return FALSE;
	}



	cout << endl << "** ��û�Ͻ� ������ ������ ��� �����Դϴ�. **" << endl << endl;

	for (int index = 0; ptr->h_aliases[index] != NULL; ++index) {
		cout << index + 1 << ". " << ptr->h_aliases[index] << endl;
	}
	
	IN_ADDR addr;
	cout << endl << endl << "** ��û�Ͻ� ������ ������ ��� IP�ּ��Դϴ�. **" << endl;
	
	for (int index = 0; ptr->h_addr_list[index] != NULL; ++index) {
		addr.S_un.S_addr = *(ULONG *)(ptr->h_addr_list[index]);
		cout << index + 1 << ". " << inet_ntoa(addr) << endl;
	}

	
	
	return TRUE;
}