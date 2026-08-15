#include <raylib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

extern std::unordered_map<std::string, std::unordered_map<char, float>> moddata;
// import functions from typeu.cpp
int trainModel(const std::string& inputData);
void loadModel(const std::string& filename);
void saveModel(const std::string& filename);

std::vector<std::string> predictWords(const std::string& input, int n = 3) {
    std::vector<std::string> suggestions;
    if (input.empty() || moddata.empty()) return suggestions;

    std::string hist = "";
    for (int len = std::min((int)input.length(), 4); len >= 1; len--) {
        std::string test = input.substr(input.length() - len);
        if (moddata.contains(test)) {
            hist = test;
            break;
        }
    }
    if (hist.empty()) return suggestions;

    auto& pool = moddata[hist]; //get next characters related to the current string
    std::vector<std::pair<char, float>> sorted(pool.begin(), pool.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    for (size_t i = 0; i < sorted.size() && (int)suggestions.size() < n; i++) {
        std::string word = "";
        word += sorted[i].first;
        std::string roll_seed = input + sorted[i].first;

        for (int step = 0; step < 12; step++) {
            std::string sub_hist = "";
            for (int s = std::min((int)roll_seed.length(), 4); s >= 1; s--) {
                std::string test = roll_seed.substr(roll_seed.length() - s);
                if (moddata.contains(test)) { sub_hist = test; break; }
            }
            if (sub_hist.empty() || moddata[sub_hist].empty()) break;

            char best_char = ' ';
            float max_w = -1.0f;
            for (const auto& p : moddata[sub_hist]) {
                if (p.second > max_w) {
                    max_w = p.second;
                    best_char = p.first;
                }
            }

            if (best_char == ' ' || best_char == '\n') break;
            word += best_char;
            roll_seed += best_char;
        }
        if (!word.empty() && std::find(suggestions.begin(), suggestions.end(), word) == suggestions.end()) {
            suggestions.push_back(word);
        }
    }

    return suggestions;
}

int main() {
    const int screenWidth = 650;
    const int screenHeight = 160;
    InitWindow(screenWidth, screenHeight, "TypeU Assistant");
    SetTargetFPS(30); //max 30 fps
    loadModel("brain.bin");
    if (moddata.empty()) {
        trainModel("the quick brown fox jumps over the lazy dog ");
        trainModel("may the force be with you ");
        trainModel("the cake is a lie ");
        trainModel("hello world ");
    }

    std::string text;
    int fontSize = 20;
    Vector2 textOrigin = { 30, 80 };

    float backspaceHeldTime = 0.0f;
    float backspaceRepeatTimer = 0.0f;

    while (!WindowShouldClose()) {
        bool tabHeld = IsKeyDown(KEY_TAB);


std::vector<std::string> suggestions = predictWords(text, 3);

// shortcuts to select which word you want to input
bool suggestionAccepted = false;

if (tabHeld) {
    if (IsKeyPressed(KEY_ONE) && suggestions.size() >= 1) {
        text += suggestions[0];
        suggestionAccepted = true;
    } else if (IsKeyPressed(KEY_TWO) && suggestions.size() >= 2) {
        text += suggestions[1];
        suggestionAccepted = true;
    } else if (IsKeyPressed(KEY_THREE) && suggestions.size() >= 3) {
        text += suggestions[2];
        suggestionAccepted = true;
    }
} else if (IsKeyPressed(KEY_TAB) && !suggestions.empty()) {
    text += suggestions[0];
    suggestionAccepted = true;
}

// If a suggestion was just accepted add a space after it and train
if (suggestionAccepted) {
    text += " ";
    trainModel(text);
}

int key = GetCharPressed();
while (key > 0) {
    bool isNumber = (key >= '1' && key <= '9');

    if (key >= 32 && key <= 126 && !(tabHeld && isNumber)) {
        text += (char)key;
            if (key == ' ') {
                trainModel(text);
            }
    }
    key = GetCharPressed();
    }
        if (IsKeyDown(KEY_BACKSPACE)) {
            backspaceHeldTime += GetFrameTime();
            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (!text.empty()) text.pop_back();
            } else if (backspaceHeldTime > 0.4f) { // 400ms delay before rapid repeat
                backspaceRepeatTimer += GetFrameTime();
                if (backspaceRepeatTimer > 0.04f) { //repeat every 40ms
                    if (!text.empty()) text.pop_back();
                    backspaceRepeatTimer = 0.0f;
                }
            }
        } else {
            backspaceHeldTime = 0.0f;
            backspaceRepeatTimer = 0.0f;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            trainModel(text + " ");
            text.clear();
        }
        
        int textWidth = MeasureText(text.c_str(), fontSize);
        Vector2 caretPos = { textOrigin.x + textWidth, textOrigin.y };

        // app visuals
        BeginDrawing();
        ClearBackground(GetColor(0x1E1E2EFF));

        //Hint
        DrawText("Type text (Tab / 1-3 to accept suggestions | Enter to commit)", 30, 15, 13, GRAY);

        //TextBox
        DrawRectangle(25, 68, screenWidth - 50, 42, GetColor(0x28283DFF));
        DrawRectangleLines(25, 68, screenWidth - 50, 42, GetColor(0x45475AFF));

        //Text
        DrawText(text.c_str(), textOrigin.x, textOrigin.y, fontSize, RAYWHITE);

        // Cursor
        if (((int)(GetTime() * 2.5)) % 2 == 0) {
            DrawRectangle(caretPos.x + 2, caretPos.y - 2, 2, fontSize + 4, SKYBLUE);
        }

        // Suggestion
        if (!suggestions.empty()) {
            std::string pillText = "";
            for (size_t i = 0; i < suggestions.size(); i++) {
                pillText += "[" + std::to_string(i + 1) + "] " + suggestions[i];
                if (i + 1 < suggestions.size()) pillText += "   ";
            }

            int pillWidth = MeasureText(pillText.c_str(), 14) + 16;
            Rectangle pillRect = {
                std::clamp(caretPos.x - 20, 30.0f, (float)screenWidth - pillWidth - 30),
                caretPos.y - 36,
                (float)pillWidth,
                24
            };

            DrawRectangleRounded(pillRect, 0.4f, 4, GetColor(0x313244FF));
            DrawRectangleRoundedLines(pillRect, 0.4f, 4, SKYBLUE);
            DrawText(pillText.c_str(), pillRect.x + 8, pillRect.y + 5, 14, RAYWHITE);
        }

        EndDrawing();
    }

    // Save and quit
    saveModel("brain.bin");
    CloseWindow();
    return 0;
}