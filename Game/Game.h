#pragma once


namespace app
{
    //=======================================================================
    // 前方宣言
    //=======================================================================

    namespace camera
    {
        class GameCamera;
    } // namespace camera


    // The starting point of your game.
    // This is a plain IGameObject running on K2EngineLow only.
    // Right now it draws nothing, so you will just see the gray clear color.
    // Add your own rendering (a triangle, a sprite, a model...) step by step.
    class Game : public IGameObject
    {
    public:
        Game();
        ~Game();
        bool Start() override;
        void Update() override;
        void Render(RenderContext& rc) override;

    private:
        ModelRender m_modelRender;
        ModelRender m_bgModelRender;

        /** ゲームカメラ */
        std::unique_ptr<camera::GameCamera> m_gameCamera;


        AnimationClip m_animationClips[2];
    };
} // namespace app