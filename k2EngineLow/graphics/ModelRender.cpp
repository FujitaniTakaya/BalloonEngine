/**
 * @file ModelRender.cpp
 * @brief モデル描画クラスの実装
 */
#include "k2EngineLowPreCompile.h"

#include "ModelRender.h"

#include "Light.h"
#include "RenderingEngine.h"


namespace nsK2EngineLow
{
    ModelRender::ModelRender()
    {}


    ModelRender::~ModelRender()
    {}


    void ModelRender::Init(
        const char* tkmFilePath,
        const bool isReceiveShadow,
        const bool isCastShadow,
        EnModelUpAxis upAxis,
        const bool isDeferredRendering,
        const char* fxFilePath
    )
    {
        ModelInitData modelInitData;
        modelInitData.m_tkmFilePath = tkmFilePath;
        modelInitData.m_modelUpAxis = upAxis;
        modelInitData.m_fxFilePath = fxFilePath;

        m_isReceiveShadow = isReceiveShadow;
        m_isCastShadow = isCastShadow;
        m_isDeferred = isDeferredRendering;


        //========================================================================
        // 影を落とす場合の処理
        //========================================================================
        if (m_isCastShadow)
        {
            ModelInitData shadowModelInitData;
            shadowModelInitData.m_tkmFilePath = tkmFilePath;
            shadowModelInitData.m_fxFilePath = "Assets/shader/drawShadowMap.fx";
            shadowModelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32_FLOAT;
            m_shadowModel.Init(shadowModelInitData);
        }
        //========================================================================
        // 影を受ける場合の処理
        //========================================================================
        if (m_isReceiveShadow)
        {
            modelInitData.m_expandShaderResoruceView[0] = &RenderingEngine::Get().GetShadowMapTexture();
        }


        //========================================================================
        // 遅延描画用の処理
        //========================================================================
        if (m_isDeferred)
        {
            modelInitData.m_psEntryPointFunc = "PSMainDeferred";
            modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
            modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
            modelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }

        modelInitData.m_expandConstantBuffer = SceneLight::Get().GetAddress();
        modelInitData.m_expandConstantBufferSize = sizeof(LightData);

        m_model.Init(modelInitData);
    }


    void ModelRender::Update()
    {
        m_model.UpdateWorldMatrix(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
        if (m_isCastShadow)
        {
            m_shadowModel.UpdateWorldMatrix(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
        }
    }


    void ModelRender::Draw(RenderContext& rc)
    {
        // モデル描画オブジェクトを登録する
        RenderingEngine::Get().Add3dObject(&m_model);
        // 遅延描画用の場合は、遅延描画用のオブジェクトリストに追加する
        if (m_isDeferred)
        {
            RenderingEngine::Get().AddDeferredRendering3dObject(&m_model);
        }
        if (m_isCastShadow)
        {
            RenderingEngine::Get().AddShadowCaster(&m_shadowModel);
        }
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


    void ModelRender::SetTRS(const Transform& transform)
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


    const Transform& ModelRender::GetTransform() const
    {
        return m_transform;
    }


    //=======================================================================
    // モデルデータ
    //=======================================================================
    Model& ModelRender::GetModel() const
    {
        return const_cast<Model&>(m_model);
    }
} // namespace nsK2EngineLow
