#ifndef NPC_HPP
#define NPC_HPP

#include "NPCDialog.hpp"
#include "Engine/Sprite.hpp"
#include <memory> // for shared_ptr

class Player;

class NPC : public Engine::Sprite {
public:
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_up;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_down;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_left;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_right;

    // 從 sprite sheet 建構
    NPC(const std::string& sheetPath, float x, float y, 
        int upCol, int upRow, int downCol, int downRow, 
        int leftCol, int leftRow, int rightCol, int rightRow,
        int tileW = 64, int tileH = 64);
    
    // 從分開的圖片建構
    NPC(const std::string& upPath, const std::string& downPath,
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
    
    private:
    std::vector<std::string> messages; // NPC的對話內容
    NPCDialog dialog;                  // 每個NPC有自己的對話框
    bool isTalking = false;            // 是否正在對話
};

#endif