#include "gameObject.h"

glm::mat4 FH::TransformComponent::GetMatrix()
{
	const float cosY{ glm::cos(rotation.y) };
	const float sinY{ glm::sin(rotation.y) };
	const float cosX{ glm::cos(rotation.x) };
	const float sinX{ glm::sin(rotation.x) };
	const float cosZ{ glm::cos(rotation.z) };
	const float sinZ{ glm::sin(rotation.z) };

	//T * (Rx * Ry * Rz) * S

	return glm::mat4
	{
		{
			scale.x * (cosY * cosZ + sinY * sinX * sinZ),
			scale.x * (cosX * sinZ),
			scale.x * (cosY * sinX * sinZ - cosZ * sinY),
			0.f
		},
		{
			scale.y * (cosZ * sinY * sinX - cosY * sinZ),
			scale.y * (cosX * cosZ),
			scale.y * (cosY * cosZ * sinX + sinY * sinZ),
			0.f
		},
		{
			scale.z * (cosX * sinY),
			scale.z * (-sinX),
			scale.z * (cosY * cosX),
			0.f
		},
		{translation, 1.f}
	};
}

glm::mat3 FH::TransformComponent::GetNormalMatrix()
{
	const float cosY{ glm::cos(rotation.y) };
	const float sinY{ glm::sin(rotation.y) };
	const float cosX{ glm::cos(rotation.x) };
	const float sinX{ glm::sin(rotation.x) };
	const float cosZ{ glm::cos(rotation.z) };
	const float sinZ{ glm::sin(rotation.z) };
	const glm::vec3 invScale{ 1.f / scale };

	//T * (Rx * Ry * Rz) * S

	return glm::mat3
	{
		{
			invScale.x * (cosY * cosZ + sinY * sinX * sinZ),
			invScale.x * (cosX * sinZ),
			invScale.x * (cosY * sinX * sinZ - cosZ * sinY),
		},
		{
			invScale.y * (cosZ * sinY * sinX - cosY * sinZ),
			invScale.y * (cosX * cosZ),
			invScale.y * (cosY * cosZ * sinX + sinY * sinZ),
		},
		{
			invScale.z * (cosX * sinY),
			invScale.z * (-sinX),
			invScale.z * (cosY * cosX),
		}
	};
}