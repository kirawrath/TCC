\begin{lstlisting}
#include <mach/zynq/ic.h>

__BEGIN_SYS

PandaBoard_IC::Interrupt_Handler PandaBoard_IC::_vector[PandaBoard_IC::INTS] =
{ 
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler, 
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler, 
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler, 
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler, 
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler,
    &PandaBoard_IC::default_handler, &PandaBoard_IC::default_handler
};

CPU::Reg32 PandaBoard_IC::_mask = 0;

void PandaBoard_IC::int_handler() {
    //CPU::Reg32 status = CPU::in32(PIC_IRQ_STATUS);
    kout << "IC::int_handler()\n"; 
    
    //disable_cpu_interface();
    //disable_distributor();
    
    //ICCIAR also has the CPU id for SGIs interrupts
    unsigned int icciar_value = CPU::in32(GIC_PROC_INTERFACE + ICCIAR);
    Interrupt_Id id = icciar_value & INTERRUPT_MASK; //0x3FF bits 0 to 9
    
    // search for interrupt id
    /*Interrupt_Id id = 0;
    CPU::Reg32 s = status;
    while (s >>= 1) {
        ++id;
    };*/
    
    db<IC>(TRC) << "IC::int_handler()\n";    
   
    // call handler
    _vector[id](id);
    //db<IC>(TRC) << "return from handler\n";
    
    //For compatility, arm recommends to preserve the entire register value
    //read from the ICCIAR when acknowledging an interrupt
    CPU::out32(GIC_PROC_INTERFACE + ICCEOI, icciar_value);
    
    //enable_cpu_interface();
    //enable_distributor();
}

void PandaBoard_IC::default_handler(Interrupt_Id id) {
    db<PandaBoard_IC>(INF) << "PandaBoard_IC::default_handler(id="<<id<<")\n";
    kout << "PandaBoard_IC::default_handler(id=" << id << ")\n";
}


void Zynq_IC::default_handler(Interrupt_Id id) {
    db<PandaBoard_IC>(INF) << "Zynq_IC::default_handler(id="<<id<<")\n";
}

Zynq_IC::Interrupt_Handler Zynq_IC::_vector[Zynq_IC::INTS] =
{ 
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler,
    &Zynq_IC::default_handler, &Zynq_IC::default_handler
};
void Zynq_IC::int_handler()
{	
	//Interrupt Acknowledge Register (ICCIAR)
    unsigned int icciar_value = CPU::in32(IC::GIC_PROC_INTERFACE | IC::ICCIAR);
	IC::Interrupt_Id id = icciar_value & IC::INTERRUPT_MASK; //0x3FF bits 0 to 9

	if(id != 29)
		kout << "Interrupt id: "<< id << '\n';

	if(id > Zynq_IC::INTS){
		if(id == 1023)
			kout << "Spurious interruption received\n";
		kout << "Error: interruption id is greater than _vector entries. Id="<<id<<".\n";
		return;
	}
    _vector[id](id);
	/* For every read of a valid Interrupt ID from the ICCIAR, the interrupt
	 * service routine on the connected processor must perform a matching write
	 * to the ICCEOIR, see End of Interrupt Register (ICCEOIR) */
	//kout << "calling out32"<<endl;
	CPU::int_disable();
    CPU::out32(IC::GIC_PROC_INTERFACE | IC::ICCEOI, icciar_value);
	//kout << "back from out32\n";
	CPU::int_enable();
}

__END_SYS
\end{lstlisting}
