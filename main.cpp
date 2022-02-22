﻿#include <algorithm>
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
ImVec2 g_window_pos_default(10, 10);
ImVec2 g_window_pos = g_window_pos_default;
int g_frame_count = 0;

// Generate today random answer from answer list.
std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-10, 10);

#if defined(WITH_GUI)
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

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

void RunWindowVibrationEffect(bool open)
{
    if (open)
    {
        g_window_pos.x += g_frame_count % 2 == 0 ? 10 : -10;
    }
    else
    {
        g_window_pos = g_window_pos_default;
    }
}

void ShowTodayPuzzle()
{
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

    static int wordLength = g_wordle->GetWordLength();
    static int guessLimit = g_wordle->GetGuessLimit();
    static std::vector<WordleState> defaultStates(wordLength, ws_unknown);
    static std::vector<std::string> guessList(guessLimit, "");
    static std::vector<std::vector<WordleState>> stateList(guessLimit, defaultStates);
    static int currentGuess = 0;
    static bool win = false;

    bool guessTrigger = false;

    if (ImGui::TreeNode("Show My Little Keyboard"))
    {
        int currentIndex = 0;
        ImVec2 gridSize(40, 40);

        for (auto i = 0; i < lettersPerLines.size(); ++i)
        {
            for (auto j = 0; j < lettersPerLines[i].size(); ++j)
            {
                ImVec2 alignment = ImVec2((float) j / 2.0f, (float) i / 2.0f);
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
                auto color = stateColorCodeForKeyboard[state];
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)color.normal);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)color.hovered);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)color.active);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
                ImGui::PushFont(g_font_20);
                if (ImGui::Button(name.c_str(), gridSize))
                {
                    guessList[currentGuess] += name;
                }
                ImGui::PopFont();
                ImGui::PopStyleVar(1);
                ImGui::PopStyleColor(3);

                ++currentIndex;
            }
        }

        bool valid = true;

        ImGui::PushFont(g_font_20);
        if (ImGui::Button("enter"))
        {
            std::cout << "[DEBUG] current guess = " << guessList[currentGuess] << "\n";

            valid = g_wordle->CheckGuessValidation(guessList[currentGuess]);
                        
            if (valid)
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
                ++g_frame_count;
                guessList[currentGuess].clear();
            }
        }
        ImGui::PopFont();

        // try to effect for N frame
        bool open = !valid;
        if (g_frame_count > 0 && g_frame_count < 10
            )
        {
            g_window_pos.x += g_frame_count % 2 == 0 ? 10 : -10;
            ++g_frame_count;
        }
        else
        {
            g_window_pos = g_window_pos_default;
            g_frame_count = 0;
        }

        ImGui::SameLine();

        ImGui::PushFont(g_font_20);
        if (ImGui::Button("back"))
        {
            if (!guessList[currentGuess].empty())
            {
                guessList[currentGuess].pop_back();
            }
        }
        ImGui::PopFont();

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Show My Guesses"))
    {
        ImVec2 gridSize(40, 40);
        ImGuiStyle& style = ImGui::GetStyle();

        for (int i = 0; i < guessLimit; ++i)
        {
            for (auto j = 0; j < wordLength; ++j) {
                if (j > 0)
                {
                    ImGui::SameLine();
                }

                //ImGui::PushID(i * wordLength + j);

                std::string value = " ";
                if (i <= currentGuess)
                {
                    if (j < guessList[i].size())
                    {
                        value = guessList[i][j];
                    }
                }

                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

                auto state = stateList[i];
                auto color = stateColorCodeForGuessList[state[j]];
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)color.normal);

                ImGui::PushFont(g_font_20);

                ImGui::Button(value.c_str(), gridSize);
                
                ImGui::PopFont();
                ImGui::PopStyleColor(1);
                ImGui::PopItemFlag();
                ImGui::PopStyleVar(1);
                //ImGui::PopID();
            }
        }
        ImGui::TreePop();
    }

    if (win)
    {
        ImGui::SetNextWindowPos(g_window_pos);
        //ImGui::SetNextWindowSize(ImVec2());
        ImGui::Begin("Win", &win);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("You Win!");
        if (ImGui::Button("New Game"))
        {
            win = false;
        }
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
    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    g_font_12 = io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 12);
    g_font_20 = io.Fonts->AddFontFromFileTTF("fonts/arial.ttf", 20);


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
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        if (show_max_dev_windows)
        {
            ImGui::SetNextWindowPos(g_window_pos);
            //ImGui::SetNextWindowSize(ImVec2());
            ImGui::Begin("Start Today's Puzzle", &show_max_dev_windows);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("...");
            ShowTodayPuzzle();
//            if (ImGui::Button("Close Me"))
//            {
//                show_max_dev_windows = false;
//            }
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