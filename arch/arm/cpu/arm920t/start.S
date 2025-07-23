/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  armboot - Startup Code for ARM920 CPU-core
 *
 *  Copyright (c) 2001	Marius Gröger <mag@sysgo.de>
 *  Copyright (c) 2002	Alex Züpke <azu@sysgo.de>
 *  Copyright (c) 2002	Gary Jennejohn <garyj@denx.de>
 */

#include <asm-offsets.h>
#include <config.h>

/*
 *************************************************************************
 *
 * Startup Code (called from the ARM reset exception vector)
 *
 * do important init only if we don't start from memory!
 * relocate armboot to ram
 * setup stack
 * jump to second stage
 *
 *************************************************************************
 */

	.globl	reset

reset:
#ifdef CONFIG_POSITION_INDEPENDENT
	/*
	 * Fix .rela.dyn relocations. This allows U-Boot to loaded to and
	 * executed at a different address than it was linked at.
	 */
pie_fixup:
	adr	r0, reset	/* r0 <- Runtime value of reset label */
	ldr	r1, =reset	/* r1 <- Linked value of reset label */
	subs	r4, r0, r1	/* r4 <- Runtime-vs-link offset */
	beq	pie_fixup_done

	adr	r0, pie_fixup
	ldr	r1, _rel_dyn_start_ofs
	add	r2, r0, r1	/* r2 <- Runtime &__rel_dyn_start */
	ldr	r1, _rel_dyn_end_ofs
	add	r3, r0, r1	/* r3 <- Runtime &__rel_dyn_end */

pie_fix_loop:
	ldr	r0, [r2]	/* r0 <- Link location */
	ldr	r1, [r2, #4]	/* r1 <- fixup */
	cmp	r1, #23		/* relative fixup? */
	bne	pie_skip_reloc

	/* relative fix: increase location by offset */
	add	r0, r4
	ldr	r1, [r0]
	add	r1, r4
	str	r1, [r0]
	str	r0, [r2]
	add	r2, #8
pie_skip_reloc:
	cmp	r2, r3
	blo	pie_fix_loop
pie_fixup_done:
#endif

	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr	r0, r0, #0xd3
	msr	cpsr, r0

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
	bl	cpu_init_crit
#endif

	bl	_main

/*------------------------------------------------------------------------------*/

	.globl	c_runtime_cpu_setup
c_runtime_cpu_setup:

	mov	pc, lr

/*
 *************************************************************************
 *
 * CPU_init_critical registers
 *
 * setup important registers
 * setup memory timing
 *
 *************************************************************************
 */


#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT)
cpu_init_crit:
	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (A) Align
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-Cache
	mcr	p15, 0, r0, c1, c0, 0

#if !CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT_ONLY)
	/*
	 * before relocating, we have to setup RAM timing
	 * because memory timing is board-dependend, you will
	 * find a lowlevel_init.S in your board directory.
	 */
	mov	ip, lr

	bl	lowlevel_init
	mov	lr, ip
#endif
	mov	pc, lr
#endif /* CONFIG_IS_ENABLED(SKIP_LOWLEVEL_INIT) */

#if CONFIG_POSITION_INDEPENDENT
_rel_dyn_start_ofs:
    .word   __rel_dyn_start - pie_fixup
_rel_dyn_end_ofs:
    .word   __rel_dyn_end - pie_fixup
#endif

