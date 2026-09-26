#ifndef AGCORE_INITCALL_H
#define AGCORE_INITCALL_H

void agcore_main(void);

/* Register fn to run during the requested AGCORE initialization phase. */
#define AGCORE_INITCALL(level, fn) \
    static __typeof__(&(fn)) const __ag_i_##fn \
    __attribute__((used, section(".agcore_initcall." level))) = (fn)

#define AGCORE_CORE_INITCALL(fn)    AGCORE_INITCALL("core", fn)
#define AGCORE_SERVICE_INITCALL(fn) AGCORE_INITCALL("service", fn)
#define AGCORE_LATE_INITCALL(fn)    AGCORE_INITCALL("late", fn)

#endif /* AGCORE_INITCALL_H */
