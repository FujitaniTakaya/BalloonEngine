#pragma once
#include "balloonEngine/ModelRender.h"


enum class EnRenderTargetType
{
	Albedo,
	Normal,
	WorldPos,
	Max
};

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

	balloonEngine::ModelRender m_modelRender;


	SpriteInitData m_spriteInitData;
	Sprite m_sprite;


	std::array<RenderTarget, static_cast<size_t>(EnRenderTargetType::Max)> m_rts;
};
