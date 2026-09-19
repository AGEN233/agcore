#ifndef AGCORE_INITCALL_H
#define AGCORE_INITCALL_H

typedef int (*agcore_initcall_t)(void);

/* Public AGCORE entry, called by the secure loader after Security PASS. */
void agcore_main(void);

/*
 * Each entry is retained and collected by common/initcall/linker.lf.  The
 * section contains the real module init function pointer, not an adapter.
 */
#define AGCORE_INITCALL(level, fn) \
    static const agcore_initcall_t __agcore_initcall_##fn \
    __attribute__((used, section(".agcore_initcall." level))) = (fn)

#define AGCORE_CORE_INITCALL(fn)    AGCORE_INITCALL("core", fn)
#define AGCORE_SERVICE_INITCALL(fn) AGCORE_INITCALL("service", fn)
#define AGCORE_LATE_INITCALL(fn)    AGCORE_INITCALL("late", fn)

#endif /* AGCORE_INITCALL_H */
