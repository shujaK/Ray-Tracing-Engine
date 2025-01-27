#pragma once

#include <glm/vec3.hpp>
#include "Ray.h"
#include <memory>
#include <vector>
#include "Interval.h"

struct HitPayload;
struct Ray;

enum ProcObjectType
{
	None = 0,
	Sphere = 1
};

class ProcObject
{
public:
	ProcObjectType Type = None;
	int MaterialIndex = 0;
	int ObjectIndex = 0;
	virtual ~ProcObject() = default;

	virtual HitPayload TraceRay(const Ray& ray, Interval ray_interval) = 0;

	virtual int getMaterialIndex() { return MaterialIndex; };
	virtual void setMaterialIndex(int i) { MaterialIndex = i; };

	virtual void DisplayMenu(bool* UIEdited) = 0;

	HitPayload Miss(const Ray& ray);
};

class pSphere: public ProcObject
{
public:
	glm::vec3 Position{ 0.0f };
	float Radius = 0.5f;
	ProcObjectType Type = Sphere;

	pSphere() = default;

	pSphere(glm::vec3 position, float radius) : Position(position), Radius(radius) {}

	HitPayload TraceRay(const Ray& ray, Interval ray_interval) override;

	pSphere* get() { return this; }

	void DisplayMenu(bool* UIEdited) override;
};

class ProcObjectList
{
public:
	std::vector<std::shared_ptr<ProcObject>> Objects;

	void add(std::shared_ptr<ProcObject> pObject)
	{
		Objects.push_back(pObject);
		Objects.back()->ObjectIndex = Objects.size() - 1;
	}


};