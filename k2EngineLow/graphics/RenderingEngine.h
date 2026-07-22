/**
 * @file RenderingEngine.h
 * @brief 描画エンジンクラスの宣言
 */
#pragma once


namespace nsK2EngineLow
{
    /** 前方宣言 */
    class ModelRender;


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
        // シャドウマップ
        //=======================================================================
    public:
        /**
         * @brief シャドウキャスターを追加する。
         * @param model シャドウキャスターとなるモデル
         */
        void AddShadowCaster(Model* model);


        /**
         * @brief ライトカメラを取得する。
         * @return ライトカメラ
         */
        Camera& GetLightCamera();


        /**
         * @brief シャドウマップテクスチャを取得する。
         * @return シャドウマップテクスチャ
         */
        Texture& GetShadowMapTexture();


    private:
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
         * @note  シーンライトの方向は Game::Start など Initialize より後に設定されることがあるため、
         *        毎フレーム呼び直して同期を取る。
         */
        void InitializeLightCamera();


    private:
        /** 描画オブジェクトのリスト */
        std::vector<Model*> m_rendering3dObjects;
        /** 遅延描画用オブジェクトのリスト */
        std::vector<Model*> m_deferredRendering3dObjects;
        /** 遅延描画用スプライト */
        Sprite m_deferredRenderingSprite;
        /** 遅延描画用レンダーターゲット */
        std::array<RenderTarget, static_cast<size_t>(RTType::Max)> m_rts;

        /** ライトカメラ */
        Camera m_lightCamera;

        RenderTarget m_shadowMap;

        std::vector<Model*> m_shadowCasters;


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
