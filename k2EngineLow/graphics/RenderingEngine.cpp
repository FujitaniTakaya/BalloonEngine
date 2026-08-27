/**
 * @file RenderingEngine.cpp
 * @brief 描画エンジンクラスの実装
 */
#include "k2EngineLowPreCompile.h"

#include "RenderingEngine.h"


namespace nsK2EngineLow
{
    namespace
    {
        /** 描画するオブジェクトの最大数 */
        inline constexpr UINT DRAW_OBUJECT_MAX = 1000;
    } // namespace


    RenderingEngine::RenderingEngine()
        : m_screenBlurPower(0.0f)
        , m_isDualBlurEnable(false)
        , m_isDoFEnable(false)
        , m_bloomIntensity(1.0f)
    {}


    void RenderingEngine::Initialize()
    {
        m_rendering3dObjects.reserve(DRAW_OBUJECT_MAX);

        //========================================================================
        // ポストプロセスを初期化
        //========================================================================
        InitializePostProcess();


        //========================================================================
        // デュアルブラーを初期化
        //========================================================================
        InitializeBloom();


        //========================================================================
        // 被写界深度を初期化
        //========================================================================
        InitializeDoF();


        //========================================================================
        // デファードレンダリングを初期化
        //========================================================================
        InitializeDeferredRendering();


        //========================================================================
        // シャドウマップ用の変数を初期化
        //========================================================================
        InitializeShadowMap();
    }


    void RenderingEngine::Execute()
    {
        auto& rc = g_graphicsEngine->GetRenderContext();


        //========================================================================
        // シャドウマップを描画
        //========================================================================
        ExecuteShadowMap(rc);


        //========================================================================
        // ポストプロセス用のレンダーターゲットを描画
        //========================================================================
        ExecutePostProcess(rc);


        //========================================================================
        // デュアルブラー用のレンダーターゲットを描画
        //========================================================================
        ExecuteBloom(rc);


        //========================================================================
        // 被写界深度用のレンダーターゲットを描画
        //========================================================================
        ExecuteDoF(rc);


        //========================================================================
        // デファードレンダリング用のレンダーターゲットを描画
        //========================================================================
        ExecuteDeferredRendering(rc);
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
    void RenderingEngine::AddShadowCaster(Model* model, EnShadowLightType shadowType)
    {
        m_shadowDatas[static_cast<size_t>(shadowType)].casters.push_back(model);
    }


    void RenderingEngine::QueryShadowMapTexture(std::function<void(Texture&)> queryFunc)
    {
        for (auto& shadowData : m_shadowDatas)
        {
            auto& shadowMapTexture = shadowData.map.GetRenderTargetTexture();
            queryFunc(shadowMapTexture);
        }
    }


    //=======================================================================
    // ヘルパー
    //=======================================================================
    void RenderingEngine::InitializePostProcess()
    {
        m_mainRenderTarget.Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D32_FLOAT);

        SpriteInitData initData;
        initData.m_width = FRAME_BUFFER_W;
        initData.m_height = FRAME_BUFFER_H;
        initData.m_fxFilePath = "Assets/shader/sprite.fx";
        initData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();

        m_screenBlur.Init(&m_mainRenderTarget.GetRenderTargetTexture(), false, false);
        m_copyToFrameBufferSprite.Init(initData);

        // ブラー済みテクスチャをフレームバッファーにコピーするスプライトを初期化する。
        SpriteInitData blurInitData = initData;
        blurInitData.m_alphaBlendMode = AlphaBlendMode::AlphaBlendMode_Add;
        blurInitData.m_textures[0] = &m_screenBlur.GetBokeTexture();
        m_copyBlurToFrameBufferSprite.Init(blurInitData);
    }


    void RenderingEngine::ExecutePostProcess(RenderContext& rc)
    {
        rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
        rc.SetRenderTargetAndViewport(m_mainRenderTarget);
        rc.ClearRenderTargetView(m_mainRenderTarget);

        for (auto* object : m_rendering3dObjects)
        {
            object->Draw(rc);
        }

        m_rendering3dObjects.clear();

        rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);

