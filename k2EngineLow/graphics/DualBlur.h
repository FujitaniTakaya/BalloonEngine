/**
 * @file DualBlur.h
 * @brief デュアルブラーの宣言
 */
#pragma once


namespace nsK2EngineLow
{
    class DualBlur
    {
    public:
        DualBlur() = default;
        ~DualBlur() = default;


    public:
        /**
         * @brief デュアルブラーを初期化する。
         * @param originalTexture ぼかしたい元テクスチャ
         */
        void Init(Texture* originalTexture);


        /**
         * @brief デュアルブラーをGPUで実行する。
         * @param rc レンダーコンテキスト
         */
        void ExecuteOnGPU(RenderContext& rc);


        /**
         * @brief デュアルブラーの結果のテクスチャを取得する。
         * @return デュアルブラーの結果のテクスチャ
         */
        Texture& GetResultTexture();


    private:
        /**
         * @brief スプライトを初期化するためのヘルパー
         * @param sprite 初期化するスプライト
         * @param srcTexture スプライトに設定する元テクスチャ
         * @param fxFilePath スプライトに設定するシェーダーファイルのパス
         * @param targetRT スプライトの描画先のレンダーターゲット
         */
        void InitSprite(
            Sprite& sprite,
            Texture* srcTexture,
            const char* fxFilePath,
            RenderTarget& targetRT
        );


        /**
         * @brief ブラー用のデータ構造体
         */
        struct BlurData
        {
            /** ブラー用のレンダーターゲット */
            RenderTarget rt;
            /** ブラー用のスプライト */
            Sprite sprite;
        };


        /**
         * @brief ブラーを実行するためのヘルパー
         * @param rc レンダーコンテキスト
         * @param blurData ブラー用のデータ構造体
         */
        void ExecuteBlur(RenderContext& rc, BlurData& blurData);


    private:
        static constexpr uint8_t NUM_DOWN = 4;
        static constexpr uint8_t NUM_UP = 3;


        /** 縮小用のブラー用データ */
        std::array<BlurData, NUM_DOWN> m_downBlur;
        /** 拡大用のブラー用データ */
        std::array<BlurData, NUM_UP> m_upBlur;
    };
} // namespace nsK2EngineLow