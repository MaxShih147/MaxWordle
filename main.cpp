#include <algorithm>
#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <map>


#include "Wordle.h"

#define WITH_GUI

#if defined(WITH_GUI)
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
#endif

Wordle* g_wordle = nullptr;
ImFont* g_font_12 = nullptr;
ImFont* g_font_20 = nullptr;
ImFont* g_font_24 = nullptr;
ImVec2 g_window_pos_default(400, 50);
ImVec2 g_window_pos = g_window_pos_default;
int g_frame_count = 0;

ImGuiWindowFlags g_main_window_flags = ImGuiWindowFlags_NoDecoration;


// Generate today random answer from answer list.
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-10, 10);


// ! Layout Section
// ! - Window
ImVec2 puzzelWindowSize(600, 800);
ImVec2 puzzelWindowPosition(400, 50);
// ! - Keyboard
ImVec2 keyboardSize(40, 50);
int keyboardSpacingY = 10;
int keyboardStartPositionX = 55;       // should be function of above parameters.
int keyboardStartPositionY = 560;      // should be function of above parameters.
ImVec2 keyboardStartPositionPerLine[3] // should be function of above parameters.
{
    ImVec2(keyboardStartPositionX, keyboardStartPositionY),
    ImVec2(keyboardStartPositionX + 24, keyboardStartPositionY + keyboardSize.y + keyboardSpacingY),
    ImVec2(keyboardStartPositionX + 72, keyboardStartPositionY + 2 * (keyboardSize.y + keyboardSpacingY))
};
// ! Enter Button
ImVec2 enterButtonSize(60, 50);
int enterButtonSpacingY = 10;
int enterButtonStartPositionX = 59;       // should be function of above parameters.
int enterButtonStartPositionY = keyboardStartPositionPerLine[2].y;
// ! Back Button
ImVec2 backButtonSize(60, 50);
int backButtonSpacingY = 10;
int backButtonStartPositionX = 463;       // should be function of above parameters.
int backButtonStartPositionY = keyboardStartPositionPerLine[2].y;
// ! Share Button
ImVec2 shareButtonSize(180, 50);
float shareButtonRound = 20.0f;
int shareButtonStartPositionX = 100;       // should be function of above parameters.
int shareButtonStartPositionY = 230;
// ! New Game Button
ImVec2 newGameButtonSize(180, 50);
float newGameButtonRound = 20.0f;
int newGameButtonStartPositionX = 100;       // should be function of above parameters.
int newGameButtonStartPositionY = 300;
// ! - Guess Panel
ImVec2 guessPanelSize(70, 70);
float guessPanelRound = 35.0f;
int guessPanelSpacingX = 10;
int guessPanelSpacingY = 10;
int guessPanelStartPositionX = 100;    // should be function of above parameters.
int guessPanelStartPositionY = 55;     // should be function of above parameters.

#if defined(WITH_GUI)
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

ImColor backgroundColor = ImColor::HSV(120.0f / 360.0f, 1.0f, 0.024f);

class ButtonColor
{
public:
    ButtonColor() {}
    ButtonColor(ImVec4 _normal, ImVec4 _hovered, ImVec4 _active)
        : normal(_normal), hovered(_hovered), active(_active)
    {
    }
    ImVec4 normal;
    ImVec4 hovered;
    ImVec4 active;
};

ButtonColor enterButtonColor
(
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f)
);
ButtonColor backButtonColor
(
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f)
);
ButtonColor newGameButtonColor
(
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f)
);
ButtonColor shareButtonColor
(
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
    (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f)
);

