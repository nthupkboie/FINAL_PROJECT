#include "Engine/GameEngine.hpp"
#include "Engine/Resources.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include "Engine/LOG.hpp"

#include "AIChatScene.hpp"

#include "httplib.h"
#include "json.hpp"
using json = nlohmann::json;

AIChatScene::AIChatScene() 
    : isActive(true),
      currentInput(""),
      font(nullptr),
      boxWidth(1600.0f),
      boxHeight(900.0f),
      boxX((1920 - 1600) / 2.0f),
      boxY((1024 - 900) / 2.0f),
      padding(40.0f) {
      textColor = al_map_rgb(255, 255, 255);
      boxColor = al_map_rgba(0, 0, 0, 220);
      Engine::LOG(Engine::INFO) << "AIChatScene constructor called";
}

void AIChatScene::Initialize() {
    Engine::LOG(Engine::INFO) << "Initializing AIChatScene";
    
    // font = al_load_ttf_font("resources/fonts/normal.ttf", 40, 0);
    font = al_load_ttf_font("Resource/fonts/normal.ttf", 40, 0);
    
    if (!font) {
        Engine::LOG(Engine::WARN) << "Failed to load custom font, using built-in";
        font = al_create_builtin_font();
    }
    
    AddMessage("AI: Hello! I'm your AI assistant. How can I help you today?");
    Engine::LOG(Engine::INFO) << "Initial greeting message added";
}

void AIChatScene::Terminate() {
    Engine::LOG(Engine::INFO) << "Terminating AIChatScene";
    if (font && font != al_create_builtin_font()) {
        Engine::LOG(Engine::INFO) << "Destroying custom font";
        al_destroy_font(font);
    }
    IScene::Terminate();
}

void AIChatScene::Update(float deltaTime) {
    IScene::Update(deltaTime);
}

void AIChatScene::Draw() const {
    // Draw semi-transparent background
    al_draw_filled_rectangle(0, 0, 1920, 1024, al_map_rgba(0, 0, 0, 150));
    
    // Draw main dialog box
    al_draw_filled_rounded_rectangle(boxX, boxY, 
                                   boxX + boxWidth, boxY + boxHeight, 
                                   20, 20, boxColor);  // Larger corner radius
    al_draw_rounded_rectangle(boxX, boxY, 
                            boxX + boxWidth, boxY + boxHeight, 
                            20, 20, al_map_rgb(255, 255, 255), 5);  // Thicker border
    
    // Calculate text area
    float textWidth = boxWidth - 2 * padding;
    float maxContentHeight = boxHeight - 140;  // More space for input
    
    // Draw chat history
    float currentY = boxY + padding;
    for (const auto& msg : messages) {
        std::vector<std::string> lines;
        SplitTextIntoLines(msg, lines, textWidth);
        
        for (const auto& line : lines) {
            if (currentY + al_get_font_line_height(font) > boxY + maxContentHeight) {
                break;
            }
            al_draw_text(font, textColor, boxX + padding, currentY, ALLEGRO_ALIGN_LEFT, line.c_str());
            currentY += al_get_font_line_height(font) + 12;  // Increased line spacing
        }
    }
    
    // Draw input area
    float inputBoxY = boxY + boxHeight - 100;  // More space for input
    al_draw_filled_rounded_rectangle(boxX + 15, inputBoxY,
                                   boxX + boxWidth - 15, boxY + boxHeight - 25,
                                   12, 12, al_map_rgba(70, 70, 70, 220));
    
    // Input text
    std::vector<std::string> inputLines;
    SplitTextIntoLines("You: " + currentInput, inputLines, textWidth - 20);
    
    float inputY = inputBoxY + 20;
    for (const auto& line : inputLines) {
        al_draw_text(font, textColor, boxX + padding, inputY, 
                    ALLEGRO_ALIGN_LEFT, line.c_str());
        inputY += al_get_font_line_height(font);
    }
    
    // Draw hint (bottom right)
    ALLEGRO_FONT* hintFont = al_load_ttf_font("Resource/fonts/normal.ttf", 40, 0);  // Larger font
    if (!hintFont) hintFont = al_create_builtin_font();
    al_draw_text(hintFont, al_map_rgb(180, 180, 180), 
                boxX + boxWidth - 25, boxY + boxHeight - 200, 
                ALLEGRO_ALIGN_RIGHT, "ESC to return to menu");
    if (hintFont && hintFont != al_create_builtin_font()) {
        al_destroy_font(hintFont);
    }
}

