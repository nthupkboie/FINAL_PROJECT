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
      isWaitingForResponse(false),  // 新增
      loadingAnimationTime(0.0f),    // 新增
      boxWidth(1400.0f),    // 更大的對話框寬度
      boxHeight(800.0f),    // 更大的對話框高度
      boxX((1920 - 1400) / 2.0f),  // 水平居中
      boxY((1024 - 800) / 2.0f),    // 垂直居中
      padding(30.0f) {      // 更大的內邊距
    textColor = al_map_rgb(255, 255, 255);
    boxColor = al_map_rgba(0, 0, 0, 220);  // 更不透明的背景
}

void AIChatScene::Initialize() {
    // 使用更大的字體
    font = al_load_ttf_font("resources/fonts/normal.ttf", 32, 0);  // 從 24 加大到 32
    
    if (!font) {
        font = al_create_builtin_font();
    }
    
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
    
    // 更新 loading 動畫時間
    if (isWaitingForResponse) {
        loadingAnimationTime += deltaTime;
    }
}


void AIChatScene::Draw() const {
    // 繪製全螢幕半透明背景
    al_draw_filled_rectangle(0, 0, 1920, 1024, al_map_rgba(0, 0, 0, 150));
    
    // 繪製主對話框
    al_draw_filled_rounded_rectangle(boxX, boxY, 
                                   boxX + boxWidth, boxY + boxHeight, 
                                   15, 15, boxColor);
    al_draw_rounded_rectangle(boxX, boxY, 
                            boxX + boxWidth, boxY + boxHeight, 
                            15, 15, al_map_rgb(255, 255, 255), 4);  // 更粗的邊框
    
    // 計算文字區域
    float textWidth = boxWidth - 2 * padding;
    float maxContentHeight = boxHeight - 120;  // 留出輸入框空間
    
    // 繪製聊天記錄
    float currentY = boxY + padding;
    for (const auto& msg : messages) {
        std::vector<std::string> lines;
        SplitTextIntoLines(msg, lines, textWidth);
        
        for (const auto& line : lines) {
            if (currentY + al_get_font_line_height(font) > boxY + maxContentHeight) {
                break;
            }
            al_draw_text(font, textColor, boxX + padding, currentY, ALLEGRO_ALIGN_LEFT, line.c_str());
            currentY += al_get_font_line_height(font) + 10;  // 更大的行距
        }
    }
    
    // 繪製輸入區域
    float inputBoxY = boxY + boxHeight - 90;
    al_draw_filled_rounded_rectangle(boxX + 10, inputBoxY,
                                   boxX + boxWidth - 10, boxY + boxHeight - 20,
                                   10, 10, al_map_rgba(70, 70, 70, 220));
    
    // 輸入文字
    std::vector<std::string> inputLines;
    SplitTextIntoLines("You: " + currentInput, inputLines, textWidth - 20);
    
    float inputY = inputBoxY + 15;
    for (const auto& line : inputLines) {
        al_draw_text(font, textColor, boxX + padding, inputY, 
                    ALLEGRO_ALIGN_LEFT, line.c_str());
        inputY += al_get_font_line_height(font);
    }
    
    // 繪製返回提示 (右下角)
    ALLEGRO_FONT* hintFont = al_load_ttf_font("resources/fonts/normal.ttf", 24, 0);
    if (!hintFont) hintFont = al_create_builtin_font();
    al_draw_text(hintFont, al_map_rgb(180, 180, 180), 
                boxX + boxWidth - 20, boxY + boxHeight - 40, 
                ALLEGRO_ALIGN_RIGHT, "ESC 返回主選單");
    if (hintFont && hintFont != al_create_builtin_font()) {
        al_destroy_font(hintFont);
    }

        // 在輸入框下方繪製 Loading 狀態
    if (isWaitingForResponse) {
        // 動態的 loading 動畫
        std::string loadingText = "AI 思考中";
        int dotCount = static_cast<int>(loadingAnimationTime * 2) % 4;
        loadingText += std::string(dotCount, '.');
        
        al_draw_text(font, al_map_rgb(150, 150, 255), 
                    boxX + boxWidth / 2, boxY + boxHeight - 100,
                    ALLEGRO_ALIGN_CENTER, loadingText.c_str());
    }
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
    if (messages.size() > 25) {  // 從 15 增加到 25
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