/**
 * @file LightCulling.cpp
 * @brief ライトカリングの実装
 */


#include "k2EngineLowPreCompile.h"

#include "LightCulling.h"


namespace nsK2EngineLow
{
    void LightCulling::Init()
    {
        m_lightCulling.LoadCS("Assets/shader/lightCulling.fx", "CSMain");
    }
} // namespace nsK2EngineLow