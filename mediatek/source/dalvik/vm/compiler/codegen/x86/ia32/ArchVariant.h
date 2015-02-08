

#ifndef DALVIK_VM_COMPILER_CODEGEN_X86_IA32_ARCHVARIANT_H_
#define DALVIK_VM_COMPILER_CODEGEN_X86_IA32_ARCHVARIANT_H_

/* Create the TemplateOpcode enum */
#define JIT_TEMPLATE(X) TEMPLATE_##X,
enum TemplateOpcode {
#include "../../../template/ia32/TemplateOpList.h"
    TEMPLATE_LAST_MARK,
};
#undef JIT_TEMPLATE

#endif  // DALVIK_VM_COMPILER_CODEGEN_X86_IA32_ARCHVARIANT_H_
