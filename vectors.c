#include <stdint.h>
#include <stdlib.h>
#include "common.h"

#define STACK_TOP 0x20002000

void entrypoint(void);

void nmi_isr(void)
{
	for (;;) ;
}

void hardfault_isr(void)
{
	puts("!HARDFAULT!\n");
	printf("%08x %08x %08x\n",
	       *(__IO u32 *) 0xe000ed04,
	       *(__IO u32 *) 0xe000ed28, *(__IO u32 *) 0xe000ed2c);
	for (;;) ;
}

void memmanage_isr(void)
{
	for (;;) ;
}

void busfault_isr(void)
{
	for (;;) ;
}

void usagefault_isr(void)
{
	for (;;) ;
}

void default_isr(void)
{
	for (;;) ;
}

void svc_isr(void)
{
	for (;;) ;
}

void debugmon_isr(void)
{
	for (;;) ;
}

void pendsv_isr(void)
{
	for (;;) ;
}

void systick_isr(void);

void *_vectors[45] __attribute__ ((section("vectors"))) = {
	(void *)STACK_TOP,
	(void *)entrypoint,
	(void *)nmi_isr,
	(void *)hardfault_isr,
	(void *)memmanage_isr,
	(void *)busfault_isr,
	(void *)usagefault_isr,
	(void *)default_isr,
	(void *)default_isr,
	(void *)default_isr,
	(void *)default_isr,
	(void *)svc_isr,
	(void *)debugmon_isr,
	(void *)default_isr, (void *)pendsv_isr, (void *)systick_isr,
};
