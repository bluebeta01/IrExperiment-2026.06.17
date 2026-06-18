#ifndef IR_H
#define IR_H
#include <stdint.h>
#include <stdbool.h>
#include "list.h"
#include "irarch.h"
#include "irbase.h"


typedef struct
{
    LinkedList irInstructions;
} IrBlock;

typedef struct
{
    int nextValue;
    Vector blocks;
    IrArch *arch;
} IrCtx;

typedef enum
{
    IRINSTTYPE_INVALID,
    IRINSTTYPE_DECL,
    IRINSTTYPE_ADD
} IrInstType;

typedef struct
{
    IrInstType type;
    ListNode listNode;
} IrInstruction;

typedef struct
{
    IrInstruction irInstruction;
    IrState *irState;
} IrInstDecl;

typedef struct
{
    IrInstruction irInstruction;
    IrState *dstState;
    IrReference operand1;
    IrReference operand2;
} IrInstAdd;

void IrCtxInit(IrCtx *ctx, IrArch *arch);
void IrCtxDestroy(IrCtx *ctx);
IrBlock *IrBlockCreate(IrCtx *ctx);
IrState *IrDeclareState(IrCtx *ctx, IrBlock* block, int size, const char* name);
IrReference IrReferenceState(IrState *state);
IrReference IrReferenceLiteral(uint64_t literalValue, bool isNegative, int size);
void IrAdd(IrBlock *block, IrState *dst, IrReference *operand1, IrReference *operand2);

void IrBlockTranslate(IrCtx *ctx, IrBlock *block);

#endif