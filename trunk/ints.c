#include "ints.h"
#include <go32.h>
#include <dpmi.h>

int install_asm_irq_handler(int irq, void (*irq_func)(void), _go32_dpmi_seginfo *old_irq)
{
	_go32_dpmi_seginfo new_vec;//
	
	_go32_dpmi_get_protected_mode_interrupt_vector(IRQ_VECTOR(irq), old_irq);
	new_vec.pm_selector = _go32_my_cs( );
	new_vec.pm_offset = (unsigned long) irq_func;
	return _go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &new_vec);
}

/** Reinstalls the interrupt handler specified in 'old_irq' for interrupt
 * 'irq'. See install_c_irq_handler().
 */
void reinstall_asm_irq_handler(int irq, _go32_dpmi_seginfo *old_irq)
{
	_go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), old_irq);
}

int install_c_irq_handler(int irq, void (*irq_func)(void), _go32_dpmi_seginfo *old_irq)
{
	_go32_dpmi_seginfo new_vec;
	
	_go32_dpmi_get_protected_mode_interrupt_vector(IRQ_VECTOR(irq), old_irq);
	new_vec.pm_selector = _go32_my_cs( );
	new_vec.pm_offset = (unsigned long) irq_func;
	_go32_dpmi_allocate_iret_wrapper(&new_vec); 
	return _go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &new_vec);
}

/** Reinstalls the interrupt handler specified in 'old_irq' for interrupt
 * 'irq'. See install_c_irq_handler().
 */
void reinstall_c_irq_handler(int irq, _go32_dpmi_seginfo *old_irq)
{
	_go32_dpmi_seginfo new_irq;
	_go32_dpmi_get_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &new_irq);
	
	_go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), old_irq);
	
	_go32_dpmi_free_iret_wrapper(&new_irq);	
}

void disable_irq(int irq) {

	
	if (irq < 8)
		outportb(PIC1_MASK, inportb(PIC1_MASK) | IRQ_MASK_BIT(irq));
	else
		outportb(PIC2_MASK, inportb(PIC2_MASK) | IRQ_MASK_BIT(irq));
}

void enable_irq(int irq) 
{
	if (irq < 8)
		outportb(PIC1_MASK, inportb(PIC1_MASK) &~ IRQ_MASK_BIT(irq));
	else
	{
		outportb(PIC1_MASK, inportb(PIC1_MASK) &~ IRQ_MASK_BIT(2));
		outportb(PIC2_MASK, inportb(PIC2_MASK) &~ IRQ_MASK_BIT(irq));
	}

}

void mask_pic(int irq)
{
	unsigned short pic;
	
	if (irq < 8)
		pic = PIC1_MASK;
	else
		pic = PIC2_MASK;
	
	unsigned char mascara = inportb(pic) | IRQ_MASK_BIT(irq); 
	outportb(pic, mascara); 
}

void unmask_pic(int irq)
{
	unsigned short pic;
	
	if (irq <= 7) 
		pic = PIC1_MASK;
	else
		pic = PIC2_MASK;
		
	unsigned char mascara = inportb(pic) & (~IRQ_MASK_BIT(irq));
	outportb(pic, mascara); 
}
