#include "timer.h"

_go32_dpmi_seginfo old_irq_timer;

void timer_enable()
{
	timer_init(TIMER_2, LCOM_MODE);
	timer_init(TIMER_0, LCOM_MODE);
	timer_load(TIMER_0, TIMER_CLK/1000);
	
	disable_irq(T0_IRQ);
	install_asm_irq_handler(T0_IRQ, t0_isr, &old_irq_timer);
	enable_irq(T0_IRQ); /* serve para fazer enable do timer 0 */
}

void timer_disable()
{
	reinstall_asm_irq_handler(T0_IRQ, &old_irq_timer);
}

void timer_init(int timer, int mode)
{
	switch (timer)
	{
		case TIMER_0:
			break;
		case TIMER_1:
			mode |= 0x40; /* bit 6 a 1 */
			break;
		default:
			mode |= 0x80; /* bit 7 a 1 */
			break;
	}
	
	outportb(TIMER_CTRL, mode);
}

void timer_load(int timer, int value)
{
	outportb(timer, LSB(value));
	outportb(timer, MSB(value));
}

void mili_sleep(int mili)
{
	int inic = milis;
	while ((milis - inic) < mili)
	{
		/* HLT; */
	}
}


int get_divider(float freq)
{
	if(!freq) return -1;
	return (int)(TIMER_CLK / freq + 0.5);
}

void sec_sleep(int secs)
{
	mili_sleep(secs*1000);
}
