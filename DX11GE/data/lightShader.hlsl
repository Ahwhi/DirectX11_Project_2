// GLOBALS //
static const int NUM_POINT_LIGHTS = 3;

struct PointLight
{
    float3 position;
    float range;
    float4 color;
};

cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
	float padding;
};

cbuffer LightBuffer
{
    // Directional light
    float4 ambientColor;
    float4 dirDiffuseColor;
    float3 lightDirection;
    float specularPower;
    float4 specularColor;

    // ★ 멀티 포인트 라이트
    PointLight pointLights[NUM_POINT_LIGHTS];

    float2 uvTiling; // (1,1)이 기본
    float2 paddingUv; // 16바이트 정렬용
};

Texture2D shaderTexture : register(t0); // 디퓨즈
Texture2D normalMap : register(t1); // 노말맵
SamplerState SampleType;

// TYPEDEFS //
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewDirection : TEXCOORD1;
    float3 worldPos : TEXCOORD2;
};

// Vertex Shader
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

    input.position.w = 1.0f;

    // world pos
    worldPosition = mul(input.position, worldMatrix);
    output.worldPos = worldPosition.xyz;

    // clip pos
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex;

    // normal → world
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    // view dir
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
    output.viewDirection = normalize(output.viewDirection);

    return output;
}

// Pixel Shader
float4 LightPixelShader(PixelInputType input) : SV_TARGET
{
    float2 tiledUV = input.tex * uvTiling;

    float4 textureColor = shaderTexture.Sample(SampleType, tiledUV);

    // ===== 노말맵 적용 =====
    float3 baseN = normalize(input.normal);

    float3 T = float3(1, 0, 0);
    float3 B = float3(0, 0, 1);
    float3x3 TBN = float3x3(T, B, baseN);

    float3 nSample = normalMap.Sample(SampleType, tiledUV).xyz;
    nSample = nSample * 2.0f - 1.0f;
    float3 n = normalize(mul(nSample, TBN));

    float4 color = ambientColor;
    float4 specular = 0;

    // === Directional light ===
    float3 dirL = normalize(-lightDirection);
    float NdotL = saturate(dot(n, dirL));
    if (NdotL > 0.0f)
    {
        color += dirDiffuseColor * NdotL;

        float3 reflection = normalize(2 * NdotL * n - dirL);
        float specAmount = pow(saturate(dot(reflection, input.viewDirection)), specularPower);
        specular += specularColor * specAmount;
    }

    // === Multi Point Lights ===
    [unroll]
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
    {
        float3 toLight = pointLights[i].position - input.worldPos;
        float dist = length(toLight);
        if (dist <= 0.0f || pointLights[i].range <= 0.0f)
            continue;

        float att = saturate(1.0f - dist / pointLights[i].range);
        if (att <= 0.0f)
            continue;

        float3 Lp = normalize(toLight);
        float NdotLp = saturate(dot(n, Lp));
        if (NdotLp <= 0.0f)
            continue;

        float strength = NdotLp * att;
        color += pointLights[i].color * strength;

        // 필요하면 스페큘러도 포인트라이트에 더해줄 수 있음
        float3 reflectionP = normalize(2 * NdotLp * n - Lp);
        float specAmountP = pow(saturate(dot(reflectionP, input.viewDirection)), specularPower);
        specular += specularColor * (specAmountP * att);
    }

    color *= textureColor;
    color = saturate(color + specular);
    return color;
}



