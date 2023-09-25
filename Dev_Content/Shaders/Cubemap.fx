#include "C:\\Users\\Maverick\\Documents\\GitHub\\Artemis\\ArtemisRuntime\\Artemis\\Artemis.Renderer.Assets\\CBStructures.h"

#define XM_PI 3.141592654f

//
// Input/Outputs
//
struct VSInput
{
    float3 Position : POSITION;
};

struct VSOutput
{
    float4 PosH : SV_Position;
    float3 PosW : POSITION;
    float3 Texture : TEXCOORD;
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
    output.Texture = _input.Position;
    
    return output;
}

//
// Constant Buffers / Resource Bindings
//
TextureCube Skybox : register(t0);
SamplerState SkyboxSampler : register(s0);

//
// Entry Point
//
float4 MainPS(VSOutput _input) : SV_TARGET
{    
    return Skybox.Sample(SkyboxSampler, _input.Texture);
}