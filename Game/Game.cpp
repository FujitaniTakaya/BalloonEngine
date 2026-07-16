#include "stdafx.h"

#include "Game.h"

#include "balloonEngineLow/Light.h"


bool Game::Start()
{
    // Load resources and set up your objects here (called once).


    m_modelRender.Init("Assets/modelData/unityChan.tkm", enModelUpAxisZ, true);
    Quaternion rot;
    rot.SetRotationDegY(180.0f);
    m_modelRender.SetRotation(rot);

    const Vector3 modelPos = m_modelRender.GetTransform().m_position;
    const Vector3 lightPos = modelPos + Vector3(100.0f, 100.0f, -100.0f);

    auto& light = balloonEngineLow::LightManager::Get();
    light.Update();

    // light.SetLightColor(g_vec4Yellow);
    Vector3 lightVec = (modelPos - lightPos);
    lightVec.Normalize();
    light.SetLightDir(lightVec);

    m_rts[static_cast<size_t>(EnRenderTargetType::Albedo)].Create(
        FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT
    );

    m_rts[static_cast<size_t>(EnRenderTargetType::Normal)].Create(
        FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN
    );

    m_rts[static_cast<size_t>(EnRenderTargetType::WorldPos)].Create(
        FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN
    );


    m_spriteInitData.m_width = FRAME_BUFFER_W;
    m_spriteInitData.m_height = FRAME_BUFFER_H;
    m_spriteInitData.m_fxFilePath = "Assets/shader/sprite.fx";
    m_spriteInitData.m_psEntryPoinFunc = "PSMainDeferred";
    m_spriteInitData.m_textures[0] =
        &m_rts[static_cast<size_t>(EnRenderTargetType::Albedo)].GetRenderTargetTexture();
    m_spriteInitData.m_textures[1] =
        &m_rts[static_cast<size_t>(EnRenderTargetType::Normal)].GetRenderTargetTexture();
    m_spriteInitData.m_textures[2] =
        &m_rts[static_cast<size_t>(EnRenderTargetType::WorldPos)].GetRenderTargetTexture();

    m_spriteInitData.m_expandConstantBuffer = light.GetSceneLightAddress();
    m_spriteInitData.m_expandConstantBufferSize = sizeof(balloonEngineLow::LightData);

    m_sprite.Init(m_spriteInitData);

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

    RenderTarget* rts[static_cast<size_t>(EnRenderTargetType::Max)];

    for (size_t i = 0; i < m_rts.size(); i++)
    {
        rts[i] = &m_rts[i];
    }


    rc.WaitUntilToPossibleSetRenderTargets(m_rts.size(), rts);
    rc.SetRenderTargets(m_rts.size(), rts);
    rc.ClearRenderTargetViews(m_rts.size(), rts);

    m_modelRender.Draw(rc);

    rc.WaitUntilFinishDrawingToRenderTargets(m_rts.size(), rts);

    g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
    m_sprite.Draw(rc);
}
