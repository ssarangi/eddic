//=======================================================================
// Copyright Baptiste Wicht 2011-2014.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "NativeBackEnd.hpp"
#include "Utils.hpp"
#include "Options.hpp"
#include "AssemblyFileWriter.hpp"
#include "Assembler.hpp"
#include "FloatPool.hpp"
#include "GlobalContext.hpp"

//Low-level Three Address Code
#include "ltac/Compiler.hpp"
#include "ltac/PeepholeOptimizer.hpp"
#include "ltac/Printer.hpp"
#include "ltac/aggregates.hpp"
#include "ltac/prologue.hpp"
#include "ltac/stack_offsets.hpp"
#include "ltac/stack_space.hpp"
#include "ltac/register_allocator.hpp"
#include "ltac/pre_alloc_cleanup.hpp"

//Code generation
#include "asm/CodeGeneratorFactory.hpp"

using namespace eddic;

void NativeBackEnd::generate(mtac::Program& program, Platform platform){
    std::string output = configuration->option_value("output");

    //Prepare the float pool
    FloatPool float_pool;

    //Allocate stack positions for aggregates that have not been allocated
    ltac::allocate_aggregates(program);

    //Generate LTAC Code
    ltac::Compiler ltacCompiler(platform, configuration);
    ltacCompiler.compile(program, float_pool);

    //Switch to LTAC Mode
    program.mode = mtac::Mode::LTAC;

    //Clean the code generated by the LTAC Compiler to ease the register allocation
    ltac::pre_alloc_cleanup(program);
    
    if(configuration->option_defined("ltac-pre")){
        ltac::Printer printer;
        printer.print(program);
    }

    //Init the structures and arrays
    //Must be done before register allocation to profit from it
    ltac::alloc_stack_space(program);

    //Allocate pseudo registers into hard registers
    ltac::register_allocation(program, platform);
    
    //Generate the prologue and epilogue of each functions
    ltac::generate_prologue_epilogue(program, configuration);

    //If specified by the configuration, replace all stack offsets using SP 
    if(configuration->option_defined("fomit-frame-pointer")){
        ltac::fix_stack_offsets(program, platform);
    }
    
    if(configuration->option_defined("ltac-alloc")){
        ltac::Printer printer;
        printer.print(program);
    }

    if(configuration->option_defined("fpeephole-optimization")){
        ltac::optimize(program, platform);
    }

    if(configuration->option_defined("ltac") || configuration->option_defined("ltac-only")){
        ltac::Printer printer;
        printer.print(program);
    }

    if(!configuration->option_defined("ltac-only")){
        auto input_file_name = configuration->option_value("input");
        auto asm_file_name = input_file_name + ".s";
        auto object_file_name = input_file_name + ".o";

        {
            timing_timer timer(program.context->timing(), "assembly_generation");

            //Generate assembly from TAC
            AssemblyFileWriter writer(asm_file_name);

            as::CodeGeneratorFactory factory;
            auto generator = factory.get(platform, writer, program, program.context);

            //Generate the code from the LTAC Program
            generator->generate(*get_string_pool(), float_pool);

            //writer's destructor flushes the file
        }

        //If it's necessary, assemble and link the assembly
        if(!configuration->option_defined("assembly")){
            timing_timer timer(program.context->timing(), "assemble");

            verify_dependencies();

            assemble(platform, asm_file_name, object_file_name, output, configuration->option_defined("debug"), configuration->option_defined("verbose"));

            //Remove temporary files
            if(!configuration->option_defined("keep")){
                remove(asm_file_name.c_str());
            }

            remove(object_file_name.c_str());
        }
    }
}
