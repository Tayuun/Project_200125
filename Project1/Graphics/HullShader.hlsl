struct VSOut
{
    float4 pos : POSITION;
    float3 texcoord : TEXCOORD;
};

struct HSCPOut
{
    float4 pos : POSITION;
    float3 texcoord : TEXCOORD;
};

struct HSOut
{
	float EdgeTessFactor[4]			: SV_TessFactor;
	float InsideTessFactor[2]		: SV_InsideTessFactor;
};

#define NUM_CPOINT 4

HSOut CalcHSPatchConstants( InputPatch<VSOut, NUM_CPOINT> ip, uint PatchID : SV_PrimitiveID)
{
    HSOut Output;

	Output.EdgeTessFactor[0]   = 1; 
	Output.EdgeTessFactor[1]   = 1; 
	Output.EdgeTessFactor[2]   = 1;
    Output.EdgeTessFactor[3]   = 1;
    Output.InsideTessFactor[0] = 1;
    Output.InsideTessFactor[1] = 1;

	return Output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]

HSCPOut main( InputPatch<VSOut, NUM_CPOINT> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
    HSCPOut Output;

	Output.pos = ip[i].pos;
    Output.texcoord = ip[i].texcoord;

	return Output;
}
