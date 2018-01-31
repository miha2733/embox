/**
 * @file imx.c
 * @brief Power management for i.MX6 boards
 * @author Denis Deryugin <deryugin.denis@gmail.com>
 * @version
 * @date 31.01.2018
 */

#include <drivers/common/memory.h>
#include <drivers/power/imx.h>
#include <drivers/serial/uart_device.h>
#include <drivers/serial/diag_serial.h>
#include <embox/unit.h>
#include <framework/mod/options.h>
#include <hal/reg.h>
#include <util/log.h>

EMBOX_UNIT_INIT(imx_power_init);

#define ANADIG_BASE		OPTION_GET(NUMBER,anadig_base)
#define ANADIG_CORE		(ANADIG_BASE + 0x30)
#define REG_SET			0x4
#define REG_CLR			0x8

#define GPC_BASE		OPTION_GET(NUMBER,gpc_base)

#define GPC_CNTR		(GPC_BASE + 0x000)
# define GPU_VPU_PUP_REQ	(1 << 1)
# define GPU_VPU_PDN_REQ	(1 << 0)
#define GPC_IMR(n)		(GPC_BASE + 0x008 + 4 * (n))

#define PGC_GPU_CTRL		(GPC_BASE + 0x260)

#define PGC_CPU_PUPSCR		(GPC_BASE + 0x2A4)
#define PGC_SW2ISO_OFFT		8
#define PGC_SW2ISO_MASK		(0x3F << PGC_SW2ISO_OFFT)
#define PGC_SW_MASK		0x3F

#define PGC_CPU_PDNSCR		(GPC_BASE + 0x2A8)
#define PGC_ISO2SW_OFFT		8
#define PGC_ISO2SW_MASK		(0x3F << PGC_ISO2SW_OFFT)
#define PGC_ISO_MASK		0x3F

#define IMR_NUM			4

static int imx_power_init(void) {
	int i;
	uint32_t temp;

	/* Default values */
	const uint32_t sw = 0xf;
	const uint32_t sw2iso = 0xf;
	const uint32_t iso = 0x1;
	const uint32_t iso2sw = 0x1;


	for (i = 0; i < IMR_NUM; i++) {
		REG32_STORE(GPC_IMR(i), 0xFFFFFFFF);
	}

	temp = REG32_LOAD(PGC_CPU_PUPSCR);
	temp &= ~(PGC_SW2ISO_MASK | PGC_SW_MASK);
	temp |= sw | (sw2iso << PGC_SW2ISO_OFFT);
	REG32_STORE(PGC_CPU_PUPSCR, temp);

	temp = REG32_LOAD(PGC_CPU_PDNSCR);
	temp &= ~(PGC_ISO2SW_MASK | PGC_ISO_MASK);
	temp |= iso | (iso2sw << PGC_ISO2SW_OFFT);
	REG32_STORE(PGC_CPU_PDNSCR, temp);

	return 0;
}

#define ANADIG_REG_TARG_MASK	0x1f
#define ANADIG_REG1_TARG_SHIFT	9	/* VDDPU */
#define ANADIG_REG2_TARG_SHIFT	18	/* VDDSOC */

static void imx_anatop_pu_enable(int enable) {
	uint32_t tmp;
	int t = 0xfffff;
	tmp = REG32_LOAD(ANADIG_CORE);
	tmp &= ANADIG_REG_TARG_MASK << ANADIG_REG2_TARG_SHIFT;
	if (((tmp >> (ANADIG_REG2_TARG_SHIFT)) & ANADIG_REG_TARG_MASK) == ANADIG_REG_TARG_MASK) {
		if (enable) {
			REG32_STORE(ANADIG_CORE + REG_SET,
					ANADIG_REG_TARG_MASK << ANADIG_REG1_TARG_SHIFT);
			while(t--);
		} else {
			REG32_STORE(ANADIG_CORE + REG_CLR,
					ANADIG_REG_TARG_MASK << ANADIG_REG1_TARG_SHIFT);
		}
	}
}

extern int clk_enable(char *clk_name);
extern int clk_disable(char *clk_name);
static void imx_power_clk(int enable) {
	if (enable) {
		clk_enable("gpu3d");
		clk_enable("gpu2d");
		clk_enable("openvg");
		clk_enable("vpu");
	} else {
		clk_disable("gpu3d");
		clk_disable("gpu2d");
		clk_disable("openvg");
		clk_disable("vpu");
	}
}

void imx_gpu_power_set(int up) {
	int t = 0xfffff;
	if (up) {
		/* Power-up */
		imx_anatop_pu_enable(1);
		/* TODO calculate delay? */
		while(t--);

		imx_power_clk(1);
		REG32_STORE(PGC_GPU_CTRL, 1);
		REG32_STORE(GPC_CNTR, GPU_VPU_PUP_REQ);

		while (REG32_LOAD(GPC_CNTR) & GPU_VPU_PUP_REQ);
		imx_power_clk(0);
	} else {
		/* Power-down */
		REG32_STORE(PGC_GPU_CTRL, 1);
		REG32_STORE(GPC_CNTR, GPU_VPU_PDN_REQ);

		while (REG32_LOAD(GPC_CNTR) & GPU_VPU_PDN_REQ);

		while(t--);
		imx_anatop_pu_enable(0);
	}
}

static struct periph_memory_desc imx_power_mem = {
	.start = GPC_BASE,
	.len   = 0x2B0,
};

PERIPH_MEMORY_DEFINE(imx_power_mem);

static struct periph_memory_desc imx_anadig_mem = {
	.start = ANADIG_BASE,
	.len   = 0x100,
};

PERIPH_MEMORY_DEFINE(imx_anadig_mem);
