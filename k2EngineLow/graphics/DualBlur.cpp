/**
 * @file DualBlur.cpp
 * @brief デュアルブラーの実装
 */
#include "k2EngineLowPreCompile.h"

#include "DualBlur.h"


namespace nsK2EngineLow
{
    void DualBlur::Init(Texture* originalTexture)
    {
        auto CreateRT = [](RenderTarget& rt, const int w, const int h) {
            rt.Create(w, h, 1, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
        };

        int w = FRAME_BUFFER_W;
        int h = FRAME_BUFFER_H;

        for (int i = 0; i < NUM_DOWN; ++i)
        {
            w /= 2;
            h /= 2;

            CreateRT(m_downBlur.at(i).rt, w, h);
        }

        for (int i = 0; i < NUM_UP; ++i)
        {
            w *= 2;
            h *= 2;

            CreateRT(m_upBlur.at(i).rt, w, h);
        }


        Texture* src = originalTexture;
        for (int i = 0; i < NUM_DOWN; ++i)
        {
            InitSprite(m_downBlur.at(i).sprite, src, "Assets/shader/bloom/dualBlurDown.fx", m_downBlur.at(i).rt);
            src = &m_downBlur.at(i).rt.GetRenderTargetTexture();
        }

        for (int i = 0; i < NUM_UP; ++i)
        {
            InitSprite(m_upBlur.at(i).sprite, src, "Assets/shader/bloom/dualBlurUp.fx", m_upBlur.at(i).rt);
            src = &m_upBlur.at(i).rt.GetRenderTargetTexture();
        }
    }


    void DualBlur::InitSprite(
        Sprite& sprite,
        Texture* srcTexture,
        const char* fxFilePath,
        RenderTarget& targetRT
    )
    {
        SpriteInitData initData;
        initData.m_textures[0] = srcTexture;
        initData.m_fxFilePath = fxFilePath;
        initData.m_width = targetRT.GetWidth();
        initData.m_height = targetRT.GetHeight();
        sprite.Init(initData);
    }


    void DualBlur::ExecuteOnGPU(RenderContext& rc)
    {
        for (int i = 0; i < NUM_DOWN; ++i)
        {
            ExecuteBlur(rc, m_downBlur.at(i));
        }

        for (int i = 0; i < NUM_UP; ++i)
        {
            ExecuteBlur(rc, m_upBlur.at(i));
        }
    }


    void DualBlur::ExecuteBlur(RenderContext& rc, BlurData& blurData)
    {
        rc.WaitUntilFinishDrawingToRenderTarget(blurData.rt);
        rc.SetRenderTargetAndViewport(blurData.rt);
        blurData.sprite.Update(Vector3::Zero, Quaternion::Identity, Vector3::One);
        blurData.sprite.Draw(rc);
        rc.WaitUntilFinishDrawingToRenderTarget(blurData.rt);
    }


    Texture& DualBlur::GetResultTexture()
    {
        return m_upBlur.at(NUM_UP - 1).rt.GetRenderTargetTexture();
    }
} // namespace nsK2EngineLow