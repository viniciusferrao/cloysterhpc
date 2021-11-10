#ifndef XCAT_H
#define XCAT_H

#include "provisioner.h"
#include "execution.h"
#include "shell.h"

#include <string>
#include <memory>

class XCAT : public Provisioner {
private:
    Shell& m_executionEngine;

private:
    void copycds (std::string_view);
    void genimage (std::string_view);
    void packimage (std::string_view);
    void createDirectoryTree ();
    void generateOtherPkgListFile ();
    void generatePostinstallFile ();
    void generateSynclistsFile ();
    void configureOSImageDefinition (std::string_view);
    void customizeImage ();

public:
    XCAT(Shell&);
    ~XCAT();

    void configureRepositories ();
    void installPackages ();
    void setDHCPInterfaces (std::string_view);
    void setDomain (std::string_view);
    void createImage (std::string_view);

    void addOpenHPCComponents (std::string_view);
};

#endif /* XCAT_H */