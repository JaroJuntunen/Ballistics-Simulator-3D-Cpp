#pragma once

// Abstract terrain interface.
// Simulation uses this for height queries and collision detection.
// Renderer samples it to build the mesh — no GPU types here.
class Terrain {
public:
	virtual ~Terrain() = default;

	// Elevation in metres at world position (x, y). Z-up coordinate system.
	virtual float heightAt(float x, float y) const = 0;

	// Extent of the terrain in world units (metres), centred on the origin.
	virtual float extent() const = 0;
};
