#include "ints.h"
 
 int install_asm_irq_handler(int irq, void (*irq_func)(void),_go32_dpmi_seginfo *old_irq)
{
	_go32_dpmi_get_protected_mode_interrupt_vector(IRQ_VECTOR(irq), old_irq);
	new_vec.pm_selector = _go32_my_cs( );
	new_vec.pm_offset = (unsigned long) irq_func;
	if( _go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &new_vec) == 0 )	
		return 0;
	else
		return 1;
}
 
int install_c_irq_handler(int irq, void (*irq_func)(void), _go32_dpmi_seginfo *old_irq)
{
	_go32_dpmi_get_protected_mode_interrupt_vector(IRQ_VECTOR(irq), old_irq);
	new_vec.pm_selector = _go32_my_cs( );
	new_vec.pm_offset = (unsigned long) irq_func;
	_go32_dpmi_allocate_iret_wrapper(&new_vec);
	_go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), &new_vec);
	
	return 1;
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

/** Reinstalls the interrupt handler specified in 'old_irq' for interrupt
 * 'irq'. See install_c_irq_handler().
 */
 
 void reinstall_asm_irq_handler(int irq, _go32_dpmi_seginfo *old_irq)
{
	_go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), old_irq);
}

void reinstall_c_irq_handler(int irq, _go32_dpmi_seginfo *old_irq)
{
	_go32_dpmi_set_protected_mode_interrupt_vector(IRQ_VECTOR(irq), old_irq);
	_go32_dpmi_free_iret_wrapper(&new_vec);
}


void disable_irq(int irq)
{
	unsigned char pic1 = inportb(PIC1_MASK);
	unsigned char pic2 = inportb(PIC2_MASK);
	
	if (irq < 8)
        //coloca bit irq da máscara do PIC-1 a 1  para inibir as interrupções do dispositivo correspondente
        outportb(PIC1_MASK, pic1 | IRQ_MASK_BIT(irq));
     else 
        //coloca bit irq da máscara do PIC-2 a 1
        outportb(PIC2_MASK, pic2 | IRQ_MASK_BIT(irq));
}

void enable_irq(int irq)
{
	unsigned char pic1 = inportb(PIC1_MASK);
	unsigned char pic2 = inportb(PIC2_MASK);

	if (irq < 8)
        //coloca bit irq da máscara do PIC-1 a 0 para inibir as interrupções do dispositivo correspondente
        outportb(PIC1_MASK, pic1 & ~IRQ_MASK_BIT(irq));
    else 
        //coloca bit irq da máscara do PIC-2 a 0
        outportb(PIC2_MASK, pic2 & ~IRQ_MASK_BIT(irq)); 
}
