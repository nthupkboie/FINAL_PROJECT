#include "NPCDialog.hpp"
#include "Engine/LOG.hpp"
#include <allegro5/allegro_primitives.h>

NPCDialog::NPCDialog() 
    : isActive(false), charDisplayTimer(0.0f), timePerChar(0.05f),
      currentMessageIndex(0), boxX(50), boxY(350), boxWidth(700), boxHeight(200),
      avatarX(70), avatarY(380), avatarSize(150),
      textX(240), textY(380), textWidth(480), textHeight(150) {
    
    InitializeResources();
}

NPCDialog::~NPCDialog() {
    // shared_ptr 會自動釋放資源
}

void NPCDialog::InitializeResources() {
    // 直接加載字體文件
    const std::string fontPath = "Resources/fonts/normal.ttf";
    const int fontSize = 24;
    
    // 嘗試加載字體
    ALLEGRO_FONT* loadedFont = al_load_ttf_font(fontPath.c_str(), fontSize, 0);
    if (!loadedFont) {
        Engine::LOG(Engine::WARN) << "Failed to load font: " << fontPath;
        // 使用內置字體作為備用
        loadedFont = al_create_builtin_font();
    }
    
    // 用 shared_ptr 管理字體生命週期
    font = std::shared_ptr<ALLEGRO_FONT>(
        loadedFont,
        [](ALLEGRO_FONT* f) {
            if (f && !al_is_font_addon_initialized()) {
                // 如果是內置字體且字體插件未初始化，不需要手動釋放
                return;
            }
            al_destroy_font(f);
        }
    );
    
    CreateDefaultDialogBox();
}

void NPCDialog::CreateDefaultDialogBox() {
    dialogBox = std::shared_ptr<ALLEGRO_BITMAP>(
        al_create_bitmap(boxWidth, boxHeight),
        [](ALLEGRO_BITMAP* bmp) { al_destroy_bitmap(bmp); }
    );
    
    // 繪製對話框樣式
    ALLEGRO_BITMAP* oldTarget = al_get_target_bitmap();
    al_set_target_bitmap(dialogBox.get());
    
    // 半透明黑色背景
    al_draw_filled_rounded_rectangle(
        0, 0, boxWidth, boxHeight, 10, 10,
        al_map_rgba(0, 0, 0, 200)
    );
    
    // 白色邊框
    al_draw_rounded_rectangle(
        0, 0, boxWidth, boxHeight, 10, 10,
        al_map_rgba(255, 255, 255, 255), 3
    );
    
    al_set_target_bitmap(oldTarget);
}

void NPCDialog::StartDialog(const std::string& npcName, 
                          std::shared_ptr<ALLEGRO_BITMAP> npcAvatar,
                          const std::vector<std::string>& messages) {
    this->npcName = npcName;
    this->npcAvatar = npcAvatar;
    this->messages.clear();
    
    for (const auto& msg : messages) {
        DialogMessage dm;
        dm.fullText = msg;
        dm.displayedText = "";
        dm.displayProgress = 0.0f;
        this->messages.push_back(dm);
    }
    
    currentMessageIndex = 0;
    isActive = true;
    charDisplayTimer = 0.0f;
}

void NPCDialog::Update(float deltaTime) {
    if (!isActive || currentMessageIndex >= messages.size()) return;
    
    DialogMessage& currentMsg = messages[currentMessageIndex];
    
    if (currentMsg.displayProgress < 1.0f) {
        charDisplayTimer += deltaTime;
        
        if (charDisplayTimer >= timePerChar) {
            charDisplayTimer = 0.0f;
            currentMsg.displayProgress = std::min(1.0f, currentMsg.displayProgress + (timePerChar * 2.0f));
            
            // 更新顯示的文字
            size_t charsToShow = static_cast<size_t>(
                currentMsg.fullText.length() * currentMsg.displayProgress
            );
            currentMsg.displayedText = currentMsg.fullText.substr(0, charsToShow);
        }
    }
}

void NPCDialog::Draw() const {
    if (!isActive || currentMessageIndex >= messages.size()) return;
    
    // 繪製對話框背景
    al_draw_bitmap(dialogBox.get(), boxX, boxY, 0);
    
    // 繪製NPC頭像
    if (npcAvatar) {
        al_draw_scaled_bitmap(
            npcAvatar.get(),
            0, 0, 
            al_get_bitmap_width(npcAvatar.get()), 
            al_get_bitmap_height(npcAvatar.get()),
            avatarX, avatarY,
            avatarSize, avatarSize,
            0
        );
        
        // 頭像邊框
        al_draw_rounded_rectangle(
            avatarX - 2, avatarY - 2,
            avatarX + avatarSize + 2, avatarY + avatarSize + 2,
            5, 5, al_map_rgb(255, 255, 255), 2
        );
    }
    
    // 繪製NPC名稱
    if (font && !npcName.empty()) {
        al_draw_text(
            font.get(),
            al_map_rgb(255, 255, 0), // 黃色名字
            avatarX + avatarSize / 2,
            avatarY - 30,
            ALLEGRO_ALIGN_CENTER,
            npcName.c_str()
        );
    }
    
    // 繪製當前消息
    const DialogMessage& currentMsg = messages[currentMessageIndex];
    if (font && !currentMsg.displayedText.empty()) {
        // 使用多行文本繪製
        al_draw_multiline_text(
            font.get(),
            al_map_rgb(255, 255, 255),
            textX, textY,
            textWidth, 30, // 每行高度
            ALLEGRO_ALIGN_LEFT,
            currentMsg.displayedText.c_str()
        );
    }
    
    // 如果當前消息完全顯示，繪製"繼續"提示
    if (currentMsg.displayProgress >= 1.0f) {
        al_draw_text(
            font.get(),
            al_map_rgb(200, 200, 200),
            boxX + boxWidth - 20,
            boxY + boxHeight - 30,
            ALLEGRO_ALIGN_RIGHT,
            "Press [T] to continue"
        );
    }
}

void NPCDialog::OnKeyPress(int keycode) {
    if (!isActive) return;
    
    if (keycode == ALLEGRO_KEY_T) {
        Advance();
    }
}

void NPCDialog::Advance() {
    if (currentMessageIndex >= messages.size()) {
        isActive = false;
        return;
    }
    
    DialogMessage& currentMsg = messages[currentMessageIndex];
    
    if (currentMsg.displayProgress < 1.0f) {
        // 跳過當前動畫，直接顯示完整文本
        currentMsg.displayProgress = 1.0f;
        currentMsg.displayedText = currentMsg.fullText;
    } else {
        // 移動到下一條消息
        currentMessageIndex++;
        
        if (currentMessageIndex >= messages.size()) {
            isActive = false;
        }
    }
}

bool NPCDialog::IsActive() const {
    return isActive;
}