#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>

struct dragCdTableEntry {
	double	velocity; // in m/s
	double	dragCoefficient;
};

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
	double	getDragCoefficientAtVelocity(double velocity) const;

	//setters
	void	setDragTable(std::shared_ptr<std::vector<dragCdTableEntry>> dragtable) { m_dragtable = dragtable; }

private:

	std::shared_ptr<std::vector<dragCdTableEntry>>	m_dragtable = nullptr; // pointer to drag coefficient table
	double	m_mass            = 0.0; // in kg
	double	m_diameter        = 0.0;
	double	m_dragCoefficient = 0.0;
	double	m_twistRate       = 3.937007874; //revolutions per meter
	double	m_stabilityFactor = 1.7;
};
