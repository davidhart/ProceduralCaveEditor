#define MAX_BLOBS 5
#define MAX_OCTAVES 5 
#define NOISE_TEXTURE_SIZE 64

struct Blob
{
	float3 Position;
	float3 Scale;
};

struct Octave
{
	float3 Scale;
	float Amplitude;
};

cbuffer PerEnvironment
{
	float Threshold;
	int NumBlobs;
		  
	Blob blobs[MAX_BLOBS] =
	{{{0, 0, 0}, {0, 0, 0}},
	 {{0, 0, 0}, {0, 0, 0}},
	 {{0, 0, 0}, {0, 0, 0}},
	 {{0, 0, 0}, {0, 0, 0}},
	 {{0, 0, 0}, {0, 0, 0}}};

	 Octave octaves[MAX_OCTAVES] =
	{{{0, 0, 0}, 0.0f},
	 {{0, 0, 0}, 0.0f},
	 {{0, 0, 0}, 0.0f},
	 {{0, 0, 0}, 0.0f},
	 {{0, 0, 0}, 0.0f}};
}

cbuffer PerChunk
{
	float CubeSize;
	int Size;
	float3 ChunkOffset;
}

Texture3D NoiseTexture;

sampler NoiseSampler = sampler_state
{
  AddressU = Wrap;
  AddressV = Wrap;
  AddressW = Wrap;
  Filter = MIN_MAG_MIP_LINEAR;
};

struct VS_INPUT
{
    float4 Pos          : POSITION;
	uint instanceID  : SV_InstanceID;              
};

struct GS_INPUT
{
    float3 Pos : POSITION;
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

float perlin3D(int seed, float3 i)
{
    float density = 0;
    [unroll(MAX_OCTAVES)] for (int o = 0; o < MAX_OCTAVES; ++o)
	{
		float v = NoiseTexture.SampleLevel(NoiseSampler, i * octaves[o].Scale / NOISE_TEXTURE_SIZE, 0).r*2.0f - 1.0f;
		density += v * octaves[o].Amplitude;
    }
   
	return density;
}


float sampleField(int i, float3 pos0)
{
	float3 pos = cubePos(i, pos0);

	float density = 0;
	
	for (int n = 0; n < NumBlobs; ++n)
	{
		density += 1 / (length((pos - blobPos(n))/blobs[n].Scale) + 0.0001f);
	}

	density += perlin3D(0, pos);

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
    GS_INPUT output = {ChunkOffset + float3((input.instanceID % (Size * Size)) % Size, 
									(input.instanceID / Size) % Size,
									input.instanceID / (Size * Size))*CubeSize};
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

	float3 vertlist[12];
	vertlist[0] = vertexInterp(Threshold, cubePos(0, pos0), sampleField(0, pos0), cubePos(1, pos0), sampleField(1, pos0));
	vertlist[1] = vertexInterp(Threshold, cubePos(1, pos0), sampleField(1, pos0), cubePos(2, pos0), sampleField(2, pos0));
	vertlist[2] = vertexInterp(Threshold, cubePos(2, pos0), sampleField(2, pos0), cubePos(3, pos0), sampleField(3, pos0));
	vertlist[3] = vertexInterp(Threshold, cubePos(3, pos0), sampleField(3, pos0), cubePos(0, pos0), sampleField(0, pos0));
	vertlist[4] = vertexInterp(Threshold, cubePos(4, pos0), sampleField(4, pos0), cubePos(5, pos0), sampleField(5, pos0));
	vertlist[5] = vertexInterp(Threshold, cubePos(5, pos0), sampleField(5, pos0), cubePos(6, pos0), sampleField(6, pos0));
	vertlist[6] = vertexInterp(Threshold, cubePos(6, pos0), sampleField(6, pos0), cubePos(7, pos0), sampleField(7, pos0));
	vertlist[7] = vertexInterp(Threshold, cubePos(7, pos0), sampleField(7, pos0), cubePos(4, pos0), sampleField(4, pos0));
	vertlist[8] = vertexInterp(Threshold, cubePos(0, pos0), sampleField(0, pos0), cubePos(4, pos0), sampleField(4, pos0));
	vertlist[9] = vertexInterp(Threshold, cubePos(1, pos0), sampleField(1, pos0), cubePos(5, pos0), sampleField(5, pos0));
	vertlist[10] = vertexInterp(Threshold, cubePos(2, pos0), sampleField(2, pos0), cubePos(6, pos0), sampleField(6, pos0));
	vertlist[11] = vertexInterp(Threshold, cubePos(3, pos0), sampleField(3, pos0), cubePos(7, pos0), sampleField(7, pos0));
	
	float3 normlist[12];
	[unroll(12)] for (int n = 0; n < 12; ++n)
	{
		normlist[n] = GetNormal(vertlist[n]);
	}

	PS_INPUT output;
	float3 pos = float3(0,0,0); 
	
	for (int i = 0; i < 16; i += 3)
	{
		if(triTableValue(cubeindex, i)>=0)
		{
			[unroll(3)] for (int v = 0; v < 3; ++v)
			{
				int verti = triTableValue(cubeindex, i+v);
				output.Pos = vertlist[verti];
				output.Normal = normlist[verti];
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
