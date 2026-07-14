/**
 * @file Transform.h
 * @brief 変換情報を保持するクラスの宣言
 */
#pragma once


namespace balloonEngineLow
{
	/**
	 * @brief 変換情報を保持するクラス
	 */
	class Transform
	{
	public:
		Transform(
			Vector3 position = g_vec3Zero,
			Quaternion rotation = g_quatIdentity,
			Vector3 scale = g_vec3One
		);
		~Transform();


	public:
		/** 座標 */
		Vector3 m_position;
		/** 回転 */
		Quaternion m_rotation;
		/**	拡大 */
		Vector3 m_scale;
	};
}