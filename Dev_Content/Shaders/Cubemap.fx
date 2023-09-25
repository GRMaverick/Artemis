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
    float4x4 ViewProjection;
    float3 EyePosition;
};

cbuffer ObjectCB : register(b1)
{
    float4x4 World;
};

struct DirectionalLight
{
    float4 Diffuse;    
    float3 Position;
    float Padding;
};

static const int LightCount = 4;
cbuffer DirectionalLightCB : register(b2)
{
    DirectionalLight DirectionalLightData[LightCount];
};

//
// Entry Point
//
VSOutput MainVS(VSInput _input)
{
    VSOutput output;
    output.PosW = mul(World, float4(_input.Position, 1.0f)).xyz;
    output.PosH = mul(ViewProjection, float4(output.PosW.xyz, 1.0f));
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