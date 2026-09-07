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

        /** シャドウマップ用ライトカメラの平行投影ボックスの一辺(ワールド単位)。 */
        inline constexpr float SHADOW_AREA_SIZE = 500.0f;

        /** シャドウマップ用ライトカメラを原点からライト方向へ後退させる距離。 */
        inline constexpr float SHADOW_LIGHT_DISTANCE = 500.0f;
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
        m_deferredRendering3dObjects.reserve(DRAW_OBUJECT_MAX);

        //========================================================================
        // ポストプロセスを初期化
        //========================================================================
        InitializePostProcess();


        //========================================================================
        // デュアルブラーを初期化
        //========================================================================
        InitializeBloom();


        //========================================================================
        // シャドウマップ用の変数を初期化
        //========================================================================
        // NOTE: InitializeDeferredRendering() がシャドウマップのテクスチャを
        //       参照するので、必ずその前に呼ぶ。
        InitializeShadowMap();


        //========================================================================
        // G-Bufferを初期化
        //========================================================================
        // NOTE: InitializeDeferredRendering() が G-Buffer のテクスチャを
        //       参照するので、必ずその前に呼ぶ。
        InitializeGBuffer();


        //========================================================================
        // G-Bufferデバッグ表示用スプライトを初期化
        //========================================================================
        // NOTE: G-Buffer のテクスチャを参照するので InitializeGBuffer() の後に呼ぶ。
        InitializeDebugSprite();


        //========================================================================
        // デファードレンダリングを初期化
        //========================================================================
        InitializeDeferredRendering();
    }


    void RenderingEngine::Execute()
    {
        auto& rc = g_graphicsEngine->GetRenderContext();

        // シーンライトを更新(視点位置をカメラから反映)。
        // モデル/デファードライティングの描画より前に1回。
        SceneLight::Get().Update();

        // 処理順
        //  1. シャドウマップを描画
        //  2. ジオメトリ(G-Buffer)生成
        //  3. ライティング計算(デファード本体)
        //  4. フォワード
        //  5. ポストプロセス
        //  6. コピー
        //  7. 2D, imgui


        //========================================================================
        // シャドウマップを描画
        ExecuteShadowMap(rc);


        //========================================================================
        // G-Bufferを描画
        ExecuteGBuffer(rc);


        //========================================================================
        // デファードレンダリング描画
        ExecuteDeferredRendering(rc);


        //========================================================================
        // フォワードレンダリング描画
        ExecuteForwardRendering(rc);


        //========================================================================
        // デュアルブラー描画
        ExecuteBloom(rc);


        //========================================================================
        // ポストプロセス描画
        ExecutePostProcess(rc);


        //========================================================================
        // G-Bufferデバッグ表示(フラグが立っている時のみ、フレームバッファーの上に重ねる)
        ExecuteDebugSprite(rc);
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
    void RenderingEngine::AddShadowCaster(Model* model, int cascadeIndex)
    {
        m_shadowDatas.at(static_cast<size_t>(cascadeIndex)).casters.push_back(model);
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


    void RenderingEngine::InitializeShadowMap()
    {
        float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

        for (int i = 0; i < NUM_SHADOW_MAP; ++i)
        {
            auto& data = m_shadowDatas.at(i);

            //========================================================================
            // シャドウマップ用のレンダーターゲットを初期化
            //========================================================================
            data.map.Create(1024, 1024, 1, 1, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT, clearColor);


            //========================================================================
            // シャドウマップ用のライトカメラを初期化
            //========================================================================
            // シャドウマップ i はディレクションライト i の影を落とすためのもの。
            // ディレクションライトは平行光源なので平行投影。
            data.ligCamera.SetUpdateProjMatrixFunc(Camera::enUpdateProjMatrixFunc_Ortho);
            data.ligCamera.SetWidth(SHADOW_AREA_SIZE);
            data.ligCamera.SetHeight(SHADOW_AREA_SIZE);

            InitializeLightCamera(data.ligCamera, i);
        }
    }


    void RenderingEngine::ExecuteShadowMap(RenderContext& rc)
    {
        for (int i = 0; i < NUM_SHADOW_MAP; ++i)
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

        // NOTE: これを書かないとうまくいかなかった
        rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());
    }


    void RenderingEngine::InitializeGBuffer()
    {
        GetRenderTarget(RTType::Albedo).Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
        GetRenderTarget(RTType::Normal).Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);
        GetRenderTarget(RTType::WorldPos).Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN);
    }


    void RenderingEngine::ExecuteGBuffer(RenderContext& rc)
    {
        // ジオメトリ生成
        if (m_deferredRendering3dObjects.empty())
        {
            return;
        }


        std::array<RenderTarget*, static_cast<size_t>(RTType::Max)> rts;
        for (size_t i = 0; i < rts.size(); ++i)
        {
            rts.at(i) = &m_rts.at(i);
        }
        const int numRT = static_cast<int>(rts.size());

        rc.WaitUntilToPossibleSetRenderTargets(numRT, rts.data());
        rc.SetRenderTargets(numRT, rts.data());
        rc.ClearRenderTargetViews(numRT, rts.data());

        for (auto* object : m_deferredRendering3dObjects)
        {
            object->Draw(rc);
        }

        m_deferredRendering3dObjects.clear();

        rc.WaitUntilFinishDrawingToRenderTargets(numRT, rts.data());
    }


    void RenderingEngine::InitializeDebugSprite()
    {
        // G-Buffer を 1/4 サイズで表示するデバッグ用スプライトを、種類の数だけ作る。
        for (size_t i = 0; i < m_rtSprites.size(); ++i)
        {
            SpriteInitData initData;
            initData.m_width = FRAME_BUFFER_W / 4;
            initData.m_height = FRAME_BUFFER_H / 4;
            initData.m_fxFilePath = "Assets/shader/sprite.fx";
            initData.m_textures[0] = &m_rts.at(i).GetRenderTargetTexture();
            m_rtSprites.at(i).Init(initData);
        }
    }


    void RenderingEngine::ExecuteDebugSprite(RenderContext& rc)
    {
        if (!m_isDebugDrawGBuffer)
        {
            return;
        }

        // ポストプロセス後のフレームバッファーの上に重ねて描く。
        rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());

        const float spriteW = static_cast<float>(FRAME_BUFFER_W / 4);
        const float spriteH = static_cast<float>(FRAME_BUFFER_H / 4);

        // スプライトの座標系は画面中央が原点(右+ / 上+)。左端に下から縦に並べる。
        const float baseX = -static_cast<float>(FRAME_BUFFER_W) * 0.5f + spriteW * 0.5f;
        const float baseY = -static_cast<float>(FRAME_BUFFER_H) * 0.5f + spriteH * 0.5f;

        for (size_t i = 0; i < m_rtSprites.size(); ++i)
        {
            const Vector3 pos = { baseX, baseY + spriteH * static_cast<float>(i), 0.0f };
            m_rtSprites.at(i).Update(pos, g_quatIdentity, g_vec3One);
            m_rtSprites.at(i).Draw(rc);
        }
    }


    void RenderingEngine::InitializeDeferredRendering()
    {
        SpriteInitData initData;
        initData.m_width = FRAME_BUFFER_W;
        initData.m_height = FRAME_BUFFER_H;
        initData.m_fxFilePath = "Assets/shader/deferredLighting.fx";
        initData.m_textures[0] = &GetRenderTarget(RTType::Albedo).GetRenderTargetTexture();
        initData.m_textures[1] = &GetRenderTarget(RTType::Normal).GetRenderTargetTexture();
        initData.m_textures[2] = &GetRenderTarget(RTType::WorldPos).GetRenderTargetTexture();

        // シャドウマップを G-Buffer に続けて t3 ～ t3+MAX_SHADOW_NUM-1 に割り当てる。
        // NOTE: Model の拡張SRVは t10 始まりだが、Sprite の拡張SRVは t20 始まりなので、
        //       共通の common/Shadow.hlsli(t10 前提)は使わず、通常テクスチャスロットで渡す。
        int shadowMapSlot = static_cast<int>(RTType::Max);
        QueryShadowMapTexture([&](Texture& shadowMap) {
            initData.m_textures[shadowMapSlot] = &shadowMap;
            ++shadowMapSlot;
        });

        initData.m_expandConstantBuffer = &SceneLight::Get().m_sceneLight;
        initData.m_expandConstantBufferSize = sizeof(LightingCB);

        m_deferredRenderingSprite.Init(initData);
    }


    void RenderingEngine::ExecuteDeferredRendering(RenderContext& rc)
    {
        // 書き出し先を変更
        rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
        rc.SetRenderTargetAndViewport(m_mainRenderTarget);
        rc.ClearRenderTargetView(m_mainRenderTarget);

        // 遅延描画用スプライトを描画
        m_deferredRenderingSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
        m_deferredRenderingSprite.Draw(rc);
    }


    void RenderingEngine::ExecuteForwardRendering(RenderContext& rc)
    {
        rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
        rc.SetRenderTargetAndViewport(m_mainRenderTarget);

        for (auto* obj : m_rendering3dObjects)
        {
            obj->Draw(rc);
        }
        m_rendering3dObjects.clear();

        rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
    }


    void RenderingEngine::InitializeBloom()
    {
        m_bloomRT.Create(FRAME_BUFFER_W, FRAME_BUFFER_H, 1, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);

        SpriteInitData luminanceInitData;
        luminanceInitData.m_width = FRAME_BUFFER_W;
        luminanceInitData.m_height = FRAME_BUFFER_H;
        luminanceInitData.m_fxFilePath = "Assets/shader/bloom/samplingLuminance.fx";
        luminanceInitData.m_textures[0] = &m_mainRenderTarget.GetRenderTargetTexture();
        luminanceInitData.m_expandConstantBuffer = &m_bloomCB;
        luminanceInitData.m_expandConstantBufferSize = sizeof(BloomCB);
        m_luminanceSprite.Init(luminanceInitData);


        m_bloomBlur.Init(&m_bloomRT.GetRenderTargetTexture());


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
        rc.WaitUntilToPossibleSetRenderTarget(m_bloomRT);
        rc.SetRenderTargetAndViewport(m_bloomRT);
        rc.ClearRenderTargetView(m_bloomRT);
        m_luminanceSprite.Draw(rc);


        // ブラーを実行する。
        m_bloomBlur.ExecuteOnGPU(rc);


        rc.WaitUntilToPossibleSetRenderTarget(m_mainRenderTarget);
        rc.SetRenderTargetAndViewport(m_mainRenderTarget);

        // NOTE: これを書かないとうまくいかなかった
        rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());


        m_bloomSprite.SetMulColor({ m_bloomIntensity, m_bloomIntensity, m_bloomIntensity, 1.0f });
        m_bloomSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
        m_bloomSprite.Draw(rc);

        rc.WaitUntilFinishDrawingToRenderTarget(m_mainRenderTarget);
    }


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


        // dof用のブラーを初期化
        // NOTE: 下の m_dofSprite が m_dofBlur.GetResultTexture() を参照するので、
        //       必ずその前に Init する。DoF は輝度抽出を通さず mainRT を直接ぼかす。
        m_dofBlur.Init(&m_mainRenderTarget.GetRenderTargetTexture());

        // dof用のスプライトを初期化
        SpriteInitData dofInitData = initData;
        dofInitData.m_fxFilePath = "Assets/shader/bloom/dof.fx";
        dofInitData.m_textures[1] = &m_dofBlur.GetResultTexture();
        dofInitData.m_expandConstantBuffer = &m_dofCB;
        dofInitData.m_expandConstantBufferSize = sizeof(DoFCB);
        m_dofSprite.Init(dofInitData);
    }


    void RenderingEngine::ExecutePostProcess(RenderContext& rc)
    {
        if (m_screenBlurPower > 0.0f)
        {
            m_screenBlur.ExecuteOnGPU(rc, m_screenBlurPower);
        }

        if (m_isDoFEnable)
        {
            m_dofBlur.ExecuteOnGPU(rc);
        }


        g_graphicsEngine->ChangeRenderTargetToFrameBuffer(rc);
        rc.SetViewportAndScissor(g_graphicsEngine->GetFrameBufferViewport());


        if (m_isDoFEnable)
        {
            m_dofSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
            m_dofSprite.Draw(rc);
        }
        else
        {
            m_copyToFrameBufferSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
            m_copyToFrameBufferSprite.Draw(rc);
        }
        if (m_screenBlurPower > 0.0f)
        {
            // GaussianBlurは中心ピクセルを直接サンプリングしないため、blurPowerが小さくても
            // 加算するボケ画像はほぼ元画像と同じ明るさになってしまう。
            // そのため、blurPowerに応じて加算量自体をフェードさせ、0付近で急に明るくならないようにする。
            const float fade = std::clamp(m_screenBlurPower, 0.0f, 1.0f);
            m_copyBlurToFrameBufferSprite.SetMulColor({ fade, fade, fade, 1.0f });
            m_copyBlurToFrameBufferSprite.Update(g_vec3Zero, g_quatIdentity, g_vec3One);
            m_copyBlurToFrameBufferSprite.Draw(rc);
        }
    }


    void RenderingEngine::InitializeLightCamera(Camera& cmr, const int index)
    {
        // 1灯目のディレクションライトの向きの初期値を1度だけ設定する。
        // 2灯目以降は DirectionLight のデフォルト値(真下)のまま。ImGui で調整する。
        if (index == 0)
        {
            auto& light = SceneLight::Get();
            const Vector3 lightPos = { 500.0f, 500.0f, 0.0f };
            Vector3 lightVec = (Vector3::Zero - lightPos);
            lightVec.Normalize();
            light.m_sceneLight.directionLights.at(0).lightDir.Set(lightVec);
        }

        UpdateLightCamera(cmr, index);
    }


    void RenderingEngine::UpdateLightCamera(Camera& cmr, const int index)
    {
        auto& light = SceneLight::Get().m_sceneLight;

        // シャドウマップ index はディレクションライト index の影用。
        // ワールド原点を中心に、そのライトの方向から見下ろす平行投影。
        // カバー範囲(width/height)は InitializeShadowMap で設定済み。
        // 毎フレーム変わるのはライト方向依存の位置だけ。
        Vector3 lightDir = light.directionLights.at(index).lightDir;
        lightDir.Normalize();

        cmr.SetPosition(lightDir * -SHADOW_LIGHT_DISTANCE);
        cmr.SetTarget(Vector3::Zero);
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
    // デバッグ表示用
    //=======================================================================
    bool& RenderingEngine::GetDebugDrawGBufferEnable()
    {
        return m_isDebugDrawGBuffer;
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