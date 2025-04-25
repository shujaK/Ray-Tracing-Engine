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

	static uint32_t ConvertLinearToGamma(const glm::vec4& color)
	{
		Interval val_interval(0.0f, 0.999f);

		float r = std::sqrt(color.r);
			uint8_t R = (uint8_t) (val_interval.clamp(r) * 255.0f);

		float g = std::sqrt(color.g);
			uint8_t G = (uint8_t) (val_interval.clamp(g) * 255.0f);

		float b = std::sqrt(color.b);
			uint8_t B = (uint8_t) (val_interval.clamp(b) * 255.0f);

		float a = std::sqrt(color.a);
		uint8_t A = (uint8_t) (val_interval.clamp(a) * 255.0f);

		uint32_t res = (A << 24) | (B << 16) | (G << 8) | R;
		return res;
	}

	static uint32_t pcg_hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	// random float from 0 to 1
	static float RandomFloat(uint32_t& seed)
	{
		seed = pcg_hash(seed);
		return (float) seed / (float) std::numeric_limits<uint32_t>::max();
	}

	static glm::vec3 RandomInUnitSphere(uint32_t& seed)
	{
		// generate x, y, z 
		float x = RandomFloat(seed) * 2.0f - 1.0f;
		float y = RandomFloat(seed) * 2.0f -1.0f;
		float z = RandomFloat(seed) * 2.0f -1.0f;

		return glm::normalize(glm::vec3(x, y, z));
	}

	static glm::vec3 RandomInUnitHemiSphere(uint32_t& seed, glm::vec3 normal)
	{
		glm::vec3 vec = RandomInUnitSphere(seed);
		if (dot(vec, normal) > 0.0) // In the same hemisphere as the normal
			return vec;
		else
			return -vec;
	}
}