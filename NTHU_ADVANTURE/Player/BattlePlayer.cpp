#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include <cmath>

#include "BattlePlayer.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IScene.hpp"
#include "Engine/LOG.hpp"
#include "Scene/BattleScene.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/LogScene.hpp"
#include "Scene/CGLakeScene.hpp"
#include "Engine/Resources.hpp"

BattleScene *BattlePlayer::getBattleScene() {
    return dynamic_cast<BattleScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}

Engine::Point BattlePlayer::curPos = Engine::Point(0,0);

BattlePlayer::BattlePlayer(std::string img, float x, float y) : Engine::Sprite(img, x, y) {

    // 儲存圖像
    bmpIdle_down = bmp;
    bmpIdle_left = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/left_idle.png");
    bmpIdle_right = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/right_idle.png");
    bmpIdle_up = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/up_idle.png");

    bmpUp1 = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/up1.png");
    bmpUp2 = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/up2.png");

    bmpDown1 = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/down1.png");
    bmpDown2 = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/down2.png");

    bmpLeft1 = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/left1.png");
    bmpLeft2 = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/left2.png");

    bmpRight1 = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/right1.png");
    bmpRight2 = Engine::Resources::GetInstance().GetBitmap("BattlePlayer/right2.png");

    //對齊
    Position.x = std::round(Position.x / 64.0f) * 64.0f + 32.0f;
    Position.y = std::round(Position.y / 64.0f) * 64.0f + 32.0f;

    al_get_keyboard_state(&lastKeyState);
    startPos = targetPos = Position;

    last_down = last_left = last_right = last_up = false;

    curPos.x = Position.x;
    curPos.y = Position.y;
}

