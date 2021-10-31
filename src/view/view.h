//
// Created by Vinícius Ferrão on 30/10/21.
//

#ifndef CLOYSTER_VIEW_H
#define CLOYSTER_VIEW_H

/* This is just a prototype about making the View as an Interface to be easily
 * swapped in the future if needed. There's much more organization to do before
 * we start using abstract classes.
 */
class View {
public:
    virtual ~View() = default;

    virtual void abortInstall() = 0;
    virtual void helpMessage(const char*) = 0;
};

#endif //CLOYSTER_VIEW_H
