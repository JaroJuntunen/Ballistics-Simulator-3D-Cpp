#include "app/Application.hpp"
#include <algorithm>

void Application::setMRSIinMotion(){
	if(!m_MRSI_onGoing){
		m_MRSI_onGoing = true;
		m_MRSI_timer = 0.0;
		m_MRSI_secuanceLength = 0.0;
		m_MRSI_launcherBurstLeft.clear();
		m_MRSI_ProjectileLaunched.clear();
		m_MRSI_launcherNextFireTime.clear();

		for (int i = 0; i < (int)m_launcher.size(); i++) {
			if (m_launcherSelected[i]) {
				std::sort(m_solvedFireSolutions[i].begin(), m_solvedFireSolutions[i].end(),
					[&](const FireSolution& a, const FireSolution& b) {
						return m_mrsiLongestTof ? a.tof_s > b.tof_s : a.tof_s < b.tof_s;
					});
			}
		}

		for (int i = 0; i < (int)m_launcher.size(); i++) {
			m_MRSI_ProjectileLaunched.push_back(std::vector<bool>(m_solvedFireSolutions[i].size(), false));
			m_MRSI_launcherBurstLeft.push_back(std::min(m_MRSI_burstSize, m_launcher[i].getMaxBurstRounds()));
			m_MRSI_launcherNextFireTime.push_back(0.0);
		}

		for (int i = 0; i < (int)m_launcher.size(); i++) {
			if (!m_launcherSelected[i]) continue;
			int burstCount = 0;
			for (int k = 0; k < (int)m_solvedFireSolutions[i].size(); k++) {
				if (!m_solvedFireSolutions[i][k].valid) continue;
				if (burstCount >= m_MRSI_launcherBurstLeft[i]) break;
				if (m_solvedFireSolutions[i][k].tof_s > m_MRSI_secuanceLength)
					m_MRSI_secuanceLength = m_solvedFireSolutions[i][k].tof_s;
				burstCount++;
			}
		}
	}
}

void Application::fireControlMRSI(double dt)
{
	if (m_MRSI_onGoing) {
		m_MRSI_timer += dt;
		for (int i = 0; i < (int)m_launcher.size(); i++) {
			if (!m_launcherSelected[i]) continue;
			for (int k = 0; k < (int)m_solvedFireSolutions[i].size(); k++) {

				if (m_MRSI_launcherBurstLeft[i] > 0 &&
					m_solvedFireSolutions[i][k].valid &&
					!m_MRSI_ProjectileLaunched[i][k] &&
					m_MRSI_timer >= m_MRSI_secuanceLength - m_solvedFireSolutions[i][k].tof_s &&
					m_MRSI_timer >= m_MRSI_launcherNextFireTime[i]) {

					m_launcher[i].setAzimuth(m_solvedFireSolutions[i][k].azimuth_deg);
					m_launcher[i].setElevation(m_solvedFireSolutions[i][k].elevation_deg);
					m_launcher[i].setSpeed(m_solvedFireSolutions[i][k].muzzle_velocity);
					Projectile proj = m_launcherProjectile[i];
					RigidBodyState initialState = m_launcher[i].fire(proj);
					m_trajectory.clear();
					m_trajectory.push_back(initialState);
					m_listOfProjectiles.push_back(proj);
					m_listOfTrajectories.push_back(m_trajectory);
					m_renderer.addTrajectory(m_trajectory);
					m_trajectoryAzimuths.push_back(m_launcher[i].getAzimuth());
					std::string label = "L" + std::to_string(i + 1);
					if (!m_launcher[i].getLauncherType().empty())
						label += " (" + m_launcher[i].getLauncherType() + ")";
					m_trajectoryLabels.push_back(label);
					m_MRSI_ProjectileLaunched[i][k] = true;
					m_MRSI_launcherBurstLeft[i]--;
					m_MRSI_launcherNextFireTime[i] = m_MRSI_timer + m_launcher[i].getReloadTime();
				}
			}
		}
		int stillToBeLaunched = 0;
		for (int i = 0; i < (int)m_launcher.size(); i++) {
			if (!m_launcherSelected[i]) continue;
			for (int k = 0; k < (int)m_solvedFireSolutions[i].size(); k++) {
				if (m_MRSI_launcherBurstLeft[i] > 0 &&
					m_solvedFireSolutions[i][k].valid &&
					!m_MRSI_ProjectileLaunched[i][k])
					stillToBeLaunched++;
			}
		}
		if (stillToBeLaunched == 0)
			m_MRSI_onGoing = false;
	}
};
