cbuffer Setting : register(b0)
{
    float width;
    float height;
    float pad[62];
}

cbuffer TextureData : register(b1)
{
    float4 TextureCoordinates; //x, y, x2, y2
    float2 TextureSizeInPixels;
}

struct GSOutput
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

[maxvertexcount(4)]
void main(point float4 input[1] : POSITION, inout TriangleStream<GSOutput> output)
{
    for (uint i = 0; i < 4; i++)
    {
        GSOutput element;
    
        element.pos[0] = (input[0][0] + TextureSizeInPixels[0] * (i & 1U)) * width - 1;
        element.pos[1] = -(input[0][1] + TextureSizeInPixels[1] * (i >> 1U)) * height + 1;
        element.pos[2] = input[0][2];
        element.tex[0] = TextureCoordinates[(i & 1U) * 2];
        element.tex[1] = TextureCoordinates[1 + (i >> 1) * 2];
        element.pos[3] = 1;
		
        output.Append(element);
    }
}