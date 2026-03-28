#include "wind.hpp"
#include "stb_perlin.h"

Wind::Wind()
{
	m_baseWindSpeed.x	= 0.0f;
	m_baseWindSpeed.y	= 0.0f;
	m_baseWindSpeed.z	= 0.0f;
	m_windGustSeverity	= 1.5f;
	m_gustFrequency		= 0.3f;
	m_time	= 0.0f;
}

glm::dvec3 Wind::getWindSpeed()
{
	
	float	windGustOffsetX	= stb_perlin_noise3(m_time * m_gustFrequency,   0.0, 0.0, 0.0, 0.0, 0.0);
	float	windGustOffsetY	= stb_perlin_noise3(m_time * m_gustFrequency, 100.0, 0.0, 0.0, 0.0, 0.0);
	float	windGustOffsetZ	= stb_perlin_noise3(m_time * m_gustFrequency, 0.0, 100.0, 0.0, 0.0, 0.0);
	windGustOffsetX *= m_windGustSeverity * glm::length(m_baseWindSpeed);
	windGustOffsetY *= m_windGustSeverity * glm::length(m_baseWindSpeed);
	windGustOffsetZ *= m_windGustSeverity * glm::length(m_baseWindSpeed);


	glm::dvec3	windSpeed;
	windSpeed.x	= m_baseWindSpeed.x + windGustOffsetX;
	windSpeed.y	= m_baseWindSpeed.y + windGustOffsetY;
	windSpeed.z	= m_baseWindSpeed.z + windGustOffsetZ;
	return windSpeed;
	return glm::dvec3();
}
