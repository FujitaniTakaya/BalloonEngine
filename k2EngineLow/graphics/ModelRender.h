/**
 * @file ModelRender.h
 * @brief モデル描画クラスの宣言
 */
#pragma once
#include "IRenderObject.h"
#include "math/Transform.h"


namespace nsK2EngineLow
{
    /**
     * @brief モデル描画クラス
     */
    class ModelRender : public IRenderObject
    {
    public:
        ModelRender();
        ~ModelRender() override;


    public:
        /**
         * @brief 初期化関数
         * @param tkmFilePath モデルアセットのファイルパス
         * @param isReceiveShadow 影を受けるかどうか
         * @param isCastShadow 影を落とすかどうか
         * @param upAxis モデルの上方向の軸
         * @param fxFilePath シェーダーアセットのファイルパス
         */
        void Init(
            const char* tkmFilePath,
            AnimationClip* animationClips = nullptr,
            const uint8_t animationClipNum = 0,
            const EnModelUpAxis upAxis = EnModelUpAxis::enModelUpAxisZ,
            const bool isReceiveShadow = false,
            const bool isCastShadow = false,
            const bool isDeferredRendering = false,
            const char* fxFilePath = "Assets/shader/model.fx"
        );


        /**
         * @brief 更新関数
         */
        void Update();


        /**
         * @brief 描画関数
         */
        void Draw(RenderContext& rc);


        //=======================================================================
        // トランスフォーム
        //=======================================================================
    public:
        /**
         * @brief 座標・回転・拡大を設定
         * @param position 座標
         * @param rotation 回転角度
         * @param scale 拡大率
         */
        void SetTRS(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
        /**
         * @brief 座標・回転・拡大を設定
         * @param transform トランスフォーム
         */
        void SetTRS(const Transform& transform);
        /**
         * @brief 座標を設定
         * @param position 座標
         */
        void SetPosition(const Vector3& position);
        /**
         * @brief 回転を設定
         * @param rotation 回転角度
         */
        void SetRotation(const Quaternion& rotation);
        /**
         * @brief 拡大を設定
         * @param scale 拡大率
         */
        void SetScale(const Vector3& scale);
        /**
         * @brief トランスフォームを取得
         * @return トランスフォーム
         */
        const Transform& GetTransform() const;


        //=======================================================================
        // モデルデータ
        //=======================================================================
    public:
        /**
         * @brief モデルデータを取得
         * @return モデルデータ
         */
        Model& GetModel() const;


        //=======================================================================
        // アニメーション
        //=======================================================================
    public:
        /**
         * @brief アニメーションを再生する
         * @param clipIndex 再生するアニメーションクリップのインデックス
         * @param interpolateTime インターポレーション時間
         */
        void PlayAnimation(const uint8_t clipIndex, const float interpolateTime = 0.0f);


        /**
         * @brief アニメーション再生速度を設定する
         * @param speed 再生速度
         */
        void SetAnimationSpeed(const float speed);


        //=======================================================================
        // ヘルパー
        //=======================================================================
    private:
        /**
         * @brief スケルトンを初期化する
         * @param tkmFilePath モデルアセットのファイルパス
         * @param animationClips アニメーションクリップの配列
         * @param animationClipNum アニメーションクリップの数
         */
        void InitSkeleton(const char* tkmFilePath, AnimationClip* animationClips, const uint8_t animationClipNum);


        /**
         * @brief シャドウを初期化する
         * @param tkmFilePath モデルアセットのファイルパス
         * @param modelInitData モデル初期化データ
         */
        void InitShadow(const char* tkmFilePath, ModelInitData& modelInitData);


    private:
        /** モデルのボーン */
        Skeleton m_skeleton;
        /** アニメーション */
        Animation m_animation;
        /** アニメーション再生速度 */
        float m_animationSpeed;
        /** アニメーションを渡されたか */
        bool m_isAnimated;


        /** モデルデータ */
        Model m_model;
        /** シャドウモデルデータ */
        Model m_shadowModel;
        /** トランスフォーム */
        Transform m_transform;


        /** 影を受けるかどうか */
        bool m_isReceiveShadow;
        /** 影を落とすかどうか */
        bool m_isCastShadow;


        /** デファードレンダリングするかどうか */
        bool m_isDeferred;
    };
} // namespace nsK2EngineLow