#include "stdafx.h"

#include "Game.h"

#include "Camera/GameCamera.h"
#include "imgui.h"


namespace app
{
    Game::Game()
    {}


    Game::~Game()
    {}


    bool Game::Start()
    {
        // Load resources and set up your objects here (called once).

        m_animationClips[0].Load("Assets/animData/idle.tka");
        m_animationClips[0].SetLoopFlag(true);
        m_animationClips[1].Load("Assets/animData/run.tka");
        m_animationClips[1].SetLoopFlag(true);


        m_modelRender.Init(
            "Assets/modelData/unityChan.tkm",
            m_animationClips,
            2,
            EnModelUpAxis::enModelUpAxisY,
            true,
            true
        );
        Quaternion rot;
        rot.SetRotationDegY(180.0f);
        m_modelRender.SetRotation(rot);

        m_modelRender.PlayAnimation(0, 0.0f);

        const Vector3 modelPos = m_modelRender.GetTransform().m_position;


        m_bgModelRender.Init("Assets/modelData/ground.tkm", nullptr, 0, EnModelUpAxis::enModelUpAxisZ, true, false);
        m_bgModelRender.Update();

        m_gameCamera = std::make_unique<camera::GameCamera>();
        m_gameCamera->SetTargetPosition(modelPos);
        m_gameCamera->Start();

        SceneLight::Get().m_sceneLight.ambientLight.lightColor = { 0.5f, 0.5f, 0.5f };
        return true;
    }


    void Game::Update()
    {
        // Per-frame logic goes here.

        auto& light = SceneLight::Get().m_sceneLight;

#ifdef DEBUG
        ImGui::Begin("Light");

        if (ImGui::CollapsingHeader("Direction Light"))
        {
            ImGui::SliderFloat3("Direction", &light.directionLight.lightDir.x, -1.0f, 1.0f);
            ImGui::ColorEdit3("Color", &light.directionLight.lightColor.m_colorVec3.x);
            ImGui::SliderFloat("Shininess", &light.shininess, 1.0f, 200.0f);
            ImGui::SliderFloat("Spec Intensity", &light.specIntensity, 0.0f, 5.0f);
            ImGui::SliderFloat("Bias", &light.localBias, 0.000001f, 1.0f);
        }

        if (ImGui::CollapsingHeader("Ambient Light"))
        {
            ImGui::ColorEdit3("Ambient", &light.ambientLight.lightColor.m_colorVec3.x);
        }

        if (ImGui::CollapsingHeader("Point Lights"))
        {
            ImGui::SliderInt("PointLightNum", &light.usingPointLightNum, 0, LightingCB::MAX_POINT_LIGHT_NUM);
            for (int i = 0; i < light.usingPointLightNum; ++i)
            {
                ImGui::PushID(i);
                if (ImGui::TreeNode("", "Light %d", i))
                {
                    auto& it = light.pointLights.at(i);
                    ImGui::SliderFloat3(("Position"), &it.position.x, -1000.0f, 1000.0f);
                    ImGui::DragFloat(("Range"), &it.range, 5.0f, 10.0f, 2000.0f);
                    ImGui::ColorEdit3(("Color"), &it.lightColor.m_colorVec3.x);
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }

        if (ImGui::CollapsingHeader("Spot Lights"))
        {
            ImGui::SliderInt("SpotLightNum", &light.usingSpotLightNum, 0, LightingCB::MAX_SPOT_LIGHT_NUM);
            for (int i = 0; i < light.usingSpotLightNum; ++i)
            {
                ImGui::PushID(i);
                if (ImGui::TreeNode("", "Light %d", i))
                {
                    auto& it = light.spotLights.at(i);
                    ImGui::SliderFloat3(("Position"), &it.pointLight.position.x, -1000.0f, 1000.0f);
                    ImGui::DragFloat(("Range"), &it.pointLight.range, 5.0f, 10.0f, 2000.0f);
                    ImGui::ColorEdit3(("Color"), &it.pointLight.lightColor.m_colorVec3.x);
                    ImGui::SliderFloat3(("Direction"), &it.lightDir.x, -1.0f, 1.0f);
                    ImGui::SliderFloat(("Angle"), &it.angle, Math::DegToRad(1.0f), Math::DegToRad(90.0f));
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }

        auto& re = RenderingEngine::Get();

        if (ImGui::CollapsingHeader("Bloom"))
        {
            auto& bloomCB = re.GetBloomCB();
            ImGui::SliderFloat("BloomThreshold", &bloomCB.threshold, 1.0f, 3.0f);
            ImGui::Checkbox("Dual Blur", &re.SetDualBlurEnable());

            ImGui::SliderFloat("BloomIntensity", &re.GetBloomIntensity(), 0.0f, 3.0f);
        }



        ImGui::End();
#endif // DEBUG

        RenderingEngine::Get().SetScreenBlurPower(m_screenBlurPower);

        if (g_pad[0]->IsPress(enButtonA))
        {
            m_modelRender.PlayAnimation(1, 0.3f);
        }
        else
        {
            m_modelRender.PlayAnimation(0, 0.3f);
        }

        m_modelRender.Update();

        m_gameCamera->SetTargetPosition(m_modelRender.GetTransform().m_position);
        m_gameCamera->Update();
    }


    void Game::Render(RenderContext& rc)
    {
        // Your drawing code goes here.
        // K2EngineLow already cleared the screen to gray before this is called.

        m_modelRender.Draw(rc);
        m_bgModelRender.Draw(rc);
    }
} // namespace app