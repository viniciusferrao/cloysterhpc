#ifndef XCAT_H
#define XCAT_H

#include "provisioner.h"
#include "execution.h"
#include "shell.h"
#include "../const.h"
#include "../include/magic_enum.hpp"
#include "../services/log.h"

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
    void copycds(const std::filesystem::path& isopath);
    void genimage();
    void packimage();
    void nodeset(std::string_view nodes);
    void createDirectoryTree();
    void configureOpenHPC();
    void configureTimeService();
    void configureSLURM();
    void generateOtherPkgListFile();
    void generatePostinstallFile();
    void generateSynclistsFile();
    void configureOSImageDefinition();
    void customizeImage();
    void addNode(std::string_view, std::string_view, std::string_view,
                 std::string_view, const std::optional<BMC>& bmc);
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

    explicit XCAT(const std::unique_ptr<Cluster>& cluster);
};

#endif /* XCAT_H */
