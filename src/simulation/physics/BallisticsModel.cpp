#include "BallisticsModel.hpp"
#include "simulation/core/PhysicsConstants.hpp"

RigidBodyState BallisticsModel::derivative(const RigidBodyState& state) {
	RigidBodyState result;
	result.position = state.velocity;
	result.velocity = {0.0, 0.0, -Physics::g};
	return result;
}

bool BallisticsModel::hasImpacted(const RigidBodyState &state, const Terrain &terrain)
{
	double terrainheight = terrain.heightAt(state.position.x, state.position.y);
	return state.position.z <= terrainheight;
}
