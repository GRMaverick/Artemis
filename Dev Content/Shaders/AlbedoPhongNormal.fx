//
// Input/Outputs
//
struct VSInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Texture : TEXCOORD;
	float3 Tangent : TANGENT0;
	float3 Bitangent : TANGENT1;
};

struct VSOutput
{
	float4 PosH		: SV_Position;
	float3 PosW		: POSITION;
	float3 Normal	: NORMAL;
	float2 Texture	: TEXCOORD;
	float3 Tangent : TANGENT0;
	float3 Bitangent : TANGENT1;
};

//
// Constant Buffers / Resource Bindings
//
cbuffer PassCB : register(b0)
{
	float4x4	ViewProjection;
	float3		EyePosition;
};

cbuffer ObjectCB : register(b1)
{
	float4x4 World;
	float4	MaterialDiffuse;
	float4	MaterialAmbient;
	float4	MaterialSpecular;
};

cbuffer LightCB : register(b2)
{
	float4	LightDiffuse;
	float4	LightAmbient;
	float4	LightSpecular;
	float3	LightPosition;
	float	LightSpecularPower;
};

cbuffer SpotlightCB : register(b3)
{
	float4	SpotlightDiffuse;
	float4	SpotlightAmbient;
	float4	SpotlightSpecular;
	float3	SpotlightPosition;
	float	SpotlightSpecularPower;

	float	SpotlightTheta;
	float	SpotlightPhi;
	float	SpotlightFalloff;
};

//
// Entry Point
//
VSOutput MainVS(VSInput _input)
{
	VSOutput output;

	output.PosW = mul(World, float4(_input.Position, 1.0f)).xyz;
	output.PosH = mul(ViewProjection, float4(output.PosW.xyz, 1.0f));

	output.Texture = _input.Texture;

	output.Normal = mul(World, float4(_input.Normal, 1.0f)).xyz;
	output.Normal = normalize(output.Normal);

	output.Tangent = mul(World, float4(_input.Tangent, 1.0f)).xyz;
	output.Tangent = normalize(output.Tangent);

	output.Bitangent = mul(World, float4(_input.Bitangent, 1.0f)).xyz;
	output.Bitangent = normalize(output.Bitangent);

	return output;
}

//
// Constant Buffers / Resource Bindings
//
Texture2D Albedo : register(t0);
Texture2D Normal : register(t1);

SamplerState AlbedoSampler : register(s0);
SamplerState NormalSampler : register(s1);

float3 NormalSampleToWorldSpace(float3 normalSample, float3 unitNormalW, float3 tangentW)
{	
	// Normal Mapping
	// Expand the range of the normal value from (0, +1) to (-1, +1).
    normalSample = (normalSample * 2.0f) - 1.0f;

	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	return mul(normalSample, TBN);
}

// float4 MainPS(VSOutput _input) : SV_TARGET
// {
// 	float4 color = float4(1.0f, 0.0f, 0.0f, 1.0f);
//  	float4 albedoColor = Albedo.Sample(AlbedoSampler, _input.Texture);
// 	float4 normalSample = Normal.Sample(NormalSampler, _input.Texture);
//     normalSample = (normalSample * 2.0f) - 1.0f;

// 	float3 N = _input.Normal;
// 	float3 T = normalize(_input.Tangent - dot(_input.Tangent, N) * N);
// 	float3 B = cross(N, T);

// 	float3x3 TBN = float3x3(T, B, N);
// 	float3 normal = float4(mul(normalSample, TBN), 1.0f);

	
// 	float3 lightPos = LightPosition;
//  	float diffuseAmount = saturate(dot(lightPos, normal));
//  	float4 diffuse = LightDiffuse * albedoColor;

// 	return float4(diffuse, 0.0f, 0.0f, 1.0f); //float4(bumpNormal, 1.0f);
// }

//
// Entry Point
//
float4 MainPS(VSOutput _input) : SV_TARGET
{
	float4 color = float4(0.5f, 0.0f, 0.0f, 0.0f);
	float4 texColor = Albedo.Sample(AlbedoSampler, _input.Texture);
	float4 normTex = Normal.Sample(NormalSampler, _input.Texture);

	float3 bumpNormal = NormalSampleToWorldSpace(normTex, _input.Normal, _input.Tangent);

	// Ambient Component
	float4 ambient = LightAmbient * MaterialAmbient;

	// Diffuse Component
	float3 lightPos = LightPosition;
	float3 normal = bumpNormal;
	float diffuseAmount = saturate(dot(lightPos, normal));
	float4 diffuse = LightDiffuse * texColor;

	// Specular Component
	float3 viewNormalized = normalize(EyePosition - _input.PosW);
	float reflection = normalize(2 * diffuseAmount * normal - normalize(lightPos));
	float specularAmount = pow(saturate(dot(reflection, viewNormalized)), LightSpecularPower);

	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if (LightSpecularPower > 0.0f)
	{
		specular.rgb = specularAmount * (MaterialSpecular * LightSpecular).rgb;
	}

	float4 returnColour = clamp(texColor * (specular + (diffuse * diffuseAmount) + ambient),0,1);
	return returnColour; //float4(bumpNormal, 1.0f);
}