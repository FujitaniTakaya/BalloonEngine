/**
 * @file ModelRender.cpp
 * @brief モデル描画クラスの実装
 */
#include "stdafx.h"
#include "ModelRender.h"

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
		const char* fxFilePath
	)
	{
		m_modelInitData.m_tkmFilePath = tkmFilePath;
		m_modelInitData.m_modelUpAxis = upAxis;
		m_modelInitData.m_fxFilePath = fxFilePath;

		m_modelInitData.m_expandConstantBuffer = balloonEngineLow::LightManager::Get().GetSceneLightAddress();
		m_modelInitData.m_expandConstantBufferSize = sizeof(balloonEngineLow::LightData);

		m_model.Init(m_modelInitData);
	}


	void ModelRender::Update()
	{
		m_model.UpdateWorldMatrix(
			m_transform.m_position,
			m_transform.m_rotation,
			m_transform.m_scale
		);
	}


	void ModelRender::Draw(RenderContext& rc)
	{
		m_model.Draw(rc);
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
}
