#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>

    .section    .text.relocate_vectors,"ax",%progbits

ENTRY(relocate_vectors)
    /* Remap SRAM to 00000000. */
    ldr r0, =0x3e00000c
    mov r1, #3 /* REMAP_SRAM | REMAP_ENABLE */
    str r1, [r0]

    /* Copy vectors to 00000000. */
    ldr r0, [r9, #GD_RELOCADDR] /* r0 = gd->relocaddr */
    ldr     r1, =0x00000000
    ldmia   r0!, {r2-r8,r10}
    stmia   r1!, {r2-r8,r10}
    ldmia   r0!, {r2-r8,r10}
    stmia   r1!, {r2-r8,r10}

    bx lr
ENDPROC(relocate_vectors)