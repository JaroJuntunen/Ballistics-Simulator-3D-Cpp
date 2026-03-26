#include "Projectile.hpp"

double Projectile::getDragCoefficientAtVelocity(double velocity) const
{
	if (m_dragtable.empty()) {
		return m_dragCoefficient; // return base drag coefficient if no table is available
	}
	double dragCoefficient = m_dragCoefficient; // default to the base drag coefficient
	auto i = std::lower_bound(m_dragtable.begin(), m_dragtable.end(), velocity,
		[](const dragCdTableEntry& entry, double velocity) { return entry.velocity < velocity; });
	if (i == m_dragtable.end()) {
		i = std::prev(i);
		dragCoefficient = i->dragCoefficient;
	}
	else if (i == m_dragtable.begin()) {
		dragCoefficient = i->dragCoefficient;
	}
	else {
		dragCdTableEntry higherSpeedEntry = *i;
		dragCdTableEntry lowerSpeedEntry = *std::prev(i);
		double velocityRange = higherSpeedEntry.velocity - lowerSpeedEntry.velocity;
		double velocityFraction = (velocity - lowerSpeedEntry.velocity) / velocityRange;
		dragCoefficient = lowerSpeedEntry.dragCoefficient + velocityFraction * (higherSpeedEntry.dragCoefficient - lowerSpeedEntry.dragCoefficient);
	}
	return dragCoefficient;
}
