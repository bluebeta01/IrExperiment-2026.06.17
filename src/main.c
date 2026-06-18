#include <stdlib.h>
#include <stdio.h>
#include "ir.h"
#include "mycpu.h"

int main(int argc, char **argv)
{
    IrCtx ctx;
    IrArch arch;
    IrArchMyCpuInit(&arch);
    IrCtxInit(&ctx, &arch);
    IrBlock *block1 = IrBlockCreate(&ctx);

    IrState *addDstState = IrDeclareState(&ctx, block1, 64, NULL);
    IrState *addSrcState1 = IrDeclareState(&ctx, block1, 32, NULL);
    IrState *addSrcState2 = IrDeclareState(&ctx, block1, 8, NULL);
    IrReference val1 = IrReferenceState(addSrcState1);
    IrReference val2 = IrReferenceState(addSrcState2);
    IrAdd(block1, addDstState, &val1, &val2);

//    IrState *addDstState = IrDeclareState(&ctx, block1, 16, NULL);
//    IrReference val1 = IrReferenceLiteral(10);
//    IrReference val2 = IrReferenceLiteral(20);
//    IrAdd(block1, addDstState, &val1, &val2);

    IrReference val3 = IrReferenceState(addDstState);
    IrReference val4 = IrReferenceLiteral(30);
    IrAdd(block1, addDstState, &val3, &val4);

    IrBlockTranslate(&ctx, block1);

    IrCtxDestroy(&ctx);
    IrArchMyCpuDestroy(&arch);
}