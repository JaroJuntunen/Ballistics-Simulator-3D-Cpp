#include "Application.hpp"

#include <fstream>
#include <filesystem>
#include <algorithm>

// ── Catalog loaders ───────────────────────────────────────────────────────────

std::vector<std::string> Application::loadLauncherCatalog()
{
	std::vector<std::string> catalog;
	const std::string path = "data/launchers";
	if (!std::filesystem::exists(path)) return catalog;
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (entry.path().extension() == ".json")
			catalog.push_back(entry.path().stem().string());
	}
	return catalog;
}

std::vector<std::string> Application::loadTerrainCatalog()
{
	std::vector<std::string> catalog;
	const std::string root = "data/terrain";
	if (!std::filesystem::exists(root)) return catalog;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
		if (entry.path().extension() == ".hgt")
			catalog.push_back(entry.path().string());
	}
	std::sort(catalog.begin(), catalog.end());
	return catalog;
}

// ── JSON loaders ──────────────────────────────────────────────────────────────

Launcher Application::loadLauncherFromJson(const std::string& fileName)
{
	std::ifstream file("data/launchers/" + fileName + ".json");
	if (!file.is_open()) return Launcher({0.0, 0.0, 0.0}, 0.0, 45.0, 900.0);
	json j = json::parse(file);

	glm::dvec3 position = {
		j["position"]["x"].get<double>(),
		j["position"]["y"].get<double>(),
		j["position"]["z"].get<double>()
	};
	double azimuth   = j["azimuth_deg"].get<double>();
	double elevation = j["elevation_deg"].get<double>();
	double latitude  = j["latitude_deg"].get<double>();

	m_compatibleProjectiles.clear();
	m_selectedProjectile = 0;
	for (const auto& cp : j["compatible_projectiles"]) {
		std::string name = cp["projectile"].get<std::string>();
		if (name.size() > 5 && name.substr(name.size() - 5) == ".json")
			name = name.substr(0, name.size() - 5);
		m_compatibleProjectiles.push_back({ name, cp["muzzle_velocity_ms"].get<double>() });
	}

	double speed = m_compatibleProjectiles.empty() ? 900.0 : m_compatibleProjectiles[0].muzzleVelocity;
	Launcher launcher(position, azimuth, elevation, speed);
	launcher.setLatitude(latitude);
	return launcher;
}

Projectile Application::loadProjectileFromJson(const std::string& fileName)
{
	std::ifstream file("data/projectiles/" + fileName + ".json");
	if (!file.is_open()) return Projectile(48, 0.155, 0.3, 0.322580645, 1.2);
	json j = json::parse(file);

	Projectile p(
		j["mass_kg"].get<double>(),
		j["diameter_m"].get<double>(),
		j["drag_coefficient_fallback"].get<double>(),
		j["twist_rate_rev_per_m"].get<double>(),
		j["stability_factor"].get<double>()
	);

	DragTable table;
	for (const auto& entry : j["drag_table"])
		table.push_back({ entry["velocity_ms"].get<double>(), entry["cd"].get<double>() });
	p.setDragTable(std::move(table));
	return p;
}

DragTable Application::loadDragTable()
{
	std::vector<dragCdTableEntry> newTable;
	newTable.push_back(dragCdTableEntry{0.0,    0.17});
	newTable.push_back(dragCdTableEntry{100.0,  0.17});
	newTable.push_back(dragCdTableEntry{200.0,  0.18});
	newTable.push_back(dragCdTableEntry{300.0,  0.23});
	newTable.push_back(dragCdTableEntry{350.0,  0.36});
	newTable.push_back(dragCdTableEntry{400.0,  0.47});
	newTable.push_back(dragCdTableEntry{450.0,  0.43});
	newTable.push_back(dragCdTableEntry{500.0,  0.38});
	newTable.push_back(dragCdTableEntry{600.0,  0.33});
	newTable.push_back(dragCdTableEntry{700.0,  0.29});
	newTable.push_back(dragCdTableEntry{800.0,  0.26});
	newTable.push_back(dragCdTableEntry{900.0,  0.24});
	newTable.push_back(dragCdTableEntry{1000.0, 0.22});
	return newTable;
}

// ── Scenario save / load ──────────────────────────────────────────────────────

void Application::saveScenario(const std::string& path)
{
	std::filesystem::create_directories("data/scenarios");
	json j;
	j["version"] = 1;

	if (m_selectedTerrain >= 0)
		j["terrain"] = m_terrainCatalog[m_selectedTerrain];
	else
		j["terrain"] = "";

	glm::dvec3 pos = m_launcher.getPosition();
	j["launcher"]["catalog_entry"]      = m_launcherCatalog.empty() ? "" : m_launcherCatalog[m_selectedLauncher];
	j["launcher"]["position"]           = { {"x", pos.x}, {"y", pos.y}, {"z", pos.z} };
	j["launcher"]["azimuth_deg"]        = m_launcher.getAzimuth();
	j["launcher"]["elevation_deg"]      = m_launcher.getElevation();
	j["launcher"]["muzzle_velocity_ms"] = m_launcher.getSpeed();

	j["projectile"]["catalog_entry"] = m_compatibleProjectiles.empty() ? "" : m_compatibleProjectiles[m_selectedProjectile].filename;

	glm::dvec3 wind = m_wind.getBaseWindSpeed();
	j["wind"]["base_x"]         = wind.x;
	j["wind"]["base_y"]         = wind.y;
	j["wind"]["base_z"]         = wind.z;
	j["wind"]["gust_severity"]  = m_wind.getWindGustSeverity();
	j["wind"]["gust_frequency"] = m_wind.getGustFrequency();

	std::ofstream file(path);
	file << j.dump(4);
}

