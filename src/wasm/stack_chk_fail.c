#include <stdio.h>
#include <stdlib.h>

[[noreturn]]
void __stack_chk_fail(void);

void __stack_chk_fail(void) {
    puts("stack smashing detected");
    abort();
}
