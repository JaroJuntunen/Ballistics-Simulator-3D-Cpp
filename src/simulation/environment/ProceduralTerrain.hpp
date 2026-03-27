#pragma once

#include "Terrain.hpp"

// Perlin noise terrain — single octave, configurable scale and amplitude.
// Acts as the fallback backend when no SRTM tile is loaded.
class ProceduralTerrain : public Terrain {
public:
	// extent:    total side length in metres (e.g. 2000 = 2 km × 2 km)
	// scale:     horizontal frequency of the noise (larger = broader hills)
	// amplitude: peak elevation in metres
	ProceduralTerrain(float extent    = 60000.0f,
					  float scale     = 600.0f,
					  float amplitude = 120.0f);

	float heightAt(float x, float y) const override;
	float width()                    const override { return m_extent; }
	float height()                   const override { return m_extent; }

private:
	float m_extent;
	float m_scale;
	float m_amplitude;
};
