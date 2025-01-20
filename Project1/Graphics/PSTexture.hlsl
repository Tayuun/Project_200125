Texture2DArray	Texture : register(t5);
sampler			Sampler : register(s0);

struct PSIn
{
    float4 pos : SV_POSITION;
    float3 tex : TEXCOORD;
};

float4 main(PSIn input) : SV_TARGET
{
    return Texture.Sample(Sampler, input.tex);
}