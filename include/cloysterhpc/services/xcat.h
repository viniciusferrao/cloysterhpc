/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_XCAT_H_
#define CLOYSTERHPC_XCAT_H_

#include <filesystem>
#include <string>

#include <fmt/format.h>
#include <fmt/ranges.h> // for std::vector formatters

#include <cloysterhpc/const.h>
#include <cloysterhpc/services/execution.h>
#include <cloysterhpc/services/log.h>
#include <cloysterhpc/services/provisioner.h>
#include <cloysterhpc/services/shell.h>


namespace cloyster::services {

/**
 * @class XCAT
 * @brief Manages the provisioning of nodes using the xCAT tool.
 *
 * This class provides functionalities for setting up and managing the
 * provisioning process of compute and service nodes in a cluster using xCAT.
 */
class XCAT : public Provisioner {
public:
    struct Image {
        std::vector<std::string_view> otherpkgs = {};
        // @TODO: We need to support more than one osimage (:
        //   this can be a default osimage though
        std::string osimage;
        std::filesystem::path chroot;
        std::vector<std::string> postinstall = { "#!/bin/sh\n\n" };
        std::vector<std::string> synclists;
    };

private:
    Image m_stateless;

    /**
     * @enum ImageType
     * @brief Defines the types of OS images.
     *
     * This enum specifies the types of OS images that can be created.
     */
    enum class ImageType { Install, Netboot };

    /**
     * @enum NodeType
     * @brief Defines the types of nodes.
     *
     * This enum specifies the types of nodes in the cluster.
     */
    enum class NodeType { Compute, Service };


    static void setDHCPInterfaces(std::string_view interface);
    static void setDomain(std::string_view domain);

    /**
     * @brief Copies installation media to the disk image.
     *
     * @param diskImage The path to the disk image.
     */
    void copycds(const std::filesystem::path& diskImage) const;

    /**
     * @brief Generates the OS image.
     *
     * This function creates the OS image based on the configuration.
     */
    void genimage();

    /**
     * @brief Packs the OS image.
     *
     * This function packages the OS image for deployment.
     */
    void packimage();

    /**
     * @brief Sets the nodes for a specific image.
     *
     * @param nodes The nodes to set for the image.
     */
    void nodeset(std::string_view nodes);

    /**
     * @brief Creates the necessary directory tree.
     *
     * This function sets up the directory structure required for provisioning.
     */
    static void createDirectoryTree();

    /**
     * @brief Configures OpenHPC settings.
     *
     * This function sets up OpenHPC configurations.
     */
    void configureOpenHPC();

    /**
     * @brief Configures the time service.
     *
     * This function sets up the time synchronization service.
     */
    void configureTimeService();

    /**
     * @brief Configures SLURM settings.
     *
     * This function sets up SLURM for job scheduling and management.
     */
    void configureSLURM();

    /**
     * @brief Generates the file listing other packages.
     *
     * This function creates a file that lists additional packages to install.
     */
    void generateOtherPkgListFile();

    /**
     * @brief Generates the post-installation script file.
     *
     * This function creates the post-installation script file.
     */
    void generatePostinstallFile();

    /**
     * @brief Generates the synchronization list file.
     *
     * This function creates the synchronization list file.
     */
    static void generateSynclistsFile();

    /**
     * @brief Configures the OS image definition.
     *
     * This function sets up the OS image definition in xCAT.
     */
    void configureOSImageDefinition();

    /**
     * @brief Customizes the OS image.
     *
     * This function applies customizations to the OS image.
     */
    void customizeImage();

    /**
     * @brief Adds a node to the cluster.
     *
     * @param node The node to add.
     */
    static void addNode(const cloyster::models::Node& node);

    /**
     * @brief Generates the OS image name based on type and node.
     *
     * @param imageType The type of image (Install or Netboot).
     * @param nodeType The type of node (Compute or Service).
     */
    void generateOSImageName(ImageType, NodeType);

    /**
     * @brief Generates the OS image path based on type and node.
     *
     * @param imageType The type of image (Install or Netboot).
     * @param nodeType The type of node (Compute or Service).
     */
    void generateOSImagePath(ImageType, NodeType);

    /**
     * @brief Configures settings specific to Enterprise Linux 9 (EL9).
     *
     * This function sets up configurations for EL9.
     */
    static void configureEL9();

public:
    XCAT();


    /**
     * @brief Download the repositories
     */
    void installRepositories();

    /**
     * @brief Return a list of repos for xCAT image
     */
    [[nodiscard]] std::vector<std::string> getxCATOSImageRepos() const;

    /**
     * @brief Installs the necessary packages.
     *
     * This function installs all required packages for the provisioning
     * process.
     */
    void installPackages();

    /**
     * @brief Patches xCAT to resolve bugs that aren't addressed upstream.
     *
     * This function applies patches to xCAT to fix issues during installation
     * on recent versions of the OS.
     */
    static void patchInstall();

    /**
     * @brief Sets up the provisioning environment.
     *
     * This function performs the initial setup for provisioning nodes.
     */
    void setup();

    /**
     * @brief Creates an OS image.
     *
     * This function creates an OS image for either Netboot or Install, and for
     * either Compute or Service nodes.
     *
     * @param imageType The type of image to create (default is Netboot).
     * @param nodeType The type of node to create the image for (default is
     * Compute).
     */
    void createImage(
        ImageType = ImageType::Netboot, NodeType = NodeType::Compute);

    /**
     * @brief Adds nodes to the provisioning system.
     *
     * This function registers new nodes with the provisioning system.
     */
    void addNodes();

    /**
     * @brief Sets the OS image for nodes.
     *
     * This function assigns the created OS image to the nodes.
     */
    void setNodesImage();

    /**
     * @brief Sets the boot configuration for nodes.
     *
     * This function configures the nodes to boot using the assigned OS image.
     */
    static void setNodesBoot();

    /**
     * @brief Resets the nodes.
     *
     * This function resets the nodes.
     */
    static void resetNodes();

    /**
     * @brief Configures InfiniBand settings.
     *
     * This function sets up InfiniBand interconnect settings.
     */
    void configureInfiniband();

    /**
    * @brief Return the Image
    */
    [[nodiscard]] Image getImage() const;
};

};

template <>
struct fmt::formatter<cloyster::services::XCAT::Image> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const cloyster::services::XCAT::Image& image, FormatContext& ctx) const
    -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "XCAT::Image({}, {})", 
                              image.osimage, image.otherpkgs);
    }
};

#endif // CLOYSTERHPC_XCAT_H_
