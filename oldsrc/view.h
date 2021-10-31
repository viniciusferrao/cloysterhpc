#ifndef VIEW_H
#define VIEW_H

#include <vector>
#include <string>

/* This is just a prototype about making the View as an Interface to be easily
 * swapped in the future if needed. There's much more organization to do before
 * we start using abstract classes.
 */
class View {
public:
    virtual ~View() = default;

    virtual std::string timezoneSelection
        (const std::vector<std::string>&) = 0;
    virtual std::string localeSelection
        (const std::vector<std::string>&) = 0;
};

#endif /* VIEW_H */
