/**
 * @file ModelRender.h
 * @brief モデル描画クラスの宣言
 */
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
		 * @param tkmFilePath モデルアセットのファイルパス
		 * @param upAxis モデルの上方向の軸
		 * @param fxFilePath シェーダーアセットのファイルパス
		 */
		void Init(
			const char* tkmFilePath,
			EnModelUpAxis upAxis = EnModelUpAxis::enModelUpAxisY,
			const char* fxFilePath = "Assets/shader/model.fx"
		);


		/**
		 * @brief 更新関数
		 */
		void Update();


		/**
		 * @brief 描画関数
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
		void SetTRS(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
		/**
		 * @brief 座標・回転・拡大を設定
		 * @param transform トランスフォーム
		 */
		void SetTRS(const balloonEngineLow::Transform& transform);
		/**
		 * @brief 座標を設定
		 * @param position 座標
		 */
		void SetPosition(const Vector3& position);
		/**
		 * @brief 回転を設定
		 * @param rotation 回転角度
		 */
		void SetRotation(const Quaternion& rotation);
		/**
		 * @brief 拡大を設定
		 * @param scale 拡大率
		 */
		void SetScale(const Vector3& scale);
		/**
		 * @brief トランスフォームを取得
		 * @return トランスフォーム
		 */
		const balloonEngineLow::Transform& GetTransform() const;


	private:
		/** モデル初期化データ */
		ModelInitData m_modelInitData;
		/** モデルデータ */
		Model m_model;
		/** トランスフォーム */
		balloonEngineLow::Transform m_transform;
	};
}