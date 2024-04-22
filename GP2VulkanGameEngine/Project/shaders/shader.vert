#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 outColor;

layout(set = 0, binding = 0) uniform GlobalUniBuf
{
	mat4 projectionViewMatrix;
	vec3 directionToLight;
} uniBuf;

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

const float AMB_LIGHT = 0.03;

void main() 
{
	gl_Position = uniBuf.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	float lightIntensity = AMB_LIGHT + max(dot(normalWorldSpace, uniBuf.directionToLight), 0);
	outColor = color * lightIntensity;
}