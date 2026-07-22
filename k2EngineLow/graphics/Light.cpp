#include "k2EngineLowPreCompile.h"

#include "Light.h"


namespace nsK2EngineLow
{
    DirectionLight::DirectionLight()
        : lightDir(
              0.0f,
              -1.0f,
              0.0f
          ) // ゼロベクトルは normalize で NaN になるため、真上からの光を初期値にする
        , pad(0.0f)
        , lightColor(LightColor::White)
    {}




    /***************************************/


    namespace
    {
        /** デフォルトの環境光の色 */
        static const ColorVec3 DEFAULT_AMBIENT_COLOR = { 0.3f, 0.3f, 0.3f };
    } // namespace


    AmbientLight::AmbientLight()
        : lightColor(DEFAULT_AMBIENT_COLOR)
    {}




    /***************************************/


    LightData::LightData()
        : directionLight()
        , ambientLight()
        , eyePosition(g_vec3Zero)
        , pad(0.0f)
        , LVP(Matrix::Identity)
    {}




    /***************************************/


    void SceneLight::Update()
    {
        // カメラ位置を毎フレーム反映(鏡面反射用)。
        // カメラの取得方法は自作エンジンの実装に合わせて置き換えること。
        m_sceneLight.eyePosition.Set(g_camera3D->GetPosition());
    }




    /***************************************/


    void SceneLight::SetLightDir(const Vector3& lightDir)
    {
        m_sceneLight.directionLight.lightDir.Set(lightDir);
    }


    void SceneLight::SetLightColor(const Vector4& lightColor)
    {
        m_sceneLight.directionLight.lightColor.m_colorVec3.Set(lightColor);
    }


    void SceneLight::SetAmbientColor(const Vector4& lightColor)
    {
        m_sceneLight.ambientLight.lightColor.m_colorVec3.Set(lightColor);
    }


    const LightData& SceneLight::GetSceneLight() const
    {
        return m_sceneLight;
    }


    LightData* SceneLight::GetAddress()
    {
        return &m_sceneLight;
    }


    void SceneLight::SetLightLVP(const Matrix& lvp)
    {
        m_sceneLight.LVP = lvp;
    }
} // namespace nsK2EngineLow