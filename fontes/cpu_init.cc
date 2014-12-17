\begin{lstlisting}
// EPOS-- ARMV7 CPU Mediator Initialization

#include <cpu.h>
#include <mmu.h>
#include <tsc.h>

__BEGIN_SYS

void ARMV7::init()
{
    db<Init, ARMV7>(TRC) << "ARMV7::init()\n";

	if(Traits<MMU>::enabled) MMU::init();
	if(Traits<TSC>::enabled) TSC::init();

	return;
}

__END_SYS
\end{lstlisting}
