#include "Renderer.h"

#include <execution>

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t) (color.r * 255.0f);
		uint8_t g = (uint8_t) (color.g * 255.0f);
		uint8_t b = (uint8_t) (color.b * 255.0f);
		uint8_t a = (uint8_t) (color.a * 255.0f);

		uint32_t res = (a << 24) | (b << 16) | (g << 8) | r;
		return res;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// dont need to resize
		if (width == m_FinalImage->GetWidth() && height == m_FinalImage->GetHeight())
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_FrameIndex = 1;

	m_ImageHorizontalIter.resize(width);
	m_ImageVerticalIter.resize(height);

	for (uint32_t i = 0; i < width; i++)
	{
		m_ImageHorizontalIter[i] = i;
	}

	for (uint32_t i = 0; i < height; i++)
	{
		m_ImageVerticalIter[i] = i;
	}
}

void Renderer::Render(const Camera& camera, const Scene& scene)
{
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (m_FrameIndex == 1)
	{
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));
	}

	auto height = m_FinalImage->GetHeight();
	auto width = m_FinalImage->GetWidth();
	float aspectRatio = (float) width / (float) height;

#define MT 1
#if MT
	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
		[this](uint32_t& y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
				[this, y](uint32_t& x)
				{
					auto color = PerPixel(x, y);
					m_AccumulationData[y * m_FinalImage->GetWidth() + x] += color;

					glm::vec4 accumulatedColor = m_AccumulationData[y * m_FinalImage->GetWidth() + x] / (float) m_FrameIndex;

					accumulatedColor = glm::clamp(accumulatedColor, 0.0f, 1.0f);
					m_ImageData[y * m_FinalImage->GetWidth() + x] = Utils::ConvertToRGBA(accumulatedColor);
				});
		});
#else
	for (uint32_t y = 0; y < height; y++)
	{
		float v = (float) y / (float) height;
		for (uint32_t x = 0; x < width; x++)
		{
			auto color = PerPixel(x, y);
			m_AccumulationData[y * width + x] += color;

			glm::vec4 accumulatedColor = m_AccumulationData[y * width + x] / (float) m_FrameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, 0.0f, 1.0f);
			m_ImageData[y * width + x] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}
#endif
	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
	{
		m_FrameIndex++;
	}
	else
	{
		m_FrameIndex = 1;
	}
}

Renderer::HitPayload Renderer::ClickQueryObject(int x, int y)
{
	Ray ray;
	ray.origin = m_ActiveCamera->GetPosition();
	ray.direction = m_ActiveCamera->GetRayDirections()[y * m_FinalImage->GetWidth() + x];

	return TraceRay(ray);
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.origin = m_ActiveCamera->GetPosition();
	ray.direction = m_ActiveCamera->GetRayDirections()[y * m_FinalImage->GetWidth() + x];

	glm::vec3 light = glm::vec3(0.0f);
	glm::vec3 contribution(1.0f);
	for (int i = 0; i < m_Settings.Bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);

		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyCol = glm::vec3(0.6f, 0.7f, 0.9f);
			// light += skyCol * contribution;
			break;
		}

		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& mat = m_ActiveScene->Materials[sphere.MaterialIndex];

		light += mat.GetEmission();
		contribution *= mat.Albedo;

		ray.origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
	}

	return glm::vec4(light, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{
	if (m_ActiveScene->Spheres.empty())
		return Miss(ray);

	glm::vec3 rayDir = ray.direction;

	int closestSphere = -1;
	float hitDistance = FLT_MAX;
	for (uint32_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{

		const Sphere& sphere = m_ActiveScene->Spheres[i];
		glm::vec3 rayOriginToSphere = ray.origin - sphere.Position;

		float a = glm::dot(rayDir, rayDir);
		float b = 2.0f * glm::dot(rayDir, rayOriginToSphere);
		float c = glm::dot(rayOriginToSphere, rayOriginToSphere) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4 * a * c;

		if (discriminant < 0.0f)
			continue;

		float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
		float t1 = (-b - glm::sqrt(discriminant)) / (2.0f * a);

		float closestT = glm::min(t0, t1);
		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = i;
		}
	}

	if (closestSphere < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphere);

}


Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

	glm::vec3 origin = ray.origin - closestSphere.Position;
	payload.WorldPosition = origin + ray.direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	payload.WorldPosition += closestSphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	payload.ObjectIndex = -1;

	return payload;
}