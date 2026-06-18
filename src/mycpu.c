#include "mycpu.h"
#include "mycpuinst.h"
#include <stdio.h>

//Adds all the arch register ids to vector
static void VectorAllRegisters(Vector *vector)
{
    for (int i = 0; i < 8; i++)
        vec_push(int, vector, &i);
}

//Moves part of an operand or the whole thing into a register. If the operand is larger than a register, use offset to
//control which part of the operand is loaded.
static void MoveOperandToRegister(MyCpuProgram *program, LinkedList* routineList, IrArchPhysicalLocation *operand, int offsetInBits, uint8_t registerId)
{
    if (operand->physLocationType == IRARCH_PHYS_LOCATION_REGISTER)
    {
        MyCpuInstCCreate(program, routineList, MYCPU_INST_MOV_33, registerId, (uint8_t)operand->registerId, 0);
        return;
    }

    if (operand->physLocationType == IRARCH_PHYS_LOCATION_LITERAL)
    {
        int64_t value = operand->reference.irLiteral.literalValue;
        value >>= offsetInBits;
        int16_t truncValue = (int16_t)value;
        MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV_36, registerId, 0, 0, truncValue);
        return;
    }

    int offsetInBytes = offsetInBits / 8;

    if (operand->physLocationType == IRARCH_PHYS_LOCATION_GLOBAL_MEMORY)
    {
        int effectiveAddress = operand->address + offsetInBytes;
        int bytesToLoad = (operand->reference.size - offsetInBits) / 8;
        if (bytesToLoad < 0)
            bytesToLoad = 0;

        if (bytesToLoad == 0)
        {
            MyCpuInstCCreate(program, routineList, MYCPU_INST_XOR_67, registerId, registerId, 0);
            return;
        }

        if (bytesToLoad == 1)
            MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV8_48, 0, registerId, 0, effectiveAddress);
        else
            MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV_40, 0, registerId, 0, effectiveAddress);

        return;
    }

    if (operand->physLocationType == IRARCH_PHYS_LOCATION_STACK)
    {
        int effectiveAddress = operand->address + offsetInBytes;
        int bytesToLoad = (operand->reference.size - offsetInBits) / 8;
        if (bytesToLoad < 0)
            bytesToLoad = 0;

        if (bytesToLoad == 0)
        {
            MyCpuInstCCreate(program, routineList, MYCPU_INST_XOR_67, registerId, registerId, 0);
            return;
        }

        if (bytesToLoad == 1)
            MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV8_47, MYCPU_REG_BP, registerId, 0, effectiveAddress);
        else
            MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV_39, MYCPU_REG_BP, registerId, 0, effectiveAddress);

        return;
    }
}

static void MoveRegisterToDestination(MyCpuProgram *program, LinkedList* routineList, IrArchPhysicalLocation *dstOperand, int offset, int srcRegister, int srcSizeBytes)
{
    if (dstOperand->physLocationType == IRARCH_PHYS_LOCATION_REGISTER)
    {
        MyCpuInstCCreate(program, routineList, MYCPU_INST_MOV_33, dstOperand->registerId, srcRegister, 0);
        return;
    }

    if (dstOperand->physLocationType == IRARCH_PHYS_LOCATION_STACK)
    {
        int effectiveAddress = dstOperand->address + offset;
        if (srcSizeBytes == 1)
            MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV8_45, MYCPU_REG_BP, srcRegister, 0, effectiveAddress);
        else
            MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV_37, MYCPU_REG_BP, srcRegister, 0, effectiveAddress);
        return;
    }

    if (dstOperand->physLocationType == IRARCH_PHYS_LOCATION_GLOBAL_MEMORY)
    {
        int effectiveAddress = dstOperand->address + offset;
        if (srcSizeBytes == 1)
            MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV8_46, 0, srcRegister, 0, effectiveAddress);
        else
            MyCpuInstDCreate(program, routineList, MYCPU_INST_MOV_38, 0, srcRegister, 0, effectiveAddress);
        return;
    }
}

