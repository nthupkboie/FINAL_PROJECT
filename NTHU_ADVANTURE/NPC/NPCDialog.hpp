#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <string>
#include <vector>
#include <memory>

class NPCDialog {
public:
    NPCDialog();
    ~NPCDialog();

    void StartDialog(const std::string& npcName, 
                   std::shared_ptr<ALLEGRO_BITMAP> npcAvatar,
                   const std::vector<std::string>& messages);
    void Update(float deltaTime);
    void Draw() const;
    void OnKeyPress(int keycode);
    void Advance();
    bool IsActive() const;

private:
    struct DialogMessage {
        std::string fullText;
        std::string displayedText;
        float displayProgress;
    };

    bool isActive;
    std::string npcName;
    std::shared_ptr<ALLEGRO_BITMAP> npcAvatar;
    std::vector<DialogMessage> messages;
    size_t currentMessageIndex;
    
    // 直接管理字體資源
    std::shared_ptr<ALLEGRO_FONT> font;
    std::shared_ptr<ALLEGRO_BITMAP> dialogBox;
    
    float charDisplayTimer;
    float timePerChar;
    
    // 對話框尺寸和位置
    float boxX, boxY, boxWidth, boxHeight;
    float avatarX, avatarY, avatarSize;
    float textX, textY, textWidth, textHeight;
    
    void InitializeResources();
    void CreateDefaultDialogBox();
};