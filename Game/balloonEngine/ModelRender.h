#pragma once
#include "balloonEngineLow/Transform.h"


namespace balloonEngine
{
	/**
	 * @brief モデル描画クラス
	 */
	class ModelRender
	{
	public:
		ModelRender();
		~ModelRender();


	public:
		/**
		 * @brief 初期化関数
		 * @param ddsFilePath 画像アセットのファイルパス
		 * @param width 幅
		 * @param height 高さ
		 * @param fxFilePath シェーダーアセットのファイルパス
		 */
		void Init(
			const char* tkmFilePath,
			EnModelUpAxis upAxis = EnModelUpAxis::enModelUpAxisY,
			const char* fxFilePath = "Assets/shader/model.fx"
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
		 * @brief トランスフォームを設定
		 * @param transform トランスフォーム
		 */
		inline void SetTransform(const balloonEngineLow::Transform& transform)
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
		void SetScale(const Vector3& scale)
		{
			m_transform.m_scale = scale;
		}
		/**
		 * @brief トランスフォームを取得
		 * @return トランスフォーム
		 */
		const balloonEngineLow::Transform& GetTransform() const
		{
			return m_transform;
		}
	private:
		/** トランスフォーム */
		balloonEngineLow::Transform m_transform;
		/** モデル初期化データ */
		ModelInitData m_modelInitData;
		/** モデルデータ */
		Model m_model;
	};
}