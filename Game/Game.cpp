#include "stdafx.h"
#include "Game.h"


bool Game::Start()
{
	// Load resources and set up your objects here (called once).
	m_modelRender.Init("Assets/modelData/unityChan.tkm", enModelUpAxisZ);
	m_modelRender.SetPosition(Vector3(-100.0f, 0.0f, 0.0f));
	Quaternion rot;
	rot.SetRotationDegY(180.0f);
	m_modelRender.SetRotation(rot);
	m_modelRender.SetScale(Vector3(1.5f, 1.5f, 1.5f));
	m_modelRender.Update();
	return true;
}


void Game::Update()
{
	// Per-frame logic goes here.
}


void Game::Render(RenderContext& rc)
{
	// Your drawing code goes here.
	// K2EngineLow already cleared the screen to gray before this is called.
	m_modelRender.Draw(rc);
}
