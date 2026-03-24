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
};
