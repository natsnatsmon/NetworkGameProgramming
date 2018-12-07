#pragma once
#include "Renderer.h"
#include "Object.h"
#include "Global.h"

// ScnMgr : ��� ������, ������Ʈ�� �����Ѵ�!
class SceneManager
{
private:
	Object *m_Objects[MAX_OBJECTS];
	Renderer *m_Renderer;

	GLuint m_TestTexture = 0;
	GLuint m_ShadowTexture = 0;
	GLuint m_BulletTexture = 0;

public:
	SceneManager();
	~SceneManager();

	void RenderScene();

	void Update(float eTime);

	void InputKey(int key);

	void ApplyForce(float forceX, float forceY, float eTime);

	void Shoot(int shootID);
};
