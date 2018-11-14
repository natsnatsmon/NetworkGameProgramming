#include <Windows.h>
#include <iostream>

#define BUFSIZE 10

HANDLE hReadEvent;
HANDLE hWriteEvent;
int buf[BUFSIZE];

DWORD WINAPI WriteThread(LPVOID arg) {
	DWORD retVal;

	for(int k = 1; k <= 500; ++k) {
		// �б� �Ϸ� ���
		retVal = WaitForSingleObject(hReadEvent, INFINITE);
		if (retVal != WAIT_OBJECT_0) { break; }


		// ���� ���ۿ� ������ ����
		for (int i = 0; i < BUFSIZE; ++i) { buf[i] = k; }


		// ���� �Ϸ� �˸�
		SetEvent(hWriteEvent);
	}

	return 0;
}

DWORD WINAPI ReadThread(LPVOID arg) {
	DWORD retVal;

	while (1) {
		// ���� �Ϸ� ���
		retVal = WaitForSingleObject(hWriteEvent, INFINITE);
		if (retVal != WAIT_OBJECT_0) { break; }

		
		// ���� ������ ���
		std::cout << "Thread : " << GetCurrentThreadId();
		for (int i = 0; i < BUFSIZE; ++i) {
			std::cout << buf[i];
		}
		std::cout << "\n";


		// ���� �ʱ�ȭ
		ZeroMemory(buf, sizeof(buf));


		// �б� �Ϸ� �˸�
		SetEvent(hReadEvent);
	}

	return 0;
}

int main(int argc, char* argv[]) {

	// �ڵ� ���� �̺�Ʈ �� �� ���� (���� ���ȣ, ��ȣ ����)
	hWriteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hWriteEvent == NULL) { return 1; }
}