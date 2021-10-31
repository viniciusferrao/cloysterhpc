//
// Created by Vinícius Ferrão on 31/10/21.
//

#include "shell.h"

#ifdef _DEBUG_
#include <iostream>
#endif
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/process.hpp>

void Shell::runCommand(const std::string& command) {
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
