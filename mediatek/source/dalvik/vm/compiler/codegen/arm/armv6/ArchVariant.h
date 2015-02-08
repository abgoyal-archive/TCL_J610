

#ifndef _DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV6_ARCHVARIANT_H
#define _DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV6_ARCHVARIANT_H

/* Create the TemplateOpcode enum */
#define JIT_TEMPLATE(X) TEMPLATE_##X,
typedef enum {
#include "../../../template/armv5te-vfp/TemplateOpList.h"
    TEMPLATE_LAST_MARK,
} TemplateOpCode;
#undef JIT_TEMPLATE

#endif /* _DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV6_ARCHVARIANT_H */
