#pragma once

#include <glm/glm.hpp>
#include "simulation/core/Integrator.hpp"
#include "simulation/environment/wind.hpp"
#include "simulation/environment/Terrain.hpp"
#include "simulation/projectiles/Projectile.hpp"
#include "simulation/physics/PhysicsConstants.hpp"

class BallisticsModel {
public:
	static RigidBodyState derivative(const RigidBodyState& state, const Projectile& projectile, Wind& wind, double phi);
	static bool hasImpacted(const RigidBodyState& state, const Terrain& terrain);
private:
};
