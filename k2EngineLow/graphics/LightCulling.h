/**
 * @file LightCulling.h
 * @brief ライトカリングの定義
 */


namespace nsK2EngineLow
{
    class LightCulling
    {
    public:
        void Init();


    public:
        LightCulling();
        ~LightCulling();


    private:
        Shader m_lightCulling;
        RootSignature m_rootSignature;
        PipelineState m_pipelineState;
        DescriptorHeap m_descriptorHeap;
    };
} // namespace nsK2EngineLow