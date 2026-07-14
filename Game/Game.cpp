#include "stdafx.h"
#include "Game.h"


bool Game::Start()
{
	// Load resources and set up your objects here (called once).
	m_modelInitData.m_tkmFilePath = "Assets/modelData/unityChan.tkm";
	m_modelInitData.m_fxFilePath = "Assets/shader/model.fx";

	m_model.Init(m_modelInitData);
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
	m_model.Draw(rc);
}
