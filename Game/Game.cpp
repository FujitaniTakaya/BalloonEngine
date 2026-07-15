#include "stdafx.h"
#include "Game.h"

#include "balloonEngineLow/Light.h"


bool Game::Start()
{
	// Load resources and set up your objects here (called once).


	m_modelRender.Init("Assets/modelData/unityChan.tkm", enModelUpAxisZ);
	Quaternion rot;
	rot.SetRotationDegY(180.0f);
	m_modelRender.SetRotation(rot);

	const Vector3 modelPos = m_modelRender.GetTransform().m_position;
	const Vector3 lightPos = modelPos + Vector3(100.0f, 0.0f, 0.0f);

	auto& light = balloonEngineLow::LightManager::Get();
	light.SetLightColor(g_vec4Yellow);
	light.SetLightDir(modelPos - lightPos);

	return true;
}


void Game::Update()
{
	// Per-frame logic goes here.

	Quaternion rot = m_modelRender.GetTransform().m_rotation;

	rot.AddRotationDegY(0.1);
	m_modelRender.SetRotation(rot);
	m_modelRender.Update();
}


void Game::Render(RenderContext& rc)
{
	// Your drawing code goes here.
	// K2EngineLow already cleared the screen to gray before this is called.
	m_modelRender.Draw(rc);
}
