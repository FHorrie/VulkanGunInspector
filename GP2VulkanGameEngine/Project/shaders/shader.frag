#version 450
#extension GL_KHR_vulkan_glsl : enable

const float g_PI = 3.141592654f;
const float g_SpecularReflectance = 5.f;

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragTangent;

layout(location = 0) out vec4 outColor;

struct DirectionalLight 
{
    vec4 direction;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	DirectionalLight directionalLight;
    vec3 cameraPos;
    bool useNormals;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D textureDiffuseImage;
layout(set = 1, binding = 1) uniform sampler2D textureNormalImage;
layout(set = 1, binding = 2) uniform sampler2D textureRoughnessImage;
layout(set = 1, binding = 3) uniform sampler2D textureSpecularImage;
layout(set = 1, binding = 4) uniform sampler2D textureAOImage;

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

vec3 GetLambert(vec3 diffuseSample, vec3 kd)
{
    return diffuseSample * kd / g_PI;
}

vec3 GetNormal()
{
    const vec3 normalSample = texture(textureNormalImage, fragUV).rgb * 2.0;

    const vec3 binormal = normalize(cross(normalize(fragNormal), normalize(fragTangent)));
    const mat3 tangentSpaceAxis = mat3(normalize(fragTangent), binormal, normalize(fragNormal));

    return normalize(tangentSpaceAxis * normalSample);
}

float NormalDistGGX(float alpha, vec3 normal, vec3 halfvec)
{
    const float alphaSq = alpha * alpha;

    const float NormDotHalf = max(dot(normal, halfvec), 0.0);
    const float NormDotHalfSq = NormDotHalf * NormDotHalf;

    float denom = pow(g_PI * (NormDotHalfSq * (alphaSq - 1.0) + 1.0), 2.0);
    denom = max(denom, 0.0000001);

    return alphaSq / denom;
}

//Schlick - Beckmann Geometry Shadowing Function
float SchlickGeo(float alpha, vec3 normal, vec3 dir)
{
    float NormDotDir = max(dot(normal, dir), 0.0);

    float k = alpha / 2.0;
    float denom = NormDotDir * (1.0 - k) + k;
    denom = max(denom, 0.0000001);

    return NormDotDir / denom;
}

//Smith model
float Smith(float alpha, vec3 normal, vec3 viewDir, vec3 lightDir)
{
    return SchlickGeo(alpha, normal, viewDir) * SchlickGeo(alpha, normal, lightDir);
}

vec3 Fresnel(vec3 F0, vec3 viewDir, vec3 halfvec)
{
    return F0 + (vec3(1.0) - F0) * pow(1 - max(dot(viewDir, halfvec), 0.0), g_SpecularReflectance);
}

vec3 PBR()
{
	const vec3 diffuseSample = texture(textureDiffuseImage, fragUV).rgb;
	const float roughnessSample = texture(textureRoughnessImage, fragUV).r;
	const float specularSample = texture(textureSpecularImage, fragUV).r;
	const float aoSample = texture(textureAOImage, fragUV).r;

    vec3 sampledNormal = GetNormal();

	const vec3 viewDir = normalize(ubo.cameraPos - fragPosWorld);
    const vec3 lightDir = normalize(-ubo.directionalLight.direction.xyz);
    const vec3 halfVec = normalize(viewDir + lightDir);
    const vec3 diffuseMetallic = mix(vec3(0.04), diffuseSample, specularSample);

    const vec3 ks = Fresnel(diffuseMetallic, viewDir, halfVec);
    const vec3 kd = vec3(1.f) - ks;

    const vec3 lambert = GetLambert(diffuseSample, kd);

    const vec3 cookTorrenceNum = NormalDistGGX(roughnessSample, sampledNormal, halfVec) 
        * Smith(roughnessSample, sampledNormal, viewDir, lightDir) 
        * Fresnel(diffuseMetallic, viewDir, halfVec);

    float cookTorrenceDenom = 4.0 * max(dot(viewDir, sampledNormal), 0.0) * max(dot(lightDir, sampledNormal), 0.0);
    cookTorrenceDenom = max(cookTorrenceDenom, 0.000001);

    const vec3 cookTorrence = cookTorrenceNum / cookTorrenceDenom;

    const vec3 BRDF = lambert + cookTorrence;

    vec3 observedLight = BRDF * ubo.directionalLight.color.xyz * ubo.directionalLight.color.w * max(dot(lightDir, sampledNormal), 0.0);
    observedLight *= aoSample;

    return observedLight;
}

void main() 
{
    outColor = vec4(PBR(), 1.0);
}