#define MAX_BLOBS 5

struct Blob
{
	float3 Position;
	float Radius;
};

cbuffer UserOptions
{
	float Threshold;
	
	int NumBlobs;

	float CubeSize;
					  
	Blob blobs[MAX_BLOBS] =
	{{{0, 0, 0}, 0.5f},
	 {{0, 0, 0}, 0.5f},
	 {{0, 0, 0}, 0.5f},
	 {{0, 0, 0}, 0.5f},
	 {{0, 0, 0}, 0.5f}};
	
	float AnimationSpeed;
}


struct VS_INPUT
{
    float4 Pos          : POSITION;              
};

struct GS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
	float3 Pos : POSITION;
	float3 Normal : NORMAL;
};

tbuffer EdgeTable
{
	static const float3 vertDecals[8] = 
	{
		float3 (0.0f, 0.0f, 0.0f),
		float3 (1.0f, 0.0f, 0.0f),
		float3 (1.0f, 1.0f, 0.0f),
		float3 (0.0f, 1.0f, 0.0f),
		float3 (0.0f, 0.0f, -1.0f),
		float3 (1.0f, 0.0f, -1.0f),
		float3 (1.0f, 1.0f, -1.0f),
		float3 (0.0f, 1.0f, -1.0f),
	};

	int Edges[256];
	
	int TriTable[256][16];
}

float3 cubePos(int i, float3 pos0)
{
	return pos0 + vertDecals[i]*CubeSize;
}

float3 blobPos(int n)
{	
	return blobs[n].Position;
}

static const int octaves = 5;
static const float zoom = 0.3f;
static const float persistance = 0.5f;
static const float power = 2.7f;
static const float scale = 0.28f;

float random(int seed, float3 i)
{
   int s = int(seed + i.x * 54139 + i.y * 50021 + i.z * 54311);
   s = s ^ s >> 11;
   s = s ^ (s << 7 & 0x9d2c5680);
   s = s ^ (s << 15 & 0xefc60000);
   s = s ^ (s >> 18);
   return frac(s/150377.0); 
}

float interp(float a, float b, float x)
{
   return lerp(a, b, smoothstep(0, 1, x));
}

float perlin3D(int seed, float3 i)
{
   float density = 0;
   
   for (int o = 0; o < octaves; ++o)
   {
      float frequency = pow(2, o);
      float amplitude = pow(persistance, o);

	  float3 v = i * frequency / zoom;
	  float3 floorv = floor(v);
      
      float na = random(seed, float3(floorv.x, floorv.y, floorv.z));
      float nb = random(seed, float3(floorv.x+1, floorv.y, floorv.z));
      float nc = random(seed, float3(floorv.x, floorv.y+1, floorv.z));
      float nd = random(seed, float3(floorv.x+1, floorv.y+1, floorv.z));
      
      float ne = random(seed, float3(floorv.x, floorv.y, floorv.z+1));
      float nf = random(seed, float3(floorv.x+1, floorv.y, floorv.z+1));
      float ng = random(seed, float3(floorv.x, floorv.y+1, floorv.z+1));
      float nh = random(seed, float3(floorv.x+1, floorv.y+1, floorv.z+1));
      
      float la = interp(na, nb, v.x - floorv.x);
      float lb = interp(nc, nd, v.x - floorv.x);
      float lc = interp(la, lb, v.y - floorv.y);
      
      float ld = interp(ne, nf, v.x - floorv.x);
      float le = interp(ng, nh, v.x - floorv.x);
      float lf = interp(ld, le, v.y - floorv.y);
      
      density += interp(lc, lf, v.z - floorv.z) * amplitude;
   }
   
   return clamp(scale * pow(density, power),0,1);
}

float sampleField(int i, float3 pos0)
{
	float3 pos = cubePos(i, pos0);

	float density = 0;
	
	for (int n = 0; n < NumBlobs; ++n)
	{		
		density += blobs[n].Radius*1/(length(pos-blobPos(n))+0.0001f);
	}

	density += (perlin3D(0, pos)-0.5f)*2.0f;

	return density;
}

float3 GetNormal(float3 pos)
{
	float3 d = float3(sampleField(0, pos+float3(CubeSize, 0.0f, 0.0f)) - sampleField(0, pos - float3(CubeSize, 0.0f, 0.0f)),
					  sampleField(0, pos+float3(0.0f, CubeSize, 0.0f)) - sampleField(0, pos - float3(0.0f, CubeSize, 0.0f)),
					  sampleField(0, pos+float3(0.0f, 0.0f, CubeSize)) - sampleField(0, pos - float3(0.0f, 0.0f, CubeSize)));

	return normalize(d);
}

int edgeTableValue(int i)
{
	return Edges[i];
}

float3 vertexInterp(float isolevel, float3 v0, float l0, float3 v1, float l1)
{
	return lerp(v0, v1, (isolevel-l0)/(l1-l0));
}

int triTableValue(int i, int j)
{
	return TriTable[i][j];
}

