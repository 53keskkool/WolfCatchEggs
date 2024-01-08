#include "PlatformPrecomp.h"
#include "App.h"
#include "TranslationManager.h"
#include "Core/json.hpp"
#include <fstream>

bool TranslationManager::Init(string lang)
{
	m_texts.clear();

	m_lang = lang;
	string filePath = "interface/texts/" + lang + ".json";
	FileInstance file(filePath, false);
	if (!file.IsLoaded())
	{
		LogError("Couldn't open translation file %s, is it in right place?", filePath.c_str());
		return false;
	}

	try
	{
		nlohmann::json j = nlohmann::json::parse(file.GetAsChars());

		m_name = j["name"];

		nlohmann::json strings = j["strings"];
		for (auto o : strings.items()) {
			m_texts[o.key()] = o.value().get<string>();
		}
	}
	catch (std::exception& e)
	{
		LogError("Caught exception while loading translation '%s': %s", lang.c_str(), e.what());
		return false;
	}

	string defaultFilePath = "interface/texts/en.json";
	if (defaultFilePath != filePath)
	{
		FileInstance defaultFile(defaultFilePath, false);
		if (defaultFile.IsLoaded())
		{
			try
			{
				nlohmann::json j = nlohmann::json::parse(defaultFile.GetAsChars());

				nlohmann::json strings = j["strings"];
				if (strings.size() > m_texts.size()) {
					for (auto& o : strings.items()) {
						if (m_texts.find(o.key()) == m_texts.end()) {
							m_texts[o.key()] = o.value().get<string>();
							if (strings.size() <= m_texts.size()) break;
						}
					}
				}
			}
			catch (std::exception& e)
			{
				LogError("Caught exception while loading default translation 'en': %s", e.what());
			}
		}
		else LogError("Couldn't open default translation 'en', skipping it...");
	}

	return true;
}

string TranslationManager::GetText(const string &input)
{
	if (input.find('{') == string::npos) return input; //there are no text that needs translation
	
	string output = input;
	size_t start = 0; //where we start to find our vars...

	while (true)
	{
		size_t strStart = string::npos;
		size_t strEnd = string::npos;

		uint16 openBrackets = 0;
		size_t brStart = 0;
		for (size_t i = start; i < output.length(); i++)
		{
			if (output[i] == '\\')
			{ //this means we ignore next character
				i++;
				continue;
			}

			if (output[i] == '{')
			{
				openBrackets++;
				if (openBrackets == 1) strStart = i;
				if (openBrackets == 2) brStart = i;
			}
			else if (output[i] == '}')
			{
				if (openBrackets == 2)
				{
					output.replace(brStart, i - brStart + 1, DisableSensitiveChars(GetText(output.substr(brStart, i - brStart + 1))));
					i = brStart - 1;
				}
				else if (openBrackets == 1)
				{
					strEnd = i;
					break;
				}
				if (openBrackets > 0) openBrackets--;
			}
		}

		if (strStart == string::npos || strEnd == string::npos) break;

		string str = output.substr(strStart + 1, strEnd - strStart - 1);

		vector<string> parms;
		size_t lastParmPos = 0;
		for (size_t i = 0; i < str.length(); i++)
		{
			if (str[i] == '\\')
			{
				i++;
				continue;
			}

			if (str[i] == ',')
			{
				parms.push_back(str.substr(lastParmPos, i - lastParmPos));
				lastParmPos = i + 1;
			}
		}
		parms.push_back(str.substr(lastParmPos));

		if (parms.size() < 1)
		{
		FAILED:
			start = strEnd + 1;
			continue;
		}
		for (auto& p : parms) p = EnableSensitiveChars(p);
		if (parms[0].length() < 1) goto FAILED; //something's wrong
		if (m_texts.find(parms[0]) == m_texts.end()) goto FAILED;
		int digit = 1;
		if (parms[0][0] == '!' && parms.size() >= 2)
		{ //that's a plural
			digit = abs(atoi(parms[1].c_str()));
			parms.erase(parms.begin() + 1);
		}

		str = m_texts[parms[0]];
		if (digit != 1) {
			if (m_lang == "ru")
			{
				vector<string> pluralVariation = StringTokenize(str, ",");
				if (pluralVariation.size() == 3) {
					digit %= 100;
					if (digit / 10 != 1) digit %= 10;
					if (digit == 1) str = pluralVariation[0];
					else if (digit >= 2 && digit <= 4) str = pluralVariation[1];
					else str = pluralVariation[2];
				}
				else if (pluralVariation.size() > 0) str = pluralVariation[0];
			}
		}
		parms.erase(parms.begin());

		if (parms.size() > 0)
		{
			size_t varStart = 0;
			while ((varStart = str.find('{', varStart)) != string::npos)
			{
				if (varStart > 0)
				{
					if (str[varStart - 1] == '\\')
					{
						varStart++;
						continue;
					}
				}

				size_t varEnd = str.find('}', varStart);
				while (varEnd != string::npos)
				{
					if (varEnd < 1) break;
					if (str[varEnd - 1] != '\\') break;
					varEnd = str.find('}', varEnd + 1);
				}
				if (varEnd == string::npos)
				{
					break; //there are no valid vars here
				}

				size_t varLen = varEnd - varStart - 1;
				string var = str.substr(varStart + 1, varLen);
				if (var.length() < 1) continue;
				vector<string> varParms = StringTokenize(var, ",");
				if (varParms.size() < 1) continue; //invalid var
				
				uint16 varNum = atoi(varParms[0].c_str());
				if (varNum >= parms.size()) continue;

				if (varParms.size() < 2) str.replace(varStart, varLen + 2, parms[varNum]);
				else
				{
					if (varParms[1].length() == 1)
					{
						switch (varParms[1][0])
						{
						default:
							str.replace(varStart, varLen + 2, parms[varNum]);
							break;
						}
					}
				}
			}
		}

		str = DisableSensitiveChars(str);
		output.replace(strStart, strEnd - strStart + 1, str);
		start = strStart + str.length();
	}

	return EnableSensitiveChars(output);
}

string TranslationManager::DisableSensitiveChars(const string& input)
{
	string output = input;
	size_t pos = 0;
	while ((pos = output.find('\\', pos)) != string::npos)
	{
		output.replace(pos, 1, "\\\\");
		pos += 2;
	}
	pos = 0;
	while ((pos = output.find('{', pos)) != string::npos)
	{
		output.replace(pos, 1, "\\{");
		pos += 2;
	}
	pos = 0;
	while ((pos = output.find('}', pos)) != string::npos)
	{
		output.replace(pos, 1, "\\}");
		pos += 2;
	}
	pos = 0;
	while ((pos = output.find(',', pos)) != string::npos)
	{
		output.replace(pos, 1, "\\,");
		pos += 2;
	}
	pos = 0;
	return output;
}

string TranslationManager::EnableSensitiveChars(const string& input)
{
	if (input.find('\\') == string::npos) return input;

	string output = input;
	size_t pos = 0;
	while ((pos = output.find("\\,", pos)) != string::npos)
	{
		output.replace(pos, 2, ",");
		pos++;
	}
	pos = 0;
	while ((pos = output.find("\\}", pos)) != string::npos)
	{
		output.replace(pos, 2, "}");
		pos++;
	}
	pos = 0;
	while ((pos = output.find("\\{", pos)) != string::npos)
	{
		output.replace(pos, 2, "{");
		pos++;
	}
	pos = 0;
	while ((pos = output.find("\\\\", pos)) != string::npos)
	{
		output.replace(pos, 2, "\\");
		pos++;
	}
	pos = 0;
	return output;
}