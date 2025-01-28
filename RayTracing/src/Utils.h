#pragma once
#include "Interval.h"

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