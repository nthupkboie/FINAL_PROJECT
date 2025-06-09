#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include <cmath>

#include "Player.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/IScene.hpp"
#include "Engine/LOG.hpp"
#include "Scene/PlayScene.hpp"

#include "Engine/Resources.hpp"

PlayScene *Player::getPlayScene() {
    return dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}

Player::Player(std::string img, float x, float y) : Engine::Sprite(img, x, y) {

    // 儲存圖像
    bmpIdle = bmp;

    bmpUp1 = Engine::Resources::GetInstance().GetBitmap("player/run1_player.png");
    bmpUp2 = Engine::Resources::GetInstance().GetBitmap("player/run2_player.png");

    bmpDown1 = Engine::Resources::GetInstance().GetBitmap("player/run1_player.png");
    bmpDown2 = Engine::Resources::GetInstance().GetBitmap("player/run2_player.png");

    bmpLeft1 = Engine::Resources::GetInstance().GetBitmap("player/run1_player.png");
    bmpLeft2 = Engine::Resources::GetInstance().GetBitmap("player/run2_player.png");

    bmpRight1 = Engine::Resources::GetInstance().GetBitmap("player/run1_player.png");
    bmpRight2 = Engine::Resources::GetInstance().GetBitmap("player/run2_player.png");

    //對齊
    Position.x = std::round(Position.x / 64.0f) * 64.0f + 32.0f;
    Position.y = std::round(Position.y / 64.0f) * 64.0f + 32.0f;

    al_get_keyboard_state(&lastKeyState);
    startPos = targetPos = Position;
}

void Player::Update(float deltaTime) {
    // 更新動畫計時器
    animationTimer += deltaTime;
    if (animationTimer >= 0.6f) animationTimer -= 0.6f; // 每 0.6 秒循環

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

    // 處理移動
    if (isMoving) {
        // 更新移動進度（0.3 秒完成）
        moveProgress += deltaTime / 0.3f;
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
                    bmp = (std::fmod(animationTimer, 0.6f) < 0.3f) ? bmpUp1 : bmpUp2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_W;
                } else if (latestKey == ALLEGRO_KEY_S) {
                    tmp.y += 64.0f; // 向下
                    bmp = (std::fmod(animationTimer, 0.6f) < 0.3f) ? bmpDown1 : bmpDown2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_S;
                } else if (latestKey == ALLEGRO_KEY_A) {
                    tmp.x -= 64.0f; // 向左
                    bmp = (std::fmod(animationTimer, 0.6f)) ? bmpLeft1 : bmpLeft2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_A;
                } else if (latestKey == ALLEGRO_KEY_D) {
                    tmp.x += 64.0f; // 向右
                    bmp = (std::fmod(animationTimer, 0.6f) < 0.3f) ? bmpRight1 : bmpRight2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_D;
                }

                if (keyPressed) {
                    startPos = Position;
                    targetPos = tmp;
                    moveProgress = 0.0f;
                    isMoving = true;
                }
            } else {
                bmp = bmpIdle;
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
                    bmp = (std::fmod(animationTimer, 0.6f) < 0.3f) ? bmpUp1 : bmpUp2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_W;
                } else if (latestKey == ALLEGRO_KEY_S) {
                    tmp.y += 64.0f; // 向下
                    bmp = (std::fmod(animationTimer, 0.6f) < 0.3f) ? bmpDown1 : bmpDown2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_S;
                } else if (latestKey == ALLEGRO_KEY_A) {
                    tmp.x -= 64.0f; // 向左
                    bmp = (std::fmod(animationTimer, 0.6f) < 0.3f) ? bmpLeft1 : bmpLeft2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_A;
                } else if (latestKey == ALLEGRO_KEY_D) {
                    tmp.x += 64.0f; // 向右
                    bmp = (std::fmod(animationTimer, 0.6f) < 0.3f) ? bmpRight1 : bmpRight2;
                    keyPressed = true;
                    lastDirection = ALLEGRO_KEY_D;
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
            bmp = bmpIdle;
            animationTimer = 0;
            lastDirection = 0;
        }
    }

    // 更新上一次鍵盤狀態
    lastKeyState = kbState;

    Sprite::Update(deltaTime);
}

void Player::Draw() const {
    Sprite::Draw();
}

