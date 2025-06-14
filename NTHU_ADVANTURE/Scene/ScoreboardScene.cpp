#include "ScoreboardScene.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "Engine/AudioHelper.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include "Engine/LanguageManager.hpp"
#include <iostream>
#include "Engine/Resources.hpp"
using namespace std;

void ScoreboardScene::Initialize() {

    cout << "ScoreboardScene::Initialize() started";

    labelTitle = nullptr;
    labelRankHeader = nullptr;
    LabelNameHeader = nullptr;
    LabelScoreHeader = nullptr;
    LabelStageHeader = nullptr;
    LabelTimeHeader = nullptr;
    LabelPrevLabel = nullptr;
    LabelNextLabel = nullptr;
    LabelBackLabel = nullptr;

    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    // 清除之前的動畫元素
    animatedElements.clear();
    
    // 載入分數
    LoadScores();
    
    if (scores.empty()) {
        AddSampleScores();
    }

    // 標題 - 從上方飛入
    // auto* title = new Engine::Label("Scoreboard", "pirulen.ttf", 80, halfW, -100, 255, 255, 255, 255, 0.5, 0.5);
    // AddNewObject(title);
    labelTitle = new Engine::Label("", "title.ttf", 80, halfW, -100, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(labelTitle);
    animatedElements.push_back({
        labelTitle,
        Engine::Point(halfW, -100),
        Engine::Point(halfW, 100),
        0.0f,  // 立即開始
        0.5f,  // 持續時間
        0.0f
    });

    // 顯示當前頁面的分數
    int startIdx = currentPage * entriesPerPage;
    int endIdx = std::min(startIdx + entriesPerPage, (int)scores.size());
    
    // 定義左對齊的起始位置和列間距
    const int leftMargin = 250;
    const int columnSpacing = 30;
    const int rankWidth = 150;
    const int nameWidth = 200;
    const int scoreWidth = 200;
    const int stageWidth = 200;
    const int timeWidth = 200;
    
    // 計算各列起始X位置
    int rankX = leftMargin;
    int nameX = rankX + rankWidth + columnSpacing;
    int scoreX = nameX + nameWidth + columnSpacing;
    int stageX = scoreX + scoreWidth + columnSpacing;
    int timeX = stageX + stageWidth + columnSpacing;
    
    // 列標題 - 從左側飛入
    int headerY = 190;
    //auto* rankHeader = new Engine::Label("Rank", "pirulen.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    // AddNewObject(rankHeader); 
    labelRankHeader = new Engine::Label("", "title.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    AddNewObject(labelRankHeader); 
    animatedElements.push_back({
        labelRankHeader,
        Engine::Point(-100, headerY),
        Engine::Point(rankX, headerY),
        0.2f,  // 延遲0.2秒
        0.4f,  // 持續時間
        0.0f
    });
    
    // auto* nameHeader = new Engine::Label("Player", "pirulen.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    // AddNewObject(nameHeader);
    LabelNameHeader = new Engine::Label("", "normal.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    AddNewObject(LabelNameHeader);
    animatedElements.push_back({
        LabelNameHeader,
        Engine::Point(-100, headerY),
        Engine::Point(nameX, headerY),
        0.3f,  // 延遲0.3秒
        0.4f,  // 持續時間
        0.0f
    });
    
    // auto* scoreHeader = new Engine::Label("Score", "pirulen.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    // AddNewObject(scoreHeader);
    LabelScoreHeader = new Engine::Label("", "title.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    AddNewObject(LabelScoreHeader);
    animatedElements.push_back({
        LabelScoreHeader,
        Engine::Point(-100, headerY),
        Engine::Point(scoreX, headerY),
        0.4f,  // 延遲0.4秒
        0.4f,  // 持續時間
        0.0f
    });
    
    // auto* stageHeader = new Engine::Label("Stage", "pirulen.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    // AddNewObject(stageHeader);
    LabelStageHeader = new Engine::Label("", "title.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    AddNewObject(LabelStageHeader);
    animatedElements.push_back({
        LabelStageHeader,
        Engine::Point(-100, headerY),
        Engine::Point(stageX, headerY),
        0.5f,  // 延遲0.5秒
        0.4f,  // 持續時間
        0.0f
    });
    
    // auto* timeHeader = new Engine::Label("Time", "pirulen.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    // AddNewObject(timeHeader);
    LabelTimeHeader = new Engine::Label("", "title.ttf", 30, -100, headerY, 255, 255, 0, 255, 0.0, 0.5);
    AddNewObject(LabelTimeHeader);
    animatedElements.push_back({
        LabelTimeHeader,
        Engine::Point(-100, headerY),
        Engine::Point(timeX, headerY),
        0.6f,  // 延遲0.6秒
        0.4f,  // 持續時間
        0.0f
    });
    
    // 分數條目 - 從右側飛入
    for (int i = startIdx; i < endIdx; i++) {
        const auto& entry = scores[i];
        int yPos = 250 + (i % entriesPerPage) * 50;
        
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", localtime(&entry.timestamp));
        
        // 排名
        auto *rankLabel = new Engine::Label(std::to_string(i+1) + ".", "pirulen.ttf", 30, 
                     w + 100, yPos, 255, 255, 255, 255, 0.0, 0.5);
        AddNewObject(rankLabel);
        animatedElements.push_back({
            rankLabel,
            Engine::Point(w + 100, yPos),
            Engine::Point(rankX, yPos),
            0.7f + (i - startIdx) * 0.1f,  // 延遲時間遞增
            0.4f,
            0.0f
        });
        
        // 玩家名稱
        auto *nameLabel = new Engine::Label(entry.playerName, "pirulen.ttf", 30, 
                     w + 100, yPos, 255, 255, 255, 255, 0.0, 0.5);
        AddNewObject(nameLabel);
        animatedElements.push_back({
            nameLabel,
            Engine::Point(w + 100, yPos),
            Engine::Point(nameX, yPos),
            0.7f + (i - startIdx) * 0.1f,
            0.4f,
            0.0f
        });
        
        // 分數
        auto *scoreLabel = new Engine::Label(std::to_string(entry.score), "pirulen.ttf", 30, 
                     w + 100, yPos, 255, 255, 255, 255, 0.0, 0.5);
        AddNewObject(scoreLabel);
        animatedElements.push_back({
            scoreLabel,
            Engine::Point(w + 100, yPos),
            Engine::Point(scoreX, yPos),
            0.7f + (i - startIdx) * 0.1f,
            0.4f,
            0.0f
        });
        
        // 關卡
        auto *stageLabel = new Engine::Label("Stage " + std::to_string(entry.stage), "pirulen.ttf", 30, 
                     w + 100, yPos, 255, 255, 255, 255, 0.0, 0.5);
        AddNewObject(stageLabel);
        animatedElements.push_back({
            stageLabel,
            Engine::Point(w + 100, yPos),
            Engine::Point(stageX, yPos),
            0.7f + (i - startIdx) * 0.1f,
            0.4f,
            0.0f
        });
        
        // 時間
        auto *timeLabel = new Engine::Label(timeStr, "pirulen.ttf", 30, 
                     w + 100, yPos, 255, 255, 255, 255, 0.0, 0.5);
        AddNewObject(timeLabel);
        animatedElements.push_back({
            timeLabel,
            Engine::Point(w + 100, yPos),
            Engine::Point(timeX, yPos),
            0.7f + (i - startIdx) * 0.1f,
            0.4f,
            0.0f
        });
    }

    // 頁面信息 - 淡入效果
    auto* pageInfo = new Engine::Label(
        "Page " + std::to_string(currentPage+1) + " of " + 
        std::to_string((scores.size() + entriesPerPage - 1) / entriesPerPage),
        "pirulen.ttf", 30, halfW, h - 35, 255, 255, 255, 0, 0.5, 0.5);
    //AddNewObject(pageInfo);
    animatedElements.push_back({
        pageInfo,
        Engine::Point(halfW, h - 35),
        Engine::Point(halfW, h - 35),
        1.2f,  // 延遲1.2秒
        0.5f,  // 持續時間
        0.0f
    });

    // 上一頁按鈕 - 從左側飛入
    if (currentPage > 0) {
        auto* prevBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 
                                             -300, h - 110, 200, 50);
        prevBtn->SetOnClickCallback([this]() {
            currentPage--;
            Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
        });
        AddNewControlObject(prevBtn);
        animatedElements.push_back({
            prevBtn,
            Engine::Point(-300, h - 110),
            Engine::Point(halfW - 250, h - 110),
            1.0f,  // 延遲1.0秒
            0.5f,  // 持續時間
            0.0f
        });
        
        LabelPrevLabel = new Engine::Label("", "pirulen.ttf", 24, -300, h - 85, 0, 0, 0, 255, 0.5, 0.5);
        AddNewObject(LabelPrevLabel);
        animatedElements.push_back({
            LabelPrevLabel,
            Engine::Point(-300, h - 85),
            Engine::Point(halfW - 150, h - 85),
            1.0f,
            0.5f,
            0.0f
        });
    }

    // 下一頁按鈕 - 從右側飛入
    if ((currentPage + 1) * entriesPerPage < scores.size()) {
        auto* nextBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 
                                             w + 300, h - 110, 200, 50);
        nextBtn->SetOnClickCallback([this]() {
            currentPage++;
            Engine::GameEngine::GetInstance().ChangeScene("scoreboard");
        });
        AddNewControlObject(nextBtn);
        animatedElements.push_back({
            nextBtn,
            Engine::Point(w + 300, h - 110),
            Engine::Point(halfW + 50, h - 110),
            1.0f,  // 延遲1.0秒
            0.5f,  // 持續時間
            0.0f
        });
        
        LabelNextLabel = new Engine::Label("NEXT PAGE", "pirulen.ttf", 24, w + 300, h - 85, 0, 0, 0, 255, 0.5, 0.5);
        AddNewObject(LabelNextLabel);
        animatedElements.push_back({
            LabelNextLabel,
            Engine::Point(w + 300, h - 85),
            Engine::Point(halfW + 150, h - 85),
            1.0f,
            0.5f,
            0.0f
        });
    }

    // 返回按鈕 - 從下方飛入
    auto* backBtn = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", 
                                         halfW + 500, h + 100, 200, 50);
    backBtn->SetOnClickCallback([]() {
        Engine::GameEngine::GetInstance().ChangeScene("start");
    });
    AddNewControlObject(backBtn);
    animatedElements.push_back({
        backBtn,
        Engine::Point(halfW + 500, h + 100),
        Engine::Point(halfW + 500, h - 60),
        1.2f,  // 延遲1.2秒
        0.6f,  // 持續時間
        0.0f
    });
    
    LabelBackLabel = new Engine::Label("BACK", "title.ttf", 24, halfW + 600, h + 125, 0, 0, 0, 255, 0.5, 0.5);
    AddNewObject(LabelBackLabel);
    animatedElements.push_back({
        LabelBackLabel,
        Engine::Point(halfW + 600, h + 125),
        Engine::Point(halfW + 600, h - 35),
        1.2f,
        0.6f,
        0.0f
    });

    auto* pageLabel = new Engine::Label(
    "Page " + std::to_string(currentPage+1) + " of " + 
    std::to_string((scores.size() + entriesPerPage - 1) / entriesPerPage),
    "pirulen.ttf", 30, halfW, h + 20, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(pageLabel);
    animatedElements.push_back({
        pageLabel,
        Engine::Point(halfW, h + 20),
        Engine::Point(halfW, h - 35),
        1.4f,
        0.6f,
        0.0f
    });

    currentLanguage = LanguageManager::GetInstance().GetCurrentLanguage();
    RefreshLabels();

    bgmId = AudioHelper::PlayBGM("scoreboard.ogg");
}

void ScoreboardScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
    
    // 更新所有動畫元素
    for (auto& element : animatedElements) {
        element.elapsed += deltaTime;
        
        if (element.elapsed >= element.delay) {
            float progress = (element.elapsed - element.delay) / element.duration;
            progress = std::min(progress, 1.0f);  // 限制在0-1之間
            
            // 計算當前位置 (線性插值)
            float x = element.startPos.x + (element.endPos.x - element.startPos.x) * progress;
            float y = element.startPos.y + (element.endPos.y - element.startPos.y) * progress;
            
            // 如果是Label
            if (auto* label = dynamic_cast<Engine::Label*>(element.object)) {
                if (element.startPos == element.endPos) { // 淡入效果
                    // label->color.a = static_cast<unsigned char>(255 * progress);
                } else {
                    label->Position.x = x;
                    label->Position.y = y;
                }
            } 
            // 如果是ImageButton
            else if (auto* button = dynamic_cast<Engine::ImageButton*>(element.object)) {
                button->Position.x = x;
                button->Position.y = y;
            }
        }
    }

    std::string newLang = LanguageManager::GetInstance().GetCurrentLanguage();
    if (newLang != currentLanguage) {
        currentLanguage = newLang;
        RefreshLabels();
    }
}

void ScoreboardScene::LoadScores() {
    scores.clear();
    std::ifstream file("scoreboard.dat");
    
    if (file.is_open()) {
        ScoreEntry entry;
        while (file >> entry.playerName >> entry.score >> entry.stage >> entry.timestamp) {
            scores.push_back(entry);
        }
        file.close();
    }
    
    // 排序分數
    std::sort(scores.begin(), scores.end());
    
    // 只保留前100名
    if (scores.size() > maxEntries) {
        scores.resize(maxEntries);
    }
}

void ScoreboardScene::SaveScores() {
    std::ofstream file("scoreboard.dat");
    
    for (const auto& entry : scores) {
        file << entry.playerName << " " << entry.score << " " 
             << entry.stage << " " << entry.timestamp << "\n";
    }
    
    file.close();
}

void ScoreboardScene::AddSampleScores() {
    // 添加一些範例分數
    time_t now = time(nullptr);
    scores = {
        {"Player1", 5000, 3, now - 86400},
        {"Player2", 4500, 2, now - 172800},
        {"Player3", 4000, 3, now - 259200},
        {"Player4", 3500, 1, now - 345600},
        {"Player5", 3000, 2, now - 432000}
    };
    
    // 保存範例分數
    SaveScores();
}

void ScoreboardScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    IScene::Terminate();
}

void ScoreboardScene::OnKeyDown(int keyCode) {
    // ESC鍵返回
    if (keyCode == ALLEGRO_KEY_ESCAPE) {
        Engine::GameEngine::GetInstance().ChangeScene("start");
    }
}

void ScoreboardScene::AddScore(const std::string& playerName, int score, int stage){
    ScoreEntry newEntry{playerName, score, stage, time(nullptr)};
    
    // 臨時載入分數
    std::vector<ScoreEntry> tempScores;
    std::ifstream file("scoreboard.dat");
    if (file.is_open()) {
        ScoreEntry entry;
        while (file >> entry.playerName >> entry.score >> entry.stage >> entry.timestamp) {
            tempScores.push_back(entry);
        }
        file.close();
    }
    
    // 添加新分數
    tempScores.push_back(newEntry);
    
    // 排序
    std::sort(tempScores.begin(), tempScores.end());
    
    // 只保留前100名
    if (tempScores.size() > 100) {
        tempScores.resize(100);
    }
    
    // 保存
    std::ofstream outFile("scoreboard.dat");
    for (const auto& entry : tempScores) {
        outFile << entry.playerName << " " << entry.score << " " 
                << entry.stage << " " << entry.timestamp << "\n";
    }
    outFile.close();
}

// void ScoreboardScene::RefreshLabels() {
//     labelTitle->SetText(LanguageManager::GetInstance().GetText("scoreboard"));
//     labelRankHeader->SetText(LanguageManager::GetInstance().GetText("RankHeader"));
//     LabelNameHeader->SetText(LanguageManager::GetInstance().GetText("NameHeader"));
//     LabelScoreHeader->SetText(LanguageManager::GetInstance().GetText("ScoreHeader"));
//     LabelStageHeader->SetText(LanguageManager::GetInstance().GetText("StageHeader"));
//     LabelTimeHeader->SetText(LanguageManager::GetInstance().GetText("TimeHeader"));
//     LabelPrevLabel->SetText(LanguageManager::GetInstance().GetText("PrevLabel"));
//     LabelNextLabel->SetText(LanguageManager::GetInstance().GetText("NextLabel"));
//     LabelBackLabel->SetText(LanguageManager::GetInstance().GetText("BackLabel"));
// }

void ScoreboardScene::RefreshLabels() {
    if (labelTitle) labelTitle->SetText(LanguageManager::GetInstance().GetText("scoreboard"));
    if (labelRankHeader) labelRankHeader->SetText(LanguageManager::GetInstance().GetText("RankHeader"));
    if (LabelNameHeader) LabelNameHeader->SetText(LanguageManager::GetInstance().GetText("NameHeader"));
    if (LabelScoreHeader) LabelScoreHeader->SetText(LanguageManager::GetInstance().GetText("ScoreHeader"));
    if (LabelStageHeader) LabelStageHeader->SetText(LanguageManager::GetInstance().GetText("StageHeader"));
    if (LabelTimeHeader) LabelTimeHeader->SetText(LanguageManager::GetInstance().GetText("TimeHeader"));
    
    // 只有當這些標籤存在時才設置文字
    if (LabelPrevLabel) LabelPrevLabel->SetText(LanguageManager::GetInstance().GetText("PrevLabel"));
    if (LabelNextLabel) LabelNextLabel->SetText(LanguageManager::GetInstance().GetText("NextLabel"));
    if (LabelBackLabel) LabelBackLabel->SetText(LanguageManager::GetInstance().GetText("BackLabel"));
}
