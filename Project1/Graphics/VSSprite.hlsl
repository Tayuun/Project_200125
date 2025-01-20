struct SInstanceData
{
    float x;
    float y;
    uint SpriteID;
    float pad;
};
struct STextureData
{
    float2 size;
    float2 coord[4];
};

StructuredBuffer<STextureData>  TextureData     : register(t4);
StructuredBuffer<SInstanceData> InstanceData    : register(t0);

cbuffer settings : register(b1)
{
    float width_ratio;
    float height_ratio;
    float2 pad;
};

struct VSOut
{
    float4 pos      : SV_POSITION;
    float3 texcoord : TEXCOORD;
};

VSOut main(float4 pos : POSITION, uint VertexID : SV_VertexID, uint InstanceID : SV_InstanceID)
{
    VSOut Out;
    Out.pos = pos;
    const SInstanceData Data    = InstanceData[InstanceID];
    const uint spriteID         = Data.SpriteID;
    float2 size                 = TextureData[spriteID].size;
	
    
    Out.pos[0] = Data.x;
    Out.pos[1] = -Data.y;
    Out.pos[0] += size.x * (VertexID & 1U);
    Out.pos[1] -= size.y * (VertexID >> 1);
    Out.pos[0] *= width_ratio * 2.f;
    Out.pos[1] *= height_ratio * 2.f;
    
    Out.pos[0] -= 1;
    Out.pos[1] += 1;
    
    Out.pos[2] = 0.5f;
	
    Out.texcoord = float3(TextureData[spriteID].coord[VertexID], 0);
    
    return Out;
}