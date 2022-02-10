//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newt.h"
#include <optional>
#include <memory>

// TODO: Enhance this method, use std::transform? Lambda functions?
std::vector<std::string> Newt::fieldMenu (
                                        const char* title,
                                        const char* message,
                                        const std::vector<std::string>& items,
                                        const char* helpMessage)
{
    std::vector<std::pair<std::string,std::string>> pairs;
    std::vector<std::string> returnVector;

    pairs.reserve(items.size());
    for (const auto& item : items)
        pairs.emplace_back(std::make_pair(item, std::string{}));

    const auto& returnPairs = fieldMenu(title, message, pairs, helpMessage);

    returnVector.reserve(returnPairs.size());
    for (const auto& returnPair : returnPairs)
        returnVector.emplace_back(returnPair.second);

    return returnVector;
}

std::vector<std::pair<std::string,std::string>> Newt::fieldMenu (
        const char* title,
        const char* message,
        // TODO: std::optional on second pair
        const std::vector<std::pair<std::string,std::string>>& items,
        const char* helpMessage)
{

    int returnValue;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 20;

    size_t vectorSize = items.size();
    auto fieldEntries = std::make_unique<char*[]>(vectorSize + 1);
    auto field = std::make_unique<newtWinEntry[]>(vectorSize + 1);

    // This "for loop" will populate newtWinEntry with the necessary data to be
    // displayed on the interface.
    // Please note that field[i].value is a char** because it's passing data by
    // reference in C style, since the data can be modified by the newt form,
    // it's not an array of char*
    for (size_t i = 0 ; i < vectorSize ; i++) {
        field[i].text = const_cast<char*>(items[i].first.c_str());
        fieldEntries[i] = const_cast<char*>(items[i].second.c_str());

        // TODO: Check is there's a way to hide &
        field[i].value = &fieldEntries[i];

        // TODO: Fix this hack to enable password fields
        if (items[i].first.find("Password") != std::string::npos ||
            items[i].first.find("password") != std::string::npos)
            field[i].flags = NEWT_FLAG_PASSWORD;
        else
            field[i].flags = 0;
    }

    field[vectorSize].text = nullptr;
    field[vectorSize].value = nullptr;
    field[vectorSize].flags = 0;

    std::vector<std::pair<std::string,std::string>> returnVector;

    question:
    returnValue = newtWinEntries(const_cast<char*>(title),
                                 const_cast<char*>(message),
                                 suggestedWidth, flexDown, flexUp,
                                 maxHeightList, field.get(),
                                 const_cast<char*>(MSG_BUTTON_OK),
                                 MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            // TODO: The view should now check for this, it's a passive view
            if (hasEmptyField(field.get()))
                goto question;

            returnVector.reserve(vectorSize);
            for (size_t i = 0 ; field[i].text ; i++) {
                returnVector.emplace_back(
                        std::make_pair<std::string,std::string>(
                                field[i].text, *field[i].value));
            }

            return returnVector;
        case 2:
            abortInstall();
            break;
        case 3:
            this->helpMessage(helpMessage);
            goto question;
        default:
            throw std::runtime_error(
                    "Invalid return value from fields on newt library");
    }

    throw std::runtime_error("Invalid return path on newt library");
}
