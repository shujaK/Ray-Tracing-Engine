#include "Renderer.h"

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

void Renderer::Render()
{
	auto height = m_FinalImage->GetHeight();
	auto width = m_FinalImage->GetWidth();

	float aspectRatio = (float) width / (float) height;

	for (uint32_t y = 0; y < height; y++)
	{
		float v = (float) y / (float) height;
		for (uint32_t x = 0; x < width; x++)
		{
			float u = (float) x / (float) width;
			auto uv = (glm::vec2(u * aspectRatio, v) * 2.0f) - 1.0f;

			m_ImageData[y * width + x] = PerPixel(uv);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 uv)
{
	glm::vec3 rayOrigin = glm::vec3(0.0f, 0.0f, 2.0f);
	glm::vec3 rayDir = glm::normalize(glm::vec3(uv, -1.0f));
	float radius = 0.5f;

	float a = glm::dot(rayDir, rayDir);
	float b = 2.0f * glm::dot(rayDir, rayOrigin);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	float discriminant = b * b - 4 * a * c;

	if (discriminant >= 0)
	{
		return 0xffff00ff;
	}
	else
	{
		return 0xff000000;
	}
}
