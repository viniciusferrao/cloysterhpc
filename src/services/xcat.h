/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_XCAT_H_
#define CLOYSTERHPC_XCAT_H_

#include "provisioner.h"
#include "execution.h"
#include "shell.h"
#include "../const.h"
#include "../services/log.h"

#include <magic_enum.hpp>

#include <string>
#include <memory>
#include <filesystem>

class XCAT : public Provisioner {
private:
    enum class ImageType { Install, Netboot };
    enum class NodeType { Compute, Service };

private:
    const std::unique_ptr<Cluster> &m_cluster;

    struct {
        std::vector<std::string_view> otherpkgs = {};
        std::string osimage;
        std::filesystem::path chroot;
        std::vector<std::string> postinstall = { "#!/bin/sh\n\n" };
        std::vector<std::string> synclists = {};
    } m_stateless;

private:
    void setDHCPInterfaces(std::string_view interface);
    void setDomain(std::string_view domain);
    void copycds(const std::filesystem::path& diskImagePath);
    void genimage();
    void packimage();
    void nodeset(std::string_view nodes);
    void createDirectoryTree();
    void configureOpenHPC();
    void configureTimeService();
    void configureInfiniband();
    void configureSLURM();
    void generateOtherPkgListFile();
    void generatePostinstallFile();
    void generateSynclistsFile();
    void configureOSImageDefinition();
    void customizeImage();
    void addNode(const Node& node);
    void generateOSImageName(ImageType, NodeType);
    void generateOSImagePath(ImageType, NodeType);


public:
    void configureRepositories();
    void installPackages();
    void setup();

    void createImage(ImageType = ImageType::Netboot,
                     NodeType = NodeType::Compute);
    void addNodes();
    void setNodesImage();
    void setNodesBoot();
    void resetNodes();

    explicit XCAT(const std::unique_ptr<Cluster>& cluster);
};

#endif // CLOYSTERHPC_XCAT_H_
