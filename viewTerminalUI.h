//
// Created by Vinícius Ferrão on 27/10/21.
//

#ifndef CLOYSTER_VIEWTERMINALUI_H
#define CLOYSTER_VIEWTERMINALUI_H

#include <vector>
#include <string>
#include <newt.h>
#include "viewSubscriber.h"

class viewTerminalUI /*: public view */ {
private:
    viewSubscriber* m_subscriber;
    std::vector<std::string> m_fields;

public:
    viewTerminalUI();
    ~viewTerminalUI();

    void subscribe(viewSubscriber* subscriber);

    void abortInstall();

    /* TODO: Remove this method */
    void setLabelText(const std::string &text);

    std::vector<std::string> getAnswers();

    void welcomeMessage ();
    void timezoneSelection(const std::vector<std::string>&);
    void localeSelection(const std::vector<std::string>&);


private:
    std::string drawListMenu (const char*, const char*,
                              const std::vector<std::string>&,
                              const char*);

};

#endif //CLOYSTER_VIEWTERMINALUI_H
