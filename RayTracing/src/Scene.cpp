#include "Scene.h"


// void to_json(nlohmann::json& j, const Material& m)
// {
//     j = nlohmann::json{
//         {"Albedo", {m.Albedo.r, m.Albedo.g, m.Albedo.b}},
//         {"Roughness", m.Roughness},
//         {"Metallic", m.Metallic},
//         {"EmissionColor", {m.EmissionColor.r, m.EmissionColor.g, m.EmissionColor.b}},
//         {"EmissionStrength", m.EmissionStrength}
//     };
// }
// 
// void from_json(const nlohmann::json& j, Material& m)
// {
//     m.Albedo = glm::vec3(j.at("Albedo")[0], j.at("Albedo")[1], j.at("Albedo")[2]);
//     m.Roughness = j.at("Roughness");
//     m.Metallic = j.at("Metallic");
//     m.EmissionColor = glm::vec3(j.at("EmissionColor")[0], j.at("EmissionColor")[1], j.at("EmissionColor")[2]);
//     m.EmissionStrength = j.at("EmissionStrength");
// }
// 
// void to_json(nlohmann::json& j, const Sphere& s)
// {
//     j = nlohmann::json{
//         {"Position", {s.Position.x, s.Position.y, s.Position.z}},
//         {"Radius", s.Radius},
//         {"MaterialIndex", s.MaterialIndex}
//     };
// }
// 
// void from_json(const nlohmann::json& j, Sphere& s)
// {
//     s.Position = glm::vec3(j.at("Position")[0], j.at("Position")[1], j.at("Position")[2]);
//     s.Radius = j.at("Radius");
//     s.MaterialIndex = j.at("MaterialIndex");
// }
void Scene::addSphere()
{
	pSphere sphere;
	sphere.Position = { 0.0f, 0.0f, 0.0f };
	sphere.Radius = 1.0f;
	sphere.MaterialIndex = 0;
	ProcObjects.add(std::make_shared<pSphere>(sphere));
}
