//
// Created by Vinícius Ferrão on 30/10/21.
//

#include "newt.h"

#include "../functions.h"
#include <cstring> /* strlen() */
#include <cstdio> /* sprintf() */
#include <newt.h>

Newt::Newt()
    : m_flexDown(2)
    , m_flexUp(2) {
    newtInit();
    newtCls();

    // Get the terminal size
    newtGetScreenSize(&m_cols, &m_rows);
    m_suggestedWidth = m_cols / 2;
    m_dataWidth = m_suggestedWidth * 2 / 3;
    // Line count: title, box top border, padding, text message (var),
    // padding (before list), padding (after list), button (4), padding,
    // box bottom border, shadow, status.
    m_maxListHeight = m_rows - 14;

    /* Push the title to the top left corner */
    newtDrawRootText(0, 0, TUIText::title);

    /* Add the default help line in the bottom */
    newtPushHelpLine(TUIText::helpLine);
    newtRefresh();
}

Newt::~Newt() {
    newtFinished();
}

void Newt::abort() {
    // TODO: We should only destroy the view and not terminate the application
    this->~Newt();
    LOG_WARN("{}", TUIText::abort);
    std::exit(EXIT_SUCCESS);
}

// TODO: Remove this method; this check must be done outside the view
bool Newt::hasEmptyField(const struct newtWinEntry *entries) {
    /* This may result in a buffer overflow if the string is > 63 chars */
    char message[63] = {};

    /* This for loop will check for empty values on the entries, and it will
     * return true if any value is empty based on the length of the string.
     */
    for (unsigned i = 0 ; entries[i].text ; i++) {
        if (strlen(*entries[i].value) == 0) {
            sprintf(message, "%s cannot be empty\n", entries[i].text);

            newtWinMessage(nullptr, const_cast<char *>(TUIText::Buttons::ok), message);
            return true;
        }
    }

    return false;
}

/**
  * Show a progress message dialog
  * @param title
  * @param message
  * @param command
  * @param fPercent A function to transform a line
  * into a percent (a 0 to 100 value)
  */
bool Newt::progressMenu(const char* title,
                  const char* message,
                  cloyster::CommandProxy&& cmd,
                  std::function<double(std::string)> fPercent)
{

    std::string text;

    auto* form = newtForm(nullptr, nullptr, 0);

    auto* progress = newtScale(10, -1, 61, 1000);
    auto* label = newtTextbox(-1, -1, 61, 1, NEWT_TEXTBOX_WRAP);
    newtTextboxSetText(label, text.c_str());

    char* dtitle = strdup(title);

    newtGrid grid = newtCreateGrid(1, 3);
    newtComponent b1;
    newtGrid buttonGrid = newtButtonBar(const_cast<char*>(TUIText::Buttons::cancel), &b1, NULL);
    newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, progress,
                     1, 1, 0, 0, 0, NEWT_GRID_FLAG_GROWX);
    newtGridSetField(grid, 0, 1, NEWT_GRID_COMPONENT, label,
                     1, 1, 0, 0, 0, NEWT_GRID_FLAG_GROWX | NEWT_GRID_FLAG_GROWY);
    newtGridSetField(grid, 0, 2, NEWT_GRID_SUBGRID, buttonGrid,
                     0, 1, 0, 0, 0, NEWT_GRID_FLAG_GROWX);
    newtGridWrappedWindow(grid, dtitle);

    newtFormAddComponents(form, progress, label, b1,  nullptr);
    newtFormWatchFd(form, cmd.pipe_stream.pipe().native_source(), NEWT_FD_READ);
    newtScaleSet(progress, 0);
    newtDrawForm(form);

    newtExitStruct es = {};
    newtFormRun(form, &es);
    while (es.reason == 2) {

        auto last_line = cmd.getline();
        if (!last_line)
            break;

        newtTextboxSetText(label, last_line->c_str());
        newtScaleSet(progress, unsigned(fPercent(*last_line) * 10));

        newtFormRun(form, &es);
    }

    newtFormDestroy(form);
    return es.u.co != b1;

}

void Newt::helpMessage (const char* message) {
    newtBell();
    newtWinMessage(const_cast<char*>(TUIText::Help::title),
                   const_cast<char*>(TUIText::Buttons::ok),
                   const_cast<char*>(message));
}
