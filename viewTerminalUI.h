//
// Created by Vinícius Ferrão on 27/10/21.
//

#ifndef CLOYSTER_VIEWTERMINALUI_H
#define CLOYSTER_VIEWTERMINALUI_H

#include <string>
#include <newt.h>
#include "viewSubscriber.h"

class viewTerminalUI /*: public view */ {
private:
    viewSubscriber* m_subscriber;

public:
    viewTerminalUI();
    ~viewTerminalUI();

    void subscribe(viewSubscriber* subscriber);

    void setLabelText(const std::string &text);
    std::string getUserText();

private:
    void welcomeMessage ();

};

#endif //CLOYSTER_VIEWTERMINALUI_H
