#include "mycpuinst.h"

static MyCpuInst *InstructionCreate(MyCpuProgram *program, MyCpuInstType type, uint8_t reg0, uint8_t reg1, uint8_t reg2, int16_t immA, int16_t immB, int16_t immD)
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
        .type = type
    };
    return inst;
}

static void InstructionDestroy(MyCpuInst *inst)
{
    free(inst);
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
    MyCpuInst *inst = InstructionCreate(program, type, 0, 0, 0, literal, 0, 0);
    if (routineList)
        list_append(routineList, &inst->listNode);
}

void MyCpuInstBCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, int16_t literal)
{
    MyCpuInst *inst = InstructionCreate(program, type, reg0, 0, 0, 0, literal, 0);
    if (routineList)
        list_append(routineList, &inst->listNode);
}

void MyCpuInstCCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, uint8_t reg1, uint8_t reg2)
{
    MyCpuInst *inst = InstructionCreate(program, type, reg0, reg1, reg2, 0, 0, 0);
    if (routineList)
        list_append(routineList, &inst->listNode);
}

void MyCpuInstDCreate(MyCpuProgram *program, LinkedList *routineList, MyCpuInstType type, uint8_t reg0, uint8_t reg1, uint8_t reg2, int16_t literal)
{
    MyCpuInst *inst = InstructionCreate(program, type, reg0, reg1, reg2, 0, 0, literal);
    if (routineList)
        list_append(routineList, &inst->listNode);
}
