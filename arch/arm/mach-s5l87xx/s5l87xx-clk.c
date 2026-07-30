#include <init.h>
#include <asm/io.h>
#include <dm/ofnode.h>
#include <asm/arch-s5l87xx/s5l87xx.h>

typedef struct {
    uint32_t pad[18];
    uint32_t gates[10];
    /*
        uint32_t pad0[18]; // 0x00 - 0x47
        uint32_t gate0;    // 0x48
        uint32_t gate1;    // 0x4C
        uint32_t pad1[2];  // 0x50 - 0x57
        uint32_t gate2;    // 0x58
        uint32_t pad2[3];  // 0x5C - 0x67
        uint32_t gate3;    // 0x68
        uint32_t gate4;    // 0x6C
    */
} s5l87xx_clkcon;

#define S5L87XX_CLKCON ((volatile s5l87xx_clkcon *)S5L87XX_CLK_BASE)

/*
 * Sentinel for an unused second clock gate in the device-tree
 * "samsung,clock-gates" property. A real gate is encoded as gate * 32 + bit.
 */
#define S5L87XX_CLKGATE_NONE 0xffffffff

void s5l87xx_enable_clkgate_bit(uint8_t gate, uint8_t bit) {
    uint32_t mask = ~BIT(bit);
#if IS_ENABLED(CONFIG_S5L8701)
    uint32_t value = readl(S5L87XX_PWRCON(gate));
    value &= mask;
    writel(value, S5L87XX_PWRCON(gate));
#else
    uint32_t value = readl(&S5L87XX_CLKCON->gates[gate]);
    value &= mask;
    writel(value, &S5L87XX_CLKCON->gates[gate]);
#endif
}

static void s5l87xx_ungate_encoded(uint32_t encoded) {
    if (encoded == S5L87XX_CLKGATE_NONE)
        return;
    s5l87xx_enable_clkgate_bit(encoded / 32, encoded % 32);
}

/*
 * Ungate a clock by name. The name -> {gate, bit} mapping lives in the device
 * tree under the "samsung,s5l87xx-clkgates" node ("clock-gate-names" paired
 * with "samsung,clock-gates", two cells per gate). Works pre-relocation: it
 * reads the flat tree directly, so callers earlier than the FDT setup (e.g.
 * the debug UART) must use s5l87xx_enable_clkgate_bit() instead.
 */
void s5l87xx_enable_clkgate(const char *id) {
    ofnode node = ofnode_by_compatible(ofnode_null(),
                                       "samsung,s5l87xx-clkgates");
    if (!ofnode_valid(node))
        panic("s5l87xx_enable_clkgate: no clkgates node in device tree");

    int idx = ofnode_stringlist_search(node, "clock-gate-names", id);
    if (idx < 0)
        panic("s5l87xx_enable_clkgate: unknown id %s", id);

    uint32_t gate1, gate2;
    if (ofnode_read_u32_index(node, "samsung,clock-gates", idx * 2, &gate1) ||
        ofnode_read_u32_index(node, "samsung,clock-gates", idx * 2 + 1, &gate2))
        panic("s5l87xx_enable_clkgate: malformed gates for %s", id);

    log_debug("s5l87xx: ungating %s\n", id);
    s5l87xx_ungate_encoded(gate1);
    s5l87xx_ungate_encoded(gate2);
}
