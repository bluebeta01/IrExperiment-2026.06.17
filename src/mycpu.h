#ifndef MYCPU_H
#define MYCPU_H
#include "irarch.h"
#define MYCPU_REG_BP 4
#define MYCPU_IMMB_MAX 31
#define MYCPU_IMMB_MIN -32

void IrArchMyCpuInit(IrArch *arch);
void IrArchMyCpuDestroy(IrArch *arch);

#endif