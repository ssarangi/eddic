//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <iostream>
#include <memory>
#include <vector>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "assert.hpp"
#include "Options.hpp"

using namespace eddic;

namespace po = boost::program_options;

bool eddic::WarningUnused;
bool eddic::WarningCast;
int eddic::OLevel = 2;

std::shared_ptr<po::variables_map> options;
std::vector<std::pair<std::string, std::vector<std::string>>> triggers;

bool desc_init = false;
po::options_description desc("Usage : eddic [options] source.eddi");

std::pair<std::string, std::string> numeric_parser(const std::string& s){
    if (s.find("-32") == 0) {
        return make_pair("32", std::string("true"));
    } else if (s.find("-64") == 0) {
        return make_pair("64", std::string("true"));
    } else {
        return make_pair(std::string(), std::string());
    }
}

void add_trigger(const std::string& option, std::vector<std::string> childs){
   triggers.push_back(std::make_pair(option, childs)); 
}

bool eddic::parseOptions(int argc, const char* argv[]) {
    try {
        //Only if the description has not been already defined
        if(!desc_init){
            desc.add_options()
                ("help,h", "Generate this help message")
                ("assembly,S", "Generate only the assembly")
                ("keep,k", "Keep the assembly file")
                ("version", "Print the version of eddic")
                ("output,o", po::value<std::string>()->default_value("a.out"), "Set the name of the executable")
                
                ("debug,g", "Add debugging symbols")
                
                ("quiet,q", "Do not print anything")
                ("verbose,v", "Make the compiler verbose")
                ("dev,d", "Activate development mode (very verbose)")
                ("perfs", "Display performance information")

                ("O0", "Disable all optimizations")
                ("O1", "Enable low-level optimizations")
                ("O2", "Enable all optimizations. This can be slow for big programs.")
                ("fno-inline-functions", "Disable inlining")
                
                ("32", "Force the compilation for 32 bits platform")
                ("64", "Force the compilation for 64 bits platform")

                ("warning-all", "Enable all the warning messages")
                ("warning-unused", po::bool_switch(&WarningUnused), "Warn about unused variables, parameters and functions")
                ("warning-cast", po::bool_switch(&WarningCast), "Warn about useless casts")

                ("ast", "Print the Abstract Syntax Tree representation of the source")
                ("ast-only", "Only print the Abstract Syntax Tree representation of the source (do not continue compilation after printing)")

                ("mtac", "Print the medium-level Three Address Code representation of the source")
                ("mtac-opt", "Print the medium-level Three Address Code representation of the source before any optimization has been performed")
                ("mtac-only", "Only print the medium-level Three Address Code representation of the source (do not continue compilation after printing)")

                ("ltac", "Print the low-level Three Address Code representation of the source")
                ("ltac-only", "Only print the low-level Three Address Code representation of the source (do not continue compilation after printing)")
               
                ("input", po::value<std::string>(), "Input file");

            add_trigger("warning-all", {"warning-unused", "warning-cast"});
            
            desc_init = true;
        }

        //Add the option of the input file
        po::positional_options_description p;
        p.add("input", -1);

        //Create a new set of options
        options = std::make_shared<po::variables_map>();

        //Parse the command line options
        po::store(po::command_line_parser(argc, argv).options(desc).extra_parser(numeric_parser).positional(p).run(), *options);
        po::notify(*options);

        //Triggers dependent options
        for(auto& trigger : triggers){
            if(option_defined(trigger.first)){
                for(auto& child : trigger.second){
                    boost::any test = std::string("true");
                    const_cast<boost::program_options::variable_value&>((*options)[child]).value() = test;
                }
            }
        }

        if(options->count("O0") + options->count("O1") + options->count("O2") > 1){
            std::cout << "Invalid command line options : only one optimization level should be set" << std::endl;

            return false;
        }

        if(options->count("64") && options->count("32")){
            std::cout << "Invalid command line options : a compilation cannot be both 32 and 64 bits" << std::endl;

            return false;
        }

        if(options->count("O0")){
            OLevel = 0;
        } else if(options->count("O1")){
            OLevel = 1;
        } else if(options->count("O2")){
            OLevel = 2;
        } 

        if(options->count("warning-all")){
            WarningUnused = true;
            WarningCast = true;
        }
    } catch (const po::ambiguous_option& e) {
        std::cout << "Invalid command line options : " << e.what() << std::endl;

        return false;
    } catch (const po::unknown_option& e) {
        std::cout << "Invalid command line options : " << e.what() << std::endl;

        return false;
    } catch (const po::multiple_occurrences& e) {
        std::cout << "Only one file can be compiled" << std::endl;

        return false;
    }

    return true;
}

bool eddic::option_defined(const std::string& option_name){
    ASSERT(options, "The options have not been initialized");

    return options->count(option_name);
}

std::string eddic::option_value(const std::string& option_name){
    ASSERT(options, "The options have not been initialized");

    return (*options)[option_name].as<std::string>();
}

void eddic::printHelp(){
    std::cout << desc << std::endl;
}

void eddic::printVersion(){
    std::cout << "eddic version 1.0.2" << std::endl;
}