//static void Add(MyCpuProgram *program, IrArchPhysicalLocation *operands, Vector *scratchRegisters)
//{
    //uint8_t scratchReg0 = (uint8_t)vec_at(int, scratchRegisters, 0);
    //uint8_t scratchReg1 = (uint8_t)vec_at(int, scratchRegisters, 1);

    //int sizeInBytes = operands[0].reference.size;
    //if (operands[1].reference.size > sizeInBytes)
        //sizeInBytes = operands[1].reference.size;
    //int remainder = sizeInBytes % 8;
    //sizeInBytes /= 8;
    //if (remainder > 0)
        //sizeInBytes += 1;

    //for (int offset = 0; offset < sizeInBytes; offset += 2)
    //{
        //MoveOperandToRegister(program, &operands[0], offset, scratchReg0);
        //MoveOperandToRegister(program, &operands[1], offset, scratchReg1);
        //if (offset == 0)
            //MyCpuInstCCreate(program, MYCPU_INST_ADD_5, scratchReg0, scratchReg1, 0);
        //else
            //MyCpuInstCCreate(program, MYCPU_INST_ADC_2, scratchReg0, scratchReg1, 0);

        //int bytesToMove = sizeInBytes - offset;
        //MoveRegisterToDestination(program, &operands[2], offset, scratchReg0, bytesToMove);
    //}
//}

static bool IsImmediatelyAddable(IrLiteral *literal)
{
    if (literal->literalValue <= MYCPU_IMMB_MAX)
        return true;
    if (literal->literalValue >= MYCPU_IMMB_MIN)
        return true;
    return false;
}

//Returns the list machine instructions generated by this routine.
//registerIds: An array of register ids sorted from least to most relevant. Less relevant registers will be clobbered first.
//clobberedRegisters: An out array of registered clobbered by this routine.
static LinkedList AddOptimized(
    MyCpuProgram *program,
    IrArchPhysicalLocation *operands,
    int *registerIds,
    int *clobberedRegisters,
    int *clobberedRegisterCount
)
{
    LinkedList routineList = {0};
    IrArchPhysicalLocation *op0 = &operands[0];
    IrArchPhysicalLocation *op1 = &operands[1];
    IrArchPhysicalLocation *dstOp = &operands[2];
    
    //Prefer the literal to be the right hand size since addition is commutative
    if (op0->reference.type == IRREF_LITERAL)
    {
        IrArchPhysicalLocation *temp = op0;
        op0 = op1;
        op1 = temp;
    }

    int clobberCount = 0;
    int clobberSearchIdx = 0;
    int addResultRegister = 0;

    //We'll add directly into op1's register if it is in a register
    if (op0->physLocationType == IRARCH_PHYS_LOCATION_REGISTER)
        addResultRegister = op0->registerId;
    else
    {
        addResultRegister = registerIds[clobberSearchIdx];

        //Avoid register overlap
        if (addResultRegister == op1->registerId && op1->physLocationType == IRARCH_PHYS_LOCATION_REGISTER)
        {
            clobberSearchIdx++;
            addResultRegister = registerIds[clobberSearchIdx];
        }
        clobberSearchIdx++;
    }

    clobberedRegisters[clobberCount] = addResultRegister;
    clobberCount++;

    //If op1 is already in a register, we shouldn't have to move it and clobber another register
    int addSourceRegister = op1->registerId;
    if (op1->physLocationType == IRARCH_PHYS_LOCATION_STACK ||
        op1->physLocationType == IRARCH_PHYS_LOCATION_GLOBAL_MEMORY ||
        op1->physLocationType == IRARCH_PHYS_LOCATION_LITERAL &&
        !IsImmediatelyAddable(&op1->reference.irLiteral))
    {
        addSourceRegister = registerIds[clobberSearchIdx];
        //Avoid register overlap
        if (addSourceRegister == addResultRegister)
        {
            clobberSearchIdx++;
            addSourceRegister = registerIds[clobberSearchIdx];
            clobberSearchIdx++;
        }
        clobberedRegisters[clobberCount] = addSourceRegister;
        clobberCount++;
    }

    int sizeInBits = op0->reference.size;
    if (op1->reference.size > sizeInBits)
        sizeInBits = op1->reference.size;

    for (int offset = 0; offset < sizeInBits; offset += 16)
    {
        //We've exhausted the bits of the first operand and will be adding op1 to 0
        if (offset >= op0->reference.size)
        {
            MoveOperandToRegister(program, &routineList, op1, offset, addSourceRegister);
            MoveRegisterToDestination(program, &routineList, dstOp, offset, addSourceRegister, (sizeInBits - offset) / 8);
            continue;
        }

        //Move the first operand into position, unless it is already there
        if (offset != 0 || op0->physLocationType != IRARCH_PHYS_LOCATION_REGISTER)
        {
            MoveOperandToRegister(program, &routineList, op0, offset, addResultRegister);
        }

        //We can do the add in an immediate fashion
        if (offset == 0 && op1->physLocationType == IRARCH_PHYS_LOCATION_LITERAL && IsImmediatelyAddable(&op1->reference.irLiteral))
        {
            MyCpuInstBCreate(program, &routineList, MYCPU_INST_ADD_4, addResultRegister, (int16_t)op1->reference.irLiteral.literalValue);
            MoveRegisterToDestination(program, &routineList, dstOp, offset, addResultRegister, (sizeInBits - offset) / 8);
            continue;
        }

        //Move the second operand into position, unless it is already there
        if (offset != 0)
        {
            MoveOperandToRegister(program, &routineList, op1, offset, addSourceRegister);
        }

        if (offset == 0)
            MyCpuInstCCreate(program, &routineList, MYCPU_INST_ADD_5, addResultRegister, addSourceRegister, 0);
        else
            MyCpuInstCCreate(program, &routineList, MYCPU_INST_ADC_2, addResultRegister, addSourceRegister, 0);

        MoveRegisterToDestination(program, &routineList, dstOp, offset, addResultRegister, (sizeInBits - offset) / 8);
    }

    *clobberedRegisterCount = clobberCount;
    return routineList;
}

