struct PSIn
{
	float4 position : SV_POSITION;
	float4 color	: COLOR;
};

float4 main(PSIn In) : SV_TARGET
{
	return In.color;
}