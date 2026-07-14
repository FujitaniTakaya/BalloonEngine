/**
 * @file SpriteRender.cpp
 * @brief スプライト描画クラスの実装
 */
#include "stdafx.h"
#include "SpriteRender.h"


namespace BalloonEngine
{
	namespace
	{
		/** デフォルトのピボット */
		static const Vector2 DEFAULT_PIVOT = { 0.5f, 0.5f };
		/** ピボットの最小値 */
		static constexpr float MIN_PIVOT = 0.0f;
		/** ピボットの最大値 */
		static constexpr float MAX_PIVOT = 1.0f;
	}


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
		m_spriteInitData.m_ddsFilePath.at(0) = ddsFilePath;
		m_spriteInitData.m_fxFilePath = fxFilePath;
		m_spriteInitData.m_width = width;
		m_spriteInitData.m_height = height;

		m_sprite.Init(m_spriteInitData);
	}


	void SpriteRender::Update()
	{
		m_sprite.Update(
			m_transform.m_position,
			m_transform.m_rotation,
			m_transform.m_scale,
			m_pivot
		);
		m_sprite.SetMulColor(m_mulColor);
	}


	void SpriteRender::Draw(RenderContext& rc)
	{
		m_sprite.Draw(rc);
	}


	void SpriteRender::SetPivot(const Vector2& pivot)
	{
		m_pivot.x = std::clamp(pivot.x, MIN_PIVOT, MAX_PIVOT);
		m_pivot.y = std::clamp(pivot.y, MIN_PIVOT, MAX_PIVOT);
	}

}