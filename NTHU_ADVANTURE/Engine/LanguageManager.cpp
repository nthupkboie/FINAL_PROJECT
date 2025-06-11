#include "LanguageManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

LanguageManager& LanguageManager::GetInstance() {
    static LanguageManager instance;
    return instance;
}

void LanguageManager::LoadLanguage(const std::string& langCode) {
    dictionary.clear();
    currentLanguage = langCode;
    std::ifstream file("Resource/lang_" + langCode + ".txt");

    std::cout << "[LANG] Try to load: " << "Resource/lang_" + langCode + ".txt" << std::endl;


    // 檢查檔案是否成功開啟
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open language file: " << "Resource/lang_" + langCode + ".txt" << std::endl;
        return;
    }
    std::cout << "[LANG] File opened successfully." << std::endl;
    



    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        // if (std::getline(iss, key, '=') && std::getline(iss, value)) {
        //     dictionary[key] = value;
        //     std::cout << "[LANG] Loaded key: " << key << " -> " << value << std::endl;
        // }
        auto trim = [](std::string& str) {
            str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
                return !std::isspace(ch);
            }));
            str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), str.end());
        };

        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            trim(key);
            trim(value);
            dictionary[key] = value;
            std::cout << "[LANG] Loaded key: [" << key << "] -> [" << value << "]" << std::endl;
        }
    }
    std::cout << "[LANG] Language set to: " << currentLanguage << std::endl;

    if (dictionary.find("settings") != dictionary.end())
        std::cout << "[DEBUG] settings key = " << dictionary["settings"] << std::endl;
    else
        std::cout << "[DEBUG] settings key not found!" << std::endl;

    std::cout << "[LANG] All loaded keys:" << std::endl;
    for (auto& pair : dictionary) {
        std::cout << " - [" << pair.first << "] = " << pair.second << std::endl;
    }


}
std::string LanguageManager::GetText(const std::string& key) const {
    auto it = dictionary.find(key);
    if (it != dictionary.end()) {
        return it->second;
    } else {
        std::cerr << "[LanguageManager] ⚠️ Key not found: '" << key << "'\n";
        return key;  // fallback
    }
}

const std::string& LanguageManager::GetCurrentLanguage() const {
    return currentLanguage;
}
