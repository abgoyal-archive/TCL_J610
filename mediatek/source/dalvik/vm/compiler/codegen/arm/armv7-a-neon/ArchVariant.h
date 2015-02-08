

#ifndef DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV5TE_VFP_ARCHVARIANT_H_
#define DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV5TE_VFP_ARCHVARIANT_H_

/* Create the TemplateOpcode enum */
#define JIT_TEMPLATE(X) TEMPLATE_##X,
typedef enum {
#include "../../../template/armv5te-vfp/TemplateOpList.h"
    TEMPLATE_LAST_MARK,
} TemplateOpcode;
#undef JIT_TEMPLATE

#endif  // DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV5TE_VFP_ARCHVARIANT_H_
