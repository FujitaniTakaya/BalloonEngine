/**
 * @file LightColor.cpp
 * @brief ライトの色情報を保持するクラスの実装
 */
#include "stdafx.h"

#include "LightColor.h"


namespace balloonEngineLow
{
    // 静的メンバの実体定義
    const ColorVec3 LightColor::White = { 1.0f, 1.0f, 1.0f };
    const ColorVec3 LightColor::Red = { 1.0f, 0.0f, 0.0f };
    const ColorVec3 LightColor::Green = { 0.0f, 1.0f, 0.0f };
    const ColorVec3 LightColor::Blue = { 0.0f, 0.0f, 1.0f };
    const ColorVec3 LightColor::Yellow = { 1.0f, 1.0f, 0.0f };
    const ColorVec3 LightColor::Cyan = { 0.0f, 1.0f, 1.0f };
    const ColorVec3 LightColor::Magenta = { 1.0f, 0.0f, 1.0f };
    const ColorVec3 LightColor::Black = { 0.0f, 0.0f, 0.0f };
    const ColorVec3 LightColor::Gray = { 0.5f, 0.5f, 0.5f };


    LightColor::LightColor()
        : m_colorVec3(White)
        , m_colorVec4(g_vec4White)
    {}
} // namespace balloonEngineLow