        if (m_screenBlurPower > 0.0f)
        {
            m_screenBlur.ExecuteOnGPU(rc, m_screenBlurPower);
        }

        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);

        if (m_screenBlurPower > 0.0f)
        {
            m_copyToFrameBufferSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
            m_copyToFrameBufferSprite.Draw(rc);

            // GaussianBlurは中心ピクセルを直接サンプリングしないため、blurPowerが小さくても
            // 加算するボケ画像はほぼ元画像と同じ明るさになってしまう。
            // そのため、blurPowerに応じて加算量自体をフェードさせ、0付近で急に明るくならないようにする。
            const float fade = std::clamp(m_screenBlurPower, 0.0f, 1.0f);
            m_copyBlurToFrameBufferSprite.SetMulColor({ fade, fade, fade, 1.0f });
            m_copyBlurToFrameBufferSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
            m_copyBlurToFrameBufferSprite.Draw(rc);
        }
        else
        {
            m_copyToFrameBufferSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
            m_copyToFrameBufferSprite.Draw(rc);
        }
    }


    void RenderingEngine::InitializeBloom()
    {
        m_luminanceRT.Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);

        SpriteInitData luminanceInitData;
        luminanceInitData.m_width = FRAME_BUFFER_W;
        luminanceInitData.m_height = FRAME_BUFFER_H;
        luminanceInitData.m_fxFilePath = "Assets/shader/bloom/samplingLuminance.fx";
        luminanceInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
        luminanceInitData.m_expandConstantBuffer = &m_bloomCB;
        luminanceInitData.m_expandConstantBufferSize = sizeof(BloomCB);
        m_luminanceSprite.Init(luminanceInitData);


        m_bloomBlur.Init(&m_luminanceRT.GetRenderTargetTexture());


        // ブラー済みの画像をフレームバッファーに加算合成するためのスプライトを初期化する。
        SpriteInitData bloomInitData = luminanceInitData;
        bloomInitData.m_fxFilePath = "Assets/shader/sprite.fx";
        bloomInitData.m_textures[0] = &m_bloomBlur.GetResultTexture();
        bloomInitData.m_alphaBlendMode = AlphaBlendMode::AlphaBlendMode_Add;
        m_bloomSprite.Init(bloomInitData);
    }


    void RenderingEngine::ExecuteBloom(RenderContext& rc)
    {
        if (!m_isDualBlurEnable)
        {
            return;
        }

        // 高輝度抽出を実行する。
        m_luminanceSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
        rc.WaitUntilToPossibleSetRenderTarget(m_luminanceRT);
        rc.SetRenderTargetAndViewport(m_luminanceRT);
        rc.ClearRenderTargetView(m_luminanceRT);
        m_luminanceSprite.Draw(rc);


        // ブラーを実行する。
        m_bloomBlur.ExecuteOnGPU(rc);


        // レンダリング先をフレームバッファーに戻し、ブラー結果を加算合成する。
        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);

        // NOTE: これを書かないとうまくいかなかった
        rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());


        m_bloomSprite.SetMulColor({ m_bloomIntensity, m_bloomIntensity, m_bloomIntensity, 1.0f });
        m_bloomSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
        m_bloomSprite.Draw(rc);
    }


    void RenderingEngine::InitializeDoF()
    {
        m_dofBlur.Init(&m_mainRenderTarget.GetRenderTargetTexture());

        SpriteInitData initData;
        initData.m_width = FRAME_BUFFER_W;
        initData.m_height = FRAME_BUFFER_H;
        initData.m_fxFilePath = "Assets/shader/bloom/dof.fx";
        initData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
        initData.m_textures[1] = &m_dofBlur.GetResultTexture();
        initData.m_expandConstantBuffer = &m_dofCB;
        initData.m_expandConstantBufferSize = sizeof(DoFCB);
        m_dofSprite.Init(initData);
    }


    void RenderingEngine::ExecuteDoF(RenderContext& rc)
    {
        if (!m_isDoFEnable)
        {
            return;
        }

        m_dofBlur.ExecuteOnGPU(rc);

        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);

        // NOTE: これを書かないとうまくいかなかった
        rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());

        m_dofSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
        m_dofSprite.Draw(rc);
    }


    void RenderingEngine::InitializeDeferredRendering()
    {
        //========================================================================
        // デファードレンダリング用のレンダーターゲットを初期化
        //========================================================================
        GetRenderTarget(RTType::Albedo).Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
        GetRenderTarget(RTType::Normal).Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
        GetRenderTarget(RTType::WorldPos).Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);


        SpriteInitData rtInitData;
        rtInitData.m_width = FRAME_BUFFER_W / 2;
        rtInitData.m_height = FRAME_BUFFER_H / 2;
        rtInitData.m_fxFilePath = "Assets/shader/sprite.fx";
        rtInitData.m_textures[0] = &GetRenderTarget(RTType::Albedo).GetRenderTargetTexture();
        m_rtSprites[static_cast<size_t>(RTType::Albedo)].Init(rtInitData);
        rtInitData.m_textures[0] = &GetRenderTarget(RTType::Normal).GetRenderTargetTexture();
        m_rtSprites[static_cast<size_t>(RTType::Normal)].Init(rtInitData);
        rtInitData.m_textures[0] = &GetRenderTarget(RTType::WorldPos).GetRenderTargetTexture();
        m_rtSprites[static_cast<size_t>(RTType::WorldPos)].Init(rtInitData);


        //========================================================================
        // デファードレンダリング用のスプライトを初期化
        //========================================================================

        SpriteInitData initData;
        initData.m_width = FRAME_BUFFER_W;
        initData.m_height = FRAME_BUFFER_H;
        initData.m_fxFilePath = "Assets/shader/sprite.fx";
        initData.m_psEntryPoinFunc = "PSMainDeferred";
        initData.m_textures[0] = &GetRenderTarget(RTType::Albedo).GetRenderTargetTexture();
        initData.m_textures[1] = &GetRenderTarget(RTType::Normal).GetRenderTargetTexture();
        initData.m_textures[2] = &GetRenderTarget(RTType::WorldPos).GetRenderTargetTexture();

        auto& light = SceneLight::Get();
        initData.m_expandConstantBuffer = &light.m_sceneLight;
        initData.m_expandConstantBufferSize = sizeof(LightingCB);

        m_deferredRenderingSprite.Init(initData);
    }


    void RenderingEngine::ExecuteDeferredRendering(RenderContext& rc)
    {
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

        for (auto& rtSprite : m_rtSprites)
        {
            rtSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
            rtSprite.Draw(rc);
        }
    }


    void RenderingEngine::InitializeShadowMap()
    {
        float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

        for (int i = 0; i < LightingCB::MAX_SHADOW_NUM; ++i)
        {
            auto& data = m_shadowDatas.at(i);

            //========================================================================
            // シャドウマップ用のレンダーターゲットを初期化
            //========================================================================
            data.map.Create(1024, 1024, 1, 1, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT, clearColor);


            //========================================================================
            // シャドウマップ用のライトカメラを初期化
            //========================================================================

            data.ligCamera.SetWidth(500.0f);
            data.ligCamera.SetHeight(500.0f);
            switch (static_cast<EnShadowLightType>(i))
            {
            case EnShadowLightType::Directional:
                // ディレクションライトは平行光源なので平行投影。
                data.ligCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
                break;
            case EnShadowLightType::Spot:
                // スポットライトは一点から円錐状に広がる光源なので透視投影。
                // 視野角(spotAngleに応じた値)はUpdateLightCameraで毎フレーム設定する。
                data.ligCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Perspective);
                break;
            default:
                K2_ASSERT(false, "error: index is out of range.");
                break;
            }
            InitializeLightCamera(data.ligCamera, i);
        }
    }


    void RenderingEngine::ExecuteShadowMap(RenderContext& rc)
    {
        for (int i = 0; i < MAX_SHADOW_NUM; ++i)
        {
            auto& data = m_shadowDatas.at(i);


            //========================================================================
            // シャドウマップ用のライトカメラを更新
            //========================================================================
            UpdateLightCamera(data.ligCamera, i);


            //========================================================================
            // シャドウマップ用のレンダーターゲットを描画
            //========================================================================
            rc.WaitUntilToPossibleSetRenderTarget(data.map);
            rc.SetRenderTargetAndViewport(data.map);
            rc.ClearRenderTargetView(data.map);

            auto& casters = data.casters;
            for (auto* model : casters)
            {
                // NOTE: シャドウマップ用の描画はライトカメラを使う
                model->Draw(rc, data.ligCamera);
            }
            casters.clear();

            rc.WaitUntilFinishDrawingToRenderTarget(data.map);
        }


        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);

        // NOTE: これを書かないとうまくいかなかった
        rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());
    }


    void RenderingEngine::InitializeLightCamera(Camera& cmr, const int index)
    {
        auto& light = nsK2EngineLow::SceneLight::Get();

        const Vector3 lightPos = { 500.0f, 500.0f, 0.0f };
        // light.m_sceneLight.directionLight.lightColor.m_colorVec3.Set(g_vec4Yellow);
        Vector3 lightVec = (Vector3::Zero - lightPos);
        lightVec.Normalize();
        light.m_sceneLight.directionLight.lightDir.Set(lightVec);

        UpdateLightCamera(cmr, index);
    }


    void RenderingEngine::UpdateLightCamera(Camera& cmr, const int index)
    {
        auto& light = SceneLight::Get().m_sceneLight;

        switch (static_cast<EnShadowLightType>(index))
        {
        case EnShadowLightType::Directional: {
            cmr.SetPosition(light.directionLight.lightDir * -500.0f);
            cmr.SetTarget(Vector3::Zero);
            break;
        }
        case EnShadowLightType::Spot: {
            const auto& spotLight = light.spotLights.at(0);
            cmr.SetPosition(spotLight.pointLight.position);
            cmr.SetTarget(spotLight.pointLight.position + spotLight.lightDir * 100.0f);
            // imguiでangle/rangeが変わりうるので、毎フレーム視野角と遠平面を合わせ直す。
            // NOTE: spotLight.angleは中心軸から円錐の縁までの半頂角。
            //       カメラのSetViewAngleは全画角(FovAngleY)を取るため2倍にする。
            //       180°ちょうどだとtanが発散して射影行列が破綻するのでクランプする。
            cmr.SetViewAngle(std::min<float>(spotLight.angle * 2.0f, Math::DegToRad(179.0f)));
            cmr.SetFar(spotLight.pointLight.range * 100.0f);
            break;
        }
        default:
            K2_ASSERT(false, "error: index is out of range.");
            break;
        }

        cmr.SetUp({ 1.0f, 0.0f, 0.0f });
        cmr.Update();

        // ライトカメラから見た位置への変換行列をシーンライトに設定
        light.LVP.at(index) = cmr.GetViewProjectionMatrix();
    }


    //=======================================================================
    // ブルーム用の定数バッファ
    //=======================================================================
    BloomCB& RenderingEngine::GetBloomCB()
    {
        return m_bloomCB;
    }

    float& RenderingEngine::GetBloomIntensity()
    {
        return m_bloomIntensity;
    }

    bool& RenderingEngine::SetDualBlurEnable()
    {
        return m_isDualBlurEnable;
    }


    //=======================================================================
    // 被写界深度用の定数バッファ
    //=======================================================================
    DoFCB& RenderingEngine::GetDoFCB()
    {
        return m_dofCB;
    }


    bool& RenderingEngine::GetDoFEnable()
    {
        return m_isDoFEnable;
    }


    //=======================================================================
    // BloomCB
    //=======================================================================
    BloomCB::BloomCB()
        : threshold(1.0f)
    {
    }


    //=======================================================================
    // DoFCB
    //=======================================================================
    DoFCB::DoFCB()
        : focusDistance(10.0f)
        , focusRange(5.0f)
    {
    }
} // namespace nsK2EngineLow