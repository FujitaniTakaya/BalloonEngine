/**
 * @file Transform.cpp
 * @brief 変換情報を保持するクラスの実装
 */
#include "stdafx.h"

#include "Transform.h"


namespace balloonEngineLow
{
    Transform::Transform(
        Vector3 position,
        Quaternion rotation,
        Vector3 scale
    )
        : m_position(position)
        , m_rotation(rotation)
        , m_scale(scale)
    {}


    Transform::~Transform()
    {}
} // namespace balloonEngineLow