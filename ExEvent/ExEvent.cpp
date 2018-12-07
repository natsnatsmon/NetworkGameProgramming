#include <Windows.h>
#include <iostream>

#define BUFSIZE 10

HANDLE hReadEvent;
HANDLE hWriteEvent;
int buf[BUFSIZE];

DWORD WINAPI WriteThread(LPVOID arg) {
	DWORD retVal;

	for(int k = 1; k <= 500; ++k) {
		// 읽기 완료 대기
		retVal = WaitForSingleObject(hReadEvent, INFINITE);
		if (retVal != WAIT_OBJECT_0) { break; }


		// 공유 버퍼에 데이터 저장
		for (int i = 0; i < BUFSIZE; ++i) { buf[i] = k; }


		// 쓰기 완료 알림
		SetEvent(hWriteEvent);
	}

	return 0;
}

int threadNum = 0;

DWORD WINAPI ReadThread(LPVOID arg) {
	int threadN = ++threadNum;
	DWORD retVal;

	while (1) {
		// 쓰기 완료 대기
		retVal = WaitForSingleObject(hWriteEvent, INFINITE);
		if (retVal != WAIT_OBJECT_0) { break; }

		
		// 읽은 데이터 출력
		std::cout << "Thread " << threadN << ": " << GetCurrentThreadId();
		for (int i = 0; i < BUFSIZE; ++i) {
			std::cout << buf[i];
		}
		std::cout << "\n";


		// 버퍼 초기화
		ZeroMemory(buf, sizeof(buf));


		// 읽기 완료 알림
		SetEvent(hReadEvent);
	}

	return 0;
}

int main(int argc, char* argv[]) {

	// 자동 리셋 이벤트 두 개 생성 (각각 비신호, 신호 상태)
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
