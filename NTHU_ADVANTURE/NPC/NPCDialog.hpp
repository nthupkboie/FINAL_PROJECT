#pragma once

#include <string>
#include <memory>
#include <vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

class NPCDialog {
public:
    NPCDialog();
    ~NPCDialog();

    // 初始化對話框資源
    bool Initialize();

    // 開始新的對話
    void StartDialog(const std::string& npcName, 
                    std::shared_ptr<ALLEGRO_BITMAP> npcAvatar,
                    const std::vector<std::string>& messages);

    // 更新對話框狀態 (逐字顯示)
    void Update(float deltaTime);

    // 繪製對話框
    void Draw() const;

    // 檢查對話是否正在進行
    bool IsDialogActive() const;

    // 跳到完整顯示當前訊息或跳到下一條訊息
    void AdvanceDialog();

    // 結束當前對話
    void EndDialog();

    // 新增方法
    bool IsCurrentMessageComplete() const {
        return isDisplayingFullMessage;
    }
    
    // 可選：獲取當前對話進度
    size_t GetCurrentMessageIndex() const {
        return currentMessageIndex;
    }
    
    // 可選：獲取總訊息數
    size_t GetMessageCount() const {
        return messages.size();
    }

    static bool talking;

    std::string DrawRandomLot();
private:
    // 對話狀態
    bool isActive;
    size_t currentMessageIndex;
    std::string currentDisplayText;
    float charDisplayTimer;
    float charDisplayDelay;
    bool isDisplayingFullMessage;

    // 對話內容
    std::vector<std::string> messages;
    std::string npcName;
    std::shared_ptr<ALLEGRO_BITMAP> npcAvatar;

    // 繪圖資源
    ALLEGRO_FONT* font;
    ALLEGRO_COLOR textColor;
    ALLEGRO_COLOR boxColor;
    ALLEGRO_COLOR nameColor;

    // 對話框尺寸和位置
    float boxWidth;
    float boxHeight;
    float boxX;
    float boxY;
    float avatarX;
    float avatarY;
    float textX;
    float textY;
    float nameX;
    float nameY;
    float padding;
};