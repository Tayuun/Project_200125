Texture2D   Texture : register(t6);
sampler     Sampler : register(s0);

struct PSIn
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

float4 main(PSIn input) : SV_TARGET
{
    return Texture.Sample(Sampler, input.tex);
}