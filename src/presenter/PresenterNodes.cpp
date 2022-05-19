//
// Created by Vinícius Ferrão on 02/03/22.
//

#include "PresenterNodes.h"

PresenterNodes::PresenterNodes(
        std::unique_ptr<Cluster>& model,
        std::unique_ptr<Newt>& view)
        : Presenter(model, view) {

    m_view->message(Messages::title, Messages::message);

    // Node formation questions
    // TODO: Placeholder data
    auto fields = std::to_array<
            std::pair<std::string, std::string>>({
                 {Messages::Nodes::prefix, "n"},
                 {Messages::Nodes::padding, "2"},
                 {Messages::Nodes::startIP, "172.31.22.45"},
                 {Messages::Nodes::rootPassword, "p@ssw0rd"},
                 {Messages::Nodes::ISOPath, "/root/iso/rhel-8.5-dvd1.iso"}
            });

    retry:
    fields = m_view->fieldMenu(Messages::title, Messages::Nodes::question, fields,
                               Messages::Nodes::help);

    for (const auto& field : fields) {
        if (field.first == Messages::Nodes::prefix) {
            if (std::isalpha(field.second[0] == false)) {
                m_view->message(Messages::Error::prefixLetter);
                goto retry;
            }
        }

        if (field.first == Messages::Nodes::padding) {
            if (boost::lexical_cast<std::size_t>(field.second) > 3) {
                m_view->message(Messages::Error::paddingMax);
                goto retry;
            }
        }
    }

    // TODO: Encapsulate
    std::size_t i {0};
    m_model->nodePrefix = fields[i++].second;
    m_model->nodePadding = boost::lexical_cast<std::size_t>(fields[i++].second);
    m_model->nodeStartIP = fields[i++].second;
    m_model->nodeRootPassword = fields[i++].second;
    m_model->setISOPath(fields[i++].second);

    // Number of nodes
    // TODO: Set racks and startNumber to create things like: r01n01
    auto nodes = std::to_array<
            std::pair<std::string, std::string>>({
            //     {Messages::Quantity::racks, "2"},
                 {Messages::Quantity::nodes, "5"},
            //     {Messages::Quantity::startNumber, "7"}
            });

    i = 0;
    nodes = m_view->fieldMenu(Messages::title,
                              Messages::Quantity::question,
                              nodes,
                              Messages::Quantity::help);

    m_model->nodeQuantity = boost::lexical_cast<std::size_t>(nodes[i++].second);

    // FIXME: This code is just a mess
    //  * Hardcoded data
    //  * No proper verification/passing of parameters
    //  * Nodes should be encapsulated
    //  * OS and CPU are copied constantly (that should be references)
    //  * We need a proper node building object instead
    // TODO: Support more OSes than Oracle Linux 8.5; we are hard coding the OS:
    OS nodeOS(OS::Arch::x86_64, OS::Family::Linux, OS::Platform::el8,
              OS::Distro::OL, "5.4.17-2136.302.6.1.el8uek.x86_64", 8, 5);
    // TODO: Fetch CPU data
    CPU nodeCPU(1, 1, 1);

    for (std::size_t node{1} ; node <= m_model->nodeQuantity ; ++node) {
        std::list<Connection> nodeConnections;
        auto& connection = nodeConnections.emplace_back(&m_model->getNetwork(Network::Profile::Management));
        // FIXME: Get/fetch real MAC Addresses
        connection.setMAC(fmt::format("00:00:00:00:00:{:2}", node));
        // FIXME: Properly generate and set only once (without increment)
        connection.setAddress(m_model->nodeStartIP);
        connection.incrementAddress(node - 1);

        auto nodeName = fmt::format("{}{:0>{}}", m_model->nodePrefix, node, m_model->nodePadding);

        m_model->addNode(
                nodeName,
                nodeOS,
                nodeCPU,
                std::move(nodeConnections)
                // std::optional<BMC>
        );
    }
}
