#include "stdafx.h"
#include "Game.h"


bool Game::Start()
{
	// Load resources and set up your objects here (called once).
	m_spriteRender.Init("Assets/modelData/building.dds", 128, 128);
	m_spriteRender.SetPosition(Vector3(100.0f, 0.0f, 0.0f));
	Quaternion rot;
	rot.SetRotationDegZ(45.0f);
	m_spriteRender.SetRotation(rot);
	m_spriteRender.SetScale(Vector3(2.0f, 2.0f, 1.0f));
	m_spriteRender.SetMulColor(g_vec4Black);
	m_spriteRender.Update();
	return true;
}


void Game::Update()
{
	// Per-frame logic goes here.

	auto trs = m_spriteRender.GetTransform();

	trs.m_position.x += 0.1;
	trs.m_rotation.AddRotationDegZ(0.001);
	trs.m_scale.x += 0.001;
	trs.m_scale.y += 0.001;

	m_spriteRender.SetTRS(trs);
	m_spriteRender.Update();
}


void Game::Render(RenderContext& rc)
{
	// Your drawing code goes here.
	// K2EngineLow already cleared the screen to gray before this is called.
	m_spriteRender.Draw(rc);
}
