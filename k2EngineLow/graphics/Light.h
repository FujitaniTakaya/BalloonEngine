/**
 * @file Light.h
 * @brief ライト関連
 */
#pragma once
#include "math/LightColor.h"


namespace nsK2EngineLow
{
    /**
     * @brief ディレクションライト
     */
    struct DirectionLight
    {
        /** ライトの方向 */
        Vector3 lightDir;
        /** パディング(HLSLの16バイトアライメント合わせ) */
        float pad;
        /** ライトの色 */
        ColorVec3 lightColor;


        DirectionLight();
        ~DirectionLight() = default;
    };




    /**********************************************/


    /**
     * @brief アンビエントライト
     */
    struct AmbientLight
    {
        /** 環境光の色 */
        ColorVec3 lightColor;


        AmbientLight();
        ~AmbientLight() = default;
    };




    /**********************************************/


    /**
     * @brief ライトデータ(model.fx の LightCb と1:1対応)
     * @note  ここを変更したらシェーダー側の cbuffer も必ず変更すること。
     */
    struct LightData
    {
        /** ディレクションライト */
        DirectionLight directionLight;
        /** 環境光の色 */
        AmbientLight ambientLight;
        /** 視点(カメラ)位置。鏡面反射で使用 */
        Vector3 eyePosition;
        /** パディング */
        float pad;
        /** ライトカメラから見た位置への変換行列 */
        Matrix LVP;


        LightData();
        ~LightData() = default;
    };




    /**********************************************/


    /**
     * @brief シーンライト
     * @note  シーンライトはシングルトンで管理する。
     *        シーンライトの更新は、モデルの描画より前に1回
     */
    class SceneLight : public Noncopyable
    {
        //=======================================================================
        // 更新
        //=======================================================================
    public:
        /**
         * @brief 更新関数(毎フレーム、モデルの描画より前に1回呼ぶ)
         * @note  視点位置をカメラから反映する。
         */
        void Update();


        //=======================================================================
        // シーンライト
        //=======================================================================
    public:
        /**
         * @brief ライトの方向を設定
         * @param lightDir ライトの方向
         */
        void SetLightDir(const Vector3& lightDir);
        /**
         * @brief ライトの色を設定
         * @param lightColor ライトの色
         */
        void SetLightColor(const Vector4& lightColor);
        /**
         * @brief アンビエントカラーを設定
         * @param ambientColor アンビエントカラー
         */
        void SetAmbientColor(const Vector4& lightColor);
        /**
         * @brief シーンライトを取得(読み取り用)
         * @return シーンライト
         */
        const LightData& GetSceneLight() const;
        /**
         * @brief シーンライトのアドレスを取得(定数バッファー登録用)
         * @note  ModelRender::Init から m_expandConstantBuffer に渡すために使う。
         * @return シーンライトのアドレス
         */
        LightData* GetAddress();
        /**
         * @brief ライトカメラから見た位置への変換行列を設定
         * @param lvp ライトカメラから見た位置への変換行列
         */
        void SetLightLVP(const Matrix& lvp);



    private:
        /** シーンライト */
        LightData m_sceneLight;


        //=======================================================================
        // シングルトン
        //=======================================================================
    public:
        /**
         * @brief ライトマネージャーを取得
         * @return ライトマネージャー
         */
        static SceneLight& Get()
        {
            static SceneLight instance;
            return instance;
        }

    private:
        SceneLight() = default;
        ~SceneLight() = default;
    };

} // namespace nsK2EngineLow