#ifndef IRARCH_H
#define IRARCH_H
#include "list.h"
#include "irbase.h"

typedef enum
{
    IRARCH_LOCATION_INVALID,
    IRARCH_LOCATION_REGISTER, //The value must live in a register
    IRARCH_LOCATION_MEMORY, //The value must live in memory
} IrArchValueLocation;

typedef struct
{
    IrArchValueLocation requiredLocation;
    Vector validRegisterIds;
} IrArchValueLocationRequirement;

typedef struct
{
    int registerId;
    int size;
} IrArchRegister;

typedef enum
{
    IRARCH_PHYS_LOCATION_INVALID,
    IRARCH_PHYS_LOCATION_REGISTER,
    IRARCH_PHYS_LOCATION_STACK,
    IRARCH_PHYS_LOCATION_GLOBAL_MEMORY,
    IRARCH_PHYS_LOCATION_LITERAL
} IrArchPhysicalLocationType;

typedef struct
{
    IrArchPhysicalLocationType physLocationType;
    IrReference reference;

    union
    {
        int registerId;
        int address;
    };
} IrArchPhysicalLocation;

typedef void (*InstDstRequirmentsCallback)(IrReference *operands, IrArchValueLocationRequirement *dstRequirment);

typedef LinkedList (*InstTranslateCallback)(
    void *program,
    IrArchPhysicalLocation *operands,
    int *registerIds,
    int *clobberedRegisters,
    int *clobberedRegisterCount
);

typedef struct
{
    Vector registers;
    int pointerSize;
    void *program;  //Pointer to the backend program context
    InstTranslateCallback addCallback;
    InstDstRequirmentsCallback addDstRequirementsCallback;
} IrArch;


#endif