static std::vector<std::vector<char>> lettersPerLines
{
        {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
        {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L'},
        {'Z', 'X', 'C', 'V', 'B', 'N', 'M'}
};

static std::map<char, WordleState> letterStates
{
    {'A', ws_unknown}, {'B', ws_unknown}, {'C', ws_unknown}, {'D', ws_unknown}, {'E', ws_unknown},
    {'F', ws_unknown}, {'G', ws_unknown}, {'H', ws_unknown}, {'I', ws_unknown}, {'J', ws_unknown},
    {'K', ws_unknown}, {'L', ws_unknown}, {'M', ws_unknown}, {'N', ws_unknown}, {'O', ws_unknown},
    {'P', ws_unknown}, {'Q', ws_unknown}, {'R', ws_unknown}, {'S', ws_unknown}, {'T', ws_unknown},
    {'U', ws_unknown}, {'V', ws_unknown}, {'W', ws_unknown}, {'X', ws_unknown}, {'Y', ws_unknown},
    {'Z', ws_unknown}
};

static std::map<char, bool> fixedStateToCorrect
{
    {'A', false}, {'B', false}, {'C', false}, {'D', false}, {'E', false},
    {'F', false}, {'G', false}, {'H', false}, {'I', false}, {'J', false},
    {'K', false}, {'L', false}, {'M', false}, {'N', false}, {'O', false},
    {'P', false}, {'Q', false}, {'R', false}, {'S', false}, {'T', false},
    {'U', false}, {'V', false}, {'W', false}, {'X', false}, {'Y', false},
    {'Z', false}
};


static std::map<WordleState, ButtonColor> stateColorCodeForKeyboard
{
    {ws_unknown, ButtonColor(
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f))},
    {ws_correct, ButtonColor(
        (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f),
        (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.7f, 0.7f),
        (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.8f, 0.8f))},
    {ws_included, ButtonColor(
        (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.6f, 0.6f),
        (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.7f, 0.7f),
        (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.8f, 0.8f)) },
    {ws_excluded, ButtonColor(
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.1f),
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.2f),
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.3f))}
};

static std::map<WordleState, ButtonColor> stateColorCodeForGuessList
{
    {ws_unknown, ButtonColor(
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.4f),
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.5f),
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.6f))},
    {ws_correct, ButtonColor(
        (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.6f, 0.6f),
        (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.7f, 0.7f),
        (ImVec4)ImColor::HSV(2.0f / 7.0f, 0.8f, 0.8f))},
    {ws_included, ButtonColor(
        (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.6f, 0.6f),
        (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.7f, 0.7f),
        (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.8f, 0.8f)) },
    {ws_excluded, ButtonColor(
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.1f),
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.2f),
        (ImVec4)ImColor::HSV(0.0f / 7.0f, 0.0f, 0.3f))}
};

std::map<std::string, ImGuiKey> keyNameMap
{
    {"A", ImGuiKey_A}, {"B", ImGuiKey_B}, {"C", ImGuiKey_C}, {"D", ImGuiKey_D}, {"E", ImGuiKey_E},
    {"F", ImGuiKey_F}, {"G", ImGuiKey_G}, {"H", ImGuiKey_H}, {"I", ImGuiKey_I}, {"J", ImGuiKey_J},
    {"K", ImGuiKey_K}, {"L", ImGuiKey_L}, {"M", ImGuiKey_M}, {"N", ImGuiKey_N}, {"O", ImGuiKey_O},
    {"P", ImGuiKey_P}, {"Q", ImGuiKey_Q}, {"R", ImGuiKey_R}, {"S", ImGuiKey_S}, {"T", ImGuiKey_T},
    {"U", ImGuiKey_U}, {"V", ImGuiKey_V}, {"W", ImGuiKey_W}, {"X", ImGuiKey_X}, {"Y", ImGuiKey_Y},
    {"Z", ImGuiKey_Z}, {"enter", ImGuiKey_Enter}, {"back", ImGuiKey_Backspace}
};

