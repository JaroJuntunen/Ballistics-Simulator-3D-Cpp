#pragma once

class Projectile {
public:
	Projectile(double mass, double diameter, double dragCoefficient, double twistRate, double stabilityFactor)
		: m_mass(mass), m_diameter(diameter), m_dragCoefficient(dragCoefficient),
			m_twistRate(twistRate), m_stabilityFactor(stabilityFactor) {}

	// getters
	double	getMass()				const { return m_mass; }
	double	getDiameter()			const { return m_diameter; }
	double	getDragCoefficient()	const { return m_dragCoefficient; }
	double	getTwistRate()			const { return m_twistRate; }
	double	getStabilityFactor()	const { return m_stabilityFactor; }

private:
	double	m_mass            = 0.0;
	double	m_diameter        = 0.0;
	double	m_dragCoefficient = 0.0;
	double	m_twistRate       = 3.937007874; //revolutions per meter
	double	m_stabilityFactor = 1.7;
};
