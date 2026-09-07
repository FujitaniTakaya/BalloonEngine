/**
 * @file ShadowRef.h
 * @brief シャドウマップの枚数に関する定数。
 * @note  Light.h と RenderingEngine.h の両方から参照されるため、
 *        余計な依存を持ち込まないよう、この定数専用の軽量ヘッダーに分離している。
 */
#pragma once


namespace nsK2EngineLow
{
    /**
     * @brief シャドウマップの枚数。
     * @note  ディレクションライトの最大数と一致する。各ディレクションライトが
     *        自分専用のシャドウマップに影を1枚落とす(ライト i → シャドウマップ i)。
     *        LightingCB::MAX_DIRECTION_LIGHT_NUM と必ず一致させること。
     */
    static constexpr int NUM_SHADOW_MAP = 4;

    /** シャドウマップの最大数(ディレクションライトの最大数と一致) */
    static constexpr int MAX_SHADOW_NUM = NUM_SHADOW_MAP;
} // namespace nsK2EngineLow
