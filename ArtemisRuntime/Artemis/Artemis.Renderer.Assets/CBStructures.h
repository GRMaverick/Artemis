#pragma once

#if defined(__cplusplus)
	#include <DirectXMath.h>
	#include "Vectors/Vector3.h"
	#include "Vectors/Vector4.h"

	#define CB_FLOAT4X4 DirectX::XMMATRIX
	#define CB_FLOAT3 Artemis::Maths::Vector3
	#define CB_FLOAT4 Artemis::Maths::Vector4
#else
	#define CB_FLOAT4X4 float4x4
	#define CB_FLOAT3 float3
	#define CB_FLOAT4 float4
#endif

struct ConstantBuffer_Pass
{
	CB_FLOAT4X4		ViewProjection;
	CB_FLOAT3		EyePosition;
};

struct ConstantBuffer_DLight
{
	CB_FLOAT4 Diffuse;
	CB_FLOAT3 Position;
	float Padding;
};

struct ConstantBuffer_Object
{
	CB_FLOAT4X4 World;
};
