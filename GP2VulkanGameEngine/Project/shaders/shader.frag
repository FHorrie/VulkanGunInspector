#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragUV;

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
	vec4 ambientlightColor;
	DirectionalLight directionalLights[3];
	int nDirectionalLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D textureDiffuseImage;
layout(set = 1, binding = 1) uniform sampler2D textureNormalImage;
layout(set = 1, binding = 2) uniform sampler2D textureRoughnessImage;
layout(set = 1, binding = 3) uniform sampler2D textureMetallicImage;
layout(set = 1, binding = 4) uniform sampler2D textureAOImage;

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	vec3 diffuseLight = ubo.ambientlightColor.xyz * ubo.ambientlightColor.w;
	vec3 surfaceNormal = normalize(fragNormalWorld);

    // Calculate diffuse lighting from directional lights
    for (int i = 0; i < ubo.nDirectionalLights; i++) 
	{
        DirectionalLight light = ubo.directionalLights[i];
        float cosAngle = max(dot(surfaceNormal, -normalize(vec3(light.direction))), 0);
        vec3 lightIntensity = light.color.xyz * light.color.w;

        diffuseLight += lightIntensity * cosAngle;
    }

	vec3 imageColor = texture(textureNormalImage, fragUV).rgb;

    outColor = vec4((diffuseLight * fragColor) * imageColor, 1.0);
}