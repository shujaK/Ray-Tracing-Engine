#include "Renderer.h"

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
}

void Renderer::Render(const Camera& camera, const Scene& scene)
{
	Ray ray;
	ray.origin = camera.GetPosition();
	
	auto height = m_FinalImage->GetHeight();
	auto width = m_FinalImage->GetWidth();
	float aspectRatio = (float) width / (float) height;

	for (uint32_t y = 0; y < height; y++)
	{
		float v = (float) y / (float) height;
		for (uint32_t x = 0; x < width; x++)
		{
			ray.direction = camera.GetRayDirections()[y * width + x];

			auto color = TraceRay(ray, scene);
			color = glm::clamp(color, 0.0f, 1.0f);
			m_ImageData[y * width + x] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Ray& ray, const Scene& scene)
{
    if (scene.Spheres.empty())
        return glm::vec4(0, 0, 0, 1);

    glm::vec3 rayDir = ray.direction;

    const Sphere* closestSphere = nullptr;
    float hitDistance = FLT_MAX;
    for (const Sphere& sphere : scene.Spheres)
    {
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
        if (closestT < hitDistance)
        {
            hitDistance = closestT;
            closestSphere = &sphere;
        }
    }

    if (closestSphere == nullptr)
        return glm::vec4(0, 0, 0, 1);

    glm::vec3 hitPoint = ray.origin + rayDir * hitDistance;
    glm::vec3 normal = glm::normalize(hitPoint - closestSphere->Position);

    glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

    glm::vec3 sphereCol = closestSphere->Albedo;
    float dot = glm::max(0.0f, glm::dot(normal, -lightDir));
    sphereCol *= dot;

    return glm::vec4(sphereCol, 1.0f);
}
