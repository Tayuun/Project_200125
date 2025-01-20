Texture2D Tex : register(t0);
sampler Sam : register(s0);

struct PSIn
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};


float4 main(PSIn In) : SV_TARGET
{
    return Tex.Sample(Sam, In.tex);
}