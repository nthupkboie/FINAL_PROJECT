#include "NPCDialog.hpp"
#include "Engine/Resources.hpp"
#include "Engine/LOG.hpp"

#include <allegro5/allegro_primitives.h>


NPCDialog::NPCDialog() 
    : isActive(false),
      currentMessageIndex(0),
      charDisplayTimer(0.0f),
      charDisplayDelay(0.05f),  // 每個字顯示的間隔時間(秒)
      isDisplayingFullMessage(false),
      font(nullptr),
      boxWidth(600.0f),
      boxHeight(200.0f),
      boxX(50.0f),
      boxY(400.0f),
      padding(20.0f)
{
    textColor = al_map_rgb(255, 255, 255);  // 白色文字
    boxColor = al_map_rgba(0, 0, 0, 200);   // 半透明黑色背景
    nameColor = al_map_rgb(255, 215, 0);    // 金色NPC名字
    
    // 計算其他位置
    avatarX = boxX + padding;
    avatarY = boxY + padding;
    textX = boxX + padding + 128 + padding;  // 頭像寬度假設為128
    textY = boxY + padding + 30;  // 稍微下移避免與名字重疊
    nameX = textX;
    nameY = boxY + padding;
}

NPCDialog::~NPCDialog() {
    // 字體由Resources管理，不需要在這裡釋放
}

bool NPCDialog::Initialize() {
    // 使用 Resources 的 GetFont 方法，傳入字體名稱和大小
    font = Engine::Resources::GetInstance().GetFont("normal.ttf", 18).get();
    if (!font) {
        Engine::LOG(Engine::WARN) << "Failed to load font for NPCDialog";
        return false;
    }
    return true;
}

void NPCDialog::StartDialog(const std::string& npcName, 
                          std::shared_ptr<ALLEGRO_BITMAP> npcAvatar,
                          const std::vector<std::string>& messages) {
    if (messages.empty()) {
        Engine::LOG(Engine::WARN) << "Attempted to start dialog with no messages";
        return;
    }

    this->npcName = npcName;
    this->npcAvatar = npcAvatar;
    this->messages = messages;
    
    currentMessageIndex = 0;
    currentDisplayText.clear();
    charDisplayTimer = 0.0f;
    isDisplayingFullMessage = false;
    isActive = true;
}

void NPCDialog::Update(float deltaTime) {
    if (!isActive || isDisplayingFullMessage) return;

    // 逐字顯示效果
    charDisplayTimer += deltaTime;
    if (charDisplayTimer >= charDisplayDelay) {
        charDisplayTimer = 0.0f;
        
        const std::string& fullMessage = messages[currentMessageIndex];
        if (currentDisplayText.length() < fullMessage.length()) {
            currentDisplayText = fullMessage.substr(0, currentDisplayText.length() + 1);
        } else {
            isDisplayingFullMessage = true;
        }
    }
}

void NPCDialog::Draw() const {
    if (!isActive) return;

    // 繪製對話框背景
    al_draw_filled_rounded_rectangle(boxX, boxY, 
                                    boxX + boxWidth, boxY + boxHeight, 
                                    10, 10, boxColor);
    
    // 繪製NPC頭像
    if (npcAvatar) {
        al_draw_bitmap(npcAvatar.get(), avatarX, avatarY, 0);
    }
    
    // 繪製NPC名字
    if (font && !npcName.empty()) {
        al_draw_text(font, nameColor, nameX, nameY, ALLEGRO_ALIGN_LEFT, npcName.c_str());
    }
    
    // 繪製對話文字
    if (font && !currentDisplayText.empty()) {
        // 使用多行文字繪製
        float lineHeight = al_get_font_line_height(font);
        float currentY = textY;
        std::string remainingText = currentDisplayText;
        
        while (!remainingText.empty()) {
            // 計算這一行可以容納多少文字
            size_t spacePos = remainingText.find(' ');
            size_t lineEnd = remainingText.length();
            float textWidth = 0;
            
            // 簡單的文字換行處理
            for (size_t i = 0; i < remainingText.length(); ++i) {
                const char* testStr = remainingText.substr(0, i+1).c_str();
                textWidth = al_get_text_width(font, testStr);
                
                if (textWidth > (boxWidth - textX - padding * 2)) {
                    // 如果超過寬度，找前一個空格處斷行
                    if (spacePos != std::string::npos && spacePos < i) {
                        lineEnd = spacePos;
                    } else {
                        lineEnd = i;
                    }
                    break;
                }
                
                if (remainingText[i] == ' ') {
                    spacePos = i;
                }
            }
            
            std::string line = remainingText.substr(0, lineEnd);
            al_draw_text(font, textColor, textX, currentY, ALLEGRO_ALIGN_LEFT, line.c_str());
            
            if (lineEnd >= remainingText.length()) break;
            remainingText = remainingText.substr(lineEnd + 1);
            currentY += lineHeight;
        }
    }
}

bool NPCDialog::IsDialogActive() const {
    return isActive;
}

void NPCDialog::AdvanceDialog() {
    if (!isActive) return;

    if (!isDisplayingFullMessage) {
        // 如果還沒顯示完整訊息，直接顯示完整訊息
        currentDisplayText = messages[currentMessageIndex];
        isDisplayingFullMessage = true;
    } else {
        // 如果已經顯示完整訊息，跳到下一條
        currentMessageIndex++;
        if (currentMessageIndex < messages.size()) {
            currentDisplayText.clear();
            isDisplayingFullMessage = false;
        } else {
            // 沒有更多訊息了，結束對話
            EndDialog();
        }
    }
}

void NPCDialog::EndDialog() {
    isActive = false;
}