#ifndef IRBASE_H
#define IRBASE_H
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    IRREF_INVALID,
    IRREF_LITERAL,
    IRREF_STATE
} IrReferenceType;

typedef struct
{
    int64_t literalValue;
} IrLiteral;

typedef struct
{
    int valueNumber;
    int valueSize;
    char *valueName;
} IrState;

typedef struct
{
    IrReferenceType type;
    int size;
    union
    {
        IrLiteral irLiteral;
        IrState *irState;
    };
} IrReference;

#endif