#pragma once
#include "simulation/core/RigidBodyState.hpp"
#include "simulation/projectiles/Projectile.hpp"
#include "simulation/environment/wind.hpp"
#include <functional>
#include <vector>
typedef std::vector<RigidBodyState> Trajectory;

using DerivativeFn = std::function<RigidBodyState(const RigidBodyState&, const Projectile& projectile, Wind& wind, double phi)>;
using StopFn = std::function<bool(const RigidBodyState&)>;

class Integrator {
public:
	static RigidBodyState step(const RigidBodyState& s,const Projectile& projectile,Wind& wind,double phi, double dt, DerivativeFn deriv);
	static Trajectory simulateSteps(const RigidBodyState& state, Projectile& projectile,Wind wind, double phi,double dt, DerivativeFn deriv, StopFn stop);
private:

};
