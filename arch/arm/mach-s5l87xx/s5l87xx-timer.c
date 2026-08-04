/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2026 The freemyipod team (freemyipod.org)
 */

#include <init.h>
#include <asm/io.h>
#include <asm/arch-s5l87xx/s5l87xx.h>
#include <asm/arch-s5l87xx/s5l87xx-clk.h>

struct s5l87xx_timer {
    uint32_t con;     // 0x00
    uint32_t cmd;     // 0x04
    uint32_t data0;   // 0x08
    uint32_t data1;   // 0x0c
    uint32_t pre;     // 0x10
    uint32_t cnt;     // 0x14
};

enum s5l87xx_timer_id {
    //  Timers A, B, C, D: 16-bit
    S5L87XX_TIMER_A = 0,
    S5L87XX_TIMER_B = 1,
    S5L87XX_TIMER_C = 2,
    S5L87XX_TIMER_D = 3,
    // Timer E: 64-bit (unimplemented, different registers from others)
    S5L87XX_TIMER_E = 4,
    // Timers F, G, H, I: 32-bit
    S5L87XX_TIMER_F = 5,
    S5L87XX_TIMER_G = 6,
    S5L87XX_TIMER_H = 7,
    S5L87XX_TIMER_I = 8,
};

#define SYSTEM_TIMER S5L87XX_TIMER_F

enum s5l87xx_timer_cmd {
    S5L87XX_TIMER_CMD_STOP = 0,
    S5L87XX_TIMER_CMD_START = 1,
    S5L87XX_TIMER_CMD_CLR = 2,
};

static struct s5l87xx_timer *s5l87xx_timer_registers(enum s5l87xx_timer_id id) {
    switch (id) {
    case S5L87XX_TIMER_A:
        return (struct s5l87xx_timer *)S5L87XX_TIMER_BASE;
    case S5L87XX_TIMER_B:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x20);
    case S5L87XX_TIMER_C:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x40);
    case S5L87XX_TIMER_D:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x60);
    case S5L87XX_TIMER_E:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x80);
    case S5L87XX_TIMER_F:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0xa0);
    case S5L87XX_TIMER_G:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0xc0);
    case S5L87XX_TIMER_H:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0xe0);
    case S5L87XX_TIMER_I:
        return (struct s5l87xx_timer *)(S5L87XX_TIMER_BASE + 0x100);
    default:
        panic("requested invalid timer id %d", id);
    }
}

static const char* s5l87xx_timer_clockgate(enum s5l87xx_timer_id id) {
#if IS_ENABLED(CONFIG_S5L8702)
    /* S5L8701 and S5L8702 gate all timers behind a single clock gate. */
    return "timer";
#else
    switch (id) {
    case S5L87XX_TIMER_A:
        return "timer0";
    case S5L87XX_TIMER_B:
        return "timer1";
    case S5L87XX_TIMER_C:
        return "timer2";
    case S5L87XX_TIMER_D:
        return "timer3";
    case S5L87XX_TIMER_E:
        return "timer4";
    case S5L87XX_TIMER_F:
        return "timer5";
    case S5L87XX_TIMER_G:
        return "timer6";
    case S5L87XX_TIMER_H:
        return "timer7";
    case S5L87XX_TIMER_I:
        return "timer8";
    default:
        panic("requested invalid timer id %d", id);
    }
#endif
}

static void s5l87xx_timer_configure_interval(enum s5l87xx_timer_id id) {
    log_debug("s5l87xx_timer: configuring %d in interval mode\n", id);
    s5l87xx_enable_clkgate(s5l87xx_timer_clockgate(id));

    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);

	writel(S5L87XX_TIMER_CMD_STOP, &timer->cmd);
	writel(0x40, &timer->con);
	writel(0xb, &timer->pre);
	writel(0xffffffff, &timer->data0);
	writel(S5L87XX_TIMER_CMD_CLR, &timer->cmd);
}

static void s5l87xx_timer_start(enum s5l87xx_timer_id id) {
    log_debug("s5l87xx_timer: starting %d\n", id);
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
	writel(S5L87XX_TIMER_CMD_START, &timer->cmd);
}

// unused
#if 0
static void s5l87xx_timer_stop(enum s5l87xx_timer_id id) {
    log_debug("s5l87xx_timer: stopping %d\n", id);
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
	writel(S5L87XX_TIMER_CMD_STOP, &timer->cmd);
}
#endif

static uint32_t s5l87xx_timer_read(enum s5l87xx_timer_id id) {
    volatile struct s5l87xx_timer *timer = s5l87xx_timer_registers(id);
    return readl(&timer->cnt);
}

int timer_init(void)
{
    s5l87xx_timer_configure_interval(SYSTEM_TIMER);
    s5l87xx_timer_start(SYSTEM_TIMER);

    return 0;
}

unsigned long timer_read_counter(void)
{
    return s5l87xx_timer_read(SYSTEM_TIMER);
}
