#pragma once

#include "simulation/core/Integrator.hpp"
#include "simulation/launchers/Launcher.hpp"
#include "simulation/environment/Terrain.hpp"
#include "simulation/physics/BallisticsModel.hpp"

struct FireSolution {
	double azimuth_deg  = 0.0;
	double elevation_deg = 0.0;
	double tof_s        = 0.0;
	bool   valid        = false;
};

typedef std::vector<FireSolution> SolvedFireSolutions;

class FireSolutionSolver {
	public:
		//returns if it found a fireSolution
		static SolvedFireSolutions solveFireSolutionForLauncher(const Launcher& launcher, Terrain& terrain, Projectile proj, Wind wind, glm::dvec3 target);

	private:
	static double	getLandingDistance(const Launcher &l,const glm::dvec3& landingPoint);
};