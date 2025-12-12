static const int NUM_POINT_LIGHTS = 3;

struct PointLight
{
    float3 position;
    float range;
    float4 color;
};

cbuffer CameraBuffer : register (b0)
{
	float4x4 matView;
	float4x4 matProj;
};

cbuffer TransBuffer : register (b1)
{
	float4x4 matWorld;
};

cbuffer ColorBuffer : register (b2)
{
	float4 fColor;
};

cbuffer BoneBuffer : register (b3)
{
	float4x4 matBones[128];
};

cbuffer CameraPosBuffer : register(b4)
{
    float3 cameraPosition;
    float pad1;
};

cbuffer LightBuffer : register(b5)
{
    float4 ambientColor;
    float4 dirDiffuseColor;
    float3 lightDirection;
    float specularPower;
    float4 specularColor;

    PointLight pointLights[NUM_POINT_LIGHTS];
};

float4x4 IdentityMatrix =
{
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};

struct VertexInputType
{
	float3 position : POSITION;
    float3 normal : NORMAL;
	float2 tex		: TEXCOORD;
	uint4 boneIdx	: BONEID;
	float4 weight	: WEIGHT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
};

Texture2D shaderTexture;
SamplerState SampleType;

// Vertex Shader
PixelInputType vsMain(VertexInputType input)
{
    PixelInputType output;

    float finalWeight = 1 - (input.weight[0] + input.weight[1] + input.weight[2]);

    float4x4 boneTransform = matBones[input.boneIdx[0]] * input.weight[0];
    boneTransform += matBones[input.boneIdx[1]] * input.weight[1];
    boneTransform += matBones[input.boneIdx[2]] * input.weight[2];
    boneTransform += matBones[input.boneIdx[3]] * finalWeight;

    float4 worldPos = mul(float4(input.position, 1.f), boneTransform);
    worldPos = mul(worldPos, matWorld);

    // normal도 동일한 boneTransform으로
    float3 worldNormal = mul(input.normal, (float3x3) boneTransform);
    worldNormal = mul(worldNormal, (float3x3) matWorld);
    worldNormal = normalize(worldNormal);

    float4 clipPos = mul(worldPos, matView);
    clipPos = mul(clipPos, matProj);

    output.position = clipPos;
    output.tex = input.tex;
    output.normal = worldNormal;
    output.viewDirection = normalize(cameraPosition - worldPos.xyz);
    output.worldPos = worldPos.xyz;

    return output;
}

// Pixel Shader
float4 psMain(PixelInputType input) : SV_TARGET
{
    float4 textureColor = shaderTexture.Sample(SampleType, input.tex);

    // 디폴트: ambient
    float4 color = ambientColor;
    float4 specular = float4(0, 0, 0, 0);

    // 방향성 라이트
    float3 lightDir = -normalize(lightDirection);
    float NdotL = saturate(dot(input.normal, lightDir));
    if (NdotL > 0)
    {
        color += dirDiffuseColor * NdotL;

        float3 reflection = normalize(2 * NdotL * input.normal - lightDir);
        float specFactor = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
        specular = specularColor * specFactor;
    }

    // 포인트 라이트
    [unroll]
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
    {
        float3 toPoint = pointLights[i].position - input.worldPos;
        float dist = length(toPoint);
        if (dist <= 0.0f || pointLights[i].range <= 0.0f)
            continue;

        float atten = saturate(1 - dist / pointLights[i].range);
        if (atten <= 0.0f)
            continue;

        float3 pDir = normalize(toPoint);
        float NdotLp = saturate(dot(input.normal, pDir));
        if (NdotLp <= 0.0f)
            continue;

        color += pointLights[i].color * (NdotLp * atten);
    }

    color = saturate(color) * textureColor;
    color = saturate(color + specular);
    return color;
}
