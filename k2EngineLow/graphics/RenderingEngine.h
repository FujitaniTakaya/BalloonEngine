/**
 * @file RenderingEngine.h
 * @brief 描画エンジンクラスの宣言
 */
#pragma once
#include "ShadowRef.h"


namespace nsK2EngineLow
{
    /**
     * @brief レンダーターゲットの種類
     * @note 遅延描画用のレンダーターゲットの種類を表す列挙型
     * Albedo: アルベド（拡散反射）カラー
     * Normal: 法線ベクトル
     * WorldPos: ワールド座標
     */
    enum class EnRenderTargetType
    {
        Albedo = 0,
        Normal,
        WorldPos,
        Max
    };

    using RTType = EnRenderTargetType;


    /**
     * @brief ブラー用の定数バッファ
     */
    struct BloomCB
    {
        /** ブラーのしきい値 */
        float threshold;
        float padding[3];


        BloomCB();
        ~BloomCB() = default;
    };


    /**
     * @brief 被写界深度用の定数バッファ
     */
    struct DoFCB
    {
        /** ピントが合う距離 */
        float focusDistance;
        /** ピントが合う幅 */
        float focusRange;
        float padding[2];

        DoFCB();
        ~DoFCB() = default;
    };



    /**
     * @brief 描画エンジンクラス
     */
    class RenderingEngine
    {
    private:
        RenderingEngine();
        ~RenderingEngine() = default;


        //=======================================================================
        // 描画
        //=======================================================================
    public:
        /** @brief 初期化関数 */
        void Initialize();
        /** @brief 描画関数 */
        void Execute();
        /**
         * @brief 描画オブジェクトを追加する。
         * @param render3dObject 描画オブジェクト
         */
        void Add3dObject(Model* render3dObject);


        /**
         * @brief 遅延描画用オブジェクトを追加する。
         * @param render3dObject 遅延描画用オブジェクト
         */
        void AddDeferredRendering3dObject(Model* render3dObject);


        //=======================================================================
        // ポストプロセス
        //=======================================================================
    public:
        /**
         * @brief スクリーンブラーの強さを設定する。
         * @param power ブラーの強さ
         */
        void SetScreenBlurPower(float& power)
        {
            m_screenBlurPower = power;
        }


        //=======================================================================
        // シャドウマップ
        //=======================================================================
    public:
        /**
         * @brief シャドウキャスターを追加する。
         * @param model シャドウキャスターとなるモデル
         * @param cascadeIndex 追加先のカスケード番号(0 ～ NUM_SHADOW_MAP-1)
         */
        void AddShadowCaster(Model* model, int cascadeIndex);


        /**
         * @brief シャドウマップのテクスチャを取得する。
         * @param queryFunc シャドウマップのテクスチャを取得するための関数
         */
        void QueryShadowMapTexture(std::function<void(Texture&)>);


        //=======================================================================
        // カプセル
        //=======================================================================
    private:
        /** @brief シャドウマップを初期化する。 */
        void InitializeShadowMap();

        /** @brief シャドウマップを実行する。 */
        void ExecuteShadowMap(RenderContext& rc);


    private:
        /** @brief G-Bufferを初期化する。 */
        void InitializeGBuffer();

        /** @brief G-Bufferを実行する。 */
        void ExecuteGBuffer(RenderContext& rc);

        /** @brief G-Bufferデバッグ表示用スプライトを初期化する。 */
        void InitializeDebugSprite();

        /** @brief G-Bufferデバッグ表示用スプライトを描画する(フラグが立っている時のみ)。 */
        void ExecuteDebugSprite(RenderContext& rc);


    private:
        /** @brief 遅延描画を初期化する。 */
        void InitializeDeferredRendering();

        /** @brief 遅延描画を実行する。 */
        void ExecuteDeferredRendering(RenderContext& rc);


    private:
        /** @brief フォワード描画を実行する。 */
        void ExecuteForwardRendering(RenderContext& rc);


    private:
        /** @brief ブルームを初期化する。 */
        void InitializeBloom();

        /** @brief ブルームを実行する。 */
        void ExecuteBloom(RenderContext& rc);


    private:
        /** @brief ポストプロセスを初期化する。 */
        void InitializePostProcess();

        /** @brief ポストプロセスを実行する。 */
        void ExecutePostProcess(RenderContext& rc);




        /**
         * @brief レンダーターゲットの種類を指定して取得する。
         * @param type レンダーターゲットの種類
         * @return レンダーターゲット
         */
        RenderTarget& GetRenderTarget(RTType type)
        {
            return m_rts[static_cast<size_t>(type)];
        }


        /**
         * @brief ライトカメラを現在のシーンライトの方向に合わせて更新する。
         */
        void InitializeLightCamera(Camera& cmr, const int index);


