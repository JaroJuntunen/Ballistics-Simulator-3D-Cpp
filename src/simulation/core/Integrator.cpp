#include "Integrator.hpp"

RigidBodyState Integrator::step(const RigidBodyState &state, double dt, DerivativeFn deriv)
{
	RigidBodyState k1, k2, k3, k4;
	k1 = deriv(state);
	k2 = deriv(state + k1 * (dt / 2.0));
	k3 = deriv(state + k2 * (dt / 2.0));
	k4 = deriv(state + k3 * dt);
	RigidBodyState newState = state + (k1 + 2.0 * k2 + 2.0 * k3 + k4) * (dt / 6.0);
	return newState;
}

std::vector<RigidBodyState> Integrator::simulateSteps(const RigidBodyState &state, double dt, DerivativeFn deriv, StopFn stop)
{
	std::vector<RigidBodyState> path;
	path.push_back(state);
	while (!stop(path.back()) && path.size() < 100000) {
		path.push_back(step(path.back(), dt, deriv));
	}
	return (path);
}
