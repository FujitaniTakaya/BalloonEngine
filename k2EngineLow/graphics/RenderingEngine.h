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

        /** @brief デュアルブラーを初期化する。 */
        void InitializeDualBlur();

        /** @brief デュアルブラーを実行する。 */
        void ExecuteDualBlur(RenderContext& rc);

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
        // ブラー用
        //=======================================================================
    public:
        /**
         * @brief ブラー用の定数バッファを取得する。
         * @return ブラー用の定数バッファ
         */
        BloomCB& GetBloomCB();


        /**
         * @brief ブラーの強さを取得する(加算合成時の乗算値)。
         * @return ブラーの強さ
         */
        float& GetBloomIntensity();


        void SetDualBlurEnable(bool enable);


    private:
        /** 描画オブジェクトのリスト */
        std::vector<Model*> m_rendering3dObjects;
        /** 遅延描画用オブジェクトのリスト */
        std::vector<Model*> m_deferredRendering3dObjects;
        /** 遅延描画用スプライト */
        Sprite m_deferredRenderingSprite;
        /** 遅延描画用レンダーターゲット */
        std::array<RenderTarget, static_cast<size_t>(RTType::Max)> m_rts;


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
        // デュアルブラー
        //=======================================================================
    private:
        struct BlurData
        {
            /** ブラー用レンダーターゲット */
            RenderTarget rt;
            /** ブラー用スプライト */
            Sprite sprite;
        };


        /** 輝度抽出の結果 */
        BlurData m_luminance;
        /** メイン用のブラー */
        BlurData m_mainBlur;
        /** ダウンスケール用のブラー */
        std::array<BlurData, 4> m_downBlur;
        /** アップスケール用のブラー */
        std::array<BlurData, 3> m_upBlur;
        /** ブラー用の定数バッファ */
        BloomCB m_bloomCB;
        /** ブラー済みの画像をフレームバッファーにコピーするためのスプライト */
        Sprite m_copyDualBlurToFrameBufferSprite;
        /** デュアルブラーの有効化フラグ */
        bool m_isDualBlurEnabl;
        /** ブラーの強さ(加算合成時の乗算値) */
        float m_bloomIntensity;



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
