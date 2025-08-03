#pragma once


#include "Utility/PerlinNoise.hpp"
#include "Utility/Performance.hpp"

#include <cmath>

namespace Utility
{
	struct Perlin
	{
		struct Params
		{
			uint8_t octaves;
			float scale;
			float persistence;
			float lacunarity;
			float exponentiation;
			float height;

			Params()
				: octaves         (7u)
				, scale           (375.f)
				, persistence     (1.3f)
				, lacunarity      (2.3f)
				, exponentiation  (8.5f)
				, height          (2048.f)
			{}
		};

		static float Get(float x, float y, const Params& params = Params{})
		{
			PERF(GeneratePerlinNoise);

			float xs    = x / params.scale;
			float ys    = y / params.scale;
			float G     = std::pow(2.f, -params.persistence);
			float amp   = 1.f;
			float freq  = 1.f;
			float norm  = 0.f;
			float total = 0.f;
			auto perlin = siv::BasicPerlinNoise<float>{};

			for (uint8_t o = 0; o < params.octaves; ++o)
			{
				float nv  = perlin.noise2D(xs * freq, ys * freq) * 0.5f + 0.5f;
				total    += nv * amp;
				norm     += amp;
				amp      *= G;
				freq     *= params.lacunarity;
			}

			total /= norm;
			return std::pow(total, params.exponentiation) * params.height;
		}


		struct NoiseResult
		{
			float value; // Noise value
			float dx;    // Partial derivative w.r.t x
			float dy;    // Partial derivative w.r.t y
		};
		static NoiseResult noise2DWithDerivative(const siv::BasicPerlinNoise<float>& noise, float x, float y) noexcept
		{
			auto FadeDerivative = [](float t) noexcept { return 30 * t * t * (t * (t - 2) + 1); };

			constexpr float Z = static_cast<float>(SIVPERLIN_DEFAULT_Z);
			const float X0 = std::floor(x);
			const float Y0 = std::floor(y);
			const float Z0 = std::floor(Z);

			const int32_t ix = static_cast<int32_t>(X0) & 255;
			const int32_t iy = static_cast<int32_t>(Y0) & 255;
			const int32_t iz = static_cast<int32_t>(Z0) & 255;

			const float fx = x - X0;
			const float fy = y - Y0;
			const float fz = Z - Z0;

			const float u = siv::perlin_detail::Fade(fx);
			const float v = siv::perlin_detail::Fade(fy);
			const float w = siv::perlin_detail::Fade(fz);
			const float du = FadeDerivative(fx);
			const float dv = FadeDerivative(fy);
			// w derivative is zero because z is constant

			auto& permutation = noise.serialize();

			const uint8_t A = (permutation[ix] + iy) & 255;
			const uint8_t B = (permutation[(ix + 1) & 255] + iy) & 255;

			const uint8_t AA = (permutation[A] + iz) & 255;
			const uint8_t AB = (permutation[(A + 1) & 255] + iz) & 255;
			const uint8_t BA = (permutation[B] + iz) & 255;
			const uint8_t BB = (permutation[(B + 1) & 255] + iz) & 255;

			// Gradients at each corner
			const float g000 = siv::perlin_detail::Grad(permutation[AA], fx, fy, fz);
			const float g100 = siv::perlin_detail::Grad(permutation[BA], fx - 1, fy, fz);
			const float g010 = siv::perlin_detail::Grad(permutation[AB], fx, fy - 1, fz);
			const float g110 = siv::perlin_detail::Grad(permutation[BB], fx - 1, fy - 1, fz);
			const float g001 = siv::perlin_detail::Grad(permutation[(AA + 1) & 255], fx, fy, fz - 1);
			const float g101 = siv::perlin_detail::Grad(permutation[(BA + 1) & 255], fx - 1, fy, fz - 1);
			const float g011 = siv::perlin_detail::Grad(permutation[(AB + 1) & 255], fx, fy - 1, fz - 1);
			const float g111 = siv::perlin_detail::Grad(permutation[(BB + 1) & 255], fx - 1, fy - 1, fz - 1);

			const float x00 = siv::perlin_detail::Lerp(g000, g100, u);
			const float x10 = siv::perlin_detail::Lerp(g010, g110, u);
			const float x01 = siv::perlin_detail::Lerp(g001, g101, u);
			const float x11 = siv::perlin_detail::Lerp(g011, g111, u);

			const float y0 = siv::perlin_detail::Lerp(x00, x10, v);
			const float y1 = siv::perlin_detail::Lerp(x01, x11, v);

			const float value = siv::perlin_detail::Lerp(y0, y1, w);

			// Partial derivative w.r.t x
			const float dx00 = (g100 - g000);
			const float dx10 = (g110 - g010);
			const float dx01 = (g101 - g001);
			const float dx11 = (g111 - g011);

			const float dy0 = siv::perlin_detail::Lerp(dx00, dx10, v);
			const float dy1 = siv::perlin_detail::Lerp(dx01, dx11, v);
			const float dx = du * siv::perlin_detail::Lerp(dy0, dy1, w);

			// Partial derivative w.r.t y
			const float dy00 = (x10 - x00);
			const float dy01 = (x11 - x01);
			const float dy = dv * siv::perlin_detail::Lerp(dy00, dy01, w);

			return { value, dx, dy };
		}

		struct Result
		{
			float height;
			glm::vec3 normal;
		};
		static Result GetWithNormal(float x, float y, const Params& params = Params{})
		{
			PERF(GeneratePerlinNoiseWithNormal);

			float xs       = x / params.scale;
			float ys       = y / params.scale;
			float G        = std::pow(2.0f, -params.persistence);
			float amp      = 1.f;
			float freq     = 1.f;
			float norm     = 0.f;
			float total    = 0.f;
			float dx_total = 0.f;
			float dy_total = 0.f;
			auto perlin = siv::BasicPerlinNoise<float>{};

			for (uint8_t o = 0; o < params.octaves; ++o)
			{
				auto result = noise2DWithDerivative(perlin, xs * freq, ys * freq);

				float nv = result.value * 0.5f + 0.5f;
				total += nv * amp;

				// derivative scaled by amp
				dx_total += result.dx * freq * amp;
				dy_total += result.dy * freq * amp;

				norm += amp;
				amp  *= G;
				freq *= params.lacunarity;
			}

			total    /= norm;
			dx_total /= norm;
			dy_total /= norm;

			// Apply exponentiation curve to height
			float finalHeight = std::pow(total, params.exponentiation) * params.height;

			// Apply exponentiation chain rule for derivatives
			float exponentFactor = params.exponentiation * std::pow(total, params.exponentiation - 1) * params.height;



			glm::vec3 normal = glm::normalize(glm::vec3(
				-dx_total * exponentFactor / params.scale,
				1.0f,
				-dy_total * exponentFactor / params.scale));

			// normal = glm::normalize(glm::vec3(dx_total, 1.0f, dy_total));

			return {finalHeight, normal};
		}
	};
} // namespace Utility