void ShowTodayPuzzle()
{
    static bool win = false;
    static int wordLength = g_wordle->GetWordLength();
    static int guessLimit = g_wordle->GetGuessLimit();
    static int currentGuess = 0;
    static std::vector<WordleState> defaultStates(wordLength, ws_unknown);
    static std::vector<std::string> guessList(guessLimit, "");
    static std::vector<std::vector<WordleState>> stateList(guessLimit, defaultStates);

    ImGuiStyle& style = ImGui::GetStyle();

    //if (ImGui::TreeNode("Keyboard, Gamepad & Navigation State"))
    //{
    //    ImGuiIO& io = ImGui::GetIO();

    //    struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; } }; // Hide Native<>ImGuiKey duplicates when both exists in the array
    //    const ImGuiKey key_first = 0;
    //    //ImGui::Text("Keys down:");          for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (funcs::IsLegacyNativeDupe(key)) continue; if (ImGui::IsKeyDown(key)) { ImGui::SameLine(); ImGui::Text("\"%s\" %d (%.02f secs)", ImGui::GetKeyName(key), key, ImGui::GetKeyData(key)->DownDuration); } }
    //    //ImGui::Text("Keys pressed:");       for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (funcs::IsLegacyNativeDupe(key)) continue; if (ImGui::IsKeyPressed(key)) { ImGui::SameLine(); ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key); } }
    //    //ImGui::Text("Keys released:");      for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (funcs::IsLegacyNativeDupe(key)) continue; if (ImGui::IsKeyReleased(key)) { ImGui::SameLine(); ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key); } }
    //    //ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
    //    //ImGui::Text("Chars queue:");        for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
    //    //ImGui::Text("NavInputs down:");     for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputs[i] > 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f (%.02f secs)", i, io.NavInputs[i], io.NavInputsDownDuration[i]); }
    //    //ImGui::Text("NavInputs pressed:");  for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] == 0.0f) { ImGui::SameLine(); ImGui::Text("[%d]", i); }

    //    // Draw an arbitrary US keyboard layout to visualize translated keys
    //    {
    //        const ImVec2 key_size = ImVec2(35.0f, 35.0f);
    //        const float  key_rounding = 3.0f;
    //        const ImVec2 key_face_size = ImVec2(25.0f, 25.0f);
    //        const ImVec2 key_face_pos = ImVec2(5.0f, 3.0f);
    //        const float  key_face_rounding = 2.0f;
    //        const ImVec2 key_label_pos = ImVec2(7.0f, 4.0f);
    //        const ImVec2 key_step = ImVec2(key_size.x - 1.0f, key_size.y - 1.0f);
    //        const float  key_row_offset = 9.0f;

    //        ImVec2 board_min = ImGui::GetCursorScreenPos();
    //        ImVec2 board_max = ImVec2(board_min.x + 3 * key_step.x + 2 * key_row_offset + 10.0f, board_min.y + 3 * key_step.y + 10.0f);
    //        ImVec2 start_pos = ImVec2(board_min.x + 5.0f - key_step.x, board_min.y);

    //        struct KeyLayoutData { int Row, Col; const char* Label; ImGuiKey Key; };
    //        const KeyLayoutData keys_to_display[] =
    //        {
    //            { 0, 0, "", ImGuiKey_Tab },      { 0, 1, "Q", ImGuiKey_Q }, { 0, 2, "W", ImGuiKey_W }, { 0, 3, "E", ImGuiKey_E }, { 0, 4, "R", ImGuiKey_R },
    //            { 1, 0, "", ImGuiKey_CapsLock }, { 1, 1, "A", ImGuiKey_A }, { 1, 2, "S", ImGuiKey_S }, { 1, 3, "D", ImGuiKey_D }, { 1, 4, "F", ImGuiKey_F },
    //            { 2, 0, "", ImGuiKey_LeftShift },{ 2, 1, "Z", ImGuiKey_Z }, { 2, 2, "X", ImGuiKey_X }, { 2, 3, "C", ImGuiKey_C }, { 2, 4, "V", ImGuiKey_V }
    //        };

    //        ImDrawList* draw_list = ImGui::GetWindowDrawList();
    //        draw_list->PushClipRect(board_min, board_max, true);
    //        for (int n = 0; n < IM_ARRAYSIZE(keys_to_display); n++)
    //        {
    //            const KeyLayoutData* key_data = &keys_to_display[n];
    //            ImVec2 key_min = ImVec2(start_pos.x + key_data->Col * key_step.x + key_data->Row * key_row_offset, start_pos.y + key_data->Row * key_step.y);
    //            ImVec2 key_max = ImVec2(key_min.x + key_size.x, key_min.y + key_size.y);
    //            draw_list->AddRectFilled(key_min, key_max, IM_COL32(204, 204, 204, 255), key_rounding);
    //            draw_list->AddRect(key_min, key_max, IM_COL32(24, 24, 24, 255), key_rounding);
    //            ImVec2 face_min = ImVec2(key_min.x + key_face_pos.x, key_min.y + key_face_pos.y);
    //            ImVec2 face_max = ImVec2(face_min.x + key_face_size.x, face_min.y + key_face_size.y);
    //            draw_list->AddRect(face_min, face_max, IM_COL32(193, 193, 193, 255), key_face_rounding, ImDrawFlags_None, 2.0f);
    //            draw_list->AddRectFilled(face_min, face_max, IM_COL32(252, 252, 252, 255), key_face_rounding);
    //            ImVec2 label_min = ImVec2(key_min.x + key_label_pos.x, key_min.y + key_label_pos.y);
    //            draw_list->AddText(label_min, IM_COL32(64, 64, 64, 255), key_data->Label);
    //            if (ImGui::IsKeyDown(key_data->Key))
    //                draw_list->AddRectFilled(key_min, key_max, IM_COL32(255, 0, 0, 128), key_rounding);
    //        }
    //        draw_list->PopClipRect();
    //        ImGui::Dummy(ImVec2(board_max.x - board_min.x, board_max.y - board_min.y));
    //    }
    //    ImGui::TreePop();
    //}

    // ! START POS
    ImGui::SetCursorPos(ImVec2(200, 200));

    for (auto i = 0; i < lettersPerLines.size(); ++i)
    {
        ImGui::SetCursorPos(keyboardStartPositionPerLine[i]);
        for (auto j = 0; j < lettersPerLines[i].size(); ++j)
        {
            ImVec2 alignment = ImVec2((float)j / 2.0f, (float)i / 2.0f);
            std::string name(1, lettersPerLines[i][j]);
            if (j > 0)
            {
                ImGui::SameLine();
            }

            auto state = letterStates[lettersPerLines[i][j]];
            if (fixedStateToCorrect[lettersPerLines[i][j]])
            {
                state = ws_correct;
            }
            const auto& color = stateColorCodeForKeyboard[state];
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)color.normal);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)color.hovered);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)color.active);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::PushFont(g_font_20);
            if (ImGui::Button(name.c_str(), keyboardSize) || 
                ImGui::IsKeyPressed(keyNameMap[name]))
            {
                if (guessList[currentGuess].size() < wordLength)
                {
                    guessList[currentGuess] += name;
                }
            }
            ImGui::PopFont();
            ImGui::PopStyleVar(1);
            ImGui::PopStyleColor(3);
        }
    }

    ImGui::SetCursorPos(ImVec2(enterButtonStartPositionX, enterButtonStartPositionY));
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)enterButtonColor.normal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)enterButtonColor.hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)enterButtonColor.active);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushFont(g_font_20);
    if (ImGui::Button("enter", enterButtonSize) || 
        ImGui::IsKeyPressed(keyNameMap["enter"]))
    {
        std::cout << "[DEBUG] current guess = " << guessList[currentGuess] << "\n";

        if (g_wordle->CheckGuessValidation(guessList[currentGuess]))
        {
            stateList[currentGuess] = g_wordle->GetGuessResult(guessList[currentGuess]);
            for (auto i = 0; i < guessList[currentGuess].size(); ++i)
            {
                letterStates[guessList[currentGuess][i]] = stateList[currentGuess][i];
                if (stateList[currentGuess][i] == ws_correct)
                {
                    fixedStateToCorrect[guessList[currentGuess][i]] = true;
                }
            }

            bool _win = true;
            for (auto i = 0; i < stateList[currentGuess].size(); ++i)
            {
                _win &= stateList[currentGuess][i] == ws_correct;
            }
            win = _win;
            ++currentGuess;
        }
        else
        {
            //++g_frame_count;
            guessList[currentGuess].clear();
        }
    }
    ImGui::PopFont();
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(3);

    // try to effect for N frame
    //bool open = !valid;
    //if (g_frame_count > 0 && g_frame_count < 10
    //    )
    //{
    //    g_window_pos.x += g_frame_count % 2 == 0 ? 10 : -10;
    //    ++g_frame_count;
    //}
    //else
    //{
    //    g_window_pos = g_window_pos_default;
    //    g_frame_count = 0;
    //}

    ImGui::SetCursorPos(ImVec2(backButtonStartPositionX, backButtonStartPositionY));
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)backButtonColor.normal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)backButtonColor.hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)backButtonColor.active);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushFont(g_font_20);
    if (ImGui::Button("back", backButtonSize) || 
        ImGui::IsKeyPressed(keyNameMap["back"]))
    {
        if (!guessList[currentGuess].empty())
        {
            guessList[currentGuess].pop_back();
        }
    }
    ImGui::PopFont();
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(3);

    // ! GUESS PANEL SECTION
    for (int i = 0; i < guessLimit; ++i)
    {
        ImGui::SetCursorPos(
            ImVec2(
                guessPanelStartPositionX, 
                guessPanelStartPositionY + i * (guessPanelSize.y + guessPanelSpacingY)
            )
        );
        for (auto j = 0; j < wordLength; ++j) {
            if (j > 0)
            {
                ImGui::SameLine();
            }

            std::string value = " ";
            if (i <= currentGuess)
            {
                if (j < guessList[i].size())
                {
                    value = guessList[i][j];
                }
            }

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, guessPanelRound);
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

            const auto& color = stateColorCodeForGuessList[stateList[i][j]];
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)color.normal);

            ImGui::PushFont(g_font_24);

            ImGui::Button(value.c_str(), guessPanelSize);

            ImGui::PopFont();
            ImGui::PopStyleColor(1);
            ImGui::PopItemFlag();
            ImGui::PopStyleVar(1);
        }
    }

    if (win || currentGuess >= guessLimit)
    {
        // ! Try to freeze main window first.
        g_main_window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration/* | ImGuiWindowFlags_NoBackground*/;
        bool open = true;
        ImGui::SetNextWindowPos(ImVec2(500, 250));
        ImGui::SetNextWindowSize(ImVec2(384, 400));
        ImGui::Begin("End", &open, window_flags);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        //ImGui::Text("You Win!");

        ImGui::SetCursorPos(ImVec2(shareButtonStartPositionX, shareButtonStartPositionY));
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)shareButtonColor.normal);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)shareButtonColor.hovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)shareButtonColor.active);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, shareButtonRound);
        ImGui::PushFont(g_font_24);

        if (ImGui::Button("Share", shareButtonSize))
        {
        }

        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(1);

        
        ImGui::SetCursorPos(ImVec2(newGameButtonStartPositionX, newGameButtonStartPositionY));
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)newGameButtonColor.normal);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)newGameButtonColor.hovered);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)newGameButtonColor.active);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, newGameButtonRound);
        ImGui::PushFont(g_font_24);

        if (ImGui::Button("New Game", newGameButtonSize))
        {
            win = false;
            g_wordle->GenerateNewPuzzle();

            // ! Reset all states.
            for (auto i = 0; i < lettersPerLines.size(); ++i)
            {
                for (auto j = 0; j < lettersPerLines[i].size(); ++j)
                {
                    letterStates[lettersPerLines[i][j]] = ws_unknown;
                    fixedStateToCorrect[lettersPerLines[i][j]] = false;
                }
            }
 
            wordLength = g_wordle->GetWordLength();
            guessLimit = g_wordle->GetGuessLimit();
            currentGuess = 0;

            guessList = std::vector<std::string>(guessLimit, "");
            stateList = std::vector<std::vector<WordleState>>(guessLimit, defaultStates);

            // ! Unfreeze the main window
            g_main_window_flags = ImGuiWindowFlags_NoDecoration;
        }

        ImGui::PopFont();
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(1);

        ImGui::End();
    }
}
#endif

