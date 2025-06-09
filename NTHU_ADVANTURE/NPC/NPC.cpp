#include "NPC.hpp"
#include "Player/Player.hpp"
#include "Engine/Resources.hpp"
#include "Engine/LOG.hpp"
#include <cmath>
#include <allegro5/allegro_primitives.h>

// 從 sprite sheet 載入子圖的輔助函數
static std::shared_ptr<ALLEGRO_BITMAP> LoadSpriteFromSheet(
    const std::string& sheetKey, int col, int row, 
    int tileW = 64, int tileH = 64) 
{
    auto sheetPtr = Engine::Resources::GetInstance().GetBitmap(sheetKey);
    if (!sheetPtr) return nullptr;

    ALLEGRO_BITMAP* sheet = sheetPtr.get();
    ALLEGRO_BITMAP* sub = al_create_sub_bitmap(sheet, col * tileW, row * tileH, tileW, tileH);
    return std::shared_ptr<ALLEGRO_BITMAP>(sub, [](ALLEGRO_BITMAP* bmp) {
        // 注意: 不實際刪除 sub-bitmap，由父圖管理生命週期
    });
}

// 從 sprite sheet 建構
NPC::NPC(const std::string& sheetPath, float x, float y, 
         int upCol, int upRow, int downCol, int downRow,
         int leftCol, int leftRow, int rightCol, int rightRow,
         int tileW, int tileH)
    : Engine::Sprite(sheetPath, x, y) 
{
    bmpIdle_up = LoadSpriteFromSheet(sheetPath, upCol, upRow, tileW, tileH);
    bmpIdle_down = LoadSpriteFromSheet(sheetPath, downCol, downRow, tileW, tileH);
    bmpIdle_left = LoadSpriteFromSheet(sheetPath, leftCol, leftRow, tileW, tileH);
    bmpIdle_right = LoadSpriteFromSheet(sheetPath, rightCol, rightRow, tileW, tileH);
    
    bmp = bmpIdle_down; // 預設朝下
    
    // 對齊到網格中心
    Position.x = std::round(Position.x / tileW) * tileW + tileW/2;
    Position.y = std::round(Position.y / tileH) * tileH + tileH/2;

    dialog.Initialize();
}

// 從分開的圖片建構
NPC::NPC(const std::string& upPath, const std::string& downPath,
         const std::string& leftPath, const std::string& rightPath,
         float x, float y)
    : Engine::Sprite(downPath, x, y) // 使用向下圖作為基礎
{
    bmpIdle_up = Engine::Resources::GetInstance().GetBitmap(upPath);
    bmpIdle_down = Engine::Resources::GetInstance().GetBitmap(downPath);
    bmpIdle_left = Engine::Resources::GetInstance().GetBitmap(leftPath);
    bmpIdle_right = Engine::Resources::GetInstance().GetBitmap(rightPath);
    
    bmp = bmpIdle_down; // 預設朝下
    
    // 自動計算圖塊大小 (假設所有方向圖片大小相同)
    if (bmpIdle_down) {
        int tileW = al_get_bitmap_width(bmpIdle_down.get());
        int tileH = al_get_bitmap_height(bmpIdle_down.get());
        Position.x = std::round(Position.x / tileW) * tileW + tileW/2;
        Position.y = std::round(Position.y / tileH) * tileH + tileH/2;
    }

    dialog.Initialize();
}

void NPC::Update(float deltaTime, const Player* player) {
    ALLEGRO_KEYBOARD_STATE kbState;
    al_get_keyboard_state(&kbState);

    // 檢查是否相鄰且按T鍵
    Engine::Point playerPos = player->Position;
    float distX = playerPos.x - Position.x;
    float distY = playerPos.y - Position.y;
    bool isAdjacent = (std::abs(distX) <= 64.0f && std::abs(distY) <= 64.0f);

    // 檢查Enter鍵是否剛被按下
    static bool enterWasDown = false;
    bool enterIsDown = al_key_down(&kbState, ALLEGRO_KEY_ENTER);
    bool enterPressed = enterIsDown && !enterWasDown;
    enterWasDown = enterIsDown;

    if (isAdjacent && al_key_down(&kbState, ALLEGRO_KEY_T)) {
        if (!isTalking && !messages.empty()) {
            FacePlayer(player);
            isTalking = true;
            dialog.StartDialog("NPC", bmpIdle_down, messages);
            // 重置Enter鍵狀態，避免立即觸發
            enterWasDown = true;
        }
    }

    // 更新對話框
    if (isTalking) {
        // 如果當前句子還沒顯示完，繼續更新動畫
        if (!dialog.IsCurrentMessageComplete()) {
            dialog.Update(deltaTime);
        }
        // 如果當前句子已顯示完且按下Enter，推進到下一句
        else if (enterPressed) {
            dialog.AdvanceDialog();
            
            // 如果對話結束
            if (!dialog.IsDialogActive()) {
                isTalking = false;
            }
        }
    }

    Engine::Sprite::Update(deltaTime);
}

void NPC::FacePlayer(const Player* player) {
    Engine::Point playerPos = player->Position;
    float dx = playerPos.x - Position.x;
    float dy = playerPos.y - Position.y;

    // 更精確的四方向判斷
    if (std::abs(dx) > std::abs(dy)) {
        // 水平方向
        bmp = (dx > 0) ? bmpIdle_right : bmpIdle_left;
    } else {
        // 垂直方向
        bmp = (dy > 0) ? bmpIdle_down : bmpIdle_up;
    }
}

void NPC::Draw() const {
    Engine::Sprite::Draw();

    // 繪製對話框
    if (isTalking) {
        dialog.Draw();
    }
}