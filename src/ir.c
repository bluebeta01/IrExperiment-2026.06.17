#include "ir.h"
#include <string.h>
#include "mycpudebug.h"

static void AppendInstruction(IrBlock *block, IrInstruction *instruction)
{
    list_append(&block->irInstructions, &instruction->listNode);
}

static IrInstAdd *IrInstAddCreate(IrState *dstState, IrReference *operand0, IrReference *operand1)
{
    IrInstAdd *add = malloc(sizeof(IrInstAdd));
    *add = (IrInstAdd)
    {
        .irInstruction = (IrInstruction)
        {
            .type = IRINSTTYPE_ADD
        },
        .dstState = dstState,
        .operand0 = *operand0,
        .operand1 = *operand1
    };
    return add;
}

static IrInstDecl *IrInstDeclCreate(IrState *state)
{
    IrInstDecl *decl = malloc(sizeof(IrInstDecl));
    *decl = (IrInstDecl)
    {
        .irInstruction = (IrInstruction)
        {
            .type = IRINSTTYPE_DECL
        },
        .irState = state
    };
    return decl;
}

static void IrInstructionDestroy(IrInstruction* inst)
{
    free(inst);
}

static void IrBlockDestroy(IrBlock *block)
{
    ListNode *instListNode = block->irInstructions.first;
    while (instListNode)
    {
        ListNode *next = instListNode->next;
        IrInstruction *inst = list_value(IrInstruction, listNode, instListNode);
        IrInstructionDestroy(inst);
        instListNode = next;
    }
}

void IrCtxInit(IrCtx *ctx, IrArch *arch)
{
    *ctx = (IrCtx)
    {
        .blocks = vec_new(IrBlock*, 10),
        .arch = arch
    };
}

void IrCtxDestroy(IrCtx *ctx)
{
    for (int i = 0; i < ctx->blocks.size; i++)
    {
        IrBlockDestroy(vec_at(IrBlock*, &ctx->blocks, i));
    }
    vec_free(&ctx->blocks);
}

IrBlock *IrBlockCreate(IrCtx *ctx)
{
    IrBlock *block = malloc(sizeof(IrBlock));
    *block = (IrBlock){0};
    vec_push(IrBlock*, &ctx->blocks, &block);
    return block;
}

IrState *IrDeclareState(IrCtx *ctx, IrBlock* block, int size, const char* name)
{
    IrState *state = malloc(sizeof(IrState));
    char *valueName = NULL;
    if (name)
    {
        int nameLen = strlen(name);
        valueName = malloc(nameLen + 1);
        strcpy(valueName, name);
        valueName[nameLen] = 0;
    }
    *state = (IrState)
    {
        .valueNumber = ctx->nextValue++,
        .valueName = valueName,
        .valueSize = size
    };

    IrInstDecl *decl = IrInstDeclCreate(state);
    AppendInstruction(block, (IrInstruction*)decl);

    return state;
}

IrReference IrReferenceState(IrState *state)
{
    return (IrReference)
    {
        .type = IRREF_STATE,
        .irState = state,
        .size = state->valueSize
    };
}

IrReference IrReferenceLiteral(int64_t literalValue)
{
    return (IrReference)
    {
        .type = IRREF_LITERAL,
        .size = 64,
        .irLiteral = (IrLiteral)
        {
            .literalValue = literalValue
        }
    };
}

void IrAdd(IrBlock *block, IrState *dst, IrReference *operand0, IrReference *operand1)
{
    IrInstAdd *addInst = IrInstAddCreate(dst, operand0, operand1);
    AppendInstruction(block, (IrInstruction*)addInst);
}

void IrBlockTranslate(IrCtx *ctx, IrBlock *block)
{
    ListNode *instListNode = block->irInstructions.first;
    IrInstruction *inst = NULL;
    while (instListNode)
    {
        inst = list_value(IrInstruction, listNode, instListNode);
        if (inst->type == IRINSTTYPE_ADD) break;
        instListNode = instListNode->next;
    }

    if (!inst) return;

    IrInstAdd *addInst = (IrInstAdd*)inst;
    int registerIds[] = { 4, 3, 2, 1 };
    int clobbers[4] = {0};
    int cobberCount = 0;
    IrArchPhysicalLocation operands[3] = 
    {
        (IrArchPhysicalLocation)
        {
            .physLocationType = IRARCH_PHYS_LOCATION_STACK,
            .reference = addInst->operand0,
            .address = -20
        },
        (IrArchPhysicalLocation)
        {
            .physLocationType = IRARCH_PHYS_LOCATION_STACK,
            .reference = addInst->operand1,
            .address = -30
        },
        (IrArchPhysicalLocation)
        {
            .physLocationType = IRARCH_PHYS_LOCATION_STACK,
            .address = -8
            //.registerId = 5
        },
    };

    LinkedList instructionList = ctx->arch->addCallback(ctx->arch->program, operands, registerIds, clobbers, &cobberCount);

    MyCpuPrettyPrintInstructions(instructionList.first);
}