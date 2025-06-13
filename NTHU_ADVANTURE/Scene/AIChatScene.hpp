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

    void SplitTextIntoLines(const std::string& text, std::vector<std::string>& lines, float maxWidth) const;
public:
    explicit AIChatScene();
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void Draw() const override;
    void OnKeyDown(int keyCode) override;
    
    void AddMessage(const std::string& message);
    void SendMyMessage();
    void BackToStart();

    bool isWaitingForResponse;
    float loadingAnimationTime;
};

#endif // AICHATSCENE_HPP