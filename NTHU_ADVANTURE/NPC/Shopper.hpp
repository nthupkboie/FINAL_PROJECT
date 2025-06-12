#ifndef SHOPPER_HPP
#define SHOPPER_HPP

#include "NPCDialog.hpp"
#include "Engine/Sprite.hpp"
#include "UI/Component/ImageButton.hpp"
#include <memory> // for shared_ptr

class Player;

class Shopper : public Engine::Sprite {
public:
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_up;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_down;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_left;
    std::shared_ptr<ALLEGRO_BITMAP> bmpIdle_right;

    // 從 sprite sheet 建構
    Shopper(const std::string& name, std::shared_ptr<ALLEGRO_BITMAP> avatar, const std::string& sheetPath, float x, float y, 
        int upCol, int upRow, int downCol, int downRow, 
        int leftCol, int leftRow, int rightCol, int rightRow,
        int tileW = 64, int tileH = 64);
    
    // 從分開的圖片建構
    Shopper(const std::string& name, std::shared_ptr<ALLEGRO_BITMAP> avatar, const std::string& upPath, const std::string& downPath,
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
    
    //private:
    std::vector<std::string> messages; // NPC的對話內容
    NPCDialog dialog;                  // 每個NPC有自己的對話框
    static bool isTalking;            // 是否正在對話

    const std::string& GetName() const { return npcName; }
    void SetName(const std::string& name) { npcName = name; }
    void SetAvatar(std::shared_ptr<ALLEGRO_BITMAP> avatar) { npcAvatar = avatar; }

    bool enterWasDown = false;
    static bool canBuy; //講完話了沒
private:
    std::string npcName;
    std::shared_ptr<ALLEGRO_BITMAP> npcAvatar;
    bool showShopButtons = false; // 新增：控制購買按鈕顯示
    Engine::ImageButton* buyButton = nullptr; // 新增：購買按鈕
    Engine::ImageButton* cancelButton = nullptr; // 新增：取消按鈕
};

#endif