#pragma comment(lib, "ws2_32")
#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <bitset>

using namespace std;


BOOL IsLittleEndian(u_short, u_short);
//BOOL IsLittleEndian(u_long, u_long);
//BOOL IsBigEndian(u_short, u_short);
BOOL IsBigEndian(u_long, u_long);



int main(int argc, char *argv[]) {

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	// 연습문제 1. 내 컴퓨터가 어떤 바이트 정렬 방식을 사용하는지 확실하게 입증할 것

	// hton의 원리 = 8비트 단위로 나누어 순서를 바꿔주는 것
	// short - 00011011 01011100       long - 01000011 11000110 11000010 00011010
	//				  \/                      -------- ======== ******** ^^^^^^^^      
	//				  /\                      vvvvvvvv ******** ======== --------
	//		   01011100 00011011		      00011010 11000010 11000110 01000011

	u_short s_data = 0x1234;
	u_short htons_data = htons(s_data);

	u_long l_data = 0x12853476;
	u_long htonl_data = htonl(l_data);


	bool isLittle = IsLittleEndian(s_data, htons_data);
	bool isBig = IsBigEndian(l_data, htonl_data);

	cout << "내 컴퓨터가 리틀엔디안인가요? " << '\t' << std::boolalpha << isLittle << endl;
	cout << "내 컴퓨터가 빅엔디안인가요? " << '\t' << isBig << endl;


	// 바로꺼짐방지용
	getchar();

	WSACleanup();
	return 0;
}


BOOL IsLittleEndian(u_short data, u_short hton_data) {
	cout << "\n\t\t\t\t\t - short - \n\n";
	cout << "\t\t\t     data : " << hex << data << "(hex) " << bitset<16>(data) << "(bin)" << endl;
	cout << "\t\t\t\t    --==      --------========" << endl;
	cout << "host to network data (Big Endian) : " << hex << hton_data << "(hex) " << bitset<16>(htons(data)) << "(bin)" << endl << endl;

	if (hton_data != data)
		return TRUE;
	else
		return FALSE;
}

BOOL IsBigEndian(u_long data, u_long hton_data) {
	cout << "\n\t\t\t\t\t - long - \n\n";
	cout << "\t\t\t     data : " << hex << data << "(hex) " << bitset<32>(data) << "(bin)" << endl;
	cout << "\t\t\t\t    --==**^^      --------========********^^^^^^^^" << endl;
	cout << "host to network data (Big Endian) : " << hex << hton_data << "(hex) " << bitset<32>(hton_data) << "(bin)" << endl << endl;

	if (hton_data == data)
		return TRUE;
	else
		return FALSE;
}