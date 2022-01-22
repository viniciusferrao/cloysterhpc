//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newt.h"
#include <optional>
#include <memory>

std::vector<std::string> Newt::fieldMenu (
        const char* title,
        const char* message,
        // TODO: std::pair and std::optional for already set variables
        //const std::vector<std::pair<std::string,std::optional<std::string>>>& items,
        const std::vector<std::string>& items,
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

    for (unsigned i = 0 ; i < vectorSize ; i++) {
        field[i].text = const_cast<char*>(items[i].c_str());
        //field[i].value = fieldEntries.get() + i;
        field[i].value = &fieldEntries[i];
        /* TODO: Fix this hack to enable password fields */
        if (items[i].find("Password") != std::string::npos ||
            items[i].find("password") != std::string::npos)
            field[i].flags = NEWT_FLAG_PASSWORD;
        else
            field[i].flags = 0;
    }
    field[vectorSize].text = nullptr;
    field[vectorSize].value = nullptr;
    field[vectorSize].flags = 0;

    std::vector<std::string> entries;

    question:
    returnValue = newtWinEntries(const_cast<char *>(title),
                                 const_cast<char *>(message),
                                 suggestedWidth, flexDown, flexUp,
                                 maxHeightList, field.get(),
                                 const_cast<char *>(MSG_BUTTON_OK),
                                 MSG_BUTTON_CANCEL, MSG_BUTTON_HELP, NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            if (hasEmptyField(field.get()))
                goto question;
#ifdef _DEBUG_
            debugEntries(field.get());
#endif
            for (unsigned i = 0 ; field[i].value ; i++) {
                entries.emplace_back(*field[i].value);
            }

            return entries;
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
