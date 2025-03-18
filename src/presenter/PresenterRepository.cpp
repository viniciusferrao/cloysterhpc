/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cloysterhpc/functions.h>
#include <cloysterhpc/presenter/PresenterRepository.h>
#include <ranges>

namespace cloyster::presenter {

PresenterRepository::PresenterRepository(
    std::unique_ptr<Cluster>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    auto manager = cloyster::Singleton<services::repos::RepoManager>::get();

    manager->initializeDefaultRepositories();

    const auto allRepos = manager->listRepos();
    using UISelectionAdapterTy
        = std::vector<std::tuple<std::string, std::string, bool>>;
    auto allReposUIAdapter = allRepos | std::views::transform([](auto& entry) {
        return std::make_tuple(std::string { entry->id() },
            std::string { entry->name() }, entry->enabled());
    }) | std::ranges::to<UISelectionAdapterTy>();

    const auto& [ret, toEnable] = m_view->multipleSelectionMenu(Messages::title,
        Messages::General::question, Messages::General::help,
        allReposUIAdapter);

    LOG_DEBUG("{} repos selected", toEnable.size());

    if (ret == 1) {
        auto toDisable = allReposUIAdapter
            | std::views::filter([&toEnable](auto& tuple) {
                  auto& [id, _name, _state] = tuple;
                  return !cloyster::utils::isIn(toEnable, id);
              })
            | std::views::transform([](auto& tuple) {
                  auto& [id, _name, _state] = tuple;
                  return id;
              })
            | std::ranges::to<std::vector<std::string>>();

        manager->disable(toDisable);
        manager->enable(toEnable);
    }
}

}
