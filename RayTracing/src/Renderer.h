#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include <memory>


class Renderer {
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);

	void Render();

	void SetColor(const glm::vec4& color) { m_color = color; }

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_FinalImage; }
private:
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
	glm::vec4 m_color;
	glm::vec4 PerPixel(glm::vec2 coord);
};

