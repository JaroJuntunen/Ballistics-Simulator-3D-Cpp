#pragma once
#include "RigidBodyState.hpp"
#include <functional>
#include <vector>

using DerivativeFn = std::function<RigidBodyState(const RigidBodyState&)>;
using StopFn = std::function<bool(const RigidBodyState&)>;

class Integrator {
public:
	static RigidBodyState step(const RigidBodyState& s, double dt, DerivativeFn deriv);
	static std::vector<RigidBodyState> simulateSteps(const RigidBodyState& state, double dt, DerivativeFn deriv, StopFn stop);
private:

};
