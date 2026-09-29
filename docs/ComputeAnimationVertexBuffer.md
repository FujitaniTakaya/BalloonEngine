# ComputeAnimationVertexBuffer 徹底解説

対象ファイル
- [k2EngineLow/graphics/ComputeAnimationVertexBuffer.h](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.h)
- [k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp)
- [Game/Assets/shader/preProcess/ComputeAnimationVertexBuffer.fx](../Game/Assets/shader/preProcess/ComputeAnimationVertexBuffer.fx)

このメモは、Phase8「TBR / LightCulling」でコンピュートシェーダのクラス設計を組み立てる際に、
「型」として真似できる既存実装（GPUスキニング）を自分の言葉で読み解くための解説です。
LightCulling自体の答えではなく、**コンピュートシェーダをラップするC++クラスの共通パターン**を
理解することが目的です。

---

## 1. 何をするクラスか

頂点シェーダで毎フレーム行っていたスキニング（ボーン変形）を、描画の前段階で
**コンピュートシェーダに肩代わりさせて1回だけ計算し、結果を頂点バッファとして保存しておく**
ためのクラスです。

- 入力: スキニング前の頂点データ（位置・法線・接線・従法線・ボーンインデックス・ボーンウェイト）
- 処理: ボーン行列を使って頂点をワールド空間へ変形
- 出力: 変形済みの頂点データ（`VertexBuffer` として、以降の通常描画パスでそのまま使える）

「CPUでスキニング計算する」でも「頂点シェーダで毎回計算する」でもなく、
「コンピュートシェーダで“事前計算”してキャッシュする」という設計です。
インスタンシング描画（同じモデルを複数体同時に描く）にも対応しており、
インスタンスごとに異なるワールド行列を適用できます。

---

## 2. コンピュートシェーダを使うC++クラスに共通する5点セット

このクラスは、DirectX12でコンピュートシェーダを動かすために必要な要素をすべて持っています。
LightCullingを含め、以後コンピュートシェーダを書くたびに毎回この5点が登場します。

| # | 要素 | このクラスでの実体 | 役割 |
|---|------|---------------------|------|
| 1 | RootSignature | `SMeshLocal::m_rootSignature` | シェーダから見えるCBV/SRV/UAVの「窓の数と並び」を定義 |
| 2 | PipelineState (Compute用) | `SMeshLocal::m_pipelineState` | コンパイル済みシェーダ＋RootSignatureをGPUにセットする実体 |
| 3 | DescriptorHeap | `m_descriptorHeap`（クラスに1個） | CBV/SRV/UAVの実データへのポインタ集合置き場 |
| 4 | 入出力バッファ | `StructuredBuffer` (SRV) / `RWStructuredBuffer` (UAV) | GPU上の配列データ本体 |
| 5 | Dispatch呼び出し | `Dispatch()` 内の `rc.Dispatch(x, y, z)` | 実際にシェーダをスレッドグループ単位で起動する命令 |

まずこの対応関係を頭に入れてから、以下の詳細を読むと理解しやすくなります。

---

## 3. メンバ変数の詳細

### 3.1 定数バッファ `CB_0`（[.h L79-83](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.h#L79-L83)）

```cpp
struct CB_0 {
    Matrix worldMatrix;     // ワールド行列
    int numVertex;          // 頂点数
    int outputOffset = 0;   // 出力オフセット（実際はDispatch()内でnumInstanceが入る）
};
```

