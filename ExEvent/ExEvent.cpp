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

int threadNum = 0;

DWORD WINAPI ReadThread(LPVOID arg) {
	int threadN = ++threadNum;
	DWORD retVal;

	while (1) {
		// ���� �Ϸ� ���
		retVal = WaitForSingleObject(hWriteEvent, INFINITE);
		if (retVal != WAIT_OBJECT_0) { break; }

		
		// ���� ������ ���
		std::cout << "Thread " << threadN << ": " << GetCurrentThreadId();
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
	hReadEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (hReadEvent == NULL) { return 1; }

	HANDLE hThread[3];
	hThread[0] = CreateThread(NULL, 0, WriteThread, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);
	hThread[2] = CreateThread(NULL, 0, ReadThread, NULL, 0, NULL);


	WaitForMultipleObjects(3, hThread, TRUE, INFINITE);

	CloseHandle(hWriteEvent);
	CloseHandle(hReadEvent);

	return 0;
}
