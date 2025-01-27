#include "Renderer.h"
#include "Interval.h"
#include <execution>

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		Interval val_interval(0.0f, 0.999f);

		uint8_t r = (uint8_t) (val_interval.clamp(color.r) * 255.0f);
		uint8_t g = (uint8_t) (val_interval.clamp(color.g) * 255.0f);
		uint8_t b = (uint8_t) (val_interval.clamp(color.b) * 255.0f);
		uint8_t a = (uint8_t) (val_interval.clamp(color.a) * 255.0f);

		uint32_t res = (a << 24) | (b << 16) | (g << 8) | r;
		return res;
	}

	static uint32_t pcg_hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	static float RandomFloat(uint32_t& seed)
	{
		seed = pcg_hash(seed);
		return (float) seed / (float) std::numeric_limits<uint32_t>::max();
	}

	static glm::vec3 RandomInUnitSphere(uint32_t& seed)
	{
		return glm::normalize(glm::vec3(
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f));
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

HitPayload Renderer::ClickQueryObject(int x, int y)
{
	Ray ray;
	ray.origin = m_ActiveCamera->GetPosition();
	ray.direction = m_ActiveCamera->GetRayDirections()[y * m_FinalImage->GetWidth() + x];

	return TraceRay(ray, Interval(0, infinity));
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.origin = m_ActiveCamera->GetPosition();
	ray.direction = m_ActiveCamera->GetRayDirections()[y * m_FinalImage->GetWidth() + x];

	uint32_t seed = y * m_FinalImage->GetWidth() + x;
	seed *= m_FrameIndex;

	glm::vec3 light = glm::vec3(0.0f);
	glm::vec3 contribution(1.0f);

	int aliasNum = m_Settings.Antialiasing ? m_Settings.AntialiasingSamples : 1;
	for (int j = 0; j < aliasNum; j++)
	{
		if (m_Settings.Antialiasing)
		{
			seed += j;
			float u = (float) (Utils::RandomFloat(seed) * 2.0f - 1) / (m_Settings.AntialiasingFactor);
			float v = (float) (Utils::RandomFloat(seed) * 2.0f - 1) / (m_Settings.AntialiasingFactor);
			float w = (float) (Utils::RandomFloat(seed) * 2.0f - 1) / (m_Settings.AntialiasingFactor);

			ray.direction += glm::vec3(u, v, w);
		}
		for (int i = 0; i < m_Settings.Bounces; i++)
		{
			seed += i;
			HitPayload payload = TraceRay(ray, Interval(0, m_Settings.FarDistance));

			if (payload.HitDistance < 0.0f)
			{
				glm::vec3 skyCol = glm::vec3(0.6f, 0.7f, 0.9f);
				light += skyCol * contribution;
				break;
			}

			int materialIndex = m_ActiveScene->ProcObjects.Objects[payload.ObjectIndex]->getMaterialIndex();
			const Material& mat = m_ActiveScene->Materials[materialIndex];

			light += mat.GetEmission();
			contribution *= mat.Albedo;

			ray.origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
			ray.direction = glm::normalize(payload.WorldNormal + Walnut::Random::InUnitSphere());
		}
	}
	light /= (float) aliasNum;
	contribution /= (float) aliasNum;
	

	return glm::vec4(light, 1.0f);
}

HitPayload Renderer::TraceRay(const Ray& ray, Interval ray_interval)
{
	if (m_ActiveScene->ProcObjects.Objects.empty())
		return Miss(ray);

	HitPayload finalPayload;

	bool hitAny = false;
	float hitDistance = ray_interval.max;
	for (uint32_t i = 0; i < m_ActiveScene->ProcObjects.Objects.size(); i++)
	{
		auto payload = m_ActiveScene->ProcObjects.Objects[i]->TraceRay(ray, Interval(ray_interval.min, hitDistance));
		if (payload.HitDistance > 0.0f && payload.HitDistance < hitDistance)
		{
			finalPayload = payload;
			hitAny = true;
			hitDistance = payload.HitDistance;
		}
	}

	if (!hitAny)
		return Miss(ray);

	return finalPayload;

}


// HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
// {
// 	HitPayload payload;
// 	payload.HitDistance = hitDistance;
// 	payload.ObjectIndex = objectIndex;
// 
// 	pSphere* closestSphere = (pSphere *) m_ActiveScene->ProcObjects.Objects[objectIndex].get();
// 
// 	glm::vec3 origin = ray.origin - closestSphere->Position;
// 	payload.WorldPosition = origin + ray.direction * hitDistance;
// 	payload.WorldNormal = glm::normalize(payload.WorldPosition);
// 
// 	payload.WorldPosition += closestSphere.Position;
// 
// 	return payload;
// }

HitPayload Renderer::Miss(const Ray& ray)
{
	HitPayload payload;
	payload.HitDistance = -1.0f;
	payload.ObjectIndex = -1;

	return payload;
}