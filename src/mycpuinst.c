#include "mycpuinst.h"
#include <stdio.h>

static MyCpuInst *LabelCreate(MyCpuProgram *program)
{
    MyCpuInst *inst = malloc(sizeof(MyCpuInst));
    vec_push(MyCpuInst*, &program->instructions, &inst);
    *inst = (MyCpuInst)
    {
        .type = MYCPU_INST_LABEL,
        .labelId = ++program->labelIdCounter
    };
    return inst;
}

static MyCpuInst *InstructionCreate(MyCpuProgram *program, MyCpuInstType type, uint8_t reg0, uint8_t reg1, uint8_t reg2, int16_t immA, int16_t immB, int16_t immD, int labelId)
{
    MyCpuInst *inst = malloc(sizeof(MyCpuInst));
    vec_push(MyCpuInst*, &program->instructions, &inst);
    *inst = (MyCpuInst)
    {
        .immA = immA,
        .immB = immB,
        .immD = immD,
        .reg0 = reg0,
        .reg1 = reg1,
        .reg2 = reg2,
        .type = type,
        .labelId = labelId
    };
    return inst;
}

static void InstructionDestroy(MyCpuInst *inst)
{
    free(inst);
}

//Converts an instruction to its mnemonic form and stores in buffer
static void MyCpuInstToString(MyCpuInst *inst, char *buffer, int bufferLen)
{
    const char *mnemonic = MyCpuInstMnemonics[inst->type];

    char *immDStr[32];
    if (inst->labelId)
        snprintf(immDStr, sizeof(immDStr), "label-%d", inst->labelId);
    else
        snprintf(immDStr, sizeof(immDStr), "%d", inst->immD);

    switch (inst->type)
    {
    case MYCPU_INST_NOP_0:
    {
        snprintf(buffer, bufferLen, "%s", mnemonic);
        break;
    }
    case MYCPU_INST_ADC_1:
    case MYCPU_INST_ADD_4:
    case MYCPU_INST_CMP_12:
    case MYCPU_INST_MUL_51:
    case MYCPU_INST_MULS_54:
    case MYCPU_INST_SUB_64:
    case MYCPU_INST_MOV_32:
    {
        snprintf(buffer, bufferLen, "%s r%d, %d", mnemonic, (int)inst->reg0, (int)inst->immB);
        break;
    }
    case MYCPU_INST_ADC_2:
    case MYCPU_INST_ADD_5:
    case MYCPU_INST_AND_7:
    case MYCPU_INST_CMP_13:
    case MYCPU_INST_MUL_52:
    case MYCPU_INST_MULS_55:
    case MYCPU_INST_OR_58:
    case MYCPU_INST_SUB_65:
    case MYCPU_INST_XOR_67:
    case MYCPU_INST_MOV_33:
    {
        snprintf(buffer, bufferLen, "%s r%d, r%d", mnemonic, (int)inst->reg0, (int)inst->reg1);
        break;
    }
    case MYCPU_INST_ADC_3:
    case MYCPU_INST_ADD_6:
    case MYCPU_INST_AND_8:
    case MYCPU_INST_CMP_14:
    case MYCPU_INST_MUL_53:
    case MYCPU_INST_MULS_56:
    case MYCPU_INST_OR_59:
    case MYCPU_INST_SUB_66:
    case MYCPU_INST_XOR_68:
    case MYCPU_INST_MOV_36:
    {
        snprintf(buffer, bufferLen, "%s r%d, %s", mnemonic, (int)inst->reg0, immDStr);
        break;
    }
    case MYCPU_INST_CALL_9:
    case MYCPU_INST_EXT_15:
    case MYCPU_INST_JEQ_17:
    case MYCPU_INST_JMP_20:
    case MYCPU_INST_JOF_23:
    case MYCPU_INST_JSF_26:
    case MYCPU_INST_SHL_62:
    case MYCPU_INST_SHR_63:
    {
        snprintf(buffer, bufferLen, "%s r%d", mnemonic, (int)inst->reg0);
        break;
    }
    case MYCPU_INST_CALL_10:
    case MYCPU_INST_JMP_21:
    case MYCPU_INST_JEQ_18:
    case MYCPU_INST_JOF_24:
    case MYCPU_INST_JSF_27:
    {
        snprintf(buffer, bufferLen, "%s %s", mnemonic, immDStr);
        break;
    }
    case MYCPU_INST_CALL_11:
    {
        snprintf(buffer, bufferLen, "%s r%d+%s", mnemonic, (int)inst->reg0, immDStr);
        break;
    }
    case MYCPU_INST_JEQ_16:
    case MYCPU_INST_JMP_19:
    case MYCPU_INST_JOF_22:
    case MYCPU_INST_JSF_25:
    {
        snprintf(buffer, bufferLen, "%s r%d+%d", mnemonic, (int)inst->reg0, (int)inst->immB);
        break;
    }
    case MYCPU_INST_LMUL_28:
    case MYCPU_INST_LMULS_30:
    {
        snprintf(buffer, bufferLen, "%s r%d, r%d, %s", mnemonic, (int)inst->reg0, (int)inst->reg1, immDStr);
        break;
    }
    case MYCPU_INST_LMUL_29:
    case MYCPU_INST_LMULS_31:
    {
        snprintf(buffer, bufferLen, "%s r%d, r%d, r%d", mnemonic, (int)inst->reg0, (int)inst->reg1, (int)inst->reg2);
        break;
    }
    case MYCPU_INST_POP_60:
    case MYCPU_INST_PUSH_61:
    {
        snprintf(buffer, bufferLen, "%s r%d", mnemonic, (int)inst->reg1);
        break;
    }
    case MYCPU_INST_MOV_34:
    case MYCPU_INST_MOV8_43:
    {
        snprintf(buffer, bufferLen, "%s [r%d], r%d", mnemonic, (int)inst->reg0, (int)inst->reg1);
        break;
    }
    case MYCPU_INST_MOV_35:
    case MYCPU_INST_MOV8_44:
    {
        snprintf(buffer, bufferLen, "%s r%d, [r%d]", mnemonic, (int)inst->reg1, (int)inst->reg0);
        break;
    }
    case MYCPU_INST_MOV_37:
    case MYCPU_INST_MOV8_45:
    {
        snprintf(buffer, bufferLen, "%s [r%d+%s], r%d", mnemonic, (int)inst->reg0, immDStr, (int)inst->reg1);
        break;
    }
    case MYCPU_INST_MOV_38:
    case MYCPU_INST_MOV8_46:
    {
        snprintf(buffer, bufferLen, "%s [%s], r%d", mnemonic, immDStr, (int)inst->reg1);
        break;
    }
    case MYCPU_INST_MOV_39:
    case MYCPU_INST_MOV8_47:
    {
        snprintf(buffer, bufferLen, "%s r%d, [r%d+%s]", mnemonic, (int)inst->reg1, (int)inst->reg0, immDStr);
        break;
    }
    case MYCPU_INST_MOV_40:
    case MYCPU_INST_MOV8_48:
    {
        snprintf(buffer, bufferLen, "%s r%d, [%s]", mnemonic, (int)inst->reg1, immDStr);
        break;
    }
    case MYCPU_INST_MOV_41:
    case MYCPU_INST_MOV8_49:
    {
        snprintf(buffer, bufferLen, "%s [r%d+r%d], r%d", mnemonic, (int)inst->reg0, (int)inst->reg2, (int)inst->reg1);
        break;
    }
    case MYCPU_INST_MOV_42:
    case MYCPU_INST_MOV8_50:
    {
        snprintf(buffer, bufferLen, "%s r%d, [r%d+r%d]", mnemonic, (int)inst->reg1, (int)inst->reg0, (int)inst->reg2);
        break;
    }
    default:
    {
        snprintf(
            buffer,
            bufferLen,
            "Unknown Instruction: %s, %d, %d, %d, %d, %d, %d\n",
            mnemonic,
            (int)inst->reg0,
            (int)inst->reg1,
            (int)inst->reg2,
            (int)inst->immA,
            (int)inst->immB,
            (int)inst->immD);
        break;
    }
    }
}

