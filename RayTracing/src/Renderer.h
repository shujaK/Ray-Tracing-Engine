#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>

using namespace Scene_;
class Renderer {
public:
	struct Settings
	{
		bool Accumulate = true;
		int Bounces = 5;
	};

private:

	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;
	uint32_t m_FrameIndex = 1;

	Settings m_Settings;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	struct HitPayload {
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;
		float HitDistance;

		int ObjectIndex;
	};

	std::vector<uint32_t> m_ImageHorizontalIter, m_ImageVerticalIter;
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);

	void Render(const Camera& camera, const Scene& scene);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }

	HitPayload ClickQueryObject(int x, int y);

	void ResetFrameIndex() { m_FrameIndex = 1; }

	Settings& GetSettings() { return m_Settings; }

private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);
};

