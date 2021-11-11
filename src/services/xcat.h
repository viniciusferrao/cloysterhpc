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
    void configureRepositories ();
    void installPackages ();
    void setDHCPInterfaces (std::string_view);
    void setDomain (std::string_view);
    void createImage (std::string_view);

    void addOpenHPCComponents (std::string_view);
};

#endif /* XCAT_H */