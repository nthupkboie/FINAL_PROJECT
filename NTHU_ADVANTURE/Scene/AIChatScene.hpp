#ifndef AICHATSCENE_HPP
#define AICHATSCENE_HPP

#include "Engine/IScene.hpp"
#include <allegro5/allegro.h>
#include <vector>
#include <string>

class AIChatScene : public Engine::IScene {
private:
    bool isActive;
    std::vector<std::string> messages;
    std::string currentInput;
    ALLEGRO_FONT* font;
    ALLEGRO_COLOR textColor;
    ALLEGRO_COLOR boxColor;
    
    float boxWidth;
    float boxHeight;
    float boxX;
    float boxY;
    float padding;
    
public:
    AIChatScene();
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void OnKeyDown(int keyCode) override;
    
    void AddMessage(const std::string& message);
    void SendMessage();
    void BackToStart();
};

#endif // AICHATSCENE_HPP