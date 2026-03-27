#pragma once

// Abstract terrain interface.
// Simulation uses this for height queries and collision detection.
// Renderer samples it to build the mesh — no GPU types here.
class Terrain {
public:
	virtual ~Terrain() = default;

	// Elevation in metres at world position (x, y). Z-up coordinate system.
	virtual float heightAt(float x, float y) const = 0;

	// East-west extent in world metres, centred on the origin.
	virtual float width() const = 0;

	// North-south extent in world metres, centred on the origin.
	virtual float height() const = 0;
};
