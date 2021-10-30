#include "functions.h"

#include <cstdlib> /* getenv() */
#include <iostream>

#define ENABLE_GETOPT_H
#ifdef ENABLE_GETOPT_H
#include <getopt.h>
#endif

/* This is when we just don't care anymore */
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/process.hpp>

/* getopt code:
 * Return optind to manipulate argc and argv outside function.
 * If GETOPT_H is not available will always return 0
 * 
 * Important things to note:
 * If an option was successfully found, then getopt() returns the option 
 * character. If all command-line options have been parsed, then getopt()
 * returns -1. If getopt() encounters an option character that was not in 
 * optstring, then '?' is returned. If getopt() encounters an option with a
 * missing argument, then the return value depends on the first character in
 * optstring: if it is ':', then ':' is returned; otherwise '?' is returned.
 * https://www.man7.org/linux/man-pages/man3/getopt.3.html
 * 
 * Finally, we should consider reimplementing this with boost:
 * boost::program_options
 */
int parseArguments(int argc, char **argv) {
    int ch;
    const char *shortOptions = ":a:hv";

    while (true) {
#ifdef ENABLE_GETOPT_H
        int optionIndex = 0;
        static struct option const longOptions[] = {
            {"help", no_argument, nullptr, 'h'},
            {"version", no_argument, nullptr, 'v'},
            {nullptr, 0, nullptr, 0}
        };

        ch = getopt_long(argc, argv, shortOptions, longOptions, &optionIndex);
#else
        ch = getopt(argc, argv, shortOptions);
#endif
        if (ch == -1)
            break;

        switch (ch) {
            case 'a':
                printf("Argumento passado: %s\n", optarg);
                break;
            case 'h':
                printf("HELP PLACEHOLDER\n");
                break;
            case 'v':
                printf("Development Version\n");
                break;
            case ':':
                printf("Missing argument for option -%c\n", optopt);
                exit(EXIT_FAILURE);
                break;
            case 0:
            case '?':
                printf("Unknown option -%c\n", optopt);
                exit(EXIT_FAILURE);
                break;
            default:
                printf("Usage: %s [OPTION]\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* I have no idea why this is needed */
    /*argc -= optind;
    argv += optind;
    */

#ifdef ENABLE_GETOPT_H
    return optind;
#else
    return 0;
#endif
}

/* Returns a specific environment varible when requested.
 * If the variable is not set it will return as an empty string. That's by 
 * design and not considered a bug right now.
 */
std::string getEnvironmentVariable (const std::string& key) {
    char *value = getenv(key.c_str());
    return value == nullptr ? std::string("") : std::string(value);
}

/* Read .conf file */
std::string readConfig (const std::string& filename) {
    boost::property_tree::ptree tree;

    try {
        boost::property_tree::ini_parser::read_ini(filename, tree);
    } 
    
    catch(boost::property_tree::ini_parser_error) {
#ifdef _DEBUG_
        std::cerr << filename << " not found" << std::endl;
#endif
    }

    std::string value = tree.get<std::string>("headnode.LANG",
                                                    "en_US.utf8");
#ifdef _DEBUG_
    std::cout << "Read configFile variables:" << std::endl;
    std::cout << "LANG: " << value << std::endl;
#endif

    return value;
}

/* Write .conf file function */
void writeConfig (const std::string& filename) {
    boost::property_tree::ptree tree;

    tree.put("headnode.LANG", getEnvironmentVariable("LANG"));

    boost::property_tree::write_ini(filename, tree);
}

/* Execution engine code */
void runCommand (const std::string& command) {
#ifndef _DUMMY_
    boost::process::ipstream pipe_stream;
    boost::process::child c(command, boost::process::std_out > pipe_stream);

    std::string line;

    while (pipe_stream && std::getline(pipe_stream, line) && !line.empty())
#ifdef _DEBUG_
        std::cerr << line << std::endl;
#endif

    c.wait();
#else
    std::cout << "exec: " << command << std::endl;
#endif
}
