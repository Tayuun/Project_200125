cbuffer settings : register(b1)
{
    float width_ratio;
    float height_ratio;
    float2 pad;
};

struct SInstanceData
{
    float4 pos;
    float4 color;
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

StructuredBuffer<SInstanceData> InstanceData : register(t1);


VSOut main(float4 pos : POSITION, uint VertexID : SV_VertexID, uint InstanceID : SV_InstanceID )
{
    VSOut Out;
    
    Out.pos[0] = InstanceData[InstanceID].pos[VertexID * 2U] * 2.f * width_ratio;
    Out.pos[1] = -InstanceData[InstanceID].pos[VertexID * 2U + 1.f] * 2.f * height_ratio;
    
    Out.pos[0] -= 1;
    Out.pos[1] += 1;
    
    Out.pos[2] = 0.4f;
    Out.pos[3] = pos.w;
    
    Out.color = InstanceData[InstanceID].color;
    
	return Out;
}