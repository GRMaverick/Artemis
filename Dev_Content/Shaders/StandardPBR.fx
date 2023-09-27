#include "C:\\Users\\Maverick\\Documents\\GitHub\\Artemis\\ArtemisRuntime\\Artemis\\Artemis.Renderer.Assets\\CBStructures.h"

#define PIXEL_SHADER_RESOURCE_2D(name, reg) \
    Texture2D name : register(t##reg); \
    SamplerState name##Sampler : register(s##reg);
#define PIXEL_SHADER_RESOURCE_CUBE(name, reg) \
    TextureCube name : register(t##reg); \
    SamplerState name##Sampler : register(s##reg);

#define XM_PI 3.141592654f

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
    float4 PosH : SV_Position;
    float3 PosW : POSITION;
    float3 Normal : NORMAL;
    float2 Texture : TEXCOORD;
    float3 Tangent : TANGENT0;
    float3 Bitangent : TANGENT1;
};

//
// Constant Buffers / Resource Bindings
//
cbuffer PassCB : register(b0)
{
    ConstantBuffer_Pass cbPass;
};

cbuffer ObjectCB : register(b1)
{
    ConstantBuffer_Object cbObject;
};

static const int LightCount = 4;
cbuffer DirectionalLightCB : register(b2)
{
    ConstantBuffer_DLight cbDirectionalLightData[LightCount];
};

//
// Entry Point
//
VSOutput MainVS(VSInput _input)
{
    VSOutput output;

    output.PosW = mul(cbObject.World, float4(_input.Position, 1.0f)).xyz;
    output.PosH = mul(cbPass.ViewProjection, float4(output.PosW.xyz, 1.0f));

    output.Texture = _input.Texture;

    output.Normal = mul(cbObject.World, float4(_input.Normal, 1.0f)).xyz;
    output.Normal = normalize(output.Normal);

    output.Tangent = mul(cbObject.World, float4(_input.Tangent, 1.0f)).xyz;
    output.Tangent = normalize(output.Tangent);

    output.Bitangent = mul(cbObject.World, float4(_input.Bitangent, 1.0f)).xyz;
    output.Bitangent = normalize(output.Bitangent);

    return output;
}

//
// Constant Buffers / Resource Bindings
//
PIXEL_SHADER_RESOURCE_2D(Albedo, 0);
PIXEL_SHADER_RESOURCE_2D(Normal, 1);
PIXEL_SHADER_RESOURCE_2D(AmbientOcclusion, 2);
PIXEL_SHADER_RESOURCE_2D(Metalness, 3);
PIXEL_SHADER_RESOURCE_2D(Roughness, 4);
PIXEL_SHADER_RESOURCE_CUBE(Skybox, 5);

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

float RemapRoughness(float roughness)
{
#if LIGHTING_IBL
    float nomRough = (roughness + 1);
    float remappedRoughness = nomRough * nomRough / 8;
#else
    float remappedRoughness = (roughness * roughness) / 2;
#endif
    
    return remappedRoughness;
}

float NormalDistributionFunction_GGXTR(float3 normal, float3 halfVector, float roughness)
{
    float roughnessSq = roughness * roughness;
    float nDotH = max(dot(normal, halfVector), 0.0f);
    float nDotHSq = nDotH * nDotH;

    float nom = roughnessSq;
    float denom = (nDotHSq * (roughnessSq - 1.0) + 1.0);
    denom = XM_PI * denom * denom;
    
    return nom / denom;
}

float GeometryFunction_SchlickGGX(float nDotV, float roughness)
{
    return nDotV / (nDotV * (1.0f - roughness) + roughness);
}

float GeometryFunction_Smith(float3 normal, float3 viewDirection, float3 lightDirection, float roughness)
{
    float k = RemapRoughness(roughness);
    
    float nDotV = max(dot(normal, viewDirection), 0.0f);
    float nDotL = max(dot(normal, lightDirection), 0.0f);
    
    float ggx1 = GeometryFunction_SchlickGGX(nDotV, k);
    float ggx2 = GeometryFunction_SchlickGGX(nDotL, k);
    
    return ggx1 * ggx2;
}

float Fresnel_FresnelSchlick(float3 normal, float3 halfVector, float3 viewDirection, float f0)
{
    return f0 + (1 - f0) * (1 - pow(1 - dot(normal, viewDirection), 5.0f));
}

//
// Entry Point
//
float4 MainPS(VSOutput _input) : SV_TARGET
{
    float4 albedoSample = Albedo.Sample(AlbedoSampler, _input.Texture);
    float4 normalSample = Normal.Sample(NormalSampler, _input.Texture);
    float4 roughnessSample = Roughness.Sample(RoughnessSampler, _input.Texture);
    float4 metalnessSample = Metalness.Sample(MetalnessSampler, _input.Texture);
    float4 aoSample = AmbientOcclusion.Sample(AmbientOcclusionSampler, _input.Texture);
    
    float3 viewDirection = normalize(cbPass.EyePosition - _input.PosW);
    float3 normal = NormalSampleToWorldSpace(normalSample.xyz, _input.Normal, _input.Tangent);
    
    float f0 = 0.04f;
    f0 = lerp(f0, albedoSample, metalnessSample);
    
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    //for (int i = 0; i < LightCount; ++i) // TODO: Loop on Lights.
    {
        float3 lightDirection = reflect(-cbPass.EyePosition, _input.Normal); //normalize(cbDirectionalLightData[i].Position - _input.PosW);
        float4 reflectColour = Skybox.Sample(SkyboxSampler, lightDirection);
        float3 halfVector = normalize(viewDirection + lightDirection);
        
        float distance = length(lightDirection - _input.PosW);
        float attenuation = 1.0f / (distance * distance);
        float3 radiance = reflectColour.xyz; //* attenuation;
        
        float D = NormalDistributionFunction_GGXTR(normal, halfVector, roughnessSample.x);
        float F = Fresnel_FresnelSchlick(normal, halfVector, viewDirection, f0);
        float G = GeometryFunction_Smith(normal, viewDirection, lightDirection, roughnessSample.x);
        
        float3 kS = F;
        float3 kD = (float3(1.0f, 1.0f, 1.0f) - kS) * (1.0 - metalnessSample.x);
        
        float nom = D * F * G;
        float denom = 4 * max(dot(normal, viewDirection), 0.0f) * max(dot(normal, lightDirection), 0.0f) + 0.0001; // 0.0001 to stop NaNs
        float3 specular = nom / denom;
        
        float nDotL = max(dot(normal, lightDirection), 0.0f);
        Lo += (kD * albedoSample.xyz / XM_PI + specular) * radiance * nDotL;
    }
    
    float3 ambient = aoSample.xyz * albedoSample.xyz * float3(0.03f, 0.03f, 0.03f); // Why the float3?
    
    float3 finalColour = ambient + Lo;
    
#if defined(HDR_ENABLED)
    finalColour = finalColour / (finalColour + float3(1.0f, 1.0f,1.0f));
    finalColour = pow(finalColour, float3(1.0f/2.2f,1.0f/2.2f,1.0f/2.2f));
#endif
    
    return float4(finalColour, 1.0f);
}