void Application::loadScenario(const std::string& path)
{
	std::ifstream file(path);
	if (!file.is_open()) return;
	json j = json::parse(file);

	// Terrain
	std::string terrainPath = j.value("terrain", "");
	if (terrainPath.empty()) {
		switchTerrain("");
		m_selectedTerrain = -1;
	} else {
		auto it = std::find(m_terrainCatalog.begin(), m_terrainCatalog.end(), terrainPath);
		if (it != m_terrainCatalog.end())
			m_selectedTerrain = (int)std::distance(m_terrainCatalog.begin(), it);
		switchTerrain(terrainPath);
	}

	// Launcher
	std::string launcherEntry = j["launcher"].value("catalog_entry", "");
	if (!launcherEntry.empty()) {
		auto it = std::find(m_launcherCatalog.begin(), m_launcherCatalog.end(), launcherEntry);
		if (it != m_launcherCatalog.end()) {
			m_selectedLauncher = (int)std::distance(m_launcherCatalog.begin(), it);
			m_launcher = loadLauncherFromJson(launcherEntry);
		}
	}
	glm::dvec3 pos = {
		j["launcher"]["position"]["x"].get<double>(),
		j["launcher"]["position"]["y"].get<double>(),
		j["launcher"]["position"]["z"].get<double>()
	};
	pos.z = m_terrain->heightAt((float)pos.x, (float)pos.y) + 1.0;
	m_launcher.setPosition(pos);
	m_launcher.setAzimuth(j["launcher"].value("azimuth_deg", 45.0));
	m_launcher.setElevation(j["launcher"].value("elevation_deg", 45.0));
	m_launcher.setSpeed(j["launcher"].value("muzzle_velocity_ms", 900.0));

	// Projectile
	std::string projEntry = j["projectile"].value("catalog_entry", "");
	if (!projEntry.empty()) {
		auto it = std::find_if(m_compatibleProjectiles.begin(), m_compatibleProjectiles.end(),
			[&](const CompatibleProjectile& cp) { return cp.filename == projEntry; });
		if (it != m_compatibleProjectiles.end()) {
			m_selectedProjectile = (int)std::distance(m_compatibleProjectiles.begin(), it);
			m_projectile = loadProjectileFromJson(projEntry);
			m_launcher.setSpeed(it->muzzleVelocity);
		}
	}

	// Wind
	m_wind.setBaseWindSpeed({
		j["wind"].value("base_x", 0.0),
		j["wind"].value("base_y", 0.0),
		j["wind"].value("base_z", 0.0)
	});
	m_wind.setWindGustSeverity(j["wind"].value("gust_severity", 0.0));
	m_wind.setGustFrequency(j["wind"].value("gust_frequency", 0.0));
}

// ── CSV export ────────────────────────────────────────────────────────────────

void Application::exportTrajectoryTableToCSV(const Trajectory& t)
{
	std::filesystem::create_directories("Exports");
	std::string path = "Exports/trajectoryExport.csv";
	for (int i = 1; std::filesystem::exists(path); ++i)
		path = "Exports/trajectoryExport_" + std::to_string(i) + ".csv";
	std::ofstream file(path);
	if (!file.is_open()) return;
	const char sep = (m_csvSeparator == 1) ? ';' : (m_csvSeparator == 2) ? '\t' : ',';
	file << "Range (m)" << sep << "Height(m)" << sep << "Drift(m)" << sep << "Speed(m/s)" << sep << "TOF (s)\n";

	double azRad = glm::radians(m_launcher.getAzimuth());
	glm::dvec2 right = { std::sin(azRad), -std::cos(azRad) };
	const double sampleInterval = 500.0;
	const glm::dvec3 origin     = t.front().position;
	const double physDt         = 0.01;
	double nextSample           = 0.0;
	int    index                = 0;
	for (const auto& state : t) {
		glm::dvec2 horiz = {
			state.position.x - origin.x,
			state.position.y - origin.y };
		double range = glm::length(horiz);
		if (range >= nextSample) {
			double height = state.position.z - origin.z;
			double drift  = glm::dot(horiz, right);
			double speed  = glm::length(state.velocity);
			double tof    = index * physDt;
			file << range << sep << height << sep << drift << sep << speed << sep << tof << "\n";
			nextSample += sampleInterval;
		}
		++index;
	}
	file.close();
}
