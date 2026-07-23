/**
 * @file GameCamera.cpp
 * @brief ゲームカメラクラスの実装
 */
#include "stdafx.h"

#include "GameCamera.h"


namespace app
{
    namespace camera
    {
        GameCamera::GameCamera()
            : m_toCameraPos(Vector3::Zero)
            , m_targetPosition(Vector3::Zero)
        {}


        GameCamera::~GameCamera()
        {}


        void GameCamera::Start()
        {
            // 注視点から視点までのベクトルを設定。
            m_toCameraPos.Set(0.0f, 125.0f, -250.0f);

            // カメラのニアクリップとファークリップを設定する。
            g_camera3D->SetNear(1.0f);
            g_camera3D->SetFar(10000.0f);
        }


        void GameCamera::Update()
        {
            // カメラを更新。
            // 注視点を計算する。
            Vector3 target = m_targetPosition;
            target.y += 80.0f;

            Vector3 toCameraPosOld = m_toCameraPos;
            // パッドの入力を使ってカメラを回す。
            float x = g_pad[0]->GetRStickXF();
            float y = g_pad[0]->GetRStickYF();
            // Y軸周りの回転
            Quaternion qRot;
            qRot.SetRotationDeg(Vector3::AxisY, 1.3f * x);
            qRot.Apply(m_toCameraPos);
            // X軸周りの回転。
            Vector3 axisX;
            axisX.Cross(Vector3::AxisY, m_toCameraPos);
            axisX.Normalize();
            qRot.SetRotationDeg(axisX, 1.3f * y);
            qRot.Apply(m_toCameraPos);



            Vector3 toPosDir = m_toCameraPos;
            toPosDir.Normalize();
            if (toPosDir.y < -0.2f)
            {
                // カメラが上向きすぎ。
                m_toCameraPos = toCameraPosOld;
            }
            else if (toPosDir.y > 0.9f)
            {
                // カメラが下向きすぎ。
                m_toCameraPos = toCameraPosOld;
            }

            // 視点を計算する。
            Vector3 pos = target + m_toCameraPos;
            // メインカメラに注視点と視点を設定する。
            g_camera3D->SetTarget(target);
            g_camera3D->SetPosition(pos);

            // カメラの更新。
            g_camera3D->Update();
        }


        void GameCamera::SetTargetPosition(const Vector3& position)
        {
            m_targetPosition = position;
        }
    } // namespace camera
} // namespace app