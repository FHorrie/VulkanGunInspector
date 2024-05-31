#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out vec3 fragTangent;

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
	DirectionalLight directionalLight;
	vec3 cameraPos;
	bool useNormals;
} ubo;

layout(push_constant) uniform Push
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() 
{
	vec4 worldPos = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * worldPos;

	fragPosWorld = worldPos.xyz;
	fragNormal = normalize(mat3(push.normalMatrix) * normal);
	fragUV = uv;
	fragTangent = normalize(mat3(push.normalMatrix) * tangent);
}