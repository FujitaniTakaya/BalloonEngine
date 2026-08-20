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
     * @brief ポイントライト
     */
    struct PointLight
    {
        /** ライトの位置 */
        Vector3 position;
        /** ライトの範囲 */
        float range;
        /** ライトの色 */
        ColorVec3 lightColor;


        PointLight();
        ~PointLight() = default;
    };




    /**********************************************/


    struct SpotLight
    {
        PointLight pointLight;
        Vector3 lightDir;
        float angle;


        SpotLight();
        ~SpotLight() = default;
    };




    /**********************************************/


    /**
     * @brief ライトデータ(model.fx の LightCb と1:1対応)
     * @note  ここを変更したらシェーダー側の cbuffer も必ず変更すること。
     */
    struct LightingCB
    {
        /** ディレクションライト */
        DirectionLight directionLight;
        /** 環境光の色 */
        AmbientLight ambientLight;
        /** ポイントライトの最大数 */
        static constexpr int MAX_POINT_LIGHT_NUM = 4;
        /** スポットライトの最大数 */
        static constexpr int MAX_SPOT_LIGHT_NUM = 4;
        /** シャドウマップの最大数 */
        static constexpr int MAX_SHADOW_NUM = 2;
        /** 使用するポイントライトの数 */
        int usingPointLightNum;
        /** 使用するスポットライトの数 */
        int usingSpotLightNum;
        /** パディング */
        int pad1[2];
        /** ポイントライト */
        std::array<PointLight, MAX_POINT_LIGHT_NUM> pointLights;
        /** スポットライト */
        std::array<SpotLight, MAX_SPOT_LIGHT_NUM> spotLights;
        /** 視点(カメラ)位置。鏡面反射で使用 */
        Vector3 eyePosition;
        /** パディング */
        float pad2;
        /** 鏡面反射の強さ */
        float shininess;
        /** ローカルバイアス */
        float localBias;
        /** スペキュラの強度倍率 */
        float specIntensity;
        /** パディング */
        float pad3;
        /** ライトカメラから見た位置への変換行列 */
        std::array<Matrix, MAX_SHADOW_NUM> LVP;


        LightingCB();
        ~LightingCB() = default;
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
        /** シーンライト */
        LightingCB m_sceneLight;


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