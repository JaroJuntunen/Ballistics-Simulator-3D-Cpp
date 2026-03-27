#pragma once

#include "Terrain.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cmath>

// SRTM3 terrain backend loaded from a .hgt file.
// File format: 1201x1201 int16_t samples, big-endian, row-major.
// Rows go north to south, columns go west to east.
// First sample is the NW corner, last is the SE corner.
// Void/no-data samples are -32768.
class SRTMTerrain : public Terrain {
public:
    // Loads a single .hgt tile from path.
    // The filename must follow SRTM convention: e.g. N60E025.hgt
    // Origin (world 0,0) is placed at the SW corner of the tile.
    explicit SRTMTerrain(const std::string& path);

    // Elevation in metres at world position (x, y).
    // x increases east, y increases north.
    // Returns 0 if the query falls outside the tile.
    float heightAt(float x, float y) const override;

    // East-west extent of the tile in world metres.
    float width()  const override { return m_widthM; }

    // North-south extent of the tile in world metres.
    float height() const override { return m_heightM; }

    // Origin of the tile in geographic coordinates (SW corner).
    double originLat() const { return m_originLat; }
    double originLon() const { return m_originLon; }

private:
    // Parses "N60E025" from filename to set m_originLat and m_originLon.
    void parseFilename(const std::string& filename);

    // Bilinear interpolation between the four surrounding grid samples.
    float sampleBilinear(double col, double row) const;

    // Reads a single grid sample, clamped to tile bounds.
    // Returns 0 for void samples (-32768).
    int16_t sampleAt(int row, int col) const;

    std::vector<int16_t> m_data;    // raw elevation samples, row-major N→S
    int    m_size;                  // samples per edge (1201 for SRTM3)
    double m_originLat;             // latitude of SW corner in degrees
    double m_originLon;             // longitude of SW corner in degrees
    float  m_widthM;                // east-west extent in metres
    float  m_heightM;               // north-south extent in metres
};
