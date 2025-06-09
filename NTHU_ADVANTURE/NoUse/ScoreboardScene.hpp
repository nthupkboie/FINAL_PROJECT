#ifndef ScoreboardScene_HPP
#define ScoreboardScene_HPP
#include <memory>
#include <string>

#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>

struct ScoreRecord {
    std::string score;
    std::string timestamp;  // 日期時間（例如 "2025-05-12 14:30:45"）
    std::string playerName;
    std::string mapId;              // 關卡
};

class ScoreboardScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::vector<std::string>scores;
    std::vector<std::string>names;
    std::vector<std::string>times;
    std::vector<std::string>mapIDs;
    std::vector<struct ScoreRecord>records;
    int current_page = 0; // 當前頁碼 從0開始
    int total_pages;
public:
    explicit ScoreboardScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    void LoadFromFile(void);
    void NextOnClick(int stage);
    void LastOnClick(int stage);
};

bool cmp(const ScoreRecord& a, const ScoreRecord& b);

#endif   // ScoreboardScene_HPP