void MyCpuProgramInit(MyCpuProgram* program)
{
    *program = (MyCpuProgram)
    {
        .instructions = vec_new(MyCpuInst*, 10)
    };
}

void MyCpuProgramDestroy(MyCpuProgram *program)
{
    for (int i = 0; i < program->instructions.size; i++)
    {
        InstructionDestroy(vec_at(MyCpuInst*, &program->instructions, i));
    }
    vec_free(&program->instructions);
}

void MyCpuInstACreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, int16_t literal)
{
    MyCpuInst *inst = InstructionCreate(program, type, 0, 0, 0, literal, 0, 0, 0);
    if (routineList)
        list_append(routineList, &inst->listNode);
}

void MyCpuInstBCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, int16_t literal)
{
    MyCpuInst *inst = InstructionCreate(program, type, reg0, 0, 0, 0, literal, 0, 0);
    if (routineList)
        list_append(routineList, &inst->listNode);
}

void MyCpuInstCCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, uint8_t reg1, uint8_t reg2)
{
    MyCpuInst *inst = InstructionCreate(program, type, reg0, reg1, reg2, 0, 0, 0, 0);
    if (routineList)
        list_append(routineList, &inst->listNode);
}

void MyCpuInstDCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, uint8_t reg1, uint8_t reg2, int16_t literal, int labelId)
{
    MyCpuInst *inst = InstructionCreate(program, type, reg0, reg1, reg2, 0, 0, literal, labelId);
    if (routineList)
        list_append(routineList, &inst->listNode);
}

MyCpuInst *MyCpuInstLabelCreate(MyCpuProgram *program)
{
    MyCpuInst *inst = LabelCreate(program);
    return inst;
}

void MyCpuPrettyPrintInstructions(ListNode *instructionListNode)
{
    char buffer[128];
    while (instructionListNode)
    {
        MyCpuInst *inst = list_value(MyCpuInst, listNode, instructionListNode);
        MyCpuInstToString(inst, buffer, sizeof(buffer));
        puts(buffer);
        instructionListNode = instructionListNode->next;
    }
}

const char *MyCpuInstMnemonics[] =
{
    #define X(enum, opcode, mnemonic) mnemonic
    x_mycpu_instructions
    #undef X
};