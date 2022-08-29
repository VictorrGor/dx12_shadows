struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 world_pos : POSITION;
};


struct LightPoint
{
	float4 light_color;
	float4 color_ambient;
	float3 lightPos;
	float range;
};

Texture2D GTexture0 : register(t0);
Texture2D GTexture1 : register(t1);
Texture2D CameraDepthMap : register(t2);
SamplerState ss : register(s0);

cbuffer Camera : register(b0)
{
	float4x4 vp[12];//0 - camera matrix; 1 - inverse camera matrix; Other - lights
};

static const float specFactor = 128;

cbuffer FrameBuffer : register(b1)
{
	LightPoint ll[10];
	float3 cameraPos;
};

float4 main(PS_INPUT inp) : SV_TARGET
{
	float2 projCoord;
	projCoord.x = inp.world_pos.x / 2.f + 0.5f; 
	projCoord.y = - inp.world_pos.y / 2.f + 0.5f;
	float z = CameraDepthMap.Sample(ss, projCoord);
	float4 screenCoord = float4(inp.world_pos.x, inp.world_pos.y, z, 1);
	float4 diffuseColor = GTexture0.Sample(ss, projCoord);
	float4 normal = GTexture1.Sample(ss, projCoord);
	normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y) * diffuseColor.a;
	float4 worldPos = mul(screenCoord, vp[1]);
	worldPos = float4(worldPos.x / worldPos.w, worldPos.y / worldPos.w, worldPos.z / worldPos.w, worldPos.w / worldPos.w);

	float4 ambient = ll[0].color_ambient;
	float3 diffuse = (float3)0;
	float4 specular = (float4)0;

	float4 res = (float4)0;
	if (z != 1)
	{
		float3 toEye = normalize(cameraPos - worldPos.xyz);
		for (int iLight = 0; iLight < 10; ++iLight)
		{
			float3 lightDir = ll[iLight].lightPos - worldPos.xyz;
			float lightDist = length(lightDir);
			if (lightDist <= ll[iLight].range)
			{
				lightDir = normalize(lightDir);
				float factor = lerp(0, 1, (ll[iLight].range - lightDist) / ll[iLight].range);
				diffuse += ll[iLight].light_color * diffuseColor.xyz * saturate(dot(normal.xyz, lightDir)) * factor;
				specular += ll[iLight].light_color * pow(saturate(dot(reflect(-lightDir, normal.xyz), toEye)), specFactor)* factor;
			}

		}
	}

	return specular + ambient + float4(diffuse, 0);// float4(diffuse, 0);
}