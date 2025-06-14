#include "NPC.hpp"
#include "Player/Player.hpp"
#include "Engine/Resources.hpp"
#include "Engine/LOG.hpp"
#include <cmath>
#include <allegro5/allegro_primitives.h>
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "Scene/WindCloudScene.hpp"

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
NPC::NPC(const std::string& name,std::shared_ptr<ALLEGRO_BITMAP> avatar, const std::string& sheetPath, float x, float y, 
         int upCol, int upRow, int downCol, int downRow,
         int leftCol, int leftRow, int rightCol, int rightRow,
         int tileW, int tileH)
    : Engine::Sprite(sheetPath, x, y), npcName(name), npcAvatar(avatar ? avatar : bmpIdle_down)  // 如果沒有提供頭像，默認使用朝下的圖像
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

// 從分開的圖片建構 (完全對照Player方式)
NPC::NPC(const std::string& name, std::shared_ptr<ALLEGRO_BITMAP> avatar, 
         const std::string& upPath, const std::string& downPath,
         const std::string& leftPath, const std::string& rightPath,
         float x, float y)
    : Engine::Sprite(downPath, x, y), npcName(name), npcAvatar(avatar)
{
    Size.x = 32;
    Size.y = 64;
    
    // 載入靜態圖片 (對照Player的bmpIdle_*)
    bmpIdle_up = Engine::Resources::GetInstance().GetBitmap(upPath);
    bmpIdle_down = Engine::Resources::GetInstance().GetBitmap(downPath);
    bmpIdle_left = Engine::Resources::GetInstance().GetBitmap(leftPath);
    bmpIdle_right = Engine::Resources::GetInstance().GetBitmap(rightPath);

    // 設置默認朝向 (完全對照Player邏輯)
    bmp = bmpIdle_down;
    
    // 對齊方式 (與Player一致)
    Position.x = std::round(Position.x / 64.0f) * 64.0f + 32.0f;
    Position.y = std::round(Position.y / 64.0f) * 64.0f + 32.0f;

    dialog.Initialize();
}


void NPC::Update(float deltaTime, const Player* player) {
    // 只在非對話狀態巡邏
    if (!isTalking && isPatrolling && !patrolPoints.empty()) {
        Engine::Point target = patrolPoints[currentPatrolIndex];
        Engine::Point direction = target - Position;
        float distance = sqrt(direction.x * direction.x + direction.y * direction.y);
        
        if (distance < 5.0f) { // 到達點
            currentPatrolIndex = (currentPatrolIndex + 1) % patrolPoints.size();
            waitTime = (rand() % 1000) / 1000.0f * maxWaitTime; // 0~2秒隨機等待
            // DEBUG: 打印切換到下一巡邏點
            Engine::LOG(Engine::DEBUGGING) << "NPC '" << npcName << "' reached point " 
                                         << currentPatrolIndex;
        } else {
            waitTime -= deltaTime;
            // 標準化方向向量並考慮幀時間
            direction.x /= distance;
            direction.y /= distance;
            
            // 更新位置 (使用 deltaTime 使移動速度與幀率無關)
            Position.x += direction.x * moveSpeed * deltaTime * 60.0f; // 60.0f 是基準幀率
            Position.y += direction.y * moveSpeed * deltaTime * 60.0f;
            
            UpdateFacingDirection(direction);
        }
    }

    ALLEGRO_KEYBOARD_STATE kbState;
    al_get_keyboard_state(&kbState);

    // 檢查是否相鄰且按T鍵
    Engine::Point playerPos = player->Position;
    float distX = playerPos.x - Position.x;
    float distY = playerPos.y - Position.y;
    bool isAdjacent = (std::abs(distX) <= 64.0f && std::abs(distY) <= 64.0f);

    // 檢查Enter鍵是否剛被按下
    bool enterIsDown = al_key_down(&kbState, ALLEGRO_KEY_ENTER);
    bool enterPressed = enterIsDown && !enterWasDown;
    enterWasDown = enterIsDown;

    if (isAdjacent && al_key_down(&kbState, ALLEGRO_KEY_T) && !WindCloudScene::isPlayingWordle) {
        if (!isTalking && !messages.empty()) {
            FacePlayer(player);
            isTalking = true;
            dialog.StartDialog(npcName, npcAvatar, messages);
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
                //Engine::ImageButton *btn;

                isTalking = false;
                if(triggerEvent) triggerEvent();
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

void NPC::UpdateFacingDirection(const Engine::Point& dir) {
    if (abs(dir.x) > abs(dir.y)) {
        bmp = (dir.x > 0) ? bmpIdle_right : bmpIdle_left;
    } else {
        bmp = (dir.y > 0) ? bmpIdle_down : bmpIdle_up;
    }
}

void NPC::AddPatrolPoint(const Engine::Point& point) {
    patrolPoints.push_back(point);
    
    // // DEBUG: 打印添加的巡邏點 (開發時可移除)
    // Engine::LOG(Engine::DEBUGGING) << "Added patrol point to NPC '" << npcName 
    //                               << "': (" << point.x << ", " << point.y << ")";
}

void NPC::SetTriggerEvent(std::function<void()> func) {
    triggerEvent = func;
}
