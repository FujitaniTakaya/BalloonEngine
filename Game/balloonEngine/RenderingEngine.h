/**
 * @file RenderingEngine.h
 * @brief 描画エンジンクラスの宣言
 */
#pragma once


namespace balloonEngine
{
    /** 前方宣言 */
    class ModelRender;
    class SpriteRender;


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
        // 遅延描画
        //=======================================================================
    public:
        /** @brief 初期化関数 */
        void InitializeDeferredRendering();
        /** @brief 更新関数 */
        void UpdateDeferredRendering();
        /** @brief 遅延描画関数 */
        void RenderDeferredRendering();
        /**
         * @brief 遅延描画オブジェクトを追加する。
         * @param render3dObject 遅延描画オブジェクト
         */
        void Add3dObject(ModelRender* render3dObject);



    private:
        /** 遅延描画オブジェクトのリスト */
        std::vector<ModelRender*> m_defferedRendering3dObjectList;
        /** 遅延描画用スプライト */
        Sprite m_defferedRenderingSprite;
        /** 遅延描画用レンダーターゲット */
        std::array<RenderTarget, static_cast<size_t>(RTType::Max)> m_rts;



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
} // namespace balloonEngine
