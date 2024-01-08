#pragma once
#include <unordered_map>

class TranslationManager {
public:

	bool Init(string lang);

	string GetText(const string& input); //input what ever you want to, it will try to find {...} and replace them with localized strings

	string DisableSensitiveChars(const string& input); //{ -> \{, , -> \,, etc, needed for moments when we have some user input and want to be safe

private:

	string EnableSensitiveChars(const string& input); //reverse of prev

	string m_lang = "en"; //language's code
	string m_name = "English"; //language's name

	unordered_map<string, string> m_texts; //actual translations, might also include default ones (English)

};