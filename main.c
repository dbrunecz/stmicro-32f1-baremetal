/* Copyright (C) 2020 David Brunecz. Subject to GPL 2.0 */

//#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"

struct rcc {
	__IO u32 cr;
	__IO u32 cfgr;
	__IO u32 cir;
	__IO u32 apb2rstr;
	__IO u32 apb1rstr;
	__IO u32 ahbenr;
	__IO u32 apb2enr;
	__IO u32 apb1enr;
	__IO u32 bdcr;
	__IO u32 csr;
	__IO u32 cfgr2;
};

struct gpio {
	__IO u32 cr[2];
	__IO u16 idr;
	u16 reserved1;
	__IO u16 odr;
	u16 reserved2;
	__IO u16 bsrr;
	__IO u32 bsr;
	__IO u32 lckr;
};

struct usart {
	__IO u32 sr;
	__IO u32 dr;
	__IO u32 brr;
	__IO u32 cr1;
	__IO u32 cr2;
	__IO u32 cr3;
	__IO u16 gtpr;
};

struct systick {
	__IO u32 csr;
	__IO u32 rvr;
	__IO u32 cvr;
	__IO u32 calib;
};

struct scb {
	__IO u32 cpuid;
	__IO u32 icsr;
	__IO u32 vtor;
	__IO u32 aircr;
	__IO u32 scr;
	__IO u32 ccr;
	__IO u32 shpr1;
	__IO u32 shpr2;
	__IO u32 shpr3;
	__IO u32 shcsr;
	__IO u32 cfsr;
	__IO u32 hfsr;
};

struct nvic {
	__IO u32 iser[16];
	__IO u32 icer[16];
	__IO u32 ispr[16];
	__IO u32 icpr[16];
	__IO u32 iabrr[16];
	__IO u32 rsvd1[16];
	__IO u32 ipr[128];
};

// PA4
struct dac {
	__IO u32 cr;
	__IO u32 swtrigr;
	__IO u32 dhr12r1;
	__IO u32 dhr12l1;
	__IO u32 dhr8r1;
	__IO u32 dhr12r2;
	__IO u32 dhr12l2;
	__IO u32 dhr8r2;
	__IO u32 dhr12rd;
	__IO u32 dhr12ld;
	__IO u32 dhr8rd;
	__IO u32 dor1;
	__IO u32 dor2;
	__IO u32 sr;
};

#define PERIPH_BASE             0x40000000
#define DAC                     (PERIPH_BASE + 0x07400)
#define USART1_BASE             (PERIPH_BASE + 0x13800)
#define GPIOA_BASE              (PERIPH_BASE + 0x10800)
#define GPIOC_BASE              (PERIPH_BASE + 0x11000)
#define RCC_BASE                (PERIPH_BASE + 0x21000)
#define SYSTICK                 0xe000e010
#define NVIC                    0xe000e100
#define SCB                     0xe000ed00

#define USART1_TX_GPIOPIN       9
#define USART1_RX_GPIOPIN       10

static char *prompt = "> ";
static char *banner = "\n"
    "________________________________________"
    "________________________________________\n\n";

struct systick *systick;
struct usart *usart1;
struct gpio *gpioa;
struct gpio *gpioc;
struct nvic *nvic;
struct rcc *rcc;
struct scb *scb;
struct dac *dac;

u64 uptime_x_250ns = 0;

void setbits(volatile u32 * v, u32 offs, u32 mask, u32 val)
{
	u32 t = *v;

	t &= ~(mask << offs);
	t |= ((val & mask) << offs);
	*v = t;
}

int uart_putc(int c, FILE * stream)
{
	for (; !(usart1->sr & BIT(7));) ;
	usart1->dr = c & 0xff;
	return 0;
}

int uart_getc(FILE * stream)
{
	return (usart1->sr & BIT(5)) ? usart1->dr & 0xff : -1;
}

static int w32(int argc, char *argv[]);
static int r32(int argc, char *argv[]);
static int uptime(int argc, char *argv[]);
static int test(int argc, char *argv[]);
static int list_commands(int argc, char *argv[]);

struct command {
	const char *name;
	int (*handler) (int, char **);
} commands[] = {
	{
	"r32", r32}, {
	"w32", w32}, {
	"uptime", uptime}, {
	"test", test}, {
"?", list_commands},};

static int w32(int argc, char *argv[])
{
	u32 addr, val;

	if (argc != 3) {
		printf("error %s\n", __func__);
		return -1;
	}

	addr = strtoul(argv[1], NULL, 0);
	val = strtoul(argv[2], NULL, 0);
	*((__IO u32 *) addr) = val;

	printf("W %08x %08x\n", addr, val);
	return 0;
}

