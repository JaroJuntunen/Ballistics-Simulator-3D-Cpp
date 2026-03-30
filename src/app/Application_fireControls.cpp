#include "app/Application.hpp"

void Application::setMRSIinMotion(){
	if(!m_MRSI_onGoing){
		m_MRSI_onGoing = true;
		m_MRSI_timer = 0.0;
		m_MRSI_secuanceLength = 0.0;
		for (int i = 0; i < (int)m_launcher.size(); i++) {
			for (int k = 0; k < m_solvedFireSolutions[i].size(); k++) {
				m_MRSI_ProjectileLaunched[i][k] = false;
				if (m_launcherSelected[i] && m_solvedFireSolutions[i][k].valid && m_solvedFireSolutions[i][k].tof_s > m_MRSI_secuanceLength){
					m_MRSI_secuanceLength = m_solvedFireSolutions[i][k].tof_s;
				}
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
			for (int k = 0; k < m_solvedFireSolutions[i].size(); k++) {
				
				if (m_solvedFireSolutions[i][k].valid &&
					!m_MRSI_ProjectileLaunched[i][k] &&
					(m_MRSI_timer) >=  m_MRSI_secuanceLength - m_solvedFireSolutions[i][k].tof_s) {
					m_launcher[i].setAzimuth(m_solvedFireSolutions[i][k].azimuth_deg);
					m_launcher[i].setElevation(m_solvedFireSolutions[i][k].elevation_deg);
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
				}
			}
		}
		int stillToBeLaunched = 0;
		for (int i = 0; i < (int)m_launcher.size(); i++) {
			if (!m_launcherSelected[i]) continue;
			for (int k = 0; k < 2; k++) {
				if (m_solvedFireSolutions[i][k].valid &&
					!m_MRSI_ProjectileLaunched[i][k])
					stillToBeLaunched++;
			}
		}
		if (stillToBeLaunched == 0)
			m_MRSI_onGoing = false;
	}
};