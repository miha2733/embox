/**
 * @file
 *
 * @date 31.03.2017
 * @author Anton Bondarev
 */
#include <stdint.h>
#include <string.h>

#include <drivers/common/memory.h>

#include <hal/reg.h>

#include <util/array.h>
#include <util/log.h>

#include <framework/mod/options.h>

#define CCM_BASE           OPTION_GET(NUMBER, base_addr)

#define MXC_CCM_CCR         (CCM_BASE + 0x00)
#define MXC_CCM_CCDR        (CCM_BASE + 0x04)
#define MXC_CCM_CSR         (CCM_BASE + 0x08)
#define MXC_CCM_CCSR        (CCM_BASE + 0x0c)
#define MXC_CCM_CACRR       (CCM_BASE + 0x10)
#define MXC_CCM_CBCDR       (CCM_BASE + 0x14)
#define MXC_CCM_CBCMR       (CCM_BASE + 0x18)
#define MXC_CCM_CSCMR1      (CCM_BASE + 0x1c)
#define MXC_CCM_CSCMR2      (CCM_BASE + 0x20)
#define MXC_CCM_CSCDR1      (CCM_BASE + 0x24)
#define MXC_CCM_CS1CDR      (CCM_BASE + 0x28)
#define MXC_CCM_CS2CDR      (CCM_BASE + 0x2c)
#define MXC_CCM_CDCDR       (CCM_BASE + 0x30)
#define MXC_CCM_CHSCDR      (CCM_BASE + 0x34)
#define MXC_CCM_CSCDR2      (CCM_BASE + 0x38)
#define MXC_CCM_CSCDR3      (CCM_BASE + 0x3c)

//#define MXC_CCM_CSCDR4     (CCM_BASE + 0x40)
#define MXC_CCM_CWDR        (CCM_BASE + 0x44)
#define MXC_CCM_CDHIPR      (CCM_BASE + 0x48)
//#define MXC_CCM_CDCR       (CCM_BASE + 0x4c)
//#define MXC_CCM_CTOR       (CCM_BASE + 0x50)
//#define MXC_CCM_CLPCR      (CCM_BASE + 0x54)
#define MXC_CCM_CISR        (CCM_BASE + 0x58)
#define MXC_CCM_CIMR        (CCM_BASE + 0x5c)
#define MXC_CCM_CCOSR       (CCM_BASE + 0x60)
#define MXC_CCM_CGPR        (CCM_BASE + 0x64)
#define MXC_CCM_CCGR0       (CCM_BASE + 0x68)
#define MXC_CCM_CCGR1       (CCM_BASE + 0x6c)
#define MXC_CCM_CCGR2       (CCM_BASE + 0x70)
#define MXC_CCM_CCGR3       (CCM_BASE + 0x74)
#define MXC_CCM_CCGR4       (CCM_BASE + 0x78)
#define MXC_CCM_CCGR5       (CCM_BASE + 0x7c)
#define MXC_CCM_CCGR6       (CCM_BASE + 0x80)
//#define MXC_CCM_CCGR7      (CCM_BASE + 0x84)
#define MXC_CCM_CMEOR       (CCM_BASE + 0x84)


#define MXC_CCM_CHSCCDR_IPU1_DI1_PRE_CLK_SEL_MASK   (0x7 << 15)
#define MXC_CCM_CHSCCDR_IPU1_DI1_PRE_CLK_SEL_OFFSET 15
#define MXC_CCM_CHSCCDR_IPU1_DI1_PODF_MASK          (0x7 << 12)
#define MXC_CCM_CHSCCDR_IPU1_DI1_PODF_OFFSET        12
#define MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_MASK       (0x7 << 9)
#define MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET     9
#define MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_MASK   (0x7 << 6)
#define MXC_CCM_CHSCCDR_IPU1_DI0_PRE_CLK_SEL_OFFSET 6
#define MXC_CCM_CHSCCDR_IPU1_DI0_PODF_MASK          (0x7 << 3)
#define MXC_CCM_CHSCCDR_IPU1_DI0_PODF_OFFSET        3
#define MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_MASK       (0x7)
#define MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET     0

