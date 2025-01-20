struct SInstanceData
{
    float x;
    float y;
    float scaleX;
    float scaleY;
    uint SpriteID;
    float depth;
    float2 pad;
};
struct STextureData
{
    float2 size;
    float2 coord[4];
};
struct VSOut
{
    float4 pos : SV_POSITION;
    float3 texcoord : TEXCOORD;
};

StructuredBuffer<SInstanceData> InstanceData : register(t1);
StructuredBuffer<STextureData>  TextureData  : register(t4);

cbuffer settings : register(b1)
{
    float width_ratio;
    float height_ratio;
    float pad;
};


VSOut main(float4 pos : POSITION, uint VertexID : SV_VertexID, uint InstanceID : SV_InstanceID)
{
    VSOut Out;
    Out.pos = pos;
    const SInstanceData IData = InstanceData[InstanceID];
    const uint spriteID = IData.SpriteID;
    float2 size = TextureData[spriteID].size;
	
    
    Out.pos[0] = IData.x;
    Out.pos[1] = -IData.y;
    Out.pos[0] += size.x * IData.scaleX * (VertexID & 1U);
    Out.pos[1] -= size.y * IData.scaleY * (VertexID >> 1);
    Out.pos[0] *= width_ratio * 2.f;
    Out.pos[1] *= height_ratio * 2.f;
    
    Out.pos[0] -= 1;
    Out.pos[1] += 1;
    
    Out.pos[2] = IData.depth;
	
    Out.texcoord = float3(TextureData[spriteID].coord[VertexID], 0);
    
    return Out;
}