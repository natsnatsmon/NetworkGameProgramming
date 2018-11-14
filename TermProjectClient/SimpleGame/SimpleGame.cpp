/*
Copyright 2017 Lee Taek Hee (Korea Polytech University)

This program is free software: you can redistribute it and/or modify
it under the terms of the What The Hell License. Do it plz.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.
*/

#include "stdafx.h"
#include <iostream>
#include <Windows.h>

// 헤더파일은 64/32비트에 상관없으나 lib(라이브러리)파일은 꼭 확인해주도록하쟈
#include "Dependencies\glew.h"
#include "Dependencies\freeglut.h"
#include "SceneManager.h"
#include "Global.h"
using namespace std;

SceneManager *g_scgMgr = NULL;

DWORD g_prevTime = 0;
float rect_x = 0.f, rect_y = 0.f;
float moveDistance = 1.f;

BOOL g_keyW = false;
BOOL g_keyA = false;
BOOL g_keyS = false;
BOOL g_keyD = false;

int g_shoot = SHOOT_NONE;

void RenderScene(void)
{
	// Calc Elapsed Time
	if (g_prevTime == 0) {
		g_prevTime = timeGetTime();
		return;
	}
	DWORD currTime = timeGetTime(); // curr time in milli
	DWORD elapsedTime = currTime - g_prevTime;
	g_prevTime = currTime;
	float eTime = (float)elapsedTime / 1000.f;

	//cout << "elapsedTime : " << eTime << endl;
	cout << "W :" << g_keyW << ", A :" << g_keyA << ", S :" << g_keyS << ", D :" << g_keyD << endl;

	float forceX = 0.f;
	float forceY = 0.f;
	if (g_keyA) { forceX = -1.f; }
	else if (g_keyW) { forceY = 1.f; }
	else if (g_keyS) { forceY = -1.f; }
	else if (g_keyD) { forceX = 1.f; }
	g_scgMgr->ApplyForce(forceX, forceY, eTime);

	// update
	g_scgMgr->Update(eTime);
	g_scgMgr->RenderScene();
	g_scgMgr->Shoot(g_shoot);
	glutSwapBuffers();
}

void Idle(void)
{
	RenderScene();
}

void MouseInput(int button, int state, int x, int y)
{
	RenderScene();
}

// 키보드가 press되었을 때 발생하는 이벤트
void KeyDownInput(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a' :
		g_keyA = TRUE;
		break;
	case 's':
		g_keyS = TRUE;
		break;
	case 'd':
		g_keyD = TRUE;
		break;
	case 'w' :
		g_keyW = TRUE;
		break;
	}
}

// 키보드가 떨어졌을 때 발생하는 이벤트
void KeyUpInput(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		g_keyA = FALSE;
		break;
	case 's':
		g_keyS = FALSE;
		break;
	case 'd':
		g_keyD = FALSE;
		break;
	case 'w':
		g_keyW = FALSE;
		break;
	}
}

void SpecialKeyUpInput(int key, int x, int y) {
	g_shoot = SHOOT_NONE;
}

void SpecialKeyDownInput(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		g_shoot = SHOOT_UP;
		break;
	case GLUT_KEY_DOWN:
		g_shoot = SHOOT_DOWN;
		break;
	case GLUT_KEY_LEFT:
		g_shoot = SHOOT_LEFT;
		break;
	case GLUT_KEY_RIGHT:
		g_shoot = SHOOT_RIGHT;
		break;
	}
}

int main(int argc, char **argv)
{
	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(750, 1000);
	glutCreateWindow("Network Game Programming Term Project");
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	glewInit();
	if (glewIsSupported("GL_VERSION_3_0"))
	{
		std::cout << " GLEW Version is 3.0\n ";
	}
	else
	{
		std::cout << "GLEW 3.0 not supported\n ";
	}


	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyDownInput);
	glutKeyboardUpFunc(KeyUpInput);
	glutMouseFunc(MouseInput);
	glutSpecialFunc(SpecialKeyDownInput);
	glutSpecialFunc(SpecialKeyUpInput);

	// init SceneManager
	g_scgMgr = new SceneManager;

	glutMainLoop();


    return 0;
}

