#include "ProcObject.h"
#include "Scene.h"
#include "imgui.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"

HitPayload ProcObject::Miss(const Ray& ray)
{
	HitPayload payload;
	payload.HitDistance = -1.0f;
	payload.ObjectIndex = -1;
	return payload;
}

HitPayload pSphere::TraceRay(const Ray& ray, Interval ray_interval)
{
	glm::vec3 rayDir = ray.direction;

	glm::vec3 rayOriginToSphere = ray.origin - Position;

	float a = glm::dot(rayDir, rayDir);
	float b = 2.0f * glm::dot(rayDir, rayOriginToSphere);
	float c = glm::dot(rayOriginToSphere, rayOriginToSphere) - Radius * Radius;

	float discriminant = b * b - 4 * a * c;

	if (discriminant < 0.0f)
		return Miss(ray);

	float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
	float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
	if (!ray_interval.surrounds(t0) && !ray_interval.surrounds(t1))
		return Miss(ray);

	float closestT = glm::min(t0, t1);
	if (closestT > 0.0f)
	{
		HitPayload payload;
		payload.HitDistance = closestT;
		payload.ObjectIndex = ObjectIndex;

		glm::vec3 origin = ray.origin - Position;
		payload.WorldPosition = origin + ray.direction * closestT;
		payload.WorldNormal = glm::normalize(payload.WorldPosition);

		payload.WorldPosition += Position;

		return payload;
	}

	return Miss(ray);
}

void pSphere::DisplayMenu(bool* UIEdited)
{
	*UIEdited |= ImGui::DragFloat3("Position", glm::value_ptr(Position), 0.01f);
	*UIEdited |= ImGui::DragFloat("Radius", &Radius, 0.01f, 0.0f);
	*UIEdited |= ImGui::InputInt("Material", &MaterialIndex);
}