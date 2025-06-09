#ifndef NPCDIALOG_HPP
#define NPCDIALOG_HPP

#include <string>
#include <vector>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

class NPCDialog {
public:
    NPCDialog(std::vector<std::string> dialogLines, const std::string& portraitPath);

    void Start();
    void Update(double deltaTime);
    void Draw() const;
    void Proceed(); // 切換下一句對話
    bool IsFinished() const;
    bool IsActive() const;

private:
    std::vector<std::string> lines;
    int currentLine;
    std::string currentText;
    float charDisplayTimer;
    size_t charIndex;
    bool finished;
    bool active;

    ALLEGRO_BITMAP* portrait;
    ALLEGRO_FONT* font;
};

#endif
