// ScoreboardScene.hpp
#pragma once
#include <vector>
#include <utility>
#include <string>
#include <allegro5/allegro.h>
#include "Engine/IScene.hpp"
#include "Engine/Point.hpp"
#include <allegro5/allegro_audio.h>

struct AnimatedElement {
    Engine::IObject* object;
    Engine::Point startPos;
    Engine::Point endPos;
    float delay;
    float duration;
    float elapsed;
};

class ScoreboardScene : public Engine::IScene {
private:
    struct ScoreEntry {
        std::string playerName;
        int score;
        //int stage;
        time_t timestamp;
        
        bool operator<(const ScoreEntry& other) const {
            // 按分數降序排序
            return score < other.score;
        }
    };
    
    std::vector<ScoreEntry> scores;
    int currentPage = 0;
    const int entriesPerPage = 10;
    const int maxEntries = 100;
    std::vector<AnimatedElement> animatedElements;  // <-- 在這裡宣告
    
    void LoadScores();
    void SaveScores();
    void AddSampleScores(); // 測試用
public:
    ALLEGRO_SAMPLE_ID bgmId;
    ScoreboardScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void OnKeyDown(int keyCode) override;
    static void AddScore(const std::string&playerName, int score);
};