GS_INPUT mainVS( VS_INPUT input )
{
    GS_INPUT output = {input.Pos};
    return output;
}

[maxvertexcount(16)]
void mainGS( point GS_INPUT input[1], inout TriangleStream<PS_INPUT> stream )
{
	int cubeindex = 0;
	float3 pos0 = input[0].Pos.xyz;
	if (sampleField(0, pos0) < Threshold) cubeindex = cubeindex | 1;
	if (sampleField(1, pos0) < Threshold) cubeindex = cubeindex | 2;
	if (sampleField(2, pos0) < Threshold) cubeindex = cubeindex | 4;
	if (sampleField(3, pos0) < Threshold) cubeindex = cubeindex | 8;
	if (sampleField(4, pos0) < Threshold) cubeindex = cubeindex | 16;
	if (sampleField(5, pos0) < Threshold) cubeindex = cubeindex | 32;
	if (sampleField(6, pos0) < Threshold) cubeindex = cubeindex | 64;
	if (sampleField(7, pos0) < Threshold) cubeindex = cubeindex | 128;
	
	float3 vertlist[12] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}}; // MUST initialise this because of X4850 error
	
	if ((edgeTableValue(cubeindex) & 1) != 0)
	  vertlist[0] =	vertexInterp(Threshold, cubePos(0, pos0), sampleField(0, pos0), cubePos(1, pos0), sampleField(1, pos0));
	if ((edgeTableValue(cubeindex) & 2) != 0)
	  vertlist[1] = vertexInterp(Threshold, cubePos(1, pos0), sampleField(1, pos0), cubePos(2, pos0), sampleField(2, pos0));
	if ((edgeTableValue(cubeindex) & 4) != 0)
	  vertlist[2] = vertexInterp(Threshold, cubePos(2, pos0), sampleField(2, pos0), cubePos(3, pos0), sampleField(3, pos0));
	if ((edgeTableValue(cubeindex) & 8) != 0)
	  vertlist[3] = vertexInterp(Threshold, cubePos(3, pos0), sampleField(3, pos0), cubePos(0, pos0), sampleField(0, pos0));
	if ((edgeTableValue(cubeindex) & 16) != 0)
	  vertlist[4] = vertexInterp(Threshold, cubePos(4, pos0), sampleField(4, pos0), cubePos(5, pos0), sampleField(5, pos0));
	if ((edgeTableValue(cubeindex) & 32) != 0)
	  vertlist[5] = vertexInterp(Threshold, cubePos(5, pos0), sampleField(5, pos0), cubePos(6, pos0), sampleField(6, pos0));
	if ((edgeTableValue(cubeindex) & 64) != 0)
	  vertlist[6] = vertexInterp(Threshold, cubePos(6, pos0), sampleField(6, pos0), cubePos(7, pos0), sampleField(7, pos0));
	if ((edgeTableValue(cubeindex) & 128) != 0)
	  vertlist[7] = vertexInterp(Threshold, cubePos(7, pos0), sampleField(7, pos0), cubePos(4, pos0), sampleField(4, pos0));
	if ((edgeTableValue(cubeindex) & 256) != 0)
	  vertlist[8] = vertexInterp(Threshold, cubePos(0, pos0), sampleField(0, pos0), cubePos(4, pos0), sampleField(4, pos0));
	if ((edgeTableValue(cubeindex) & 512) != 0)
	  vertlist[9] = vertexInterp(Threshold, cubePos(1, pos0), sampleField(1, pos0), cubePos(5, pos0), sampleField(5, pos0));
	if ((edgeTableValue(cubeindex) & 1024) != 0)
	  vertlist[10] = vertexInterp(Threshold, cubePos(2, pos0), sampleField(2, pos0), cubePos(6, pos0), sampleField(6, pos0));
	if ((edgeTableValue(cubeindex) & 2048) != 0)
	  vertlist[11] = vertexInterp(Threshold, cubePos(3, pos0), sampleField(3, pos0), cubePos(7, pos0), sampleField(7, pos0));
	
	PS_INPUT output;
	float3 pos = float3(0,0,0); 
	
	for (int i = 0; i < 16; i += 3)
	{
		if(triTableValue(cubeindex, i)>=0)
		{
			[unroll] for (int v = 0; v < 3; ++v)
			{
				pos = vertlist[triTableValue(cubeindex, i+v)];
				output.Pos = pos;
				output.Normal = GetNormal(pos);
				stream.Append(output);
			}
			
			stream.RestartStrip();
		}
		else
		{
			break;
		}
	}
}

GeometryShader gs = ConstructGSWithSO( CompileShader( gs_4_0, mainGS() ), "POSITION.xyz; NORMAL.xyz;" );

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

technique10 Render
{
	pass p0
	{
        SetVertexShader(CompileShader(vs_4_0, mainVS()));
        SetGeometryShader(gs);
        SetPixelShader(NULL);

		SetDepthStencilState( DisableDepth, 0 );
	}
}
