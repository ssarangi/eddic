//=======================================================================
// Copyright Baptiste Wicht 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef LTAC_REGISTER_MANAGER_H
#define LTAC_REGISTER_MANAGER_H

#include <memory>
#include <vector>
#include <unordered_set>

#include <boost/variant.hpp>

#include "Platform.hpp"
#include "FloatPool.hpp"

#include "mtac/Program.hpp"
#include "ltac/Program.hpp"

#include "asm/Registers.hpp"

namespace eddic {

namespace ltac {

struct RegisterManager {
    //The registers
    as::Registers<ltac::Register> registers;
    as::Registers<ltac::FloatRegister> float_registers;

    //Keep track of the written variables to spills them
    std::unordered_set<std::shared_ptr<Variable>> written;

    //The function being compiled
    std::shared_ptr<ltac::Function> function;

    std::shared_ptr<FloatPool> float_pool;

    RegisterManager(std::vector<ltac::Register> registers, std::vector<ltac::FloatRegister> float_registers, 
            std::shared_ptr<ltac::Function> function, std::shared_ptr<FloatPool> float_pool);
        
    /*!
     * Deleted copy constructor
     */
    RegisterManager(const RegisterManager& rhs) = delete;

    /*!
     * Deleted copy assignment operator. 
     */
    RegisterManager& operator=(const RegisterManager& rhs) = delete;

    void reserve(ltac::Register reg);
    void reserve(ltac::FloatRegister reg);
    void release(ltac::Register reg);
    void release(ltac::FloatRegister reg);

    void reset();
    
    void copy(mtac::Argument argument, ltac::FloatRegister reg);
    void copy(mtac::Argument argument, ltac::Register reg);
    
    void move(mtac::Argument argument, ltac::Register reg);
    void move(mtac::Argument argument, ltac::FloatRegister reg);

    ltac::Register get_reg(std::shared_ptr<Variable> var);
    ltac::Register get_reg_no_move(std::shared_ptr<Variable> var);
    ltac::FloatRegister get_float_reg(std::shared_ptr<Variable> var);
    ltac::FloatRegister get_float_reg_no_move(std::shared_ptr<Variable> var);
    
    void safe_move(std::shared_ptr<Variable> variable, ltac::Register reg);
    void safe_move(std::shared_ptr<Variable> variable, ltac::FloatRegister reg);
    
    ltac::Register get_free_reg();
    ltac::FloatRegister get_free_float_reg();

    void spills_all();
    
    void spills(ltac::Register reg);
    void spills(ltac::FloatRegister reg);
    
    void spills_if_necessary(ltac::Register reg, mtac::Argument arg);

    bool is_written(std::shared_ptr<Variable> variable);
    void set_written(std::shared_ptr<Variable> variable);

    void set_current(mtac::Statement statement);
        
    bool is_live(std::unordered_map<std::shared_ptr<Variable>, bool>& liveness, std::shared_ptr<Variable> variable);
    bool is_live(std::shared_ptr<Variable> variable, mtac::Statement statement);
    bool is_live(std::shared_ptr<Variable> variable);
    
    void collect_parameters(std::shared_ptr<eddic::Function> definition, PlatformDescriptor* descriptor);

    void save_registers(std::shared_ptr<mtac::Param>& param, PlatformDescriptor* descriptor);
    void restore_pushed_registers();
    
    private: 
        mtac::Statement current;
        
        //Store the Register that are saved before call
        std::vector<ltac::Register> int_pushed;
        std::vector<ltac::FloatRegister> float_pushed;

        //Allow to push needed register before the first push param
        bool first_param = true;
};

} //end of ltac

} //end of eddic

#endif