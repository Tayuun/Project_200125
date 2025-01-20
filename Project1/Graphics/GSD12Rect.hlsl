cbuffer Setting : register(b0)
{
    float width;
    float height;
    float pad[62];
}

struct GSOutput
{
    float4 col : COLOR;
	float4 pos : SV_POSITION;
};

struct GSInput
{
    float4 pos : POSITION;
    float4 col : COLOR;
};

[maxvertexcount(4)]
void main( point GSInput input[1], inout TriangleStream< GSOutput > output)
{
	for (uint i = 0; i < 4; i++)
	{
		GSOutput element;	
    
        element.pos[0] = (input[0].pos[0] + input[0].pos[2] * (i & 1U)) * width - 1;
        element.pos[1] = (-input[0].pos[1] - input[0].pos[3] * (i >> 1)) * height + 1;

        element.pos[2] = 0.4f;
        element.pos[3] = 1;

        element.col = input[0].col;
		
		output.Append(element);
	}
}