struct GSOut
{
	float4 pos : SV_POSITION;
};

struct VSOut
{
    float4 pos : SV_POSITION;
};

[maxvertexcount(200)]
void main(triangle VSOut input[3] : SV_POSITION, inout TriangleStream<GSOut> output, uint PrimitiveID : SV_PrimitiveID)
{
    const uint slices = 8; //Max 32 must be power of 2
    const uint sliceshalf = slices >> 1;
    const float angle = 3.14159265f / (slices << 1);
    
    uint a = 0, b = 0, c = 0;
    float d;
    if (PrimitiveID == 0)
    {
        a = 0;
        b = 1;
        c = 2;
        
        float width = input[c].pos.x - input[a].pos.x;
        float height = input[b].pos.y - input[a].pos.y;
    
        d = (height > width) ? (height * 0.2) : (width * 0.2);
    }
    else
    {
        a = 1;
        b = 2;
        c = 0;
        
        float width = input[c].pos.x - input[a].pos.x;
        float height = input[b].pos.y - input[a].pos.y;
    
        d = (height < width) ? (height * 0.2) : (width * 0.2);
    }   
    

    GSOut Points[7];
    
    Points[0].pos = input[c].pos + float4(-d,  d, 0.f, 0.f);
    Points[1].pos = input[c].pos + float4(-d,  0, 0.f, 0.f);
    Points[2].pos = input[a].pos + float4( d,  0, 0.f, 0.f);
    Points[3].pos = input[a].pos + float4( d,  d, 0.f, 0.f);
    Points[4].pos = input[a].pos + float4( 0,  d, 0.f, 0.f);
    Points[5].pos = input[b].pos + float4( 0, -d, 0.f, 0.f);
    Points[6].pos = input[b].pos + float4( d, -d, 0.f, 0.f);

    output.Append(Points[1]);
    output.Append(Points[2]);
    output.Append(Points[0]);
    output.Append(Points[3]);
    output.Append(Points[6]);
    output.Append(Points[4]);
    output.Append(Points[5]);
    output.RestartStrip();

    
   
    //Stright corner
    GSOut LastPoint = Points[4];
    for (uint i = 1; i < slices; i++)
    {
        GSOut p = Points[3];
        p.pos += float4(-d * cos(angle * i), -d * sin(angle * i), 0.f, 0.f);

        output.Append(Points[3]);
        output.Append(p);
        output.Append(LastPoint);
        output.RestartStrip();
        
        LastPoint = p;
    }
    
    output.Append(Points[3]);
    output.Append(Points[2]);
    output.Append(LastPoint);
    output.RestartStrip();
    
    //Other corners
    LastPoint = Points[1];
    
    for (i = 1; i <= (sliceshalf); i++)
    {
        GSOut p = Points[0];
        p.pos += float4(d * sin(angle * i), -d * cos(angle * i), 0.f, 0.f);

        output.Append(Points[0]);
        output.Append(p);
        output.Append(LastPoint);
        output.RestartStrip();
        
        LastPoint = p;
    }

    LastPoint = Points[5];
    
    for (i = 1; i <= (sliceshalf); i++)
    {
        GSOut p = Points[6];
        p.pos += float4(-d * cos(angle * i), d * sin(angle * i), 0.f, 0.f);

        output.Append(Points[6]);
        output.Append(LastPoint);
        output.Append(p);
        output.RestartStrip();
        
        LastPoint = p;
    }
}