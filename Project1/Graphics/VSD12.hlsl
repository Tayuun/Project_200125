cbuffer Setting : register(b0)
{
    float width;
    float height;
    float pad[62];
}

cbuffer TextureData : register(b1)
{
    float4 TextureCoordinates;
    float2 TextureSizeInPixels;
}

float4 main( float3 pos : POSITION) : POSITION
{
    return float4(pos, 1);
}