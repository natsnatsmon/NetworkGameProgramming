#include "stdafx.h"
#include "Object.h"
#include <math.h>
#include <float.h>

Object::Object()
{
}


Object::~Object()
{
}

void Object::Update(float eTime) {
	// 벡터의 정규화..... 벡터의 크기를 1로 만들어주는 과정이다..
	// 방향은 유지하되 크기를 1로 바꾸는 것뿐
	float magVel = sqrtf(m_velX*m_velX + m_velY * m_velY);

	float velX = m_velX / magVel;
	float velY = m_velY / magVel;

	float fricX = -velX;
	float fricY = -velY;
	
	float friction = m_coefFric * m_mass * 9.8;
	fricX *= friction;
	fricY *= friction;

	// float point는 절대 == 연산하면 안된다.... 범위 연산 해야한다...
	// FLT_EPSILON 보다 작을경우는 거의 0이라 보면 됨
	if (magVel < FLT_EPSILON) {
		m_velX = 0.f;
		m_velY = 0.f;
	}
	else {
		float accX = fricX / m_mass;
		float accY = fricY / m_mass;

		float afterVelX = m_velX + accX * eTime;
		float afterVelY = m_velY + accY * eTime;

		if (afterVelX * m_velX < 0.f) { m_velX = 0.f; }
		else { m_velX = afterVelX; }

		if (afterVelY * m_velY < 0.f) { m_velY = 0.f; }
		else { m_velY = afterVelY; }
	}

	// cal velocity
	m_velX = m_velX + m_accX * eTime;
	m_velY = m_velY + m_accY * eTime;

	// calc position
	m_posX = m_posX + m_velX * eTime;
	m_posY = m_posY + m_velY * eTime;
}

void Object::GetKind(int * kind) {
	*kind = m_kind;
}
void Object::SetKind(int kind) {
	m_kind = kind;
}


void Object::GetMass(float * m) {
	*m = m_mass;
}
void Object::SetMass(float m) {
	m_mass = m;
}

void Object::GetCoefFric(float * m) {
	*m = m_coefFric;
}
void Object::SetCoefFric(float m) {
	m_coefFric = m;
}

void Object::GetPosition(float * x, float * y, float* z) { 
	*x = m_posX;
	*y = m_posY;
	*z = m_posZ;
}
void Object::SetPosition(float x, float y, float z) { 
	m_posX = x;
	m_posY = y;
	m_posZ = z;
}

void Object::GetVel(float * x, float * y) {
	*x = m_velX;
	*y = m_velY;
}
void Object::SetVel(float x, float y) {
	m_velX = x;
	m_velY = y;
}

void Object::GetAcc(float * x, float * y) {
	*x = m_accX;
	*y = m_accY;
}
void Object::SetAcc(float x, float y) {
	m_accX = x;
	m_accY = y;
}



void Object::GetSize(float * sizeX, float * sizeY) {
	*sizeX = m_sizeX;
	*sizeY = m_sizeY;
}
void Object::SetSize(float sizeX, float sizeY) {
	m_sizeX = sizeX;
	m_sizeY = sizeY;
}

void Object::GetColor(float * R, float * G, float * B, float * A) { 
	*R = m_R;
	*G = m_G;
	*B = m_B;
	*A = m_A;
}
void Object::SetColor(float R, float G, float B, float A) {
	m_R = R;
	m_G = G;
	m_B = B;
	m_A = A;
}

void Object::ApplyForce(float forceX, float forceY, float eTime) {
	m_accX = forceX / m_mass;
	m_accY = forceY / m_mass;

	m_velX = m_velX + m_accX * eTime;
	m_velY = m_velY + m_accY * eTime;

	m_accX = 0.f;
	m_accY = 0.f;
}