#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

struct dragCdTableEntry {
	double	velocity;        // in m/s
	double	dragCoefficient;
};
typedef std::vector<dragCdTableEntry> DragTable;

class Projectile {
public:
	Projectile(double mass, double diameter, double dragCoefficient, double twistRate, double stabilityFactor, double latitudeRad)
		: m_mass(mass), m_diameter(diameter), m_dragCoefficient(dragCoefficient),
			m_twistRate(twistRate), m_stabilityFactor(stabilityFactor), m_startLatitudeRad(latitudeRad) {}

	// getters
	double	getMass()				const { return m_mass; }
	double	getDiameter()			const { return m_diameter; }
	double	getDragCoefficient()	const { return m_dragCoefficient; }
	double	getTwistRate()			const { return m_twistRate; }
	double	getStabilityFactor()	const { return m_stabilityFactor; }
	double	getLatitudeRad()		const { return m_startLatitudeRad; }
	double	getDragCoefficientAtVelocity(double velocity) const;
	bool				getIsImpacted()		const { return m_isImpacted; }
	const std::string&	getProjectileType()	const { return m_projectileType; }

	// setters
	void	setMass(double mass)						{ m_mass = mass; }
	void	setDiameter(double diameter)				{ m_diameter = diameter; }
	void	setDragCoefficient(double cd)				{ m_dragCoefficient = cd; }
	void	setTwistRate(double twistRate)				{ m_twistRate = twistRate; }
	void	setStabilityFactor(double stabilityFactor)	{ m_stabilityFactor = stabilityFactor; }
	void	setIsImpacted(bool impacted)				{ m_isImpacted = impacted; }
	void	setDragTable(DragTable table)				{ m_dragtable = std::move(table); }
	void	setProjectileType(const std::string& entry)	{ m_projectileType = entry; }

private:
	DragTable	m_dragtable;
	double		m_mass            = 0.0; // in kg
	double		m_diameter        = 0.0;
	double		m_dragCoefficient = 0.0;
	double		m_twistRate       = 3.937007874; // revolutions per meter
	double		m_stabilityFactor = 1.7;
	double		m_startLatitudeRad   = 60.0;
	bool		m_isImpacted      = false;
	std::string	m_projectileType    = "";
};
