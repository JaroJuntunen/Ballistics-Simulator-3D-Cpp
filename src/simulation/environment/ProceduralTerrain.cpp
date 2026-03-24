#include "ProceduralTerrain.hpp"

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

ProceduralTerrain::ProceduralTerrain(float extent, float scale, float amplitude)
	: m_extent(extent)
	, m_scale(scale)
	, m_amplitude(amplitude)
{}

float ProceduralTerrain::heightAt(float x, float y) const {
	float nx = x / m_scale;
	float ny = y / m_scale;
	// stb_perlin_noise3 returns values roughly in [-1, 1]
	float n = stb_perlin_noise3(nx, ny, 0.0f, 0, 0, 0);
	return n * m_amplitude;
}
