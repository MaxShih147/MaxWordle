//
// Created by WeiWei on 2022/2/20.
//

#ifndef WEISWORDLE_WORDLE_H
#define WEISWORDLE_WORDLE_H

#include <string>
#include <set>

enum WordleState
{
    ws_unknown,
    ws_correct,
    ws_included,
    ws_excluded
};

class Wordle
{
public:
    Wordle()
    : isDebug(false), answerToday(""), allowedGuessesList(std::set<std::string>()),
      guessLimit(6), wordLength(5)
    {
    }
public:
    void GenerateDebugPuzzle(const std::string& _answer);
    void GenerateNewPuzzle();
    void GenerateTodayPuzzle();
    const std::string& GetAnswer();
    const std::set<std::string>& GetAllowedGuessesList();
    bool CheckGuessValidation(const std::string& guess);
    bool PrintGuessResult(const std::string& guess);
    std::vector<WordleState> GetGuessResult(const std::string& guess);
    int GetGuessLimit();
    int GetWordLength();
//    void SetGuessLimit(int _guessLimit);
//    void SetWordLength(int _wordLength);
private:
    void LoadAllowedGuessesList(const std::string& inputDirectory);
    void LoadAnswersAlphabeticalList(const std::string& inputDirectory);
private:
    bool isDebug;
    int guessLimit;
    int wordLength;
    std::string answerToday;
    std::set<std::string> allowedGuessesList;
    std::vector<std::string> answerList;
};

#endif //WEISWORDLE_WORDLE_H