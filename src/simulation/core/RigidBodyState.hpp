#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Full 6DOF state. In 3DOF (point-mass) mode orientation stays at identity
// and angularVelocity stays at zero — same struct, same integrator, no branching.
struct RigidBodyState {
	glm::dvec3 position        = {0.0, 0.0, 0.0};
	glm::dvec3 velocity        = {0.0, 0.0, 0.0};
	glm::dquat orientation     = {1.0, 0.0, 0.0, 0.0}; // w,x,y,z — identity
	glm::dvec3 angularVelocity = {0.0, 0.0, 0.0};

	RigidBodyState operator+(const RigidBodyState& other) const {
		RigidBodyState result;
		result.position = position + other.position;
		result.velocity = velocity + other.velocity;
		return result;
	}
};

inline RigidBodyState operator*(const RigidBodyState& state, double scalar) {
	RigidBodyState result;
	result.position = state.position * scalar;
	result.velocity = state.velocity * scalar;
	return result;
}

inline RigidBodyState operator*(double scalar, const RigidBodyState& state) {
	return state * scalar;
}