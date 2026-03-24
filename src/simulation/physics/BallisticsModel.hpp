#pragma once

#include "simulation/core/Integrator.hpp"
#include "simulation/environment/Terrain.hpp"

class BallisticsModel {
public:
	static RigidBodyState derivative(const RigidBodyState& state);
	static bool hasImpacted(const RigidBodyState& state, const Terrain& terrain);
private:
};
