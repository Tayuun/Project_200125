struct SInstanceData
{
    float x;
    float y;
    float width;
    float height;
    float4 color;
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

StructuredBuffer<SInstanceData> InstanceData : register(t1);

cbuffer settings : register(b1)
{
    float width_ratio;
    float height_ratio;
    float2 pad;
};




VSOut main(float4 pos : POSITION, uint VertexID : SV_VertexID, uint InstanceID : SV_InstanceID)
{
    VSOut Out;
    Out.pos = pos;
    const SInstanceData Data = InstanceData[InstanceID];
    
    Out.pos[0] = Data.x;
    Out.pos[1] = -Data.y;
    Out.pos[0] += Data.width * (VertexID & 1U);
    Out.pos[1] -= Data.height * (VertexID >> 1);
    
    Out.pos[0] *= width_ratio * 2;
    Out.pos[1] *= height_ratio * 2;
    
    Out.pos[0] -= 1;
    Out.pos[1] += 1;
    
    Out.pos[2] = 0.4f;
	
    Out.color = Data.color;
    
    return Out;
}