void BattlePlayer::Update(float deltaTime) {
    // 更新動畫計時器
    animationTimer += deltaTime;
    if (animationTimer >= 0.4f) animationTimer -= 0.4f; // 每 0.4 秒循環

    // 獲取鍵盤狀態
    ALLEGRO_KEYBOARD_STATE kbState;
    al_get_keyboard_state(&kbState);

    // 更新按鍵時間
    static const int keys[] = {ALLEGRO_KEY_W, ALLEGRO_KEY_S, ALLEGRO_KEY_A, ALLEGRO_KEY_D};
    for (int key : keys) {
        if (al_key_down(&kbState, key) && !al_key_down(&lastKeyState, key)) {
            keyPressTimes[key] = animationTimer + deltaTime; // 記錄按下時間
        } else if (!al_key_down(&kbState, key) && al_key_down(&lastKeyState, key)) {
            keyPressTimes.erase(key); // 釋放時移除
        }
    }

    if (LogScene::haveSpeedUp) speed = 0.2f / (LogScene::haveSpeedUp);

    // 處理移動
    if (isMoving) {
        // 更新移動進度（0.3 秒完成）
        moveProgress += deltaTime / speed;
        if (moveProgress >= 1.0f) {
            Position = targetPos;
            moveProgress = 0.0f;
            isMoving = false;
            // 根據最新按鍵繼續移動
            if (!keyPressTimes.empty()) {
                // 找到最新按鍵
                int latestKey = 0;
                float latestTime = -1.0f;
                for (const auto& pair : keyPressTimes) {
                    if (pair.second > latestTime) {
                        latestKey = pair.first;
                        latestTime = pair.second;
                    }
                }
                Engine::Point tmp = Position;
                bool keyPressed = false;

                if (latestKey == ALLEGRO_KEY_W) {
                    tmp.y -= 64.0f; // 向上
                    if (!canWalk(tmp.x, tmp.y)) tmp.y += 64.0f;
                    bmp = (std::fmod(animationTimer, 0.4f) < 0.2f) ? bmpUp1 : bmpUp2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_W;
                    last_up = true;
                    last_down = last_left = last_right = false;
                } else if (latestKey == ALLEGRO_KEY_S) {
                    tmp.y += 64.0f; // 向下
                    if (!canWalk(tmp.x, tmp.y)) tmp.y -= 64.0f;
                    bmp = (std::fmod(animationTimer, 0.4f) < 0.2f) ? bmpDown1 : bmpDown2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_S;
                    last_down = true;
                    last_up = last_left = last_right = false;
                } else if (latestKey == ALLEGRO_KEY_A) {
                    tmp.x -= 64.0f; // 向左
                    if (!canWalk(tmp.x, tmp.y)) tmp.x += 64.0f;
                    bmp = (std::fmod(animationTimer, 0.4f) < 0.2f) ? bmpLeft1 : bmpLeft2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_A;
                    last_left = true;
                    last_down = last_up = last_right = false;
                } else if (latestKey == ALLEGRO_KEY_D) {
                    tmp.x += 64.0f; // 向右
                    if (!canWalk(tmp.x, tmp.y)) tmp.x -= 64.0f;
                    bmp = (std::fmod(animationTimer, 0.4f) < 0.2f) ? bmpRight1 : bmpRight2;
                    //if (std::fmod(animationTimer, 0.4f) < 0.1f) bmp = bmpRight1;
                    //else if (std::fmod(animationTimer, 0.4f) < 0.2f) bmp = bmpRight2;
                    //else if (std::fmod(animationTimer, 0.4f) < 0.3f) bmp = bmpRight3;
                    //else bmp = bmpRight4;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_D;
                    last_right = true;
                    last_down = last_left = last_up = false;
                }

                if (keyPressed) {
                    startPos = Position;
                    targetPos = tmp;
                    moveProgress = 0.0f;
                    isMoving = true;
                }
            } else {
                if (last_down) bmp = bmpIdle_down;
                else if (last_left) bmp = bmpIdle_left;
                else if (last_right) bmp = bmpIdle_right;
                else bmp = bmpIdle_up;
                animationTimer = 0;
                lastDirection = 0;
            }
        } else {
            // 插值計算當前位置
            Position = startPos + (targetPos - startPos) * moveProgress;
        }
    }

    // 檢查新輸入（僅在非移動時）
    if (!isMoving) {
        if (!keyPressTimes.empty()) {
            // 找到最新按鍵
            int latestKey = 0;
            float latestTime = -1.0f;
            for (const auto& pair : keyPressTimes) {
                if (pair.second > latestTime) {
                    latestKey = pair.first;
                    latestTime = pair.second;
                }
            }
            // 僅在初次按下時啟動移動
            if (al_key_down(&kbState, latestKey) && !al_key_down(&lastKeyState, latestKey)) {
                Engine::Point tmp = Position;
                bool keyPressed = false;

                if (latestKey == ALLEGRO_KEY_W) {
                    tmp.y -= 64.0f; // 向上
                    if (!canWalk(tmp.x, tmp.y)) tmp.y += 64.0f;
                    bmp = (std::fmod(animationTimer, 0.4f) < 0.2f) ? bmpUp1 : bmpUp2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_W;
                    last_up = true;
                    last_down = last_left = last_right = false;
                } else if (latestKey == ALLEGRO_KEY_S) {
                    tmp.y += 64.0f; // 向下
                    if (!canWalk(tmp.x, tmp.y)) tmp.y -= 64.0f;
                    bmp = (std::fmod(animationTimer, 0.4f) < 0.2f) ? bmpDown1 : bmpDown2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_S;
                    last_down = true;
                    last_up = last_left = last_right = false;
                } else if (latestKey == ALLEGRO_KEY_A) {
                    tmp.x -= 64.0f; // 向左
                    if (!canWalk(tmp.x, tmp.y)) tmp.x += 64.0f;
                    bmp = (std::fmod(animationTimer, 0.4f) < 0.2f) ? bmpLeft1 : bmpLeft2;
                    keyPressed = true;
                    last_left = true;
                    last_down = last_up = last_right = false;
                    lastDirection = ALLEGRO_KEY_A;
                } else if (latestKey == ALLEGRO_KEY_D) {
                    tmp.x += 64.0f; // 向右
                    if (!canWalk(tmp.x, tmp.y)) tmp.x -= 64.0f;
                    bmp = (std::fmod(animationTimer, 0.4f) < 0.2f) ? bmpRight1 : bmpRight2;
                    //if (std::fmod(animationTimer, 0.4f) < 0.1f) bmp = bmpRight1;
                    //else if (std::fmod(animationTimer, 0.4f) < 0.2f) bmp = bmpRight2;
                    //else if (std::fmod(animationTimer, 0.4f) < 0.3f) bmp = bmpRight3;
                    //else bmp = bmpRight4;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_D;
                    last_right = true;
                    last_down = last_left = last_up = false;
                }

                if (keyPressed) {
                    startPos = Position;
                    targetPos = tmp;
                    moveProgress = 0.0f;
                    isMoving = true;
                }
            }
        } else if (!al_key_down(&kbState, ALLEGRO_KEY_W) && !al_key_down(&kbState, ALLEGRO_KEY_S) &&
                   !al_key_down(&kbState, ALLEGRO_KEY_A) && !al_key_down(&kbState, ALLEGRO_KEY_D)) {
            //bmp = bmpIdle_down;
            if (last_down) bmp = bmpIdle_down;
            else if (last_left) bmp = bmpIdle_left;
            else if (last_right) bmp = bmpIdle_right;
            else bmp = bmpIdle_up;
            animationTimer = 0;
            lastDirection = 0;
        }
    }

    // 更新上一次鍵盤狀態
    lastKeyState = kbState;

    curPos.x = Position.x;
    curPos.y = Position.y;

    Sprite::Update(deltaTime);
}

void BattlePlayer::Draw() const {
    Sprite::Draw();
}

bool BattlePlayer::canWalk(int x, int y){
    //printf("LLLLLLLLll\n");
    if (x > BattleScene::BlockSize * BattleScene::MapWidth || x < 0 || y < 0 || y > BattleScene::BlockSize * BattleScene::MapHeight) return false;
    if (!PlayScene::inPlay){
        //BattleScene* scene = dynamic_cast<BattleScene*>(Engine::GameEngine::GetInstance().GetScene("battle"));
        if (PlayScene::inBattle){
            if (!BattleScene::collision(x, y)) {
                return false;
            }
        }
        else if (PlayScene::inCGLake){
            if (!CGLakeScene::collision(x, y)){
                return false;
            }
        }
        
    }
    
    //printf("TTTTTTTTTTTTTT\n");
    return true;
}