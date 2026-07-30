#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#pragma comment(lib, "xinput.lib")
#pragma comment(lib, "dxcompiler.lib")

// NOTE: このファイルは #include の順序に強く依存している(型の前方宣言に頼らず、
//       依存先ヘッダーが先にインクルードされている前提でコンパイルされるため)。
//       SortIncludes 等の自動整形で並び替えるとビルドが壊れるので、
//       ファイル末尾までフォーマッタの対象外にしている。
// clang-format off

#include <algorithm>
#include <array>
#include <atlbase.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <dxcapi.h>
#include <dxgi.h>
#include <dxgiformat.h>
#include <dxgitype.h>
#include <functional>
#include <list>
#include <map>
#include <math.h>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <Windows.h>
#include <winsock2.h> // Windows.h より前にインクルードしないと sockaddr 等が再定義されてビルドエラーになる。
#include <Xinput.h>


#include "DirectXTK/Inc/CommonStates.h"
#include "DirectXTK/Inc/SpriteBatch.h"
#include "DirectXTK/Inc/SpriteFont.h"

#include "Effekseer.h"
#include "EffekseerRendererDX12.h"

#include "graphics/d3dx12.h"

const int MAX_RENDERING_TARGET = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;	//�����_�����O�^�[�Q�b�g�̍ő吔�B

using namespace Microsoft::WRL;

#define USE_FPS_LIMITTER		// �L����FPS�ɏ����݂���B
static const int MAX_FPS = 60;	// �ő�FPS�BUSE_FPS_LIMITTER���L���ȂƂ��Ɏg�p�����B
#define ENABLE_DXR_ON_RTX_ONLY	// �L����DXR��RTX�V���[�Y�̂ݗL���B

#include "util/Noncopyable.h"
#include "dbg/MyAssert.h"
#include "dbg/MyLog.h"
#include "time/GameTime.h"
#include "math/Math.h"
#include "math/Vector.h"
#include "math/Matrix.h"

using namespace nsK2EngineLow;

#include "math/LightColor.h"
#include "math/Transform.h"

#include "k2EngineLow.h"
#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/IShaderResource.h"
#include "graphics/IUnorderAccessResource.h"
#include "graphics/GPUBuffer.h"
#include "graphics/Texture.h"
#include "graphics/RWStructuredBuffer.h"
#include "graphics/StructuredBuffer.h"
#include "graphics/Shader.h"
#include "graphics/PipelineState.h"
#include "graphics/ConstantBuffer.h"
#include "graphics/RootSignature.h"
#include "graphics/raytracing/Raytracing.h"
#include "graphics/GraphicsEngine.h"
#include "graphics/DescriptorHeap.h"
#include "graphics/RenderTarget.h"
#include "graphics/Animation.h"
#include "graphics/font/Font.h"

#include "graphics/RenderContext_inline.h"
#include "graphics/DescriptorHeap_inline.h"

#include "graphics/Sprite.h"
#include "graphics/Model.h"
#include "graphics/IRenderObject.h"
#include "graphics/Light.h"
#include "graphics/RenderingEngine.h"
#include "graphics/ModelRender.h"
#include "graphics/SpriteRender.h"
#include "HID/GamePad.h"

#include "graphics/GaussianBlur.h"

inline constexpr UINT FRAME_BUFFER_W = 1920;	// �t���[���o�b�t�@�̕��B
inline constexpr UINT FRAME_BUFFER_H = 1080;	// �t���[���o�b�t�@�̍����B
inline constexpr UINT UI_SPACE_WIDTH = 1920;	// UI��Ԃ̕��B
inline constexpr UINT UI_SPACE_HEIGHT = 1080;	// UI��Ԃ̍����B
inline constexpr int MAX_BONE = 512;	// �{�[���̍ő吔�B



#include "btBulletDynamicsCommon.h"
#include "BulletCollision\CollisionDispatch\btGhostObject.h"

#include "physics/RigidBody.h"
#include "physics/Physics.h"
#include "physics/MeshCollider.h"
#include "physics/SphereCollider.h"
#include "physics/BoxCollider.h"
#include "physics/CollisionAttr.h"
#include "physics/CharacterController.h"
#include "physics/PhysicsStaticObject.h"

#include "gameObject/GameObjectManager.h"
#include "gameObject/IGameObject.h"

#include <windows.h>
#include <xaudio2.h>
#include <x3daudio.h>
#include <xaudio2fx.h>


#include "graphics/effect/EffectEngine.h"
#include "sound/SoundSource.h"
#include "graphics/effect/EffectEmitter.h"
// clang-format on