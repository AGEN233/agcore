#include "agcore_initcall.h"

/* Keep ABI validation in a C translation unit owned by the framework. */
_Static_assert(sizeof(agcore_initcall_t) == sizeof(void *),
               "initcall entries must be function pointers");

extern const agcore_initcall_t _agcore_initcall_core_start;
extern const agcore_initcall_t _agcore_initcall_core_end;
extern const agcore_initcall_t _agcore_initcall_service_start;
extern const agcore_initcall_t _agcore_initcall_service_end;
extern const agcore_initcall_t _agcore_initcall_late_start;
extern const agcore_initcall_t _agcore_initcall_late_end;

void agcore_main(void)
{
    for (const agcore_initcall_t *call = &_agcore_initcall_core_start;
            call < &_agcore_initcall_core_end;
            ++call) {
        (*call)();
    }

    for (const agcore_initcall_t *call = &_agcore_initcall_service_start;
            call < &_agcore_initcall_service_end;
            ++call) {
        (*call)();
    }

    for (const agcore_initcall_t *call = &_agcore_initcall_late_start;
            call < &_agcore_initcall_late_end;
            ++call) {
        (*call)();
    }
}
