#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "inc/json.hpp"
#include "ProcObject.h"

struct HitPayload {
	glm::vec3 WorldPosition;
	glm::vec3 WorldNormal;
	float HitDistance;

	int ObjectIndex;
};

struct Material
{
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	glm::vec3 EmissionColor{ 0.0f };
	float EmissionStrength = 0.0f;

	glm::vec3 GetEmission() const { return EmissionColor * EmissionStrength; }
};

struct Sphere
{
	glm::vec3 Position{ 0.0f };
	float Radius = 0.5f;
	int MaterialIndex = 0;
};

struct Scene
{
	std::vector<Material> Materials;
	ProcObjectList ProcObjects;

	void addSphere();
	
};

// void to_json(nlohmann::json& j, const Material& m);
// void from_json(const nlohmann::json& j, Material& m);
// void to_json(nlohmann::json& j, const Sphere& s);
// void from_json(const nlohmann::json& j, Sphere& s)l
