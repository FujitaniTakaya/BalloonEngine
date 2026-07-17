/**
 * @file ModelRender.cpp
 * @brief モデル描画クラスの実装
 */
#include "stdafx.h"

#include "ModelRender.h"

#include "RenderingEngine.h"
#include "balloonEngineLow/Light.h"


namespace balloonEngine
{
    ModelRender::ModelRender()
    {}


    ModelRender::~ModelRender()
    {}


    void ModelRender::Init(
        const char* tkmFilePath,
        EnModelUpAxis upAxis,
        const bool isDeferredRendering,
        const char* fxFilePath
    )
    {
        ModelInitData modelInitData;
        modelInitData.m_tkmFilePath = tkmFilePath;
        modelInitData.m_modelUpAxis = upAxis;
        modelInitData.m_fxFilePath = fxFilePath;

        if (isDeferredRendering)
        {
            modelInitData.m_psEntryPointFunc = "PSMainDeferred";
            modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
            modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
            modelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }

        modelInitData.m_expandConstantBuffer = balloonEngineLow::SceneLight::Get().GetAddress();
        modelInitData.m_expandConstantBufferSize = sizeof(balloonEngineLow::LightData);

        m_model.Init(modelInitData);
    }


    void ModelRender::Update()
    {
        m_model.UpdateWorldMatrix(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
    }


    void ModelRender::Draw(RenderContext& rc)
    {
        // 遅延描画用のモデル描画オブジェクトとして登録する
        RenderingEngine::Get().Add3dObject(this);
    }


    //=======================================================================
    // トランスフォーム
    //=======================================================================
    void ModelRender::SetTRS(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
    {
        m_transform.m_position = position;
        m_transform.m_rotation = rotation;
        m_transform.m_scale = scale;
    }


    void ModelRender::SetTRS(const balloonEngineLow::Transform& transform)
    {
        m_transform = transform;
    }


    void ModelRender::SetPosition(const Vector3& position)
    {
        m_transform.m_position = position;
    }


    void ModelRender::SetRotation(const Quaternion& rotation)
    {
        m_transform.m_rotation = rotation;
    }


    void ModelRender::SetScale(const Vector3& scale)
    {
        m_transform.m_scale = scale;
    }


    const balloonEngineLow::Transform& ModelRender::GetTransform() const
    {
        return m_transform;
    }


    Model& ModelRender::GetModel() const
    {
        return const_cast<Model&>(m_model);
    }
} // namespace balloonEngine
