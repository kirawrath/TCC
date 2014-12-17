\begin{lstlisting}
// EPOS PandaBoard Interrupt Controller Initialization

#include <cpu.h>
#include <ic.h>
#include <machine.h>

__BEGIN_SYS

void PandaBoard_IC::init()
{
    db<Init, IC>(TRC) << "IC::init()\n";

    CPU::int_disable();
    
    kout << "IC::init()..";

    if(Machine::cpu_id() == 0)
        interrupt_distributor_init();
    //interrupt_interface_init();
    
    kout << "done!\n";

    CPU::int_enable();
}

void Zynq_IC::init()
{
    CPU::int_disable();

    kout << "IC::init()..";
    if(Machine::cpu_id() == 0)
        interrupt_distributor_init();
    kout << "done!\n";
    
    CPU::int_enable();
}
__END_SYS
\end{lstlisting}
