//
// Created by WeiWei on 2022/2/20.
//

#include <algorithm>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "Wordle.h"

//std::map<std::string, std::string> colorCode
//        {
//                {"default", "\033[0m"},
//                {"red", "\033[0;31m"},
//                {"green", "\033[0;32m"},
//                {"blue", "\033[0;34m"},
//                {"cyan", "\033[0;36m"},
//                {"magenta", "\033[0;35m"},
//                {"yellow", "\033[0;33m"},
//                {"correct", "\033[0;92m"},
//                {"included", "\033[0;93m"},
//                {"excluded", "\033[0;930m"}
//        };
static std::map<char, bool> exists;

void Wordle::GenerateDebugPuzzle(const std::string &_answer)
{
    isDebug = true;
    answerToday = _answer;
    allowedGuessesList.clear();

    for (auto &c : answerToday) {
        exists[c] = true;
    }
}

void Wordle::GenerateNewPuzzle()
{
    exists.clear();

    // Generate today random answer from answer list.
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<size_t> dist(0, answerList.size() - 1);
    answerToday = answerList[dist(mt)];

    for (auto& c : answerToday) {
        exists[c] = true;
    }
}

void Wordle::GenerateTodayPuzzle()
{
    // Setting...
    LoadAllowedGuessesList("words/wordle-allowed-guesses.txt");
    LoadAnswersAlphabeticalList("words/wordle-answers-alphabetical.txt");
}

const std::string& Wordle::GetAnswer()
{
    return answerToday;
}

const std::set<std::string>& Wordle::GetAllowedGuessesList()
{
    return allowedGuessesList;
}

bool Wordle::CheckGuessValidation(const std::string& guess)
{
    std::string _guess = guess;
    std::transform(_guess.begin(), _guess.end(), _guess.begin(),
        [](unsigned char c) { return std::tolower(c); });

    return allowedGuessesList.find(_guess) != allowedGuessesList.end();
}

bool Wordle::PrintGuessResult(const std::string& guess)
{
    // convert to lower
    std::string _guess = guess;
    std::transform(_guess.begin(), _guess.end(), _guess.begin(),
        [](unsigned char c) { return std::tolower(c); });

    std::string result;
    for (auto i = 0; i < wordLength; ++i)
    {
        if (answerToday[i] == _guess[i])
        {
            result += "\033[92;102m██\033[m ";
        }
        else if (exists[_guess[i]])
        {
            result += "\033[93;103m██\033[m ";
        }
        else
        {
            result += "\033[90;100m██\033[m ";
        }
    }
    result += "   ";
    for (auto i = 0; i < wordLength; ++i)
    {
        if (answerToday[i] == _guess[i])
        {
            result += "\033[92m";
        }
        else if (exists[_guess[i]])
        {
            result += "\033[93m";
        }
        else
        {
            result += "\033[90m";
        }
        result += _guess[i];
        result += "\033[m";
    }

    std::cout << result;
    return _guess == answerToday;
}

std::vector<WordleState> Wordle::GetGuessResult(const std::string& guess)
{
    // convert to lower
    std::string _guess = guess;
    std::transform(_guess.begin(), _guess.end(), _guess.begin(),
        [](unsigned char c) { return std::tolower(c); });

    std::vector<WordleState> result(wordLength, ws_excluded);
    for (auto i = 0; i < wordLength; ++i)
    {
        if (answerToday[i] == _guess[i])
        {
            result[i] = ws_correct;
        }
        else if (exists[_guess[i]])
        {
            result[i] = ws_included;
        }
    }
    return result;
}

int Wordle::GetGuessLimit()
{
    return guessLimit;
}

int Wordle::GetWordLength()
{
    return wordLength;
}

void Wordle::LoadAllowedGuessesList(const std::string& inputDirectory)
{
    std::ifstream allowedListStream(inputDirectory);
    if (allowedListStream.is_open())
    {
        std::string word;
        while(std::getline(allowedListStream, word))
        {
            std::transform(word.begin(), word.end(), word.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            allowedGuessesList.insert(word);
        }
    }
    else {
        // todo-max Add error code...
        return;
    }
}

void Wordle::LoadAnswersAlphabeticalList(const std::string& inputDirectory)
{
    std::ifstream answerListStream(inputDirectory);
    if (answerListStream.is_open())
    {
        std::string word;
        while(std::getline(answerListStream, word))
        {
            std::transform(word.begin(), word.end(), word.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            answerList.push_back(word);
        }
    }
    else {
        // todo-max Add error code...
        return;
    }

    // Generate today random answer from answer list.
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<size_t> dist(0, answerList.size() - 1);
    answerToday = answerList[dist(mt)];

    for (auto &c : answerToday) {
        exists[c] = true;
    }
}