static int r32(int argc, char *argv[])
{
	u32 addr, val;

	if (argc != 2) {
		printf("error %s\n", __func__);
		return -1;
	}

	addr = strtoul(argv[1], NULL, 0);
	val = *((__IO u32 *) addr);

	printf("R %08x %08x\n", addr, val);
	return 0;
}

static void divmod(u64 n, u64 d, u64 * div, u64 * mod)
{
	u32 i, tmp;

	for (*div = 0, *mod = 0; n;) {
		for (i = 0, tmp = d; tmp < n; tmp <<= 1, i++) ;
		if (!i) {
			if (tmp == n)
				(*div)++;
			else
				*mod = n;
			return;
		}
		if (tmp > n)
			i--;
		*div += (1 << i);
		n -= (d << i);
	}
}

static int uptime(int argc, char *argv[])
{
	u32 ms, s, m, h;
	u64 div, mod;
	u64 t = uptime_x_250ns + (BIT(24) - systick->cvr);

	t >>= 2;		/* 250ns => 1us */

	divmod(t, 1000, &div, &mod);
	t = div;

	divmod(t, 1000, &div, &mod);
	t = div;
	ms = mod;

	divmod(t, 60, &div, &mod);
	t = div;
	s = mod;

	divmod(t, 60, &div, &mod);
	t = div;
	m = mod;

	h = t;
	printf("%d:%02d:%02d.%03d\n", h, m, s, ms);
	return 0;
}

static int test(int argc, char *argv[])
{
	printf("systick: %08x %08x %08x %08x\n",
	       systick->csr, systick->rvr, systick->cvr, systick->calib);
	return 0;
}

static int list_commands(int argc, char *argv[])
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		printf("%s\n", commands[i].name);
	return 0;
}

#define HISTORY_DIM 10
char history[HISTORY_DIM][80];
int h_idx;
int h_first;
int h_ptr;

static int circular_buffer_prev(int current, int dim)
{
	if (current == 0)
		return dim - 1;
	return (current - 1) % dim;
}

static int circular_buffer_next(int current, int dim)
{
	return (current + 1) % dim;
}

static void blankline(void)
{
	printf
	    ("\r                                                          \r> ");
}

static char *history_lookup(int prev)
{
	if (h_idx == 0)
		return NULL;

	if (h_ptr < 0) {
		h_ptr = (h_idx - 1) % HISTORY_DIM;
	} else {
		if (prev && (h_ptr != h_first))
			h_ptr = circular_buffer_prev(h_ptr, HISTORY_DIM);
		else if (!prev && (h_ptr != h_idx))
			h_ptr = circular_buffer_next(h_ptr, HISTORY_DIM);
	}

	blankline();
	if (h_ptr != h_idx)
		printf("%s", history[h_ptr]);
	return (h_ptr != h_idx) ? history[h_ptr] : "";
}

static int update_history(char *s)
{
	int i, h_cnt = MIN(h_idx, HISTORY_DIM);

	h_ptr = -1;

	for (i = 0; i < h_cnt; i++)
		if (!strcmp(history[i], s))
			return 0;

	strcpy(history[h_idx++ % HISTORY_DIM], s);
	if (h_idx >= HISTORY_DIM)
		h_first = circular_buffer_next(h_idx, HISTORY_DIM);
	return 0;
}

#define MAX_CMD_PARAMS  5
static void handle_command(char *s)
{
	char *p, *argv[MAX_CMD_PARAMS];
	size_t len;
	int i, argc;

	if (!*s)
		return;

	update_history(s);

	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		len = strlen(commands[i].name);
		if (!strncmp(commands[i].name, s, len)
		    && (!s[len] || isspace(s[len])))
			break;
	}

	if (i == ARRAY_SIZE(commands)) {
		printf("unknown command: %s\n", s);
		return;
	}

	for (argc = 0, p = s; *p && argc < MAX_CMD_PARAMS; p++) {
		for (; *p && isspace(*p); p++) ;
		if (!p)
			break;
		argv[argc++] = p;
		for (; *p && !isspace(*p); p++) ;
		if (!*p)
			break;
		*p = '\0';
	}

	commands[i].handler(argc, argv);
}

#define ESC_UP      'A'
#define ESC_DN      'B'
#define ESC_RT      'C'
#define ESC_LF      'D'
static int check_escape_code(int c)
{
	static int state;

	switch (state) {
	case 0:
		if (c == 0x1b) {
			state++;
			return 1;
		}
		break;
	case 1:
		if (c == '[') {
			state++;
			return 1;
		}
		break;
	case 2:
		if (c >= 'A' && c <= 'D') {
			state = 0;
			return c;
		}
		break;
	}
	state = 0;
	return 0;
}

