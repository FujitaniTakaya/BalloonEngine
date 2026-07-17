/**
 * @file RenderingEngine.cpp
 * @brief 描画エンジンクラスの実装
 */
#include "stdafx.h"

#include "RenderingEngine.h"

#include "ModelRender.h"
#include "SpriteRender.h"
#include "balloonengineLow/Light.h"


namespace balloonEngine
{
    RenderingEngine::RenderingEngine()
    {}


    void RenderingEngine::InitializeDeferredRendering()
    {
        constexpr UINT width = FRAME_BUFFER_W;
        constexpr UINT height = FRAME_BUFFER_H;

        m_rts[static_cast<size_t>(RTType::Albedo)]
            .Create(width, height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
        m_rts[static_cast<size_t>(RTType::Normal)]
            .Create(width, height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
        m_rts[static_cast<size_t>(RTType::WorldPos)]
            .Create(width, height, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);


        SpriteInitData initData;
        initData.m_width = width;
        initData.m_height = height;
        initData.m_fxFilePath = "Assets/shader/sprite.fx";
        initData.m_psEntryPoinFunc = "PSMainDeferred";
        initData.m_textures[0] = &m_rts[static_cast<size_t>(RTType::Albedo)].GetRenderTargetTexture();
        initData.m_textures[1] = &m_rts[static_cast<size_t>(RTType::Normal)].GetRenderTargetTexture();
        initData.m_textures[2] = &m_rts[static_cast<size_t>(RTType::WorldPos)].GetRenderTargetTexture();

        auto& light = balloonEngineLow::SceneLight::Get();
        initData.m_expandConstantBuffer = light.GetAddress();
        initData.m_expandConstantBufferSize = sizeof(balloonEngineLow::LightData);

        m_defferedRenderingSprite.Init(initData);
    }


    void RenderingEngine::UpdateDeferredRendering()
    {}


    void RenderingEngine::RenderDeferredRendering()
    {
        RenderTarget* rts[static_cast<size_t>(RTType::Max)] = {
            &m_rts[static_cast<size_t>(RTType::Albedo)],
            &m_rts[static_cast<size_t>(RTType::Normal)],
            &m_rts[static_cast<size_t>(RTType::WorldPos)],
        };

        auto& rc = g_graphicsEngine->GetRenderContext();

        // レンダーターゲットとして設定できるようになるまで待つ
        rc.WaitUntilToPossibleSetRenderTargets(m_rts.size(), rts);
        // レンダーターゲットを設定
        rc.SetRenderTargets(m_rts.size(), rts);
        // レンダーターゲットをクリア
        rc.ClearRenderTargetViews(m_rts.size(), rts);

        // 遅延描画オブジェクトを描画
        for (ModelRender* render3dObject : m_defferedRendering3dObjectList)
        {
            render3dObject->GetModel().Draw(rc);
        }

        // レンダーターゲットヘの書き込み待ち
        rc.WaitUntilFinishDrawingToRenderTargets(m_rts.size(), rts);
        // レンダリング先をフレームバッファーに戻してスプライトをレンダリングする
        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
        // 遅延描画用スプライトを描画
        m_defferedRenderingSprite.Draw(rc);

        // 遅延描画オブジェクトのリストをクリア
        m_defferedRendering3dObjectList.clear();
    }


    void RenderingEngine::Add3dObject(ModelRender* render3dObject)
    {
        m_defferedRendering3dObjectList.push_back(render3dObject);
    }
} // namespace balloonEngine
