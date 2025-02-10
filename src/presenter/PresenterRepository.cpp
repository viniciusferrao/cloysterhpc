/*
 * Copyright 2022 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <cloysterhpc/presenter/PresenterRepository.h>

PresenterRepository::PresenterRepository(
    std::unique_ptr<Cluster<BaseRunner>>& model, std::unique_ptr<Newt>& view)
    : Presenter(model, view)
{
    auto manager = m_model->getRepoManager();

    manager.loadFiles();

    auto repo_names = manager.listRepos();
    std::vector<std::tuple<std::string, std::string, bool>> repo_strings;

    std::transform(repo_names.begin(), repo_names.end(),
        std::back_inserter(repo_strings), [](auto entry) {
            return std::make_tuple(std::string { entry.id },
                std::string { entry.name }, entry.enabled);
        });

    const auto& [ret, selectedRepositories]
        = m_view->multipleSelectionMenu(Messages::title,
            Messages::General::question, Messages::General::help, repo_strings);

    LOG_DEBUG("{} repos selected", selectedRepositories.size());

    if (ret == 1) {
        for (const auto& [id, _name, _state] : repo_strings) {
            manager.disable(id);
        }

        for (const auto& repo : selectedRepositories) {
            manager.enable(repo);
        }
    }
}
