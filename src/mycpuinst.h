#ifndef MYCPUINST_H
#define MYCPUINST_H
#include <stdint.h>
#include "list.h"

//X is instruction enum, op code, mnemonic
#define x_mycpu_instructions \
    X(MYCPU_INST_NOP_0, 0, "nop"),\
    X(MYCPU_INST_ADC_1, 1, "adc"),\
    X(MYCPU_INST_ADC_2, 2, "adc"),\
    X(MYCPU_INST_ADC_3, 3, "adc"),\
    X(MYCPU_INST_ADD_4, 4, "add"),\
    X(MYCPU_INST_ADD_5, 5, "add"),\
    X(MYCPU_INST_ADD_6, 6, "add"),\
    X(MYCPU_INST_AND_7, 7, "and"),\
    X(MYCPU_INST_AND_8, 8, "and"),\
    X(MYCPU_INST_CALL_9, 9, "call"),\
    X(MYCPU_INST_CALL_10, 10, "call"),\
    X(MYCPU_INST_CALL_11, 11, "call"),\
    X(MYCPU_INST_CMP_12, 12, "cmp"),\
    X(MYCPU_INST_CMP_13, 13, "cmp"),\
    X(MYCPU_INST_CMP_14, 14, "cmp"),\
    X(MYCPU_INST_EXT_15, 15, "ext"),\
    X(MYCPU_INST_JEQ_16, 16, "jeq"),\
    X(MYCPU_INST_JEQ_17, 17, "jeq"),\
    X(MYCPU_INST_JEQ_18, 18, "jeq"),\
    X(MYCPU_INST_JMP_19, 19, "jmp"),\
    X(MYCPU_INST_JMP_20, 20, "jmp"),\
    X(MYCPU_INST_JMP_21, 21, "jmp"),\
    X(MYCPU_INST_JOF_22, 22, "jof"),\
    X(MYCPU_INST_JOF_23, 23, "jof"),\
    X(MYCPU_INST_JOF_24, 24, "jof"),\
    X(MYCPU_INST_JSF_25, 25, "jsf"),\
    X(MYCPU_INST_JSF_26, 26, "jsf"),\
    X(MYCPU_INST_JSF_27, 27, "jsf"),\
    X(MYCPU_INST_LMUL_28, 28, "lmul"),\
    X(MYCPU_INST_LMUL_29, 29, "lmul"),\
    X(MYCPU_INST_LMULS_30, 30, "lmuls"),\
    X(MYCPU_INST_LMULS_31, 31, "lmuls"),\
    X(MYCPU_INST_MOV_32, 32, "mov"),\
    X(MYCPU_INST_MOV_33, 33, "mov"),\
    X(MYCPU_INST_MOV_34, 34, "mov"),\
    X(MYCPU_INST_MOV_35, 35, "mov"),\
    X(MYCPU_INST_MOV_36, 36, "mov"),\
    X(MYCPU_INST_MOV_37, 37, "mov"),\
    X(MYCPU_INST_MOV_38, 38, "mov"),\
    X(MYCPU_INST_MOV_39, 39, "mov"),\
    X(MYCPU_INST_MOV_40, 40, "mov"),\
    X(MYCPU_INST_MOV_41, 41, "mov"),\
    X(MYCPU_INST_MOV_42, 42, "mov"),\
    X(MYCPU_INST_MOV8_43, 43, "mov8"),\
    X(MYCPU_INST_MOV8_44, 44, "mov8"),\
    X(MYCPU_INST_MOV8_45, 45, "mov8"),\
    X(MYCPU_INST_MOV8_46, 46, "mov8"),\
    X(MYCPU_INST_MOV8_47, 47, "mov8"),\
    X(MYCPU_INST_MOV8_48, 48, "mov8"),\
    X(MYCPU_INST_MOV8_49, 49, "mov8"),\
    X(MYCPU_INST_MOV8_50, 50, "mov8"),\
    X(MYCPU_INST_MUL_51, 51, "mul"),\
    X(MYCPU_INST_MUL_52, 52, "mul"),\
    X(MYCPU_INST_MUL_53, 53, "mul"),\
    X(MYCPU_INST_MULS_54, 54, "muls"),\
    X(MYCPU_INST_MULS_55, 55, "muls"),\
    X(MYCPU_INST_MULS_56, 56, "muls"),\
    X(MYCPU_INST_OR_58, 58, "or"),\
    X(MYCPU_INST_OR_59, 59, "or"),\
    X(MYCPU_INST_POP_60, 60, "pop"),\
    X(MYCPU_INST_PUSH_61, 61, "push"),\
    X(MYCPU_INST_SHL_62, 62, "shl"),\
    X(MYCPU_INST_SHR_63, 63, "shr"),\
    X(MYCPU_INST_SUB_64, 64, "sub"),\
    X(MYCPU_INST_SUB_65, 65, "sub"),\
    X(MYCPU_INST_SUB_66, 66, "sub"),\
    X(MYCPU_INST_XOR_67, 67, "xor"),\
    X(MYCPU_INST_XOR_68, 68, "xor"),\
    X(MYCPU_INST_LABEL, 0, "label"),

typedef enum
{
    #define X(enum, opcode, mnemonic) enum
    x_mycpu_instructions
    #undef X
    MYCPU_INST_COUNT
} MyCpuInstType;

typedef struct
{
    MyCpuInstType type;
    ListNode listNode;
    int labelId;
    int16_t immA;
    int16_t immB;
    int16_t immD;
    uint8_t reg0;
    uint8_t reg1;
    uint8_t reg2;
} MyCpuInst;

typedef struct
{
    Vector instructions;
    int labelIdCounter;
} MyCpuProgram;

const char *MyCpuInstMnemonics[];

void MyCpuProgramInit(MyCpuProgram* program);
void MyCpuProgramDestroy(MyCpuProgram* program);
void MyCpuInstACreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, int16_t literal);
void MyCpuInstBCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, int16_t literal);
void MyCpuInstCCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, uint8_t reg1, uint8_t reg2);
void MyCpuInstDCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, uint8_t reg1, uint8_t reg2, int16_t literal, int labelId);
MyCpuInst *MyCpuInstLabelCreate(MyCpuProgram *program);

#endif