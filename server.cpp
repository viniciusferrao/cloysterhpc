#include "server.hpp"

#include <iostream>
#include <fstream>

#include <sys/utsname.h>

/* We should refactor to boost::property_tree on both methods: fetchValue() and
 * setOS().
 */
std::string Server::fetchValue (std::string line) {
    std::string value;

    /* Get values from keys */
    size_t pos = line.find_first_of("=");
    value = line.substr(pos + 1);

    /* Remove double quotes (") if found */
    value.erase(remove(value.begin(), value.end(), '"'), value.end());

    return value;    
}

int Server::setOS (void) {
    struct utsname system;

    uname(&system);

    if (std::string{system.machine} != "x86_64") {
        return -1;
    }

    this->arch = X86_64;
    this->os.family = std::string{system.sysname};
    this->os.kernel = std::string{system.release};

#ifdef _DEBUG_
    std::cout << "Architecture: " << this->arch << std::endl;
    std::cout << "Family: " << this->os.family << std::endl;
    std::cout << "Kernel Release: " << this->os.kernel << std::endl;
#endif

    if (std::string{system.sysname} == "Darwin") {
    //if (std::string{system.sysname} == "Linux") {
        std::string filename = "chroot/etc/os-release";
        //std::string filename = "/etc/os-release";
        std::ifstream file(filename);

        if (!file.is_open()) {
            perror(("Error while opening file " + filename).c_str());
            return -2;
        }

        /* Fetches OS information from /etc/os-release. The file is writen in a
         * key=value style.
         */
        std::string line;
        while (getline(file, line)) {
            if (line.starts_with("PLATFORM_ID=")){
                std::string parser = fetchValue(line);
                this->os.platform = parser.substr(parser.find(":") + 1);
            }

            if (line.starts_with("ID=")) {
                this->os.id = fetchValue(line);
            }

            if (line.starts_with("VERSION=")) {
                std::string parser = fetchValue(line);

                this->os.majorVersion = stoi(
                    parser.substr(0, parser.find(".")));
                
                this->os.minorVersion = stoi(
                    parser.substr(parser.find(".") + 1));
            }
        }

        if (file.bad()) {
            perror(("Error while reading file " + filename).c_str());
            return -3;
        }

        file.close();
    }

#ifdef _DEBUG_
    std::cout << "Platform: " << this->os.platform << std::endl;
    std::cout << "Distribution: " << this->os.id << std::endl;
    std::cout << "Major Version: " << this->os.majorVersion << std::endl;
    std::cout << "Minor Version: " << this->os.minorVersion << std::endl;
#endif

    return 0;
}

/* Supported releases must be a constexpr defined elsewhere and not hardcoded as
 * a value on the code.
 * Return values should be errorcodes that match the failure and not being
 * written directly on the method. Also they need to be a bitmap.
 */
int Server::checkSupportedOS (void) {
    if (this->arch != X86_64) {
        std::cout << this->arch 
            << " is not a supported architecture" << std::endl;
        return -1;
    }

    if (this->os.family != "Linux") {
        std::cout << this->os.family 
            << " is not a supported operating system" << std::endl;
        return -2;
    }

    if (this->os.platform != "el8") {
        std::cout << this->os.platform 
            << " is not a supported Linux platform" << std::endl;
        return -3;
    }

    if ((this->os.id != "rhel") && (this->os.id != "ol")) {
        std::cout << this->os.id 
            << " is not a supported Linux distribution" << std::endl;
        return -4;
    }

    if (this->os.majorVersion < 8) {
        std::cout << "This software is only supported on EL8" << std::endl;
        return -5;
    }

    return 0;
}
