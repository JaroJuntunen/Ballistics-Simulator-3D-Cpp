#pragma once

class Projectile {
public:
	Projectile(double mass, double diameter, double dragCoefficient)
		: m_mass(mass), m_diameter(diameter), m_dragCoefficient(dragCoefficient) {}

	// getters
	double	getMass()             const { return m_mass; }
	double	getDiameter()         const { return m_diameter; }
	double	getDragCoefficient()  const { return m_dragCoefficient; }

private:
	double	m_mass            = 0.0;
	double	m_diameter        = 0.0;
	double	m_dragCoefficient = 0.0;
};