HLSL側の `cbuffer cbParam : register(b0)` と1対1で対応します（[.fx L16-21](../Game/Assets/shader/preProcess/ComputeAnimationVertexBuffer.fx#L16-L21)）。

> ⚠️ 命名注意: C++側のフィールド名は `outputOffset` ですが、`Dispatch()` では
> `cb0.outputOffset = numInstance;`（[.cpp L211](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L211)）としてインスタンス数を渡しており、
> HLSL側では同じスロットを `numInstance` という名前で受けています（[.fx L20](../Game/Assets/shader/preProcess/ComputeAnimationVertexBuffer.fx#L20)）。
> 名前は違っても「b0の3番目のint」というメモリレイアウトが一致していればOK、という良い実例です
> （＝ cbuffer は名前ではなくバイトレイアウトで一致させる）。

### 3.2 メッシュ単位のリソース束 `SMeshLocal`（[.h L87-98](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.h#L87-L98)）

1つのtkmファイルは複数の「メッシュ」（部位）を持ちうるため、**メッシュごとに
コンピュートシェーダ一式を丸ごと1セット**用意します。

```cpp
struct SMeshLocal {
    ConstantBuffer m_cb0;
    RootSignature m_rootSignature;
    PipelineState m_pipelineState;
    VertexBuffer m_vertexBuffer;              // 変形前の頂点(CPU→GPUコピー元)
    VertexBuffer m_animatedVertexBuffer;      // 変形後の頂点(描画で使う実体)
    StructuredBuffer m_vertexBufferSB;        // ↑変形前をSRVとして読めるようにしたもの(t0)
    RWStructuredBuffer m_animatedVertexBufferRWSB; // 変形後を書き込むUAV(u0)
    std::unique_ptr<IndexBuffer[]> m_animatedIndexBufferArray;
    std::unique_ptr<int[]> m_originalIndexBufferSize;
    int m_numMaterial = 0;
};
```

ポイントは **`m_vertexBuffer`（元データ）と `m_vertexBufferSB`（それをラップしたSRV）が別オブジェクト**
であること、そして **`m_animatedVertexBufferRWSB`（UAV実体）と `m_animatedVertexBuffer`（頂点バッファとしての顔）が
同じメモリを指す2つの見え方**であることです。

```cpp
meshLocal.m_vertexBufferSB.Init(meshLocal.m_vertexBuffer, false);   // 既存バッファをSRV化
...
meshLocal.m_animatedVertexBufferRWSB.Init(vertexStride, numVertex * numInstance, nullptr, false); // UAVとして新規確保
meshLocal.m_animatedVertexBuffer.Init(meshLocal.m_animatedVertexBufferRWSB); // 同じ実体をVertexBufferとして被せる
```

つまり「1つのGPUバッファ」に対して「SRVとして読む」「UAVとして書く」「頂点バッファとして描画に使う」
という**複数の顔（ビュー）を使い分けている**のがこのクラスの肝です。

### 3.3 クラス共通のメンバ

- `m_boneMatricesStructureBuffer`（[.h L106](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.h#L106)）: 全ボーン行列の配列をSRV化したもの（t1）。全メッシュで共有。
- `m_descriptorHeap`（[.h L103](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.h#L103)）: 全メッシュ分のCBV/SRV/UAVをまとめて積む1つのヒープ。
- `m_boneMatrixArray`: ボーン行列配列の生ポインタ（CPU側、毎フレーム更新元）。
- `m_isFirstDispatch`: 初回だけリソースバリアを省略するためのフラグ（後述）。

---

## 4. `Init()` の詳細ステップ（[.cpp L14-148](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L14-L148)）

### Step A: tkmファイルロード & メッシュ配列確保

```cpp
m_tkmFile.Load(tkmFilePath, false, false, false);
m_numMesh = m_tkmFile.GetNumMesh();
m_meshArray = make_unique<SMeshLocal[]>(m_numMesh);
```

### Step B: メッシュごとのループ（`QueryMeshParts` のラムダ内、[L29-102](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L29-L102)）

1. **元頂点バッファ作成**
   ```cpp
   meshLocal.m_vertexBuffer.Init(vertexStride * numVertex, vertexStride);
   meshLocal.m_vertexBuffer.Copy((void*)&mesh.vertexBuffer[0]);
   meshLocal.m_vertexBufferSB.Init(meshLocal.m_vertexBuffer, false);
   ```
   CPU上のtkm頂点データをそのままGPUバッファへコピーし、SRVとして見えるようにする。

2. **インデックスバッファをインスタンス数だけ複製**（[L43-90](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L43-L90)）

   ここが最初に読むと戸惑うポイントです。なぜ複製するのか：

   - 出力頂点バッファ `m_animatedVertexBufferRWSB` は `numVertex * numInstance` 個分、
     **インスタンスごとに専用の頂点データ領域**を持つ1本の巨大なバッファです
     （`outputOffset = instanceNo * numVertex` で書き込み先をずらす、[.fx L75](../Game/Assets/shader/preProcess/ComputeAnimationVertexBuffer.fx#L75)）。
   - よって描画時のインデックスも、インスタンス2体目なら
     「頂点番号に `numVertex` を足したもの」を指さなければならない。
   - そのため、元のインデックス列を `numInstance` 回コピーしながら、
     コピーのたびに頂点番号へ `instanceNo * numVertex` を加算しています：

     ```cpp
     indexBuffer.Copy(
         (uint32_t*)&mesh.indexBuffer32Array[indexBufferNo].indices.at(0),
         numIndex,
         instanceNo * indexBufferSizeOne,  // 書き込み先バイトオフセット
         instanceNo * numVertex            // インデックス値に足すゲタ(offset)
     );
     ```

   - `m_originalIndexBufferSize` に「1インスタンス分の元のインデックス数」を保存しておき、
     `Dispatch()` の最後で `SetCount(indexCount)` により
     「今回何体分描画するか」に応じたインデックス数へ切り替えます（[L228-234](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L228-L234)）。
     つまりバッファ自体は最大インスタンス数ぶん確保しておき、実際の描画範囲だけ毎回書き換える設計です。

3. **出力用UAVバッファ確保**
   ```cpp
   meshLocal.m_animatedVertexBufferRWSB.Init(vertexStride, numVertex * numInstance, nullptr, false);
   meshLocal.m_animatedVertexBuffer.Init(meshLocal.m_animatedVertexBufferRWSB);
   ```

### Step C: シェーダのロード（シェーダバンク経由、[L104-112](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L104-L112)）

```cpp
m_shader = g_engine->GetShaderFromBank(fxFilePath, entryPoint);
if (m_shader == nullptr) {
    m_shader = new Shader;
    m_shader->LoadCS(fxFilePath, entryPoint);
    g_engine->RegistShaderToBank(fxFilePath, entryPoint, m_shader);
}
```
同じ `.fx`+エントリポイントの組み合わせなら、複数のモデルインスタンスがシェーダの
コンパイル結果を使い回す（キャッシュ）ための仕組みです。

### Step D: ボーン行列バッファ作成

```cpp
m_boneMatrixArray = boneMatrixArray;
m_boneMatricesStructureBuffer.Init(sizeof(Matrix), numBone, boneMatrixArray);
```

### Step E: メッシュごとに RootSignature と PipelineState を作成（[L119-144](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L119-L144)）

```cpp
mesh.m_rootSignature.Init(
    nullptr, 0,
    NUM_CB_ONE_DISPATCH,             // = 1 (b0)
    NUM_SRV_ONE_DISPATCH,            // = 3 (t0,t1,t2)
    NUM_UAV_ONE_DISPATCH,            // = 1 (u0)
    NUM_CB_ONE_DISPATCH  * meshNo,   // このメッシュのCBVがヒープの何番目から始まるか
    NUM_SRV_ONE_DISPATCH * meshNo,   // 同SRV
    NUM_UAV_ONE_DISPATCH * meshNo    // 同UAV
);
```

**なぜオフセットが必要か**: ディスクリプタヒープはクラス全体で1つしか作らず
（`m_descriptorHeap`）、そこに全メッシュ分のCBV/SRV/UAVを連結して詰め込みます。
メッシュ0のリソースはヒープの先頭、メッシュ1のリソースはその次…という配置になるため、
各メッシュのRootSignatureに「自分の担当範囲はヒープのどこから始まるか」を教えてやる必要が
あります。これにより「ヒープは1つ、RootSignatureとPSOはメッシュごとに複数」という構成が
成立します。

PSOは通常のグラフィックスPSOと違い、`D3D12_COMPUTE_PIPELINE_STATE_DESC` を使う点に注意
（[L137-142](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L137-L142)）。ラスタライザ設定やレンダーターゲット設定は一切なく、
`pRootSignature` と `CS`（コンピュートシェーダのバイトコード）のみで完結します。

### Step F: `CreateDescriptorHeaps()`（[.cpp L149-180](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L149-L180)）

```cpp
m_descriptorHeap.ResizeConstantBuffer(numCBTotal);
m_descriptorHeap.ResizeShaderResource(numSrvTotal);
m_descriptorHeap.ResizeUnorderAccessResource(numUavTotal);

for (int meshNo = 0; meshNo < m_numMesh; meshNo++) {
    auto& mesh = m_meshArray[meshNo];
    m_descriptorHeap.RegistShaderResource(srvNo,     mesh.m_vertexBufferSB);          // t0
    m_descriptorHeap.RegistShaderResource(srvNo + 1, m_boneMatricesStructureBuffer);  // t1
    if (worldMatrixArraySB) {
        m_descriptorHeap.RegistShaderResource(srvNo + 2, *worldMatrixArraySB);        // t2
    }
    m_descriptorHeap.RegistUnorderAccessResource(uavNo, mesh.m_animatedVertexBufferRWSB); // u0
    m_descriptorHeap.RegistConstantBuffer(cbNo, mesh.m_cb0);                          // b0

    srvNo += NUM_SRV_ONE_DISPATCH;
    uavNo += NUM_UAV_ONE_DISPATCH;
    cbNo  += NUM_CB_ONE_DISPATCH;
}
m_descriptorHeap.Commit();
```

先に「必要な総数ぶんヒープを確保（Resize）」してから、メッシュごとに登録位置をずらしながら
`Regist*` していく、という2段階です。最後の `Commit()` でGPUに実際に反映されます。

---

## 5. `Dispatch()` の詳細ステップ（[.cpp L182-237](../k2EngineLow/graphics/ComputeAnimationVertexBuffer.cpp#L182-L237)）

### Step A: リソースバリア（書き込み可能状態に戻す）

```cpp
if (!m_isFirstDispatch) {
    for (...) {
        rc.TransitionResourceState(
            m_meshArray[meshNo].m_animatedVertexBuffer.GetID3DResourceAddress(),
            D3D12_RESOURCE_STATE_GENERIC_READ,      // 前フレーム末で「読み取り用」にしていた
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS   // → 今回また書き込むためUAV状態に戻す
        );
    }
}
```

初回はまだ「読み取り用」に遷移させていない（初期状態のまま）ので、このバリアを
スキップするために `m_isFirstDispatch` フラグがあります。

### Step B: ボーン行列の更新

```cpp
if (m_boneMatrixArray) {
    m_boneMatricesStructureBuffer.Update(m_boneMatrixArray);
}
```
毎フレーム、CPU側で計算された最新のボーン行列をGPUバッファへ転送します。

### Step C: メッシュごとにDispatch

```cpp
for (int meshNo = 0; meshNo < m_numMesh; meshNo++) {
    auto& mesh = m_meshArray[meshNo];
    rc.SetComputeRootSignature(mesh.m_rootSignature);
    rc.SetComputeDescriptorHeap(m_descriptorHeap);
    rc.SetPipelineState(mesh.m_pipelineState);

    const int numVertex = mesh.m_vertexBuffer.GetNumVertex();
    const int numThreadGroup_x = (numVertex   / NUM_THREAD_X_IN_GROUP) + 1;  // 256
    const int numThreadGroup_y = (numInstance / NUM_THREAD_Y_IN_GROUP) + 1;  // 4

    CB_0 cb0;
    cb0.numVertex = numVertex;
    cb0.worldMatrix = worldMatrix;
    cb0.outputOffset = numInstance;
    mesh.m_cb0.CopyToVRAM(&cb0);

    rc.Dispatch(numThreadGroup_x, numThreadGroup_y, 1);
}
```

**スレッドグループ数の考え方**: このシェーダは「頂点×インスタンス」の2次元問題として
設計されています。HLSL側は `[numthreads(256, 4, 1)]`（[.fx L33](../Game/Assets/shader/preProcess/ComputeAnimationVertexBuffer.fx#L33)）、
つまり1グループが「頂点256個 × インスタンス4個」を担当します。よって必要なグループ数は

- X方向 = 頂点数 ÷ 256 を切り上げ（`+1` で簡易的に切り上げている。割り切れる場合は
  1グループ分無駄になるが、シェーダ側で `if (vertexNo < numVertex)` により範囲外は
  何もしないので安全）
- Y方向 = インスタンス数 ÷ 4 を切り上げ

という計算です。**「データ数をスレッド数で割って切り上げ、シェーダ側で範囲チェックする」**
というのは、コンピュートシェーダのDispatch数を決める際の定番パターンです。

### Step D: リソースバリア（読み取り可能に戻す）

```cpp
for (...) {
    rc.TransitionResourceState(
        mesh.m_animatedVertexBuffer.GetID3DResourceAddress(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_GENERIC_READ
    );
}
m_isFirstDispatch = false;
```
コンピュートシェーダで書き込んだ直後は「UAV状態」のままなので、後続の描画パスが
頂点バッファとして読めるように状態遷移させます。**UAV書き込み→バリア→読み取り、
という順序はコンピュートシェーダの結果を他のパスで使う際は必須**です。

### Step E: インデックス数の更新

```cpp
for (...) {
    for (int matNo = 0; matNo < mesh.m_numMaterial; matNo++) {
        int indexCount = mesh.m_originalIndexBufferSize[matNo] * numInstance;
        mesh.m_animatedIndexBufferArray[matNo].SetCount(indexCount);
    }
}
```
今回実際に描画するインスタンス数に応じて、使用するインデックス数を確定させます。

---

## 6. シェーダ側の詳細（[ComputeAnimationVertexBuffer.fx](../Game/Assets/shader/preProcess/ComputeAnimationVertexBuffer.fx)）

### 6.1 リソース宣言

```hlsl
StructuredBuffer<SVertex>   g_iputVertexBuffer  : register(t0); // 変形前頂点
StructuredBuffer<float4x4>  g_boneMatrix        : register(t1); // ボーン行列
StructuredBuffer<float4x4>  g_worldMatrixArray  : register(t2); // インスタンスごとのワールド行列
RWStructuredBuffer<SVertex> g_outputVertexBuffer : register(u0); // 変形後頂点(出力)
```

C++側の `RegistShaderResource(srvNo, ...)` / `RegistUnorderAccessResource(uavNo, ...)` の
登録順（t0, t1, t2, u0）と完全に一致している必要があります。

### 6.2 スレッドIDの解釈

```hlsl
[numthreads(256, 4, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID, ...)
{
    int vertexNo   = dispatchThreadId.x;  // このスレッドが担当する頂点番号
    int instanceNo = dispatchThreadId.y;  // このスレッドが担当するインスタンス番号
    if (vertexNo < numVertex && instanceNo < numInstance) { ... }
```

「1スレッド = 1頂点 × 1インスタンス」という割り当てです。`SV_DispatchThreadID` は
グループID×グループサイズ+グループ内スレッドIDで自動計算される、Dispatch全体を通した
通し番号だと考えるとよいです。

### 6.3 スキニング計算

```hlsl
if (inVertex.skinWeights[0] > 0.0f) {
    float4x4 skinMatrix = 0;
    float w = 0.0f;
    [unroll]
    for (int i = 0; i < 3; i++) {
        skinMatrix += g_boneMatrix[inVertex.indices[i]] * inVertex.skinWeights[i];
        w += inVertex.skinWeights[i];
    }
    skinMatrix += g_boneMatrix[inVertex.indices[3]] * (1.0f - w);
    ...
}
```
最大4本のボーンからウェイト付き平均でスキン行列を作る、一般的な線形ブレンドスキニング(LBS)です。
4本目のウェイトは「1.0 - 他3本の合計」として求めており、ウェイトデータの合計が1になるよう
正規化されている前提です。

### 6.4 単体描画とインスタンシング描画の分岐

```hlsl
if (numInstance == 1) {
    // 単体描画: CPU側で既にワールド変換されたスキン行列をそのまま使う
    worldMatrixLocal = skinMatrix;
} else {
    // インスタンシング: スキン行列はモデル空間のままなので、ここでワールド行列を掛ける
    worldMatrixLocal = mul(g_worldMatrixArray[instanceNo], skinMatrix);
}
```
スキンなし側も同様の分岐があります。**「1体描画なら呼び出し側が最終行列を用意する」
「複数体なら行列配列から都度引いてこのシェーダ内で合成する」**という設計であり、
コメントにある通りCPU側の実装差（1体用のパスと合わせるための最適化）に注意が必要です。

### 6.5 出力

```hlsl
int outputOffset = instanceNo * numVertex;
g_outputVertexBuffer[vertexNo + outputOffset] = inVertex;
g_outputVertexBuffer[vertexNo + outputOffset].pos      = mul(worldMatrixLocal, float4(inVertex.pos, 1.0f));
g_outputVertexBuffer[vertexNo + outputOffset].normal   = normalize(mul(worldMatrixLocal, inVertex.normal));
g_outputVertexBuffer[vertexNo + outputOffset].tangent  = normalize(mul(worldMatrixLocal, inVertex.tangent));
g_outputVertexBuffer[vertexNo + outputOffset].binormal = normalize(mul(worldMatrixLocal, inVertex.binormal));
```
まず `inVertex` を丸ごとコピーしてUV等の非変形データを引き継ぎ、その後
位置・法線・接線・従法線だけを上書きしています。`outputOffset = instanceNo * numVertex` が、
C++側でインデックスバッファに足していたゲタ（`instanceNo * numVertex`）とちょうど対応しています。

---

## 7. 全体データフロー図（テキスト）

```
[tkmファイル]
    │ Load
    ▼
[CPU: mesh.vertexBuffer]  [CPU: boneMatrixArray(毎フレーム更新)]
    │ Copy                    │ Update
    ▼                         ▼
[GPU: m_vertexBuffer]    [GPU: m_boneMatricesStructureBuffer]
    │ SRV化                    │ SRV(t1)
    ▼                         │
[t0: m_vertexBufferSB] ───────┤
                               │
[t2: worldMatrixArraySB(任意)]┤ (インスタンシング時)
                               │
                        ┌──────▼──────┐
                        │  CSMain      │  (コンピュートシェーダ)
                        │ 1頂点×1インスタンス/スレッド
                        └──────┬──────┘
                               │ 書き込み
                               ▼
                [u0: m_animatedVertexBufferRWSB]
                               │ 同じ実体をVertexBufferとして解釈
                               ▼
                  [m_animatedVertexBuffer] ── リソースバリア(UAV→GENERIC_READ) ──▶ 通常描画パスで使用
```

---

## 8. LightCulling実装時に意識すべき「型」の対応（概念のみ・答えではない）

このクラスが確立している設計パターンのうち、コンピュートシェーダを使う実装であれば
再利用できる考え方を挙げます（具体的なLightCullingのコードはここには書きません。
自分で `RootSignature` / `PipelineState` / `DescriptorHeap` / `StructuredBuffer` /
`RWStructuredBuffer` の各Initを呼び出しながら組み立ててみてください）。

- **「入力SRV → コンピュートシェーダ → 出力UAV」の3段構成**は同じ形になるはず。
  LightCullingなら「入力: ライト配列SB」「出力: タイルごとのライトインデックスを書く
  RWStructuredBuffer」という対応になる。
- **RootSignatureのCBV/SRV/UAV数とレジスタオフセット**は、このクラスでは
  「メッシュ数ぶん」複製していたが、LightCullingは基本1回のDispatchで済むはずなので
  もっとシンプルになる（オフセット計算そのものは不要かもしれない）。
- **Dispatchのグループ数計算**は「対象データ数 ÷ numthreads を切り上げ」という考え方が
  そのまま使える。LightCullingでは「対象データ数」が頂点数ではなく
  **画面のタイル数（幅方向・高さ方向）**になる。
- **リソースバリア（UAV⇔GENERIC_READ）**は、コンピュートシェーダの結果を別のパス
  （LightCullingなら次のライティングパス）で読む場合、同様に必要になる。
- **cbufferのバイトレイアウト一致**（C++の`CB_0`とHLSLの`cbParam`が同じ並び）は
  今後も繰り返し注意すべき点（[[k2enginelow-guide-conventions]] にある16バイトアライン
  の罠と合わせて意識する）。

以上が `ComputeAnimationVertexBuffer` の全体像です。次にLightCullingのクラス設計
（メンバ変数・Initの引数・Dispatchのシグネチャ）を考える際は、まず「入力データは何か」
「出力データは何か」「1スレッドは何を担当するか」の3点を自分の言葉で書き出してみることを
おすすめします。