void AIChatScene::SplitTextIntoLines(const std::string& text, std::vector<std::string>& lines, float maxWidth) const {
    std::string remainingText = text;
    
    while (!remainingText.empty()) {
        int charsToTake = 1;
        while (charsToTake <= remainingText.length()) {
            std::string testStr = remainingText.substr(0, charsToTake);
            int width = al_get_text_width(font, testStr.c_str());
            
            if (width > maxWidth) {
                charsToTake--;
                break;
            }
            
            charsToTake++;
        }
        
        if (charsToTake <= 0) {
            charsToTake = 1;
        } else if (charsToTake > remainingText.length()) {
            charsToTake = remainingText.length();
        }
        
        lines.push_back(remainingText.substr(0, charsToTake));
        remainingText = remainingText.substr(charsToTake);
        
        while (!remainingText.empty() && remainingText[0] == ' ') {
            remainingText = remainingText.substr(1);
        }
    }
}

void AIChatScene::OnKeyDown(int keyCode) {
    Engine::LOG(Engine::INFO) << "Key pressed: " << keyCode;
    
    if (keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z) {
        currentInput += 'a' + (keyCode - ALLEGRO_KEY_A);
        Engine::LOG(Engine::DEBUGGING) << "Current input: " << currentInput;
    } 
    else if (keyCode == ALLEGRO_KEY_SPACE) {
        currentInput += " ";
        Engine::LOG(Engine::DEBUGGING) << "Space added to input";
    } 
    else if (keyCode == ALLEGRO_KEY_BACKSPACE && !currentInput.empty()) {
        currentInput.pop_back();
        Engine::LOG(Engine::DEBUGGING) << "Backspace pressed, new input: " << currentInput;
    } 
    else if (keyCode == ALLEGRO_KEY_ENTER && !currentInput.empty()) {
        Engine::LOG(Engine::INFO) << "Enter pressed with input, sending message";
        SendMyMessage();
    }
    else if (keyCode == ALLEGRO_KEY_ESCAPE) {
        Engine::LOG(Engine::INFO) << "ESC pressed, returning to start scene";
        BackToStart();
    }
}

void AIChatScene::AddMessage(const std::string& message) {
    messages.push_back(message);
    
    if (messages.size() > 30) {  // Increased message history
        messages.erase(messages.begin());
    }
}

void AIChatScene::SendMyMessage() {
    Engine::LOG(Engine::INFO) << "SendMyMessage() called with input: " << currentInput;
    
    if (!currentInput.empty()) {
        Engine::LOG(Engine::INFO) << "Adding user message to chat";
        AddMessage("You: " + currentInput);

        Engine::LOG(Engine::INFO) << "Creating HTTP client";
        httplib::Client cli("localhost", 11434);

        json requestBody = {
            {"model", "deepseek-r1"},
            {"prompt", currentInput},
            {"stream", false}
        };

        Engine::LOG(Engine::INFO) << "Sending request to Ollama API";
        auto res = cli.Post("/api/generate", requestBody.dump(), "application/json");

        if (res) {
            Engine::LOG(Engine::INFO) << "Received response with status: " << res->status;
            
            if (res->status == 200) {
                try {
                    Engine::LOG(Engine::INFO) << "Parsing JSON response";
                    auto responseJson = json::parse(res->body);
                    std::string aiReply = responseJson["response"];
                    Engine::LOG(Engine::INFO) << "AI response: " << aiReply;
                    AddMessage("AI: " + aiReply);
                } catch (const std::exception& e) {
                    std::string error = "Error parsing response: " + std::string(e.what());
                    Engine::LOG(Engine::WARN) << error;
                    AddMessage("AI: " + error);
                }
            } else {
                std::string errorMsg = "Request failed! HTTP status = " + std::to_string(res->status);
                Engine::LOG(Engine::WARN) << errorMsg;
                if (!res->body.empty()) {
                    Engine::LOG(Engine::WARN) << "Error body: " << res->body;
                    errorMsg += "\nError message: " + res->body;
                }
                AddMessage("AI: " + errorMsg);
            }
        } else {
            std::string errorMsg = "No response received (Ollama might not be running)";
            Engine::LOG(Engine::WARN) << errorMsg;
            auto err = res.error();
            Engine::LOG(Engine::WARN) << "HTTP error: " << httplib::to_string(err);
            AddMessage("AI: " + errorMsg);
        }

        currentInput.clear();
        Engine::LOG(Engine::INFO) << "Input cleared after sending";
    } else {
        Engine::LOG(Engine::WARN) << "SendMyMessage called with empty input";
    }
}

void AIChatScene::BackToStart() {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}