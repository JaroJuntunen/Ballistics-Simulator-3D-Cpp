#pragma once
#include <glm/glm.hpp>
#include <string>
#include "simulation/core/RigidBodyState.hpp"
#include "simulation/projectiles/Projectile.hpp"
#include "simulation/physics/PhysicsConstants.hpp"

class Launcher {
public:
	Launcher(const glm::dvec3& position, double azimuth, double elevation, double speed)
		: m_position(position), m_azimuth(azimuth), m_elevation(elevation), m_speed(speed) {}

	RigidBodyState fire(Projectile projectile) const;

	// Getters
	glm::dvec3	getPosition()     const { return m_position; }
	double		getAzimuth()      const { return m_azimuth; }
	double		getElevation()    const { return m_elevation; }
	double		getSpeed()        const { return m_speed; }
	double		getLatitude()     const { return m_latitude; }
	double		getLatitudeInRad() const { return m_latitude * Physics::deg2rad; }
	const std::string& getLauncherType() const { return m_launcherType; }

	// Setters
	void		setPosition(const glm::dvec3& position) { m_position = position; }
	void		setAzimuth(double azimuth)    { m_azimuth = azimuth; }
	void		setElevation(double elevation) { m_elevation = elevation; }
	void		setSpeed(double speed)         { m_speed = speed; }
	void		setLatitude(double latitude)   { m_latitude = latitude; }
	void		setLauncherType(const std::string& type) { m_launcherType = type; }

private:
	glm::dvec3	m_position = {0.0, 0.0, 0.0};
	double		m_azimuth = 0.0;
	double		m_elevation = 45.0;
	double		m_speed = 100.0;

	double		m_latitude    = 60.0;
	std::string	m_launcherType = "";
};
