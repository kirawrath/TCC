\begin{lstlisting}
// EPOS-- ARMV7 CPU Mediator Implementation

#include <arch/armv7/cpu.h>
#include <system/config.h>
#include <machine.h>

__BEGIN_SYS

ARMV7::OP_Mode ARMV7::_mode = ARMV7::FULL;

void ARMV7::Context::save() volatile
{
	ASMV("nop\n");
}

void ARMV7::Context::load() const volatile
{
	db<CPU>(TRC) << "CPU::Context::load(this=" << (void*)this << ")\n";

	ASMV("ldr r0, [%0, #64]\n"
         "msr spsr_cfsx, r0\n"
         "ldmfd %0, {r0-r12,sp,lr,pc}^\n"
		 :
		 : "r" (this)
		 : "r0");
}


void ARMV7::switch_context(Context * volatile * o, Context * volatile n)
{
	kout << "Switch_context\n";
    Context * old = *o;
        
    old->_cpsr = CPU::flags();
    
    //ASMV("ldr r2, [%0, #64]" : : "r"(n) : "r2"); //geting n->_cpsr
    //ASMV("msr spsr_cfsx, r2");

    ASMV("msr spsr_cfsx, %0" : : "r"(n->_cpsr) :);
    ASMV("stmia %0, {r0-r12,sp,lr,pc} \n"              // pc is always read with a +8 offset
         "ldmfd %1, {r0-r12,sp,lr,pc}^"
          : : "r"(old), "r"(n) :);
    ASMV("nop");                                        // so the pc read above is poiting here
}


void ARMV7::power(ARMV7::OP_Mode mode)
{
    if (mode == _mode) return;
        _mode = mode;
}

extern "C" void __cxa_guard_acquire() {
    CPU::int_disable();
}
extern "C" void __cxa_guard_release() {
    CPU::int_enable();
}

__END_SYS

\end{lstlisting}
