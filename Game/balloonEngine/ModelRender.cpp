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
}