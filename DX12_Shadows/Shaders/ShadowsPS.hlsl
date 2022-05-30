struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 worldPos : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
};


struct LightPoint
{
	float4x4 light_view;
	float4x4 light_projection;
	float4 light_color;
	float4 color_ambient;
	float3 lightPos;
	float range;
};

cbuffer FrameCB: register(b1)
{
	LightPoint ll;
	float3 cameraPos;
	float padding[16]; 
}
static const float specFactor = 64;

float4 main(PS_INPUT inp) : SV_TARGET
{
	float4 ambient = inp.color * ll.color_ambient;
	float4 diffuse = (float4)0;
	float4 specular = (float4)0;
	
	float3 toEye = normalize(cameraPos-inp.worldPos.xyz);
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

	return ambient + diffuse + specular;
}