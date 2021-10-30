#include "headnode.h"

#include <iostream>
#include <fstream>
#include <algorithm> /* std::remove */
#include <map> /* std::map */

#include <sys/utsname.h>

#if __cplusplus < 202002L
#include <boost/algorithm/string.hpp>
#endif

/* We should refactor to boost::property_tree on both methods: fetchValue() and
 * setOS(); an those methods should really be on OS class and not here.
 */
std::string Headnode::fetchValue (const std::string& line) {
    std::string value;

    /* Get values from keys */
    size_t pos = line.find_first_of("=");
    value = line.substr(pos + 1);

    /* Remove double quotes (") if found */
    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());

    return value;    
}

/* TODO: Better error return codes */
int Headnode::setOS () {
    struct utsname system;

    uname(&system);

    if (std::string{system.machine} != "x86_64") {
        return -1;
    }

    this->os.arch = OS::Arch::x86_64;

    /* A map would be a better ideia:
     * std::map<std::string, Family> osFamily
     */
    if (std::string{system.sysname} == "Linux")
        this->os.family = OS::Family::Linux;
    if (std::string{system.sysname} == "Darwin")
        this->os.family = OS::Family::Darwin;

    /* Store kernel release in string format */
    this->os.kernel = std::string{system.release};

#ifdef _DEBUG_
    std::cout << "Architecture: " << (int)this->os.arch << std::endl;
    std::cout << "Family: " << (int)this->os.family << std::endl;
    std::cout << "Kernel Release: " << this->os.kernel << std::endl;
#endif

#ifdef _DUMMY_
    if (std::string{system.sysname} == "Darwin") {
        std::string filename = "chroot/etc/os-release";
#else
    if (std::string{system.sysname} == "Linux") {
        std::string filename = "/etc/os-release";
#endif
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

#if __cplusplus >= 202002L
            if (line.starts_with("PLATFORM_ID=")) {
#else
            if (boost::algorithm::starts_with(line, "PLATFORM_ID=")) {
#endif

                std::string parser = fetchValue(line);
                if (parser.substr(parser.find(":") + 1) == "el8")
                    this->os.platform = OS::Platform::el8;
            }

#if __cplusplus >= 202002L
            if (line.starts_with("ID=")) {
#else
            if (boost::algorithm::starts_with(line, "ID=")) {
#endif

                if (fetchValue(line) == "rhel")
                    this->os.distro = OS::Distro::RHEL;
                if (fetchValue(line) == "ol")
                    this->os.distro = OS::Distro::OL;
            }

#if __cplusplus >= 202002L
            if (line.starts_with("VERSION=")) {
#else
            if (boost::algorithm::starts_with(line, "VERSION=")) {
#endif

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
    std::cout << "Platform: " << (int)this->os.platform << std::endl;
    std::cout << "Distribution: " << (int)this->os.distro << std::endl;
    std::cout << "Major Version: " << this->os.majorVersion << std::endl;
    std::cout << "Minor Version: " << this->os.minorVersion << std::endl;
#endif

    return 0;
}

void Headnode::printOS () {
    std::cout << "Architecture: " << (int)this->os.arch << std::endl;
    std::cout << "Family: " << (int)this->os.family << std::endl;
    std::cout << "Kernel Release: " << this->os.kernel << std::endl;
    std::cout << "Platform: " << (int)this->os.platform << std::endl;
    std::cout << "Distribution: " << (int)this->os.distro << std::endl;
    std::cout << "Major Version: " << this->os.majorVersion << std::endl;
    std::cout << "Minor Version: " << this->os.minorVersion << std::endl;
}

/* Supported releases must be a constexpr defined elsewhere and not hardcoded as
 * a value on the code.
 * Return values should be errorcodes that match the failure and not being
 * written directly on the method. Also, they might need to be a bitmap or enum
 */
int Headnode::checkSupportedOS () {
    if (this->os.arch != OS::Arch::x86_64) {
#ifdef _DEBUG_
        std::cout << (int)this->os.arch 
            << " is not a supported architecture" << std::endl;
#endif
        return -1;
    }

    if (this->os.family != OS::Family::Linux) {
#ifdef _DEBUG_
        std::cout << (int)this->os.family 
            << " is not a supported operating system" << std::endl;
#endif
        return -2;
    }

    if (this->os.platform != OS::Platform::el8) {
#ifdef _DEBUG_
        std::cout << (int)this->os.platform 
            << " is not a supported Linux platform" << std::endl;
#endif
        return -3;
    }

    if ((this->os.distro != OS::Distro::RHEL) && 
        (this->os.distro != OS::Distro::OL)) {
#ifdef _DEBUG_
        std::cout << (int)this->os.distro 
            << " is not a supported Linux distribution" << std::endl;
#endif
        return -4;
    }

    if (this->os.majorVersion < 8) {
#ifdef _DEBUG_
        std::cout << "This software is only supported on EL8" << std::endl;
#endif
        return -5;
    }

    return 0;
}
