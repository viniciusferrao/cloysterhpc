//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newtViewListMenu.h"

std::string NewtViewListMenu::draw (const char * title, const char * message,
                                      const std::vector<std::string>& items,
                                      const char * helpMessage) {

    int returnValue;
    int selector = 0;

    int suggestedWidth = 50;
    int flexUp = 5;
    int flexDown = 5;
    int maxHeightList = 3;

    /* Newt expects a NULL terminated array of C style strings */
    std::vector<const char*> cStrings;
    cStrings.reserve(items.size() + 1);

    for (const auto& item : items) {
        cStrings.push_back(item.c_str());
    }
    cStrings.push_back(nullptr);

#if 1
    /* Goto implementation */
    question:
    returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexDown, flexUp, maxHeightList,
                              const_cast<char **>(cStrings.data()), &selector,
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL),
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

    switch(returnValue) {
        case 0:
            /* F12 is pressed, and we don't care; continue to case 1 */
        case 1:
            return items[selector];
        case 2:
            abortInstall();
            break;
        case 3:
            this->helpMessage(helpMessage);
            goto question;
    }
#endif
#if 0
    /* Gotoless implementation */
    for (;;) {
        returnValue = newtWinMenu(const_cast<char *>(title),
                              const_cast<char *>(message),
                              suggestedWidth, flexUp, flexDown, maxHeightList,
                              const_cast<char **>(items), &selector,
                              const_cast<char *>(MSG_BUTTON_OK),
                              const_cast<char *>(MSG_BUTTON_CANCEL),
                              const_cast<char *>(MSG_BUTTON_HELP), NULL);

        switch(returnValue) {
            case 0:
                /* F12 is pressed, and we don't care; continue to case 1 */
            case 1:
                return items[selector];
            case 2:
                abortInstall();
            case 3:
                this->helpMessage(helpMessage);
                continue;
        }
        break; /* for (;;) */
    }
#endif
    return "Error!"; /* We should never reach here */
}
