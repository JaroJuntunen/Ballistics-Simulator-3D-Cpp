#include "FireSolutionSolver.hpp"




SolvedFireSolutions FireSolutionSolver::solveFireSolutionForLauncher(const Launcher& launcher, Terrain& terrain, Projectile proj, Wind wind, glm::dvec3 target)
{
	SolvedFireSolutions solvedSolutions;
	StopFn stop = [&](const RigidBodyState& s) {
		return BallisticsModel::hasImpacted(s, terrain);
	};
	Launcher l = launcher;
	glm::dvec3 toTarget = target - l.getPosition();
	l.setAzimuth(glm::degrees(std::atan2(toTarget.y, toTarget.x)));
	double	distanceToTarget = glm::length(glm::dvec2(toTarget.x, toTarget.y));

	// find actual maximum range (drag shifts optimal angle below 45°)
	double maxRange = 0.0;
	RigidBodyState initialState = l.fire(proj);
	Trajectory trajectory;
	RigidBodyState landing;
	for (double testEl = 30.0; testEl <= 60.0; testEl += 5.0) {
		l.setElevation(testEl);
		initialState = l.fire(proj);
		trajectory = Integrator::simulateSteps(initialState, proj, wind, 0.01, BallisticsModel::derivative, stop);
		landing = trajectory.back();
		maxRange = std::max(maxRange, getLandingDistance(l, landing.position));
	}
	if (maxRange < distanceToTarget)
		return solvedSolutions;
	double initialAzimuth = l.getAzimuth();
	glm::dvec2 forward = glm::normalize(glm::dvec2(toTarget.x, toTarget.y));
	glm::dvec2 right   = { forward.y, -forward.x };

	for (int k = 0; k < 2; k++)
	{
		l.setAzimuth(initialAzimuth);

		for (int iterate = 0; iterate < 3; iterate++)
		{
			double angle[2] = { k * 45.0, 45.0 + k * 45.0 };
			while (angle[1] - angle[0] > 0.01)
			{
				double midAngle = (angle[0] + angle[1]) * 0.5;
				l.setElevation(midAngle);
				initialState = l.fire(proj);
				trajectory = Integrator::simulateSteps(initialState, proj, wind, 0.01, BallisticsModel::derivative, stop);
				landing = trajectory.back();
				bool landsShort = getLandingDistance(l, landing.position) < distanceToTarget;
				if (k == 0 ? landsShort : !landsShort)
					angle[0] = midAngle;
				else
					angle[1] = midAngle;
			}

			double lateralError = glm::dot(
				glm::dvec2(landing.position.x, landing.position.y) - glm::dvec2(target.x, target.y),
				right);
			if (std::abs(lateralError) < 0.5) break;
			l.setAzimuth(l.getAzimuth() + glm::degrees(std::atan2(lateralError, distanceToTarget)));
		}

		initialState = l.fire(proj);
		trajectory = Integrator::simulateSteps(initialState, proj, wind, 0.01, BallisticsModel::derivative, stop);
		landing = trajectory.back();

		glm::dvec2 landingXY = glm::dvec2(landing.position.x, landing.position.y);
		glm::dvec2 targetXY  = glm::dvec2(target.x, target.y);
		double totalError = glm::length(landingXY - targetXY);

		FireSolution sol;
		sol.azimuth_deg   = l.getAzimuth();
		sol.elevation_deg = l.getElevation();
		sol.tof_s         = (double)trajectory.size() * 0.01;
		sol.valid         = totalError <= 100.0;
		solvedSolutions.push_back(sol);
	}
	return solvedSolutions;
}

double FireSolutionSolver::getLandingDistance(const Launcher &l, const glm::dvec3 &landingPoint)
{
	return glm::length(glm::dvec2(landingPoint.x - l.getPosition().x, landingPoint.y - l.getPosition().y));
}