//Returns whether the destination operand should be allocated in memory or may be a register
static void GetAddRequirements(IrReference *operands, IrArchValueLocationRequirement *dstRequirment)
{
    if (operands[0].size > 16 || operands[1].size > 16)
    {
        dstRequirment->requiredLocation = IRARCH_LOCATION_MEMORY;
    }
    else
    {
        dstRequirment->requiredLocation = IRARCH_LOCATION_REGISTER;
        VectorAllRegisters(&dstRequirment->validRegisterIds);
    }
}

//Converts an instruction to its mnemonic form and stores in buffer
static void MyCpuInstToString(MyCpuInst *inst, char *buffer, int bufferLen)
{
    const char *mnemonic = MyCpuInstMnemonics[inst->type];
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
        snprintf(buffer, bufferLen, "%s r%d, %d", mnemonic, (int)inst->reg0, (int)inst->immD);
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
        snprintf(buffer, bufferLen, "%s %d", mnemonic, (int)inst->immD);
        break;
    }
    case MYCPU_INST_CALL_11:
    {
        snprintf(buffer, bufferLen, "%s r%d+%d", mnemonic, (int)inst->reg0, (int)inst->immD);
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
        snprintf(buffer, bufferLen, "%s r%d, r%d, %d", mnemonic, (int)inst->reg0, (int)inst->reg1, (int)inst->immD);
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
        snprintf(buffer, bufferLen, "%s [r%d+%d], r%d", mnemonic, (int)inst->reg0, (int)inst->immD, (int)inst->reg1);
        break;
    }
    case MYCPU_INST_MOV_38:
    case MYCPU_INST_MOV8_46:
    {
        snprintf(buffer, bufferLen, "%s [%d], r%d", mnemonic, (int)inst->immD, (int)inst->reg1);
        break;
    }
    case MYCPU_INST_MOV_39:
    case MYCPU_INST_MOV8_47:
    {
        snprintf(buffer, bufferLen, "%s r%d, [r%d+%d]", mnemonic, (int)inst->reg1, (int)inst->reg0, (int)inst->immD);
        break;
    }
    case MYCPU_INST_MOV_40:
    case MYCPU_INST_MOV8_48:
    {
        snprintf(buffer, bufferLen, "%s r%d, [%d]", mnemonic, (int)inst->reg1, (int)inst->immD);
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

void IrArchMyCpuInit(IrArch *arch)
{
    MyCpuProgram *program = malloc(sizeof(MyCpuProgram));
    MyCpuProgramInit(program);

    *arch = (IrArch)
    {
        .registers = vec_new(IrArchRegister, 8),
        .pointerSize = 16,
        .program = program,
        .addCallback = AddOptimized,
        .addDstRequirementsCallback = GetAddRequirements
    };

    for (int i = 0; i < 8; i++)
    {
        IrArchRegister reg = (IrArchRegister)
        {
            .registerId = i,
            .size = 16
        };
        vec_push(IrArchRegister, &arch->registers, &reg);
    }

}

void IrArchMyCpuDestroy(IrArch *arch)
{
    MyCpuProgramDestroy(arch->program);
    vec_free(&arch->registers);
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