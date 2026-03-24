#pragma once

namespace Physics {
	// Gravity
	constexpr double g             = 9.80665;       // m/s² — standard gravity (ISA)

	// Earth rotation
	constexpr double earthOmega    = 7.2921150e-5;  // rad/s — sidereal rotation rate

	// ISA atmosphere (sea level)
	constexpr double rho0          = 1.225;         // kg/m³ — air density
	constexpr double T0            = 288.15;        // K     — temperature
	constexpr double lapseRate     = 0.0065;        // K/m   — temperature lapse rate
	constexpr double scaleHeight   = 8500.0;        // m     — exponential scale height (simplified)

	// Math
	constexpr double pi            = 3.14159265358979323846;
	constexpr double deg2rad       = pi / 180.0;
	constexpr double rad2deg       = 180.0 / pi;
}
