#include "stdafx.h"

#include "Game.h"

#include "Camera/GameCamera.h"


namespace app
{
    Game::Game()
    {}


    Game::~Game()
    {}


    bool Game::Start()
    {
        // Load resources and set up your objects here (called once).


        m_modelRender.Init("Assets/modelData/unityChan.tkm", true, true);
        Quaternion rot;
        rot.SetRotationDegY(180.0f);
        m_modelRender.SetRotation(rot);

        const Vector3 modelPos = m_modelRender.GetTransform().m_position;

        m_modelRender2.Init("Assets/modelData/unityChan.tkm", true, true);
        m_modelRender2.SetRotation(rot);
        m_modelRender2.SetPosition(modelPos + Vector3(50.0f, 0.0f, 0.0f));
        m_modelRender2.Update();

        m_bgModelRender.Init("Assets/modelData/ground.tkm", true, false);
        m_bgModelRender.Update();

        m_gameCamera = std::make_unique<camera::GameCamera>();
        m_gameCamera->SetTargetPosition(modelPos);
        m_gameCamera->Start();

        return true;
    }


    void Game::Update()
    {
        // Per-frame logic goes here.

        Quaternion rot = m_modelRender.GetTransform().m_rotation;

        rot.AddRotationDegY(0.1);
        m_modelRender.SetRotation(rot);
        m_modelRender.Update();

        m_gameCamera->SetTargetPosition(m_modelRender.GetTransform().m_position);
        m_gameCamera->Update();
    }


    void Game::Render(RenderContext& rc)
    {
        // Your drawing code goes here.
        // K2EngineLow already cleared the screen to gray before this is called.

        m_modelRender.Draw(rc);
        m_modelRender2.Draw(rc);
        m_bgModelRender.Draw(rc);
    }
} // namespace app