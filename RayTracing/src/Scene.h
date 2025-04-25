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

enum materialType {
	Diffuse = 1,
	Metal = 2,
	Dielectric = 3,
	Emissive = 4,
};

struct Material
{
	int type = 1;
	glm::vec3 Albedo{ 1.0f };
	float Roughness = 0.5f;
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
	glm::vec3 SkyColour{ 0.6f, 0.7f, 0.9f };

	void addSphere();
	const Material* getMaterialFromObjectIndex(int index) const;
	
};

// void to_json(nlohmann::json& j, const Material& m);
// void from_json(const nlohmann::json& j, Material& m);
// void to_json(nlohmann::json& j, const Sphere& s);
// void from_json(const nlohmann::json& j, Sphere& s)l
