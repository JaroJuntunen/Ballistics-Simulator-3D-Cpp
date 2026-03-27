#include "simulation/environment/SRTMTerrain.hpp"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SRTMTerrain::SRTMTerrain(const std::string &path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open())
		return;
	m_size = 1201;
	m_data.resize(m_size * m_size);
	file.read((char *)m_data.data(), m_size * m_size * sizeof(int16_t));
	for (int16_t &dataEntry : m_data)
	{
		int8_t *toBeFlipped = (int8_t *)&dataEntry;
		dataEntry = ((uint8_t)toBeFlipped[0] << 8) + (uint8_t)toBeFlipped[1];
	}
	// Extract just the stem (e.g. "N60E025") from the full path for coordinate parsing
	parseFilename(std::filesystem::path(path).stem().string());
	m_heightM = 1.0f * 111320.0f;
	m_widthM  = 1.0f * 111320.0f * (float)std::cos(m_originLat * M_PI / 180.0);
}

float SRTMTerrain::heightAt(float x, float y) const
{
	if (x < 0.0f || x > m_widthM || y < 0.0f || y > m_heightM)
		return 0.0f;

	double fracCol = (x / m_widthM)          * (m_size - 1);
	double fracRow = (1.0 - y / m_heightM)   * (m_size - 1);

	return sampleBilinear(fracCol, fracRow);
}

float SRTMTerrain::sampleBilinear(double col, double row) const
{
	int c0 = (int)col;
	int r0 = (int)row;
	int c1 = c0 + 1;
	int r1 = r0 + 1;

	float tC = (float)(col - c0);
	float tR = (float)(row - r0);

	float h00 = (float)sampleAt(r0, c0);
	float h10 = (float)sampleAt(r0, c1);
	float h01 = (float)sampleAt(r1, c0);
	float h11 = (float)sampleAt(r1, c1);

	float top    = h00 + tC * (h10 - h00);
	float bottom = h01 + tC * (h11 - h01);
	return top + tR * (bottom - top);
}

int16_t SRTMTerrain::sampleAt(int row, int col) const
{
	if (row < 0) row = 0;
	if (col < 0) col = 0;
	if (row >= m_size) row = m_size - 1;
	if (col >= m_size) col = m_size - 1;

	int16_t v = m_data[row * m_size + col];
	return (v == -32768) ? 0 : v;  
}

void SRTMTerrain::parseFilename(const std::string &filename)
{
	m_originLat = std::stod(filename.substr(1, 2));
	if (filename[0] == 'S') m_originLat = -m_originLat;

	m_originLon = std::stod(filename.substr(4, 3));
	if (filename[3] == 'W') m_originLon = -m_originLon;
}
