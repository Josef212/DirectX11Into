cbuffer PerApplication : register(b0)
{
	matrix projMatrix;
}

cbuffer PerFrame : register(b1)
{
	matrix viewMatrix;
}

cbuffer PerObject : register(b2)
{
	matrix worldMatrix;
}

struct AppData
{
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VertexShaderOutput
{
	float4 color : COLOR;
	float4 position : SV_POSITION;
};

VertexShaderOutput SimpleVertexShader(AppData IN)
{
	VertexShaderOutput OUT;

	matrix mvp = mul(projMatrix, mul(viewMatrix, worldMatrix));
	OUT.position = mul(mvp, float4(IN.position, 1.0f));
	OUT.color = float4(IN.color, 1.0f);

	return OUT;
}