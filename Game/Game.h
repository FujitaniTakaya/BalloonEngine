#pragma once


// The starting point of your game.
// This is a plain IGameObject running on K2EngineLow only.
// Right now it draws nothing, so you will just see the gray clear color.
// Add your own rendering (a triangle, a sprite, a model...) step by step.
class Game : public IGameObject
{
public:
	Game() {}
	~Game() {}
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;

private:
	/** モデルの初期化データ */
	ModelInitData m_modelInitData;
	/** モデル */
	Model m_model;
};
