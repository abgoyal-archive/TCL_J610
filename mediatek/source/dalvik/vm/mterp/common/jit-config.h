

#if __ARM_ARCH_5TE__
#define JIT_PROF_SIZE_LOG_2     9
#else
#define JIT_PROF_SIZE_LOG_2     11
#endif

#define JIT_PROF_SIZE           (1 << JIT_PROF_SIZE_LOG_2)
