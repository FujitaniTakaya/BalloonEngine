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
         * @param shadowType シャドウの種類
         */
        void AddShadowCaster(Model* model, EnShadowLightType shadowType);


        /**
         * @brief シャドウマップのテクスチャを取得する。
         * @param queryFunc シャドウマップのテクスチャを取得するための関数
         */
        void QueryShadowMapTexture(std::function<void(Texture&)>);


        //=======================================================================
        // ヘルパー
        //=======================================================================
    private:
        /** @brief ポストプロセスを初期化する。 */
        void InitializePostProcess();

        /** @brief ポストプロセスを実行する。 */
        void ExecutePostProcess(RenderContext& rc);

        /** @brief ブルームを初期化する。 */
        void InitializeBloom();

        /** @brief ブルームを実行する。 */
        void ExecuteBloom(RenderContext& rc);

        /** @brief 被写界深度を初期化する。 */
        void InitializeDoF();

        /** @brief 被写界深度を実行する。 */
        void ExecuteDoF(RenderContext& rc);

        /** @brief 遅延描画を初期化する。 */
        void InitializeDeferredRendering();

        /** @brief 遅延描画を実行する。 */
        void ExecuteDeferredRendering(RenderContext& rc);

        /** @brief シャドウマップを初期化する。 */
        void InitializeShadowMap();

        /** @brief シャドウマップを実行する。 */
        void ExecuteShadowMap(RenderContext& rc);



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
        // NOTE: テスト確認用のスプライト。後で消す。
        std::array<Sprite, static_cast<size_t>(RTType::Max)> m_rtSprites;


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
        RenderTarget m_luminanceRT;
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
        /** 被写界深度用の定数バッファ */
        DoFCB m_dofCB;
        /** DoF用 */
        DualBlur m_dofBlur;
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