static char *handle_escape_code(int e)
{
	char *s = NULL;

	switch (e) {
	case ESC_UP:
	case ESC_DN:
		s = history_lookup(e == ESC_UP ? 1 : 0);
		break;
	case ESC_RT:
		break;
	case ESC_LF:
		break;
	}
	return s;
}

static void main_loop(void)
{
	int i, c, e;
	char *s, buf[81];

	h_idx = h_first = 0;
	puts(banner);
	puts(prompt);

	for (i = 0;;) {
		c = getc(0);
		if (c < 0)
			continue;

		if (c == '\b' || c == 0x7f) {
			if (i) {
				putc('\b', stdout);
				putc(' ', stdout);
				putc('\b', stdout);
				buf[--i] = 0;
			}
			continue;
		}

		if ((e = check_escape_code(c))) {
			s = handle_escape_code(e);
			if (s) {
				strcpy(buf, s);
				i = strlen(s);
			}
			continue;
		}

		if (isprint(c)) {
			putc(c, stdout);
			buf[i] = c;
		}

		if (c == '\n') {
			buf[i] = 0;
			i = 0;
			handle_command(buf);
			puts(prompt);
		} else {
			if (i++ > 79)
				i = 0;
		}
	}
}

#define GPIO_CR(x)          (x / 8)
#define GPIO_CR_OFFS(x)     ((x % 8) << 2)

#define GPIO_OUT_50MHZ      3
#define GPIO_OUT_10MHZ      1
#define GPIO_IN             0
#define GPIO_PP             0
#define GPIO_AF_PP          2

static void gpio_config(struct gpio *g, int pin, int mode, int config)
{
	setbits(&g->cr[GPIO_CR(pin)], GPIO_CR_OFFS(pin), 0xf,
		(mode & 3) | ((config & 3) << 2));
}

#define MCO_PIN     8
static void pll_init(void)
{
	gpio_config(gpioa, MCO_PIN, GPIO_OUT_50MHZ, GPIO_AF_PP);
	setbits(&rcc->cfgr, 24, 0x7, 0x4);	/* MCO sysclk */

	setbits(&rcc->cfgr, 18, 0xf, 0x4);	/* PLL = clock x 6 */
	setbits(&rcc->cr, 24, 0x1, 0x1);	/* PLL on */
	for (; !(rcc->cr & BIT(25));) ;

	setbits(&rcc->cfgr, 0, 0x3, 0x2);	/* PLL sysclk */
}

static void uart_init(void)
{
	gpio_config(gpioa, USART1_TX_GPIOPIN, GPIO_OUT_50MHZ, GPIO_AF_PP);
	gpio_config(gpioa, USART1_RX_GPIOPIN, GPIO_IN, GPIO_AF_PP);

	//24MHz-PLL 115200 => 13.0
	usart1->brr = (13 << 4) | 0;
	usart1->cr1 = BIT(2) | BIT(3) | BIT(13);
}

void entrypoint(void)
{
	usart1 = (struct usart *)USART1_BASE;
	gpioa = (struct gpio *)GPIOA_BASE;
	gpioc = (struct gpio *)GPIOC_BASE;
	rcc = (struct rcc *)RCC_BASE;
	systick = (struct systick *)SYSTICK;
	nvic = (struct nvic *)NVIC;
	scb = (struct scb *)SCB;
	dac = (struct dac *)DAC;

	rcc->apb2enr |= BIT(0);	/* AF en */
	rcc->apb2enr |= BIT(2) | BIT(4);	/* port a,c en */
	rcc->apb2enr |= BIT(14);	/* usart en */
	rcc->apb1enr |= BIT(29);	/* dac en */

	/*
	 *          HSI = 16MHz
	 *          PLL = HSI / 2 (8MHz)
	 * SYSCLK(HCLK) = PLL * 6 (24MHz)
	 *      SysTick = HCLK / 8 (4MHz)
	 */
	pll_init();

	systick->rvr = BIT(24) - 1;	/* (16 * 1024 * 1024) / 4 MHz = ~4 sec */
	systick->csr |= (BIT(1) | BIT(0));
	systick->csr |= BIT(0);

	uptime_x_250ns = 0;

	uart_init();

	gpio_config(gpioc, 7, GPIO_OUT_10MHZ, GPIO_PP);

	gpio_config(gpioa, 4, 0, 0);	/* dac analog in */
	dac->cr |= 1;

	main_loop();
}

void systick_isr(void)
{
	static u32 i;

	if (i++ & 1) {
		gpioc->odr |= BIT(7);
		dac->dhr12r1 = 0;
	} else {
		gpioc->odr &= ~BIT(7);
		dac->dhr12r1 = 0xfff;
	}
	uptime_x_250ns += BIT(24);
}
