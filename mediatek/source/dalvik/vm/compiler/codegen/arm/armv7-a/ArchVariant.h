

#ifndef DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV7_A_ARCHVARIANT_H_
#define DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV7_A_ARCHVARIANT_H_

/* Create the TemplateOpcode enum */
#define JIT_TEMPLATE(X) TEMPLATE_##X,
enum TemplateOpcode {
#include "../../../template/armv5te-vfp/TemplateOpList.h"
    TEMPLATE_LAST_MARK,
};
#undef JIT_TEMPLATE

#endif  // DALVIK_VM_COMPILER_CODEGEN_ARM_ARMV7_A_ARCHVARIANT_H_
