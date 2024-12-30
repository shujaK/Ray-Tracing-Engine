#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "inc/json.hpp"

namespace Scene_ {

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

void to_json(nlohmann::json& j, const Material& m);
void from_json(const nlohmann::json& j, Material& m);
void to_json(nlohmann::json& j, const Sphere& s);
void from_json(const nlohmann::json& j, Sphere& s);

struct Scene
{
    std::vector<Sphere> Spheres;
    std::vector<Material> Materials;

    void addSphere()
    {
        Sphere sphere;
        sphere.Position = { 0.0f, 0.0f, 0.0f };
        sphere.Radius = 1.0f;
        sphere.MaterialIndex = 0;
        Spheres.push_back(sphere);
    }
};

} // namespace Scene