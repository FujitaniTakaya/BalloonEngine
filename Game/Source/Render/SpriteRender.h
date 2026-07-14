/**
 * @file SpriteRender.h
 * @brief スプライト描画クラスの実装
 */
#pragma once
#include "Source/Core/Transform.h"


namespace BalloonEngine
{
	/**
	 * @brief スプライト描画クラス
	 */
	class SpriteRender
	{
	public:
		SpriteRender();
		~SpriteRender();


	public:
		/**
		 * @brief 初期化関数
		 * @param ddsFilePath 画像アセットのファイルパス
		 * @param width 幅
		 * @param height 高さ
		 * @param fxFilePath シェーダーアセットのファイルパス
		 */
		void Init(
			const char* ddsFilePath,
			const uint32_t width,
			const uint32_t height,
			const char* fxFilePath = "Assets/shader/sprite.fx"
		);


		/**
		 * @更新関数
		 */
		void Update();


		/**
		 * @描画関数
		 */
		void Draw(RenderContext& rc);


		//=======================================================================
		// トランスフォーム
		//=======================================================================
	public:
		/**
		 * @brief 座標・回転・拡大を設定
		 * @param position 座標
		 * @param rotation 回転角度
		 * @param scale 拡大率
		 */
		inline void SetTRS(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
		{
			m_transform.m_position = position;
			m_transform.m_rotation = rotation;
			m_transform.m_scale = scale;
		}
		/**
		 * @brief 座標・回転・拡大を設定
		 * @param transform トランスフォーム
		 */
		inline void SetTRS(const app::core::Transform& transform)
		{
			m_transform = transform;
		}
		/**
		 * @brief 座標を設定
		 * @param position 座標
		 */
		inline void SetPosition(const Vector3& position)
		{
			m_transform.m_position = position;
		}
		/**
		 * @brief 回転を設定
		 * @param rotation 回転角度
		 */
		inline void SetRotation(const Quaternion& rotation)
		{
			m_transform.m_rotation = rotation;
		}
		/**
		 * @brief 拡大を設定
		 * @param scale 拡大率
		 */
		inline void SetScale(const Vector3& scale)
		{
			m_transform.m_scale = scale;
		}
		/**
		 * @brief トランスフォームを取得
		 * @return トランスフォーム
		 */
		inline const app::core::Transform& GetTransform() const
		{
			return m_transform;
		}


		//=======================================================================
		// カラー
		//=======================================================================
	public:
		/**
		 * @brief 乗算色を設定
		 * @param mulColor 乗算色
		 */
		inline void SetMulColor(const Vector4& mulColor)
		{
			m_mulColor = mulColor;
		}
		/**
		 * @brief 乗算色を取得
		 * @return 乗算色
		 */
		inline const Vector4& GetMulColor() const
		{
			return m_mulColor;
		}


		//=======================================================================
		// 基点
		//=======================================================================
	public:
		/**
		 * @brief 基点を設定
		 * @param pivot 基点
		 */
		void SetPivot(const Vector2& pivot);
		/**
		 * @brief 基点を取得
		 * @return 基点
		 */
		inline const Vector2& GetPivot() const
		{
			return m_pivot;
		}


	private:
		/** スプライトのイニットデータ */
		SpriteInitData m_spriteInitData;
		/** スプライトデータ */
		Sprite m_sprite;
		/** トランスフォーム */
		app::core::Transform m_transform;
		/** 乗算色 */
		Vector4 m_mulColor;
		/** 基点 */
		Vector2 m_pivot;
	};
}