        /**
         * @brief ライトカメラを現在のシーンライトの方向に合わせて更新する。
         */
        void UpdateLightCamera(Camera& cmr, const int index);



        //=======================================================================
        // ブルーム用
        //=======================================================================
    public:
        /**
         * @brief ブルーム用の定数バッファを取得する。
         * @return ブルーム用の定数バッファ
         */
        BloomCB& GetBloomCB();


        /**
         * @brief ブルームの強さを取得する(加算合成時の乗算値)。
         * @return ブルームの強さ
         */
        float& GetBloomIntensity();


        /**
         * @brief デュアルブラーの有効化フラグを取得する。
         * @return デュアルブラーの有効化フラグ
         */
        bool& SetDualBlurEnable();


        //=======================================================================
        // DoF用
        //=======================================================================
    public:
        /**
         * @brief 被写界深度用の定数バッファを取得する。
         * @return 被写界深度用の定数バッファ
         */
        DoFCB& GetDoFCB();


        /**
         * @brief 被写界深度の有効化フラグを取得する。
         * @return 被写界深度の有効化フラグ
         */
        bool& GetDoFEnable();


        //=======================================================================
        // デバッグ表示用
        //=======================================================================
    public:
        /**
         * @brief G-Bufferデバッグ表示の有効化フラグを取得する。
         * @return G-Bufferデバッグ表示の有効化フラグ
         */
        bool& GetDebugDrawGBufferEnable();


    private:
        /** 描画オブジェクトのリスト */
        std::vector<Model*> m_rendering3dObjects;


        //=======================================================================
        // デファードレンダリング
        //=======================================================================
    private:
        /** デファードレンダリング用オブジェクトのリスト */
        std::vector<Model*> m_deferredRendering3dObjects;
        /** スプライト */
        Sprite m_deferredRenderingSprite;
        /** レンダーターゲット */
        std::array<RenderTarget, static_cast<size_t>(RTType::Max)> m_rts;
        /** G-Buffer デバッグ表示用スプライト(各 G-Buffer を画面隅に並べる) */
        std::array<Sprite, static_cast<size_t>(RTType::Max)> m_rtSprites;
        /** G-Buffer デバッグ表示の有効化フラグ(ImGui から操作) */
        bool m_isDebugDrawGBuffer = false;


        //========================================================================
        // ポストプロセス
        //========================================================================
    private:
        /** メインレンダーターゲット */
        RenderTarget m_mainRenderTarget;
        /** フレームバッファーにコピーするためのスプライト */
        Sprite m_copyToFrameBufferSprite;
        /** ブラー済みの画像をフレームバッファーにコピーするためのスプライト */
        Sprite m_copyBlurToFrameBufferSprite;
        /** ガウシアンブラー */
        GaussianBlur m_screenBlur;
        /** ブラーの値 */
        float m_screenBlurPower;


        //=======================================================================
        // ブルーム
        //=======================================================================
    private:
        /** ブルーム用の定数バッファ */
        BloomCB m_bloomCB;

        /** ブルーム用 */
        DualBlur m_bloomBlur;

        /** 高度抽出用RT */
        RenderTarget m_bloomRT;
        /** 高度抽出RT */
        Sprite m_luminanceSprite;
        /** ブルーム合成用RT */
        Sprite m_bloomSprite;

        /** デュアルブラーの有効化フラグ */
        bool m_isDualBlurEnable;
        /** ブルームの強さ(加算合成時の乗算値) */
        float m_bloomIntensity;


        //=======================================================================
        // DoF
        //=======================================================================
    private:
        /** 被写界深度用のブラー */
        DualBlur m_dofBlur;
        /** 被写界深度用の定数バッファ */
        DoFCB m_dofCB;
        /** 被写界深度合成用スプライト */
        Sprite m_dofSprite;
        /** 被写界深度の有効化フラグ */
        bool m_isDoFEnable;


        //=======================================================================
        // シャドウマップ
        //=======================================================================
    public:
        struct ShadowData
        {
            /** ライトカメラ */
            Camera ligCamera;
            /** シャドウマップ */
            RenderTarget map;
            /** シャドウキャスターのリスト */
            std::vector<Model*> casters;
        };


    private:
        /** シャドウデータの配列 */
        std::array<ShadowData, MAX_SHADOW_NUM> m_shadowDatas;


        //=======================================================================
        // シングルトンパターン
        //=======================================================================
    public:
        /**
         * @brief インスタンスを取得する。
         * @return インスタンス
         */
        static RenderingEngine& Get()
        {
            static RenderingEngine instance;
            return instance;
        }
    };
} // namespace nsK2EngineLow