#define MXC_CCM_CHSCCDR_EPDC_PRE_CLK_SEL_MASK       (0x7 << 15)
#define MXC_CCM_CHSCCDR_EPDC_PRE_CLK_SEL_OFFSET     15
#define MXC_CCM_CHSCCDR_EPDC_PODF_MASK              (0x7 << 12)
#define MXC_CCM_CHSCCDR_EPDC_PODF_OFFSET            12
#define MXC_CCM_CHSCCDR_EPDC_CLK_SEL_MASK           (0x7 << 9)
#define MXC_CCM_CHSCCDR_EPDC_CLK_SEL_OFFSET         9

#define CHSCCDR_CLK_SEL_LDB_DI0                     3
#define CHSCCDR_PODF_DIVIDE_BY_3                    2
#define CHSCCDR_IPU_PRE_CLK_540M_PFD                5

struct clk {
	uint32_t reg_offset;
	uint32_t bit_num;
	const char *clk_name;
};

/* TODO pass clock mode as parameter?
 *
 * In registers binary values will be as follows (from manual):
 *
 * 00 Clock is off during all modes. Stop enter hardware handshake is disabled.
 * 01 Clock is on in run mode, but off in WAIT and STOP modes
 * 10 Not applicable (Reserved).
 * 11 Clock is on during all modes, except STOP mode.
 *
 * Usually we just use 11 (i.e. 3 << offt).
 */

static const struct clk clks_repo[] = {
	{ MXC_CCM_CCGR0, 3 << 24, "dcic1" },
	{ MXC_CCM_CCGR0, 3 << 26, "dcic2" },
	{ MXC_CCM_CCGR1, 3 << 24, "gpu2d" },
	{ MXC_CCM_CCGR1, 3 << 26, "gpu3d" },
	{ MXC_CCM_CCGR2, 1,       "iahb" },
	{ MXC_CCM_CCGR2, 3 << 4,  "isfr" },
	{ MXC_CCM_CCGR2, 0x33,    "hdmi" },
	{ MXC_CCM_CCGR3, 3 << 0,  "ipu1" },
	{ MXC_CCM_CCGR3, 3 << 2,  "ipu1_di0" },
	{ MXC_CCM_CCGR3, 3 << 4,  "ipu1_di1" },
	{ MXC_CCM_CCGR3, 3 << 6,  "ipu2" },
	{ MXC_CCM_CCGR3, 3 << 8,  "ipu2_di0" },
	{ MXC_CCM_CCGR3, 3 << 10, "ipu2_di1" },
	{ MXC_CCM_CCGR3, 3 << 12, "ldb_di0" },
	{ MXC_CCM_CCGR3, 3 << 14, "ldb_di1" },
	{ MXC_CCM_CCGR3, 3 << 30, "openvg" },
};

int clk_enable(char *clk_name) {
	int i;
	log_debug("%s", clk_name);
	for (i = 0; i < ARRAY_SIZE(clks_repo); i ++) {
		if (0 == strcmp(clks_repo[i].clk_name, clk_name)) {
			REG32_ORIN(clks_repo[i].reg_offset, clks_repo[i].bit_num);
			while(REG32_LOAD(MXC_CCM_CDHIPR));

			if (!strcmp(clk_name, "ldb_di0")) {
				uint32_t reg = REG32_LOAD(MXC_CCM_CHSCDR);
				reg &= ~((7 << 6) | (7 << 3) | 7);
				reg |= (2 << 3);
				reg |= (5 << 6);
				REG32_STORE(MXC_CCM_CHSCDR, reg);
				reg |= (3 << 0);
				REG32_STORE(MXC_CCM_CHSCDR, reg);
			}

			return i;
		}
	}

	return -1;
}