#if defined(WITH_GUI)
int main()
{
    g_wordle = new Wordle;
    g_wordle->GenerateTodayPuzzle();

//    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Max Wordle", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_max_dev_windows = true;
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    g_font_12 = io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 12);
    g_font_20 = io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 20);
    g_font_24 = io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 24);


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        //{
        //    static float f = 0.0f;
        //    static int counter = 0;

        //    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        //    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

        //    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        //    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //        counter++;
        //    ImGui::SameLine();
        //    ImGui::Text("counter = %d", counter);

        //    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //    ImGui::End();
        //}

        if (show_max_dev_windows)
        {
            //ImGui::SetNextWindowPos(g_window_pos);
            //ImGui::SetNextWindowSize(puzzelWindowSize);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            ImGui::SetNextWindowPos(ImVec2(.0f, .0f));
            ImGui::SetNextWindowSize(ImVec2(width, height));

            //ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(backgroundColor)); // Set window background to red
            ImGui::Begin(
                "Start Today's Puzzle", 
                &show_max_dev_windows, 
                g_main_window_flags
            );
            ShowTodayPuzzle();
            //ImGui::PopStyleColor();
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    delete g_wordle;
    g_wordle = nullptr;

    return 0;
}

#else
int main()
{
    g_wordle = new Wordle;
    g_wordle->GenerateTodayPuzzle();

        int guessCount = g_wordle->GetGuessLimit();
        int wordLength = g_wordle->GetWordLength();
        std::string answer = g_wordle->GetAnswer();
    
        for (auto i = 0; i < guessCount; ++i)
        {
            // INPUT SECTION
            std::cout << ">> ";
            std::string guess;
            std::cin >> guess;
    
            if (guess.length() != wordLength) {
                std::cout << "[DEBUG] Please input a word with 5 letters.\n";
                return 0;
            }
    
            // convert to lower
            std::transform(guess.begin(), guess.end(), guess.begin(),
                           [](unsigned char c){ return std::tolower(c); });
    
            g_wordle->PrintGuessResult(guess);
            std::cout << "\n";
        }
    
        return 0;
}
#endif