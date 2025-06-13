#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

#include "AIChatScene.hpp"

#include "httplib.h"
#include "json.hpp"
using json = nlohmann::json;

AIChatScene::AIChatScene() 
    : isActive(true),
      currentInput(""),
      font(nullptr),
      boxWidth(800.0f),
      boxHeight(600.0f),
      boxX(240.0f),
      boxY(100.0f),
      padding(20.0f) {
    textColor = al_map_rgb(255, 255, 255);  // 白色文字
    boxColor = al_map_rgba(0, 0, 0, 200);   // 半透明黑色背景
}

void AIChatScene::Initialize() {
    // 方法1: 直接載入字體 (不使用 Resources)
    font = al_load_ttf_font("resources/fonts/normal.ttf", 24, 0);
    
    // 方法2: 如果 Resources 返回的是 ALLEGRO_FONT* 而不是 shared_ptr
    // font = Engine::Resources::GetInstance().GetFont("normal.ttf", 24);
    
    // 如果兩種方法都失敗，使用內建字體
    if (!font) {
        font = al_create_builtin_font();
    }
    
    // 初始消息
    AddMessage("AI: 你好！我是AI助手，請問有什麼可以幫你的嗎？");
}

void AIChatScene::Terminate() {
    // 只釋放我們直接載入的字體
    if (font && font != al_create_builtin_font()) {
        // 檢查是否是我們直接載入的字體
        // 如果是通過 Resources 獲取的，不要釋放
        // 這裡假設我們使用方法1直接載入
        al_destroy_font(font);
    }
    IScene::Terminate();
}

void AIChatScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
}

void AIChatScene::Draw() const {
    // 繪製對話框背景
    al_draw_filled_rectangle(boxX, boxY, 
                            boxX + boxWidth, boxY + boxHeight, 
                            boxColor);
    al_draw_rectangle(boxX, boxY, 
                     boxX + boxWidth, boxY + boxHeight, 
                     al_map_rgb(255,255,255), 2);
    
    // 計算可用寬度
    float textWidth = boxWidth - 2 * padding;
    
    // 繪製聊天記錄
    float currentY = boxY + padding;
    for (const auto& msg : messages) {
        // 分割訊息為多行
        std::vector<std::string> lines;
        SplitTextIntoLines(msg, lines, textWidth);
        
        // 繪製每一行
        for (const auto& line : lines) {
            if (currentY + al_get_font_line_height(font) > boxY + boxHeight - 60) {
                break;  // 不要繪製到輸入框區域
            }
            al_draw_text(font, textColor, boxX + padding, currentY, ALLEGRO_ALIGN_LEFT, line.c_str());
            currentY += al_get_font_line_height(font) + 5;
        }
    }
    
    // 繪製輸入框
    al_draw_filled_rectangle(boxX, boxY + boxHeight - 60, 
                            boxX + boxWidth, boxY + boxHeight - 20, 
                            al_map_rgba(64, 64, 64, 200));
    
    // 分割輸入文字為多行
    std::vector<std::string> inputLines;
    SplitTextIntoLines("You: " + currentInput, inputLines, textWidth);
    
    // 繪製輸入文字
    float inputY = boxY + boxHeight - 50;
    for (const auto& line : inputLines) {
        al_draw_text(font, textColor, boxX + padding, inputY, 
                    ALLEGRO_ALIGN_LEFT, line.c_str());
        inputY += al_get_font_line_height(font);
    }
    
    // 繪製返回提示
    al_draw_text(font, textColor, boxX + boxWidth - 150, boxY + boxHeight - 50, 
                ALLEGRO_ALIGN_LEFT, "Press ESC to back");
}

void AIChatScene::SplitTextIntoLines(const std::string& text, std::vector<std::string>& lines, float maxWidth) const {
    std::string remainingText = text;
    
    while (!remainingText.empty()) {
        // 找出適合的文字長度
        int charsToTake = 1;
        while (charsToTake <= remainingText.length()) {
            std::string testStr = remainingText.substr(0, charsToTake);
            int width = al_get_text_width(font, testStr.c_str());
            
            if (width > maxWidth) {
                // 如果超過寬度，取前一個長度
                charsToTake--;
                break;
            }
            
            charsToTake++;
        }
        
        // 確保不會無限循環
        if (charsToTake <= 0) {
            charsToTake = 1;
        } else if (charsToTake > remainingText.length()) {
            charsToTake = remainingText.length();
        }
        
        // 添加這一行
        lines.push_back(remainingText.substr(0, charsToTake));
        remainingText = remainingText.substr(charsToTake);
        
        // 移除開頭的空白
        while (!remainingText.empty() && remainingText[0] == ' ') {
            remainingText = remainingText.substr(1);
        }
    }
}

void AIChatScene::OnKeyDown(int keyCode) {
    // 處理鍵盤輸入
    if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
        currentInput += 'a' + (keyCode - ALLEGRO_KEY_A);
    } 
    else if (keyCode == ALLEGRO_KEY_SPACE) {
        currentInput += " ";
    } 
    else if (keyCode == ALLEGRO_KEY_BACKSPACE && !currentInput.empty()) {
        currentInput.pop_back();
    } 
    else if (keyCode == ALLEGRO_KEY_ENTER && !currentInput.empty()) {
        SendMyMessage();
    }
    else if (keyCode == ALLEGRO_KEY_ESCAPE) {
        BackToStart();
    }
}

void AIChatScene::AddMessage(const std::string& message) {
    messages.push_back(message);
    
    // 限制消息數量
    if (messages.size() > 15) {
        messages.erase(messages.begin());
    }
}

void AIChatScene::SendMyMessage() {
    if (!currentInput.empty()) {
        AddMessage("You: " + currentInput);

        httplib::Client cli("localhost", 11434);

        json requestBody = {
            {"model", "deepseek-r1"},  // 使用正確的模型名稱
            {"prompt", currentInput},   // 使用 "prompt" 而非 "messages"
            {"stream", false}
        };

        auto res = cli.Post("/api/generate", requestBody.dump(), "application/json");

        if (res && res->status == 200) {
            try {
                auto responseJson = json::parse(res->body);
                std::string aiReply = responseJson["response"];  // 改成 "response"
                AddMessage("AI: " + aiReply);
            } catch (const std::exception& e) {
                AddMessage("AI: 回傳格式解析錯誤：" + std::string(e.what()));
            }
        } else {
            std::string errorMsg = "AI: 請求失敗！HTTP 狀態碼 = ";
            errorMsg += res ? std::to_string(res->status) : "null（可能沒啟動 Ollama）";
            if (res) {
                errorMsg += "\n錯誤訊息：" + res->body;
            }
            AddMessage(errorMsg);
        }

        currentInput.clear();
    }
}

void AIChatScene::BackToStart() {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}