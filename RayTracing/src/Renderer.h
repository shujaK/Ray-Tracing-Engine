#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>

class Renderer {
public:
	struct Settings
	{
		bool Accumulate = true;
		int Bounces = 5;
		float FarDistance = infinity;
		bool Antialiasing = false;
		int AntialiasingSamples = 3;
		int AntialiasingFactor = 1000;
	};

private:

	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;
	uint32_t m_FrameIndex = 1;
	Settings m_Settings;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

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
	glm::vec4 PerPixel2(uint32_t x, uint32_t y);

	HitPayload TraceRay(const Ray& ray, Interval ray_interval);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);
};

