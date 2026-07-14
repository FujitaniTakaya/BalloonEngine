#include "stdafx.h"
#include "Light.h"


namespace balloonEngineLow
{
	DirectionLight::DirectionLight()
		: lightDir(0.0f, -1.0f, 0.0f)	// ゼロベクトルは normalize で NaN になるため、真上からの光を初期値にする
		, pad(0.0f)
		, lightColor(g_vec4White)
	{}




	/***************************************/


	namespace
	{
		/** デフォルトの環境光の色 */
		static const Vector4 DEFAULT_AMBIENT_COLOR = { 0.3f, 0.3f, 0.3f, 1.0f };
	}


	LightData::LightData()
		: directionLight()
		, ambientColor(DEFAULT_AMBIENT_COLOR)
		, eyePosition(g_vec3Zero)
		, pad(0.0f)
	{}




	/***************************************/


	void LightManager::Update()
	{
		// カメラ位置を毎フレーム反映(鏡面反射用)。
		// カメラの取得方法は自作エンジンの実装に合わせて置き換えること。
		m_sceneLight.eyePosition.Set(g_camera3D->GetPosition());
	}
}