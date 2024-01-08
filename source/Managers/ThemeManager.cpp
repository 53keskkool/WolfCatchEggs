#include "PlatformPrecomp.h"
#include "ThemeManager.h"
#include "Core/json.hpp"

bool ThemeManager::Init(string name) {
	m_name = name;

	FileInstance file("game/" + m_name + "/params.json");
	if (!file.IsLoaded())
	{
		LogError("Couldn't open setting file for theme '%s', is it in right place?", m_name.c_str());
		return false;
	}

	try
	{
		nlohmann::json j = nlohmann::json::parse(file.GetAsChars());

		m_primaryColor = j["primaryColor"];
		m_secondaryColor = j["secondaryColor"];
		m_scrollbarColor = j["scrollbarColor"];
		m_textColor = j["textColor"];
		m_textShadowColor = j["textShadowColor"];
		m_floorHeight = j["floorHeight"];
		m_wolfHandsYUp = j["wolfHandsYUp"];
		m_wolfHandsYDown = j["wolfHandsYDown"];
		m_allowedShelvesOverlap = j["allowedShelvesOverlap"];
		m_eggSpawnCords.x = j["eggSpawnX"];
		m_eggSpawnCords.y = j["eggSpawnY"];
	}
	catch (std::exception& e)
	{
		LogError("Caught exception while loading theme '%s': %s", m_name.c_str(), e.what());
		return false;
	}

	return true;
}

vector<pair<string,string>> ThemeManager::GetListOfThemes() {
	vector<pair<string, string>> result;

	FileInstance file("game/themes.json");
	if (!file.IsLoaded())
	{
		LogError("Couldn't open themes file, is it in right place?");
		return vector<pair<string, string>>();
	}

	try
	{
		nlohmann::json j = nlohmann::json::parse(file.GetAsChars());

		nlohmann::json themes = j["themes"];
		for (auto o : themes) {
			result.push_back({ o["codename"], o["name"] });
		}
	}
	catch (std::exception& e)
	{
		LogError("Caught exception while loading themes list: %s", e.what());
		return vector<pair<string, string>>();
	}

	for (size_t i = 0; i < result.size(); i++)
	{
		//checking if the themes' setting files actually exist, so user don't see themes they can't choose
		if (!FileExists("game/" + result[i].first + "/params.json"))
		{
			result.erase(result.begin() + i);
			i--;
		}
	}

	return result;
}

string ThemeManager::GetFilename(string texture) {
	if (m_name == "default") return texture; //default textures will be stored in the root, so we can fallback to them

	string result = texture;
	result.insert(result.find('/') + 1, m_name + "/");

	if (FileExists(result)) return result;
	
	return texture; //falling back to default
}