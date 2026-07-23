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

        m_animationClips[0].Load("Assets/animData/idle.tka");
        m_animationClips[0].SetLoopFlag(true);
        m_animationClips[1].Load("Assets/animData/run.tka");
        m_animationClips[1].SetLoopFlag(true);


        m_modelRender.Init(
            "Assets/modelData/unityChan.tkm",
            m_animationClips,
            2,
            EnModelUpAxis::enModelUpAxisY,
            true,
            true
        );
        Quaternion rot;
        rot.SetRotationDegY(180.0f);
        m_modelRender.SetRotation(rot);

        m_modelRender.PlayAnimation(1);

        const Vector3 modelPos = m_modelRender.GetTransform().m_position;


        m_bgModelRender.Init("Assets/modelData/ground.tkm", nullptr, 0, EnModelUpAxis::enModelUpAxisZ, true, false);
        m_bgModelRender.Update();

        m_gameCamera = std::make_unique<camera::GameCamera>();
        m_gameCamera->SetTargetPosition(modelPos);
        m_gameCamera->Start();

        SceneLight::Get().SetAmbientColor({ 0.5f, 0.5f, 0.5f, 1.0f });

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
        m_bgModelRender.Draw(rc);
    }
} // namespace app