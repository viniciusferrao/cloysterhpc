/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

// This file is empty since the method is now a template

#include <cloysterhpc/view/newt.h>
#include <newt.h>
#include <ranges>

static std::vector<std::string> retrieveListSelectedItems(newtComponent list)
{
    int selectCount = 0;
    char** selectItems
        = reinterpret_cast<char**>(newtListboxGetSelection(list, &selectCount));
    auto temp = std::vector(selectItems, selectItems + selectCount)
        | std::views::transform([](char* c) { return std::string { c }; });
    std::vector<std::string> ret(std::begin(temp), std::end(temp));
    return ret;
}

std::pair<int, std::vector<std::string>> Newt::multipleSelectionMenu(
    const char* title, const char* message, const char* help,
    std::vector<std::tuple<std::string, std::string, bool>> items)
{
    auto* form = newtForm(nullptr, nullptr, 0);
    newtFormSetWidth(form, m_suggestedWidth);

    auto* label = newtLabel(1, 1, message);

    auto* list = newtListbox(1, 3, m_maxListHeight, NEWT_FLAG_MULTIPLE);

    for (const auto& [key, item, enabled] : items) {
        newtListboxAppendEntry(list, item.c_str(), (void*)key.c_str());

        if (enabled)
            newtListboxSelectItem(list, key.c_str(), NEWT_FLAGS_SET);
    }

    newtGrid grid = newtCreateGrid(1, 3);
    newtComponent buttonOk, buttonCancel, buttonHelp;
    newtGrid buttonGrid = newtButtonBar(const_cast<char*>(TUIText::Buttons::ok),
        &buttonOk, const_cast<char*>(TUIText::Buttons::cancel), &buttonCancel,
        const_cast<char*>(TUIText::Buttons::help), &buttonHelp, NULL);
    newtGridSetField(grid, 0, 0, NEWT_GRID_COMPONENT, label, 1, 1, 0, 0, 0,
        NEWT_GRID_FLAG_GROWX);
    newtGridSetField(grid, 0, 1, NEWT_GRID_COMPONENT, list, 1, 1, 0, 0, 0,
        NEWT_GRID_FLAG_GROWX | NEWT_GRID_FLAG_GROWY);
    newtGridSetField(grid, 0, 2, NEWT_GRID_SUBGRID, buttonGrid, 0, 1, 0, 0, 0,
        NEWT_GRID_FLAG_GROWX);
    newtGridWrappedWindow(grid, const_cast<char*>(title));

    newtFormAddComponents(
        form, list, label, buttonOk, buttonCancel, buttonHelp, nullptr);

    newtExitStruct es = {};
    newtFormRun(form, &es);

    int retval = 0;

    while (retval == 0) {
        while (es.reason == 2) {
            newtFormRun(form, &es);
        }

        if (es.u.co == buttonOk) {
            retval = 1;
        } else if (es.u.co == buttonHelp) {
            this->helpMessage(help);
        } else {
            retval = 2;
        }
    }

    auto ret = retrieveListSelectedItems(list);
    newtFormDestroy(form);
    newtRefresh();

    return std::make_pair(retval, ret);
}
