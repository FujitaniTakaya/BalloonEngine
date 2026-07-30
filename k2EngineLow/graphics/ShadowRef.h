/**
 * @file ShadowRef.h
 * @brief シャドウマップの枚数・スロットの意味に関する定数。
 * @note  Light.h と RenderingEngine.h の両方から参照されるため、
 *        余計な依存を持ち込まないよう、この定数専用の軽量ヘッダーに分離している。
 */
#pragma once


namespace nsK2EngineLow
{
    /**
     * @brief シャドウマップの各スロットが、どのライトに対応するか。
     */
    enum class EnShadowLightType : int
    {
        Directional = 0,
        Spot = 1,
        Num,
    };

    /** シャドウマップの最大数 */
    static constexpr int MAX_SHADOW_NUM = static_cast<int>(EnShadowLightType::Num);
} // namespace nsK2EngineLow
