struct DSOut
{
    float4 pos : SV_POSITION;
    float3 texcoord : TEXCOORD;
};

struct HSCPOut
{
    float4 pos : POSITION;
    float3 texcoord : TEXCOORD;
};

struct HSOut
{
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 4
[domain("quad")]

DSOut main(
	HSOut input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HSCPOut, NUM_CONTROL_POINTS> patch)
{
    DSOut Output;
    
    Output.pos = float4
    (
        (-domain + float2(-0.5f, -0.5f)) * 2.f,
        0.5f,
        1
    );
    
    
    Output.texcoord =
    (
        (domain + float2(-0.5f, -0.5f)) * 2.f,
        0
    );

    return Output;
}
