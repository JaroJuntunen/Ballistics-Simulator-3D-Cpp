#pragma once

#include <glm/glm.hpp>

class Wind {
public:
	Wind();
	~Wind() = default;

	glm::dvec3	getWindSpeed();
	void		passWindTime(double dt) { m_time += dt; }

	glm::dvec3	getBaseWindSpeed()    const { return m_baseWindSpeed; }
	double		getWindGustSeverity() const { return m_windGustSeverity; }
	double		getGustFrequency()    const { return m_gustFrequency; }

	void		setBaseWindSpeed(const glm::dvec3& speed) { m_baseWindSpeed = speed; }
	void		setWindGustSeverity(double severity)      { m_windGustSeverity = severity; }
	void		setGustFrequency(double frequency)        { m_gustFrequency = frequency; }

private:
	glm::dvec3	m_baseWindSpeed;
	double		m_windGustSeverity;
	double		m_gustFrequency;
	double		m_time;
};
