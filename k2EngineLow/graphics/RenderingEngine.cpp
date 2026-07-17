/**
 * @file RenderingEngine.cpp
 * @brief 描画エンジンクラスの実装
 */
#include "k2EngineLowPreCompile.h"

#include "RenderingEngine.h"

#include "ModelRender.h"
#include "Light.h"


namespace balloonEngine
{
    RenderingEngine::RenderingEngine()
    {}


    void RenderingEngine::InitializeDeferredRendering()
    {
        constexpr UINT width = FRAME_BUFFER_W;
        constexpr UINT height = FRAME_BUFFER_H;

        GetRenderTarget(RTType::Albedo)
            .Create(width, height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
        GetRenderTarget(RTType::Normal)
            .Create(width, height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
        GetRenderTarget(RTType::WorldPos)
            .Create(width, height, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);


        SpriteInitData initData;
        initData.m_width = width;
        initData.m_height = height;
        initData.m_fxFilePath = "Assets/shader/sprite.fx";
        initData.m_psEntryPoinFunc = "PSMainDeferred";
        initData.m_textures[0] = &GetRenderTarget(RTType::Albedo).GetRenderTargetTexture();
        initData.m_textures[1] = &GetRenderTarget(RTType::Normal).GetRenderTargetTexture();
        initData.m_textures[2] = &GetRenderTarget(RTType::WorldPos).GetRenderTargetTexture();

        auto& light = balloonEngineLow::SceneLight::Get();
        initData.m_expandConstantBuffer = light.GetAddress();
        initData.m_expandConstantBufferSize = sizeof(balloonEngineLow::LightData);

        m_deferredRenderingSprite.Init(initData);
    }


    void RenderingEngine::RenderDeferredRendering()
    {
        // m_rts と名前を手打ちで対応させると RTType 追加時に取りこぼす恐れがあるため、
        // インデックスから機械的にポインタ配列を構築する
        std::array<RenderTarget*, static_cast<size_t>(RTType::Max)> rts;
        for (size_t i = 0; i < rts.size(); ++i)
        {
            rts[i] = &m_rts[i];
        }
        const int numRt = static_cast<int>(rts.size());

        auto& rc = g_graphicsEngine->GetRenderContext();

        // レンダーターゲットとして設定できるようになるまで待つ
        rc.WaitUntilToPossibleSetRenderTargets(numRt, rts.data());
        // レンダーターゲットを設定
        rc.SetRenderTargets(numRt, rts.data());
        // レンダーターゲットをクリア
        rc.ClearRenderTargetViews(numRt, rts.data());

        // 遅延描画オブジェクトを描画
        for (ModelRender* render3dObject : m_deferredRendering3dObjectList)
        {
            render3dObject->GetModel().Draw(rc);
        }

        // レンダーターゲットヘの書き込み待ち
        rc.WaitUntilFinishDrawingToRenderTargets(numRt, rts.data());
        // レンダリング先をフレームバッファーに戻してスプライトをレンダリングする
        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
        // 遅延描画用スプライトを描画
        m_deferredRenderingSprite.Draw(rc);

        // 遅延描画オブジェクトのリストをクリア
        m_deferredRendering3dObjectList.clear();
    }


    void RenderingEngine::Add3dObject(ModelRender* render3dObject)
    {
        m_deferredRendering3dObjectList.push_back(render3dObject);
    }
} // namespace balloonEngine
