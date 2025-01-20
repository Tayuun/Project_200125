cbuffer Setting : register(b0)
{
    float width;
    float height;
    float pad[62];
}

struct VSOut
{
    float4 pos      : POSITION;
    float4 color    : COLOR;
};


VSOut main(float4 color : COLOR, float4 pos : POS)
{
    VSOut Out;
    Out.pos = pos;
    Out.color = color;
    
    return Out;
}