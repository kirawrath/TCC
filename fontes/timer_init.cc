\begin{lstlisting}
// EPOS PandaBoard Timer Mediator Initialization

#include <timer.h>
//#include <ic.h>

__BEGIN_SYS

void Zynq_Timer::init()
{
	kout << "Timer::Init()\n";
    //load(0);
    status(INTERRUPT_CLEAR);
	//frequency(Traits<Alarm>::FREQUENCY);
    control(TIMER_AUTO_RELOAD | TIMER_IT_ENABLE);
    IC::int_vector(IC::TIMERINT0, &Zynq_Timer::int_handler);
    IC::enable(IC::TIMERINT0);
}
/*
void PandaBoard_Timer::init() 
{
   //kout << "PandaBoard_Timer::init()\n";
    load(0);
    value(0);        
    control(TIMER_AUTO_RELOAD | TIMER_IT_ENABLE);
    status(INTERRUPT_CLEAR);
    IC::int_vector(IC::TIMERINT0, &PandaBoard_Timer::int_handler);
    //kout << "PandaBoard_Timer::init() calling IC::enable()\n";
    IC::enable(IC::TIMERINT0);
   //kout << "PandaBoard_Timer::init() done\n";
}*/
__END_SYS
\end{lstlisting}
