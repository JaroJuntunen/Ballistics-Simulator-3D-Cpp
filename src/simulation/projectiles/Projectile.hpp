#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>

struct dragCdTableEntry {
	double	velocity; // in m/s
	double	dragCoefficient;
};
typedef std::shared_ptr<std::vector<dragCdTableEntry>> DragTable;

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
	bool	getIsImpacted()			const { return m_isImpacted; }

	// setters
	void	setMass(double mass)						{ m_mass = mass; }
	void	setDiameter(double diameter)				{ m_diameter = diameter; }
	void	setDragCoefficient(double cd)				{ m_dragCoefficient = cd; }
	void	setTwistRate(double twistRate)				{ m_twistRate = twistRate; }
	void	setStabilityFactor(double stabilityFactor)	{ m_stabilityFactor = stabilityFactor; }
	void	setIsImpacted(bool Impacted)					{ m_isImpacted = Impacted; }

	//setters
	void	setDragTable(DragTable dragtable) { m_dragtable = dragtable; }

private:

	DragTable	m_dragtable = nullptr; // pointer to drag coefficient table
	double	m_mass            = 0.0; // in kg
	double	m_diameter        = 0.0;
	double	m_dragCoefficient = 0.0;
	double	m_twistRate       = 3.937007874; //revolutions per meter
	double	m_stabilityFactor = 1.7;
	bool	m_isImpacted        = false;
};