int clk_disable(char *clk_name) {
	int i;

	log_debug("%s", clk_name);
	for (i = 0; i < ARRAY_SIZE(clks_repo); i ++) {
		if (0 == strcmp(clks_repo[i].clk_name, clk_name)) {
			REG32_CLEAR(clks_repo[i].reg_offset, clks_repo[i].bit_num);
			while(REG32_LOAD(MXC_CCM_CDHIPR));
			return i;
		}
	}

	return -1;
}

void clk_reg_dump(void) {
	log_debug("=================== REGISTER DUMP =====================");
	log_debug("MXC_CCM_CCR   =0x%08x", REG32_LOAD(MXC_CCM_CCR));
	log_debug("MXC_CCM_CCDR  =0x%08x", REG32_LOAD(MXC_CCM_CCDR));
	log_debug("MXC_CCM_CSR   =0x%08x", REG32_LOAD(MXC_CCM_CSR));
	log_debug("MXC_CCM_CCOSR =0x%08x", REG32_LOAD(MXC_CCM_CCOSR));
	log_debug("MXC_CCM_CCGR0 =0x%08x", REG32_LOAD(MXC_CCM_CCGR0));
	log_debug("MXC_CCM_CCGR1 =0x%08x", REG32_LOAD(MXC_CCM_CCGR1));
	log_debug("MXC_CCM_CCGR2 =0x%08x", REG32_LOAD(MXC_CCM_CCGR2));
	log_debug("MXC_CCM_CCGR3 =0x%08x", REG32_LOAD(MXC_CCM_CCGR3));
	log_debug("MXC_CCM_CCGR4 =0x%08x", REG32_LOAD(MXC_CCM_CCGR4));
	log_debug("MXC_CCM_CCGR5 =0x%08x", REG32_LOAD(MXC_CCM_CCGR5));
	log_debug("MXC_CCM_CCGR6 =0x%08x", REG32_LOAD(MXC_CCM_CCGR6));
	log_debug("================= REGISTER DUMP END ===================");


	for (int i = 0; i < 0x88 / 4 / 4; i++) {
		log_debug("0x%08x 0x%08x 0x%08x 0x%08x",
				REG32_LOAD(CCM_BASE + i * 4 * 4),
				REG32_LOAD(CCM_BASE + i * 4 * 4 + 4),
				REG32_LOAD(CCM_BASE + i * 4 * 4 + 8),
				REG32_LOAD(CCM_BASE + i * 4 * 4 + 12));

	}

	log_debug("");
	for (int i = 0x20C8000; i <= 0x20C817C; i+=4) {
		log_debug("%p =0x%08x", i, REG32_LOAD(i));
	}




	static uint32_t vals[] = {
		0x040110ff, 0x00000000, 0x00000038, 0x00000100,
		0x00000000, 0x00018d00, 0x00010204, 0x0090a800,
		0x02a12f06, 0x00490b00, 0x0ec102c1, 0x007312c1,
		0x33e71f92, 0x00012093, 0x00012090, 0x00010841,
		0x00010241, 0x00000000, 0x00000000, 0x00000003,
		0x00000000, 0x00000078, 0x041a0000, 0xffffffff,
		0x000e0101, 0x0000fe62, 0x0040000f, 0x00300000,
		0x01ff0000, 0x3770300f, 0x000cf303, 0x00000003,
	};
	return;
	for (int i = 0; i < 0x88 / 4; i++) {
		if (i != 0x7c / 4)
		REG32_STORE(CCM_BASE + i * 4, vals[i]);
		log_debug("setup 0x%x\n", i * 4);
	}
}

static struct periph_memory_desc ccm_mem = {
	.start = CCM_BASE,
	.len   = 0x100,
};

PERIPH_MEMORY_DEFINE(ccm_mem);
