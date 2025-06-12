#ifndef NPC_HPP
#define NPC_HPP

#include "NPCDialog.hpp"
#include "Engine/Sprite.hpp"
#include <memory> // for shared_ptr
#include <vector>

class Player;

class NPC : public Engine::Sprite {
public:
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_up;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_down;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_left;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_right;

    // 從 sprite sheet 建構
    NPC(const std::string& name, std::shared_ptr<ALLEGRO_BITMAP> avatar, const std::string& sheetPath, float x, float y, 
        int upCol, int upRow, int downCol, int downRow, 
        int leftCol, int leftRow, int rightCol, int rightRow,
        int tileW = 64, int tileH = 64);
    
    // 從分開的圖片建構
    NPC(const std::string& name, std::shared_ptr<ALLEGRO_BITMAP> avatar, const std::string& upPath, const std::string& downPath,
        const std::string& leftPath, const std::string& rightPath,
        float x, float y);
    
    void Update(float deltaTime, const Player* player);
    void FacePlayer(const Player* player);
    void Draw() const override;

    // 設置對話內容
    void SetMessages(const std::vector<std::string>& msgs) {
        messages = msgs;
    }
    
    // 檢查是否正在對話
    bool IsTalking() const { return isTalking; }
    void UpdateFacingDirection(const Engine::Point& dir);

    // 添加巡邏點
    void AddPatrolPoint(const Engine::Point& point);
    
    // 設置移動速度 (可選)
    void SetMoveSpeed(float speed) { moveSpeed = speed; }

    float waitTime = 0.0f;
    float maxWaitTime = 2.0f; // 最大等待2秒
private:
    std::vector<std::string> messages; // NPC的對話內容
    NPCDialog dialog;                  // 每個NPC有自己的對話框
    bool isTalking = false;            // 是否正在對話

    const std::string& GetName() const { return npcName; }
    void SetName(const std::string& name) { npcName = name; }
    void SetAvatar(std::shared_ptr<ALLEGRO_BITMAP> avatar) { npcAvatar = avatar; }

    bool enterWasDown = false;

    std::string npcName;
    std::shared_ptr<ALLEGRO_BITMAP> npcAvatar;

    std::vector<Engine::Point> patrolPoints;
    size_t currentPatrolIndex = 0;
    float moveSpeed = 1.0f;
    bool isPatrolling = true;
};

#endif