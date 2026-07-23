/**
 * @file RenderingEngine.cpp
 * @brief 描画エンジンクラスの実装
 */
#include "k2EngineLowPreCompile.h"

#include "RenderingEngine.h"

#include "Light.h"
#include "ModelRender.h"


namespace nsK2EngineLow
{
    namespace
    {
        /** 描画するオブジェクトの最大数 */
        inline constexpr UINT DRAW_OBUJECT_MAX = 1000;
    } // namespace


    RenderingEngine::RenderingEngine()
    {}


    void RenderingEngine::Initialize()
    {
        m_rendering3dObjects.reserve(DRAW_OBUJECT_MAX);

        constexpr UINT width = FRAME_BUFFER_W;
        constexpr UINT height = FRAME_BUFFER_H;


        //========================================================================
        // デファードレンダリング用のレンダーターゲットを初期化
        //========================================================================
        GetRenderTarget(RTType::Albedo).Create(width, height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
        GetRenderTarget(RTType::Normal).Create(width, height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
        GetRenderTarget(RTType::WorldPos).Create(width, height, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);


        //========================================================================
        // デファードレンダリング用のスプライトを初期化
        //========================================================================

        SpriteInitData initData;
        initData.m_width = width;
        initData.m_height = height;
        initData.m_fxFilePath = "Assets/shader/sprite.fx";
        initData.m_psEntryPoinFunc = "PSMainDeferred";
        initData.m_textures[0] = &GetRenderTarget(RTType::Albedo).GetRenderTargetTexture();
        initData.m_textures[1] = &GetRenderTarget(RTType::Normal).GetRenderTargetTexture();
        initData.m_textures[2] = &GetRenderTarget(RTType::WorldPos).GetRenderTargetTexture();

        auto& light = SceneLight::Get();
        initData.m_expandConstantBuffer = light.GetAddress();
        initData.m_expandConstantBufferSize = sizeof(LightData);

        m_deferredRenderingSprite.Init(initData);


        //========================================================================
        // シャドウマップ用のレンダーターゲットを初期化
        //========================================================================

        float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_shadowMap.Create(1024, 1024, 1, 1, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT, clearColor);


        //========================================================================
        // ライトカメラを初期化
        //========================================================================

        m_lightCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
        m_lightCamera.SetWidth(2000.0f);
        m_lightCamera.SetHeight(2000.0f);
        InitializeLightCamera();
    }


    void RenderingEngine::Execute()
    {
        auto& rc = g_graphicsEngine->GetRenderContext();

        //========================================================================
        // シャドウマップを描画
        //========================================================================
        rc.WaitUntilToPossibleSetRenderTarget(m_shadowMap);
        rc.SetRenderTargetAndViewport(m_shadowMap);
        rc.ClearRenderTargetView(m_shadowMap);
        for (auto* model : m_shadowCasters)
        {
            // NOTE: シャドウマップ用の描画はライトカメラを使う
            model->Draw(rc, m_lightCamera);
        }
        m_shadowCasters.clear();

        rc.WaitUntilFinishDrawingToRenderTarget(m_shadowMap);

        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);

        // NOTE: これを書かないとうまくいかなかった
        rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());


        for (auto* object : m_rendering3dObjects)
        {
            object->Draw(rc);
        }
        m_rendering3dObjects.clear();

        //========================================================================
        // デファードレンダリング用のレンダーターゲットを描画
        //========================================================================
        if (m_deferredRendering3dObjects.empty())
        {
            return;
        }
        std::array<RenderTarget*, static_cast<size_t>(RTType::Max)> rts;
        for (size_t i = 0; i < rts.size(); ++i)
        {
            rts.at(i) = &m_rts.at(i);
        }
        const int numRt = static_cast<int>(rts.size());


        // レンダーターゲットとして設定できるようになるまで待つ
        rc.WaitUntilToPossibleSetRenderTargets(numRt, rts.data());
        // レンダーターゲットを設定
        rc.SetRenderTargets(numRt, rts.data());
        // レンダーターゲットをクリア
        rc.ClearRenderTargetViews(numRt, rts.data());

        // 遅延描画オブジェクトを描画
        for (auto* object : m_deferredRendering3dObjects)
        {
            object->Draw(rc);
        }

        // レンダーターゲットヘの書き込み待ち
        rc.WaitUntilFinishDrawingToRenderTargets(numRt, rts.data());
        // レンダリング先をフレームバッファーに戻してスプライトをレンダリングする
        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
        // 遅延描画用スプライトを描画
        m_deferredRenderingSprite.Draw(rc);

        // 描画オブジェクトのリストをクリア
        m_deferredRendering3dObjects.clear();
    }


    void RenderingEngine::Add3dObject(Model* render3dObject)
    {
        /** 最大数を超えたら追加しない */
        if (m_rendering3dObjects.size() >= DRAW_OBUJECT_MAX)
        {
            return;
        }

        m_rendering3dObjects.push_back(render3dObject);
    }

    void RenderingEngine::AddDeferredRendering3dObject(Model* render3dObject)
    {
        m_deferredRendering3dObjects.push_back(render3dObject);
    }


    //=======================================================================
    // シャドウマップ
    //=======================================================================
    void RenderingEngine::AddShadowCaster(Model* model)
    {
        m_shadowCasters.push_back(model);
    }


    Camera& RenderingEngine::GetLightCamera()
    {
        return m_lightCamera;
    }


    Texture& RenderingEngine::GetShadowMapTexture()
    {
        return m_shadowMap.GetRenderTargetTexture();
    }


    void RenderingEngine::InitializeLightCamera()
    {
        auto& light = nsK2EngineLow::SceneLight::Get();

        const Vector3 lightPos = { 500.0f, 500.0f, 0.0f };
        // light.SetLightColor(g_vec4Yellow);
        Vector3 lightVec = (Vector3::Zero - lightPos);
        lightVec.Normalize();
        light.SetLightDir(lightVec);


        auto lightDir = light.GetSceneLight().directionLight.lightDir;

        m_lightCamera.SetPosition(lightDir * -1000.0f);
        m_lightCamera.SetTarget(Vector3::Zero);
        m_lightCamera.SetUp({ 1.0f, 0.0f, 0.0f });
        m_lightCamera.Update();

        // ライトカメラから見た位置への変換行列をシーンライトに設定
        light.SetLightLVP(m_lightCamera.GetViewProjectionMatrix());
    }
} // namespace nsK2EngineLow
