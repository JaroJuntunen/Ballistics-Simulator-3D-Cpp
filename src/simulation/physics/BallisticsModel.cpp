#include "BallisticsModel.hpp"

static glm::dvec3 getCoriolis(const RigidBodyState& state ,double phi) {

	glm::dvec3	earthsAngularVelocity {0.0, (Physics::earthOmega * std::cos(phi)) , (Physics::earthOmega * std::sin(phi))};
	glm::dvec3	coriolis = -2.0 * glm::cross(earthsAngularVelocity,  state.velocity);
	return coriolis;
}

static glm::dvec3 getSpinDrift(const Projectile& projectile,glm::dvec3 relativeVelocity, double velocity) {
	glm::dvec3	up = {0.0,0.0,1.0};
	glm::dvec3	v_unit = relativeVelocity / velocity;
	double		k = 1.0 / (projectile.getTwistRate() * velocity);
	glm::dvec3	spinDrift = projectile.getStabilityFactor() * k * glm::cross(v_unit, up);
	return spinDrift;
}

RigidBodyState BallisticsModel::derivative(const RigidBodyState& state ,const Projectile& projectile, Wind& wind, double phi) {

	glm::dvec3	relativeVelocity = state.velocity - wind.getWindSpeed();
	// Variable air dencity with relative speed compared to wind
	double	relativeAirDencity = (Physics::rho0 * std::exp(-state.position.z / 8500.0f));
	double	radius = projectile.getDiameter() * 0.5f;
	double	area = (Physics::pi * (radius * radius));
	double	velocity = std::sqrt((relativeVelocity.x * relativeVelocity.x) + (relativeVelocity.y * relativeVelocity.y) + (relativeVelocity.z * relativeVelocity.z));

	RigidBodyState result;
	if (velocity < 1e-6) return result;

	double	dragForce = 0.5f * projectile.getDragCoefficient() * relativeAirDencity * area * (velocity * velocity);
	double	deceleration = dragForce / projectile.getMass();


	result.velocity += getCoriolis(state, phi);
	result.velocity += getSpinDrift(projectile, relativeVelocity, velocity);
	result.velocity.x -= deceleration * (relativeVelocity.x / velocity);
	result.velocity.y -= deceleration * (relativeVelocity.y / velocity);
	result.velocity.z -= (Physics::g + (deceleration * (relativeVelocity.z / velocity)));

	result.position += state.velocity;
	return result;
}

bool BallisticsModel::hasImpacted(const RigidBodyState &state, const Terrain &terrain)
{
	double terrainheight = terrain.heightAt(state.position.x, state.position.y);
	return state.position.z <= terrainheight;
}
