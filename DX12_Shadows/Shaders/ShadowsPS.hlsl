
cbuffer Camera: register(b0)
{
	float4x4 vp[4];
};

struct DrawConstants
{
	uint camNum;
};
ConstantBuffer<DrawConstants> actualCamera : register(b1);

cbuffer Model: register(b2)
{
	float4x4 model;
};


struct LightPoint
{
	float4 light_color;
	float4 color_ambient;
	float3 lightPos;
	float range;
};

cbuffer FrameCB: register(b3)
{
	LightPoint ll;
	float3 cameraPos;
};



struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};


Texture2D depthMap: register(t0);
SamplerState ss: register(s0);

static const float specFactor = 64;

float4 main(PS_INPUT inp) : SV_TARGET
{
	//return inp.color;
	float bias = 0.001f;
	float4 lightSpacePos = mul(inp.worldPos, vp[1]);
	float2 projTextureCoord;
	projTextureCoord.x = (lightSpacePos.x / lightSpacePos.w) / 2.f + 0.5f;
	projTextureCoord.y = -(lightSpacePos.y / lightSpacePos.w) / 2.f + 0.5f;

	float4 ambient = inp.color * ll.color_ambient;
	float4 diffuse = (float4)0;
	float4 specular = (float4)0;

	if ((saturate(projTextureCoord.x) == projTextureCoord.x) && (saturate(projTextureCoord.y) == projTextureCoord.y))
	{
		float depthMapValue = depthMap.Sample(ss, projTextureCoord);
		float lightDepth = lightSpacePos.z / lightSpacePos.w - bias;
		if (depthMapValue > lightDepth)
		{
			float3 toEye = normalize(cameraPos - inp.worldPos.xyz);
			float3 toLight = ll.lightPos - inp.worldPos.xyz;

			float distance = length(toLight);
			toLight = normalize(toLight);
			inp.normal = normalize(inp.normal);

			float lightIntensity = saturate(dot(inp.normal, toLight));
			if (distance <= ll.range)
			{
				float factor = lerp(0, 1, (ll.range - distance) / ll.range);
				diffuse = ll.light_color * inp.color * ll.light_color * saturate(dot(inp.normal, toLight)) * factor;
				specular = ll.light_color * pow(saturate(dot(reflect(-toLight, inp.normal), toEye)), specFactor);
			}
		}
	}
	return ambient + diffuse + specular;
}