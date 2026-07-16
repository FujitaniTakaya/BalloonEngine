/**
 * @file SpriteRender.cpp
 * @brief スプライト描画クラスの実装
 */
#include "stdafx.h"

#include "SpriteRender.h"

#include <algorithm>
#include <cstdint>


namespace balloonEngine
{
    namespace
    {
        /** デフォルトのピボット */
        const Vector2 DEFAULT_PIVOT = {0.5f, 0.5f};
        /** ピボットの最小値 */
        constexpr float MIN_PIVOT = 0.0f;
        /** ピボットの最大値 */
        constexpr float MAX_PIVOT = 1.0f;
        /** 最大テクスチャ数 */
        constexpr uint8_t MAX_TEXTURE = 32;
        /** 最初のテクスチャのインデックス */
        constexpr uint8_t FIRST_TEXTURE = 0;
    } // namespace


    SpriteRender::SpriteRender()
        : m_mulColor(Vector4::White)
        , m_pivot(DEFAULT_PIVOT)
    {}


    SpriteRender::~SpriteRender()
    {}


    void SpriteRender::Init(
        const char* ddsFilePath,
        const uint32_t width,
        const uint32_t height,
        const char* fxFilePath
    )
    {
        m_spriteInitData.m_ddsFilePath.at(FIRST_TEXTURE) = ddsFilePath;
        m_spriteInitData.m_fxFilePath = fxFilePath;
        m_spriteInitData.m_width = width;
        m_spriteInitData.m_height = height;

        m_sprite.Init(m_spriteInitData);
    }


    void SpriteRender::Update()
    {
        m_sprite.Update(
            m_transform.m_position, m_transform.m_rotation, m_transform.m_scale, m_pivot
        );
        m_sprite.SetMulColor(m_mulColor);
    }


    void SpriteRender::Draw(RenderContext& rc)
    {
        m_sprite.Draw(rc);
    }


    //=======================================================================
    // トランスフォーム
    //=======================================================================
    void SpriteRender::SetTRS(
        const Vector3& position,
        const Quaternion& rotation,
        const Vector3& scale
    )
    {
        m_transform.m_position = position;
        m_transform.m_rotation = rotation;
        m_transform.m_scale = scale;
    }


    void SpriteRender::SetTRS(const balloonEngineLow::Transform& transform)
    {
        m_transform = transform;
    }


    void SpriteRender::SetPosition(const Vector3& position)
    {
        m_transform.m_position = position;
    }


    void SpriteRender::SetRotation(const Quaternion& rotation)
    {
        m_transform.m_rotation = rotation;
    }


    void SpriteRender::SetScale(const Vector3& scale)
    {
        m_transform.m_scale = scale;
    }


    const balloonEngineLow::Transform& SpriteRender::GetTransform() const
    {
        return m_transform;
    }


    //=======================================================================
    // カラー
    //=======================================================================
    void SpriteRender::SetMulColor(const Vector4& mulColor)
    {
        m_mulColor = mulColor;
    }


    const Vector4& SpriteRender::GetMulColor() const
    {
        return m_mulColor;
    }


    //=======================================================================
    // 基点
    //=======================================================================
    void SpriteRender::SetPivot(const Vector2& pivot)
    {
        m_pivot.x = std::clamp(pivot.x, MIN_PIVOT, MAX_PIVOT);
        m_pivot.y = std::clamp(pivot.y, MIN_PIVOT, MAX_PIVOT);
    }


    const Vector2& SpriteRender::GetPivot() const
    {
        return m_pivot;
    }
} // namespace balloonEngine
