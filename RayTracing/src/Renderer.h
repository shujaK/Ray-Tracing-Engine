#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>


class Renderer {
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);

	void Render(const Camera& camera, const Scene& scene);

	void SetColor(const glm::vec4& color) { m_color = color; }

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }

private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec4 m_color;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	struct HitPayload {
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;
		float HitDistance;

		int ObjectIndex;
	};
private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);
};

