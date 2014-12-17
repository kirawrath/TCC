\begin{lstlisting}
#include <mach/zynq/timer.h>
#include <machine.h>

__BEGIN_SYS

//Timer::Handler * Zynq_Timer::_handler[3] = { 0, 0, 0};
//Scheduler_Timer::Handler * Scheduler_Timer::_handler = 0;
Zynq_Timer * Zynq_Timer::_channels[CHANNELS] = {0,0,0};

void Zynq_Timer::int_handler(IC::Interrupt_Id id)
{

    if((!Traits<System>::multicore || (Machine::cpu_id() == 0)) && _channels[ALARM])
        _channels[ALARM]->_handler();

    if(_channels[SCHEDULER] && (--_channels[SCHEDULER]->_current[Machine::cpu_id()] <= 0)) {
        _channels[SCHEDULER]->_current[Machine::cpu_id()] = _channels[SCHEDULER]->_initial;
        _channels[SCHEDULER]->_handler();
    }
}
/*
void PandaBoard_Timer::int_handler(IC::Interrupt_Id id)
{
    //CPU::Reg32 clr = INTERRUPT_CLEAR;
    switch (id)
    {
        case IC::TIMERINT0: id = 0; break;
        case IC::TIMERINT1: id = 1; clr += 0x100; break;
        case IC::TIMERINT2: id = 2; clr += 0x200; break;
        default:
            return;
    }
    //clear
    switch (id)
    {
        case IC::TIMERINT0: 
            id = 1; //Alarm has ID = 1 
            break;
        //case IC::TIMERINT1: id = 1; clr += 0x100; break;
        //case IC::TIMERINT2: id = 2; clr += 0x200; break;
        default:
            return;
    }        
    
    //call handler
    if (_handler[id]) 
        _handler[id]();
}
*/
__END_SYS



\end{lstlisting}
