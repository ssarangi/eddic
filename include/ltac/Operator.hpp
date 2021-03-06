//=======================================================================
// Copyright Baptiste Wicht 2011-2016.
// Distributed under the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef LTAC_OPERATOR_H
#define LTAC_OPERATOR_H

namespace eddic {

namespace ltac {

enum class Operator : unsigned int {
    MOV,
    FMOV,
    MUL3,

    //Enter stack frame
    ENTER,

    //Leave stack frame
    LEAVE,

    //Return from function
    RET,
    PRE_RET,

    LABEL,

    //Comparisons
    CMP_INT,
    CMP_FLOAT,

    //Logical operations
    OR,
    XOR,

    //Stack manipulations
    PUSH,
    POP,
    
    //Load effective address
    LEA,

    //Shifts
    SHIFT_LEFT,
    SHIFT_RIGHT,

    //Math operations
    ADD,
    SUB,
    MUL2,
    DIV,

    //Float operations
    FADD,
    FSUB,
    FMUL,
    FDIV,

    INC,
    DEC,
    NEG,
    NOT,
    AND,

    I2F,
    F2I,

    CMOVE,
    CMOVNE,
    CMOVA,
    CMOVAE,
    CMOVB,
    CMOVBE,
    CMOVG,
    CMOVGE,
    CMOVL,
    CMOVLE,

    //Special placeholders to indicate the start of parameter passing
    PRE_PARAM,

    NOP,

    XORPS,
    MOVDQU,

    ALWAYS,

    CALL,

    //Egality
    NE,
    E,
    
    //signed comparisons
    GE,
    G,
    LE,
    L,

    //unsigned comparisons
    B,
    BE,
    A,
    AE,

    P,      //Parity
    Z,      //Zero
    NZ      //Not zero
};

bool erase_result(ltac::Operator op);
bool erase_result_complete(ltac::Operator op);

} //end of ltac

} //end of eddic

#endif
