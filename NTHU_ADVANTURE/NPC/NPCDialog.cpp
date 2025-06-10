#include "NPCDialog.hpp"
#include "Engine/Resources.hpp"
#include "Engine/LOG.hpp"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro.h>

NPCDialog::NPCDialog() 
    : isActive(false),
      currentMessageIndex(0),
      charDisplayTimer(0.0f),
      charDisplayDelay(0.05f),  // 每個字顯示的間隔時間(秒)
      isDisplayingFullMessage(false),
      font(nullptr),
      boxWidth(800.0f),    // 加寬對話框
      boxHeight(200.0f),   // 對話框高度
      boxX(240.0f),        // 水平位置 (居中：1280/2 - 800/2 = 240)
      boxY(500.0f),        // 垂直位置 (靠近底部)
      padding(25.0f)       // 內邊距加大
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
    font = Engine::Resources::GetInstance().GetFont("normal.ttf", 24).get();
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

    // 繪製對話框背景 (增加圓角效果)
    al_draw_filled_rounded_rectangle(boxX, boxY, 
                                    boxX + boxWidth, boxY + boxHeight, 
                                    15, 15, boxColor);
    al_draw_rounded_rectangle(boxX, boxY, 
                             boxX + boxWidth, boxY + boxHeight, 
                             15, 15, al_map_rgb(255,255,255), 2);

    // 繪製NPC頭像 (增加邊框效果)
    if (npcAvatar) {
        float avatarSize = 128; // 頭像大小
        // 頭像背景圓框
        al_draw_filled_circle(avatarX + avatarSize/2, avatarY + avatarSize/2, 
                             avatarSize/2 + 5, al_map_rgb(50,50,50));
        // 實際頭像 (圓形裁切)
        al_draw_scaled_bitmap(npcAvatar.get(), 
                             0, 0, 
                             al_get_bitmap_width(npcAvatar.get()),
                             al_get_bitmap_height(npcAvatar.get()),
                             avatarX, avatarY, avatarSize, avatarSize, 0);
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