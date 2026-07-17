/**
 * @file LightColor.h
 * @brief ライトの色情報を保持するクラスの宣言
 */
#pragma once


namespace balloonEngineLow
{
    /**
     * @brief ライトの色情報を保持するクラス(3次元ベクトル版)
     * @details 定数バッファ転送用にHLSLのfloat3のパッキング規則(16バイト境界)を
     *          考慮し、パディングを含めて16バイトになるように定義している。
     */
    class ColorVec3
    {
    public:
        ColorVec3() = default;

        /**
         * @brief コンストラクタ
         * @param r 赤成分(0.0f～1.0f)
         * @param g 緑成分(0.0f～1.0f)
         * @param b 青成分(0.0f～1.0f)
         */
        ColorVec3(float r, float g, float b)
            : m_colorVec3(r, g, b)
        {}

        /**
         * @brief コンストラクタ
         * @param color カラー
         */
        explicit ColorVec3(const Vector3& color)
            : m_colorVec3(color)
            , m_pad(0.0f)
        {}

        /**
         * @brief Vector4への変換
         * @param w w成分(デフォルトは1.0f)
         * @return 変換後のVector4
         */
        Vector4 ToVector4(float w = 1.0f) const
        {
            return Vector4(m_colorVec3.x, m_colorVec3.y, m_colorVec3.z, w);
        }

    public:
        /** カラー */
        Vector3 m_colorVec3;
        /** HLSL側float3の16バイト境界に合わせるためのパディング */
        float m_pad;
    };



    /**
     * @brief ライトの色情報を保持するクラス
     */
    class LightColor
    {
    private:
        using ColorVec4 = Vector4;


    public:
        static const ColorVec3 White;
        static const ColorVec3 Red;
        static const ColorVec3 Green;
        static const ColorVec3 Blue;
        static const ColorVec3 Yellow;
        static const ColorVec3 Cyan;
        static const ColorVec3 Magenta;
        static const ColorVec3 Black;
        static const ColorVec3 Gray;


    public:
        LightColor();
        ~LightColor() = default;

    public:
        /** 3次元ライトの色 */
        ColorVec3 m_colorVec3;
        /** 4次元ライトの色 */
        ColorVec4 m_colorVec4;
    };
} // namespace balloonEngineLow
