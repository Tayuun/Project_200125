struct VSOut
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

cbuffer InstanceData : register(b2)
{
    float x;
    float y;
    float scaleX;
    float scaleY;
    float depth;
    float width;
    float height;
    float pad;
}

cbuffer settings : register(b1)
{
    float width_ratio;
    float height_ratio;
    float pad2;
};

VSOut main(float4 pos : POSITION, uint VertexID : SV_VertexID)
{
    VSOut Out;
    
    const float a = (float)(VertexID & 1U);
    const float b = (float)(VertexID >> 1);
    
    Out.pos[0] = x + width * a * scaleX;
    Out.pos[1] = -y - height * b * scaleY;
    Out.pos[0] *= width_ratio * 2.f;
    Out.pos[1] *= height_ratio * 2.f;
    
    Out.pos[2] = depth;
	
    Out.pos[0] -= 1;
    Out.pos[1] += 1;
    Out.pos[3] = pos[3];
    
    Out.texcoord = float2(a, b);
    
    return Out;
}