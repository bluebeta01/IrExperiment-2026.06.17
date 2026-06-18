#ifndef MYCPU_H
#define MYCPU_H
#include "irarch.h"
#define MYCPU_REG_BP 4
#define MYCPU_IMMB_MAX 31
#define MYCPU_IMMB_MIN -32
#define MYCPU_IMMD_MAX 32767
#define MYCPU_IMMD_MIN -32768

void IrArchMyCpuInit(IrArch *arch);
void IrArchMyCpuDestroy(IrArch *arch);

#endif