#ifndef XCAT_H
#define XCAT_H

#ifdef __JETBRAINS_IDE__
#define _DEBUG_
#endif

#include "provisioner.h"
#include "execution.h"
#include "shell.h"

#include <string>
#include <memory>

class XCAT : public Provisioner {
private:
    enum class ImageType { Install, Netboot };
    enum class NodeType { Compute, Service };

private:
    struct {
        std::vector<std::string_view> otherpkgs = {};
        /* TODO: Discover chroot path */
        std::string osimage;
        const std::string chroot = "/install/netboot/ol8.4.0/x86_64/compute/rootimg";
        std::vector<std::string> postinstall = { "#!/bin/sh\n\n" };
        std::vector<std::string> synclists = {};
    } m_stateless;

private:
    void setDHCPInterfaces (std::string_view);
    void setDomain (std::string_view);
    void copycds (std::string_view);
    void genimage ();
    void packimage ();
    void nodeset();
    void createDirectoryTree ();
    void configureOpenHPC();
    void configureTimeService(const std::unique_ptr<Cluster>& cluster);
    void configureSLURM (const std::unique_ptr<Cluster>& cluster);
    void generateOtherPkgListFile ();
    void generatePostinstallFile (const std::unique_ptr<Cluster>&);
    void generateSynclistsFile ();
    void configureOSImageDefinition (const std::unique_ptr<Cluster>& cluster);
    void customizeImage ();
    void addNode(std::string_view, std::string_view, std::string_view,
                 std::string_view, std::string_view, std::string_view,
                 std::string_view);
    void generateOSImageName(const std::unique_ptr<Cluster>&,
                             ImageType, NodeType);

public:
    void configureRepositories ();
    void installPackages ();
    void setup(const std::unique_ptr<Cluster>&);

    void createImage (const std::unique_ptr<Cluster>&, std::string_view,
                      ImageType = ImageType::Netboot,
                      NodeType = NodeType::Compute);
    void addNodes(const std::unique_ptr<Cluster>&);
    void setNodesImage();
};

#endif /* XCAT_H */