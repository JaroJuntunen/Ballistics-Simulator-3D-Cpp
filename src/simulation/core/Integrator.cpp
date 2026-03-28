#include "simulation/core/Integrator.hpp"
#include <iostream>

RigidBodyState Integrator::step(const RigidBodyState &state, const Projectile& projectile,Wind &wind, double dt, DerivativeFn deriv)
{
	RigidBodyState k1, k2, k3, k4;
	k1 = deriv(state, projectile,wind, projectile.getLatitudeRad());
	k2 = deriv(state + k1 * (dt / 2.0), projectile,wind, projectile.getLatitudeRad());
	k3 = deriv(state + k2 * (dt / 2.0), projectile, wind, projectile.getLatitudeRad());
	k4 = deriv(state + k3 * dt, projectile, wind, projectile.getLatitudeRad());
	RigidBodyState newState = state + (k1 + 2.0 * k2 + 2.0 * k3 + k4) * (dt / 6.0);
	return newState;
}

Trajectory Integrator::simulateSteps(const RigidBodyState &state, Projectile& projectile,Wind wind, double dt, DerivativeFn deriv, StopFn stop)
{
	Trajectory path;
	path.push_back(state);
	while (!stop(path.back()) && path.size() < 100000) {
		wind.passWindTime(dt);
		path.push_back(step(path.back(),projectile,wind, dt, deriv));
	}
	projectile.setIsImpacted(true);
	return (path);
}
