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
    std::ifstream file("Resource/lang_" + langCode + ".txt");



    // 檢查檔案是否成功開啟
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open language file: " << "Resource/lang_" + langCode + ".txt" << std::endl;
        return;
    }
    std::cout << "[DEBUG] Successfully opened: " << "Resource/lang_" + langCode + ".txt" << std::endl;



    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            dictionary[key] = value;
        }
    }
    std::cout << "[LANG] Current 'back' = " << (langCode == "zh" ? "返回" : "Back") << std::endl;

}
std::string LanguageManager::GetText(const std::string& key) const {
    auto it = dictionary.find(key);
    return it != dictionary.end() ? it->second : key;
}
