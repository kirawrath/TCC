\begin{lstlisting}
#ifndef __zynq_timer_h
#define __zynq_timer_h

#include <timer.h>
#include <ic.h>
#include <system/meta.h>

__BEGIN_SYS


class Zynq_Timer: public Timer_Common
{
private:
	static const unsigned int PRIVATE_TIMER_BASE = 0xf8f00600;
    enum {
        PTLR    = 0x00, //Private Timer Load Register
        PTCTR   = 0x04, //Private Timer Counter Register
        PTCLR   = 0x08, //Private Timer Control Register
        PTISR   = 0x0C, //Private Timer Interrupt Status Register
        WLR     = 0x20, //Watchdog Load Register
        WCTR    = 0x24, //Watchdog Counter Register
        WCLR    = 0x28, //Watchdog Control Register
        WISR    = 0x2C, //Watchdog Interrupt Status Register
        WRSR    = 0x30, //Watchdog Reset Status Register
        WDR     = 0x34, //Watchdog Disable Register
    };
    
    enum {
        INTERRUPT_CLEAR     = 1,
        TIMER_ENABLE        = 1,
        TIMER_AUTO_RELOAD   = 2, //periodic
        TIMER_IT_ENABLE     = 4, //irq enable
        TIMER_WD_MODE       = 8,
        TIMER_PRESCALE_SHIFT = 8,        
    };


    static const CPU::Reg8 IRQ = IC::TIMERINT0;
    
   
    static void load(CPU::Reg32 val) {
        CPU::out32(PRIVATE_TIMER_BASE + PTLR, val);
    }

    static CPU::Reg32 load() {
        return CPU::in32(PRIVATE_TIMER_BASE + PTLR);
    }    
	
    static void control(CPU::Reg32 val) {
        CPU::out32(PRIVATE_TIMER_BASE + PTCLR, val);
    }

    static CPU::Reg32 control() {
        return CPU::in32(PRIVATE_TIMER_BASE + PTCLR);
    }

    //static CPU::Reg8 irq() { return IRQ /*+ _channel*/; }

    static void status(CPU::Reg32 val) {
        CPU::out32(PRIVATE_TIMER_BASE + PTISR, val);
    }

    static CPU::Reg32 status() {
        return CPU::in32(PRIVATE_TIMER_BASE + PTISR);
    }


public:
    typedef unsigned long Hertz;
    typedef CPU::Reg32 Count;   
    typedef short Channel;
    enum {
        TSC=0,
        ALARM,
        SCHEDULER,        
    };
protected:
    static const unsigned int FREQUENCY = Traits<Alarm>::FREQUENCY;
    Handler* _handler;
    Channel _channel;
	static const unsigned int CHANNELS = 3;
	static Zynq_Timer* _channels[CHANNELS];
    volatile Count _current[Traits<Machine>::MAX_CPUS];
	Count _initial;
public:

    static const unsigned int CLOCK = Traits<Machine>::CLOCK;
    
    void handler(Handler * hand) {          
        _handler = hand;
    }

   /* 
    Hertz frequency() { 
        if (_channel == TSC)
            return (CLOCK/2) / load();
        else
            return FREQUENCY / load();
    }
    void frequency(const Hertz & f) {
        db<Timer>(TRC) << "Timer_"<<_channel<<"::frequency(f="<<f<<")\n";
        if (_channel == TSC)
            load((CLOCK/2) / f);
        else
            load(FREQUENCY / f);
    };
	*/
	
private:
	static unsigned int prescale(const Hertz& f)
	{
		if(Traits<Zynq_Timer>::prescale)
			return ((CLOCK/2)/f + 1) > 0xff ? 0xff : ((CLOCK/2)/f + 1);
		return 1;
	}
	static unsigned int prescale()
	{
		return ((control() >> TIMER_PRESCALE_SHIFT) & 0xff) + 1;
	}
	static void set_prescale(const Hertz& f)
	{
        control(control() | ((prescale(f)-1) << TIMER_PRESCALE_SHIFT));
	}
public:
    static void frequency(const Hertz& f)
	{
		//set_prescale(f); //set_initial is already doing this job
		load( (CLOCK/(2*prescale())) / f );
		kout << "timer init: " << prescale() << " " << load() << endl;
	}
private:
	void set_initial(const Hertz& f)
	{
		if(_channel==ALARM)
		{
			set_prescale(f);
			_initial = (CLOCK/(2*prescale())) / f;
		}
		else
			_initial = FREQUENCY / f;
	}
public:
	Hertz frequency()
	{	
		return (CLOCK/(2*prescale())) / load();
	}
	
    static void init();

    static void int_handler(IC::Interrupt_Id id);

    void reset() {
        //db<IC>(TRC) << "Timer_"<<_channel<<"::reset()\n";
        value(load());
    }
    
    void enable(){
        db<IC>(TRC) << "Timer_"<<_channel<<"::enable()\n";
        control(control() | TIMER_ENABLE);        
    }

    void disable() {
        db<IC>(TRC) << "Timer_"<<_channel<<"::disable()\n";
        control(control() & ~(TIMER_ENABLE));
    }

    Tick read() {
        return value();
    }
    
    static void value(CPU::Reg32 val) {
        CPU::out32(PRIVATE_TIMER_BASE + PTCTR, val);
    }

    static CPU::Reg32 value() {
        return CPU::in32(PRIVATE_TIMER_BASE + PTCTR);
    }

private:
    static Hertz count2freq(const Count & c) {
	    return c ? CLOCK / c : 0;
    }

    static Count freq2count(const Hertz & f) { 
	    return f ? CLOCK / f : 0;
    }
	/***
	 *
	 * */
	public:
	Zynq_Timer(const Hertz& freq, const Handler* hand, const Channel& channel)
	{
		_channel = channel;
        handler(hand);
		set_initial(freq);
        
		//_initial will be 0 if f > CLOCK, which is reasonable.
        if(_initial && !_channels[channel])
		{
            _channels[channel] = this;
			if (channel == ALARM)
				frequency(freq);
		}
        else{
            db<Timer>(ERR) << "Timer not installed!" << endl;
            db<Timer>(ERR) << "Freq: " << freq << endl;
            db<Timer>(ERR) << "_channel["<<channel<<"] = " << _channels[channel] << endl;
            db<Timer>(ERR) << "_initial: "<<_initial << endl;
		}


        for(unsigned int i = 0; i < Traits<Machine>::MAX_CPUS; i++)
            _current[i] = _initial;

        enable();
		kout << "frequency: " << frequency() << '\n';
		kout << "Channel: " << _channel << " load: " << load() << " initial "<< _initial << '\n';
	}

	/*
	enum{
		GLOBAL_TIMER_COUNTER_LO 		= 0xf8f00200,
		GLOBAL_TIMER_COUNTER_HI 		= 0xf8f00204,
		GLOBAL_TIMER_CONTROL			= 0xf8f00208,
		GLOBAL_TIMER_INTERRUPT_STATUS	= 0xf8f0020c,
		COMPARATOR_VALUE_LO				= 0xf8f00210,
		COMPARATOR_VALUE_HI				= 0xf8f00214,
		AUTO_INCREMENT					= 0xf8f00218,
		PRIVATE_TIMER_LOAD				= 0xf8f00600,
		PRIVATE_TIMER_COUNTER			= 0xf8f00604,//Issue interrupt 29 when reaches 0.
		PRIVATE_TIMER_CONTROL			= 0xf8f00608,
		PRIVATE_TIMER_INTERRUPT_STATUS	= 0xf8f0060c,
		
		TIMER_ENABLE					= 1,
		AUTO_RELOAD						= 1<<1,
		IRQ_ENABLE						= 1<<2,
		PRESCALE_OFFSET					= 8,
	};
	//Note: The private timers stop counting when the associated processor is in debug state.
	//Writing to the Timer Load Register also writes to the Timer Counter Register.
	//All private timers are always clocked at 1/2 of the CPU frequency (CPU_3x2x).
	*/
	
};

// Timer used by Alarm

class Alarm_Timer: public Zynq_Timer
{
public:
    Alarm_Timer(const Handler * handler):
    Zynq_Timer(Zynq_Timer::FREQUENCY, handler, ALARM) {}
};

// Timer used by Thread::Scheduler
class Scheduler_Timer: public Zynq_Timer
{
private:
    typedef unsigned long Microsecond;

public:
    Scheduler_Timer(const Microsecond & quantum, const Handler & handler)
    : Zynq_Timer(1000000 / quantum, handler, SCHEDULER) {}
};
/*
// Scheduler timer using the triple time counter
// It is fully implemented, but something is still not working =/
class Scheduler_Timer : public Timer_Common
{
    static const unsigned int FREQUENCY = 1000;
	static const unsigned int TTC0_BASE = 0xf8001000; // Triple time counter 0
	static const unsigned int TTC1_BASE = 0xf8002000; // Triple time counter 1

	//TTCs registers
	enum {
		//TTC0's timer 0 issue interruption 42.
		CLOCK_CONTROL_1			= 0,
		COUNTER_CONTROL_1		= 0xC,
		COUNTER_VALUE_1			= 0X18,
		INTERVAL_COUNTER_1		= 0X24,
		MATCH_1_COUNTER_1		= 0X30,
		INTERRUPT_REGISTER_1	= 0X54,
		INTERRUPT_ENABLE_1		= 0X60,
		EVENT_CONTROL_TIMER_1	= 0X6C
	};
	enum {
		//Clock_Control control bits
		Ex_E 	= 1<<6,
		C_Src	= 1<<5,
		PS_V	= 0xf<<1,
		Ps_En	= 1,
		PRESCALE_SHIFT = 1,
		//Counter_Control control bits
		Wave_en = 1<<5,
		RST		= 1<<4,
		INTERVAL_MODE =  1<<1,
		DISABLE = 1,
		//Counter Value mask
		VALUE_MASK = 0xffff,
		//Interval Counter
		//If interval (mode) is enabled, this is the maximum value
		//that the counter will count up to or down from.
		COUNT_VALUE = 0xffff,
		//Interrupt Register
		//Interrupt Enable Register uses the same layout.
		EVENT_TIMER_OVERFLOW	= 1<<5,
		COUNTER_OVERFLOW		= 1<<4,
		MATCH2_INT				= 1<<3,
		MATCH1_INT				= 1<<2,
		MATCH0_INT				= 1<<1,
		INTERVAL_INT			= 1,
	};
private:
	static void load(CPU::Reg32 val)
	{
        CPU::out32(TTC0_BASE | INTERVAL_COUNTER_1, val);
    }

    static CPU::Reg32 load() {
        return CPU::in32(TTC0_BASE | INTERVAL_COUNTER_1);
    }    
	
    static void control(CPU::Reg32 val) {
        CPU::out32(TTC0_BASE | COUNTER_CONTROL_1, val);
    }

    static CPU::Reg32 control() {
        return CPU::in32(TTC0_BASE | COUNTER_CONTROL_1);
    }
public:
	void disable(){
		control(control() | DISABLE);
	}
	void enable(){
		control(control() & (!DISABLE));
	}

    void frequency(const Hertz& f)
	{
        unsigned int prescale = (CLOCK) / f; //timer runs at 1/6 clock speed
		//find the closest 2^N multiple that is smaller than prescale.
		unsigned int p, n = 0;
		prescale = prescale > 0xffff?0xffff:prescale;
		p = prescale;
		while(p){
			++n;
			p = p >> 1;
		}
		//Enable prescale and set its value
		n = n-1;

		CPU::out32(TTC0_BASE | CLOCK_CONTROL_1, Ps_En | (n << PRESCALE_SHIFT) );
		load( ((CLOCK/prescale) / f) & 0xffff ); //Will be 1 if f < (CLOCK)
	}
	
	Hertz frequency()
	{
		unsigned int prescale = (CPU::in32(TTC0_BASE|CLOCK_CONTROL_1) >> PRESCALE_SHIFT) & 0xf;
		unsigned int n = 2;
		while(prescale--)
			n*=2;
		return (CLOCK/(n-1)) / load();
	}
	void reset(){
		control(control() | RST);
	}

    typedef unsigned long Microsecond;
	static const unsigned int CLOCK = 111000000; //Assuming mode 6:2:1. cpu_1x would be 133mhz at 4:2:1
	static Handler * _handler;

public:
    Scheduler_Timer(const Microsecond & quantum, const Handler * handler)
	{
	//:Zynq_Timer(1000000 / quantum, handler, SCHEDULER) {
        _handler = handler;

		//Select clock input source and prescaler value.
		//Set interval value
		disable();
		//CPU::out32(0xf800074c, 6<<5);
        frequency(1000000 / quantum);
		
		//Enable interrupt
		CPU::out32(TTC0_BASE | INTERRUPT_REGISTER_1, INTERVAL_INT);
		CPU::out32(TTC0_BASE | INTERRUPT_ENABLE_1, INTERVAL_INT);
		
		//Enable/disable waveform output, enable/disable matching, set counting direction, set mode,
		//enable counter (TTC Counter Control register). This step starts the counter.
		control(Wave_en | INTERVAL_MODE); //Implicit enable
		reset();


		kout << "Scheduler timer:\n";
		kout << " frequency: " << frequency() << " vs " << (1000000/quantum) << '\n';
		kout << " load: " << load() << '\n';
		kout << control() << ' ' << CPU::in32(TTC0_BASE | CLOCK_CONTROL_1) << '\n';
		kout << "Counter value: " << CPU::in32(TTC0_BASE | COUNTER_VALUE_1) << '\n';
		kout << "Counter value: " << CPU::in32(TTC0_BASE | COUNTER_VALUE_1) << '\n';
	}


};
*/
// Timer used by ARM7_TSC
class TSC_Timer: public Zynq_Timer
{
public:
    TSC_Timer(const Hertz & freq, const Handler * handler) 
    : Zynq_Timer(1, handler, TSC) {}
};

/*

class PandaBoard_Timer: public Timer_Common
{
	protected:
	enum{
		PRIVATE_TIMER_BASE = 0x48240600,
	};
    
    // Timer and Watchdog registers - based on the PRIVATE_TIMER_BASE address
    enum {
        PTLR    = 0x00, //Private Timer Load Register
        PTCTR   = 0x04, //Private Timer Counter Register
        PTCLR   = 0x08, //Private Timer Control Register
        PTISR   = 0x0C, //Private Timer Interrupt Status Register
        WLR     = 0x20, //Watchdog Load Register
        WCTR    = 0x24, //Watchdog Counter Register
        WCLR    = 0x28, //Watchdog Control Register
        WISR    = 0x2C, //Watchdog Interrupt Status Register
        WRSR    = 0x30, //Watchdog Reset Status Register
        WDR     = 0x34, //Watchdog Disable Register
    };
    
    enum {
        INTERRUPT_CLEAR     = 1,
        TIMER_ENABLE        = 1,
        TIMER_AUTO_RELOAD   = 2, //periodic
        TIMER_IT_ENABLE     = 4,
        TIMER_WD_MODE       = 8,
        TIMER_PRESCALE_SHIFT = 8,        
    };

    static const CPU::Reg8 IRQ = IC::TIMERINT0;
    
   
    static void load(CPU::Reg32 val) {
        CPU::out32(PRIVATE_TIMER_BASE + PTLR, val);
    }

    static CPU::Reg32 load() {
        return CPU::in32(PRIVATE_TIMER_BASE + PTLR);
    }    
	
    static void control(CPU::Reg32 val) {
        CPU::out32(PRIVATE_TIMER_BASE + PTCLR, val);
    }

    static CPU::Reg32 control() {
        return CPU::in32(PRIVATE_TIMER_BASE + PTCLR);
    }

    static CPU::Reg8 irq() { return IRQ; }

    static void status(CPU::Reg32 val) {
        CPU::out32(PRIVATE_TIMER_BASE + PTISR, val);
    }

    static CPU::Reg32 status() {
        return CPU::in32(PRIVATE_TIMER_BASE + PTISR);
    }


public:
    typedef unsigned long Hertz;
    typedef CPU::Reg32 Count;   
    typedef short Channel;
    enum {
        TSC,
        ALARM,
        SCHEDULER,        
    };
protected:
    static Handler * _handler[3];
    Channel _channel;
public:

    static const unsigned int CLOCK = Traits<Machine>::CLOCK;
    static const unsigned int FREQUENCY = 1000;
    
    PandaBoard_Timer(const Hertz & freq,
         const Handler * hand,const Channel & channel) :
         _channel(channel)
    {
        db<Timer>(TRC) << "Timer(chan="<<channel<<
                          ",hdl="<<(void*)hand<<
                          ",freq="<<freq<<")\n";                
		//The timer is incremented every prescaler value+1.
		//For example, if the prescaler has a value of five then
		//the global timer is incremented every six clock cycles.
		//PERIPHCLK (clock/2) is the reference clock for this.

        unsigned int prescale = (CLOCK / 2) / freq;
        if(prescale > 0)
            --prescale;
        
        //prescaler is limited to 1 byte
        if(prescale > 0xFF)
            prescale = 0xFF;
            
        frequency(freq);
        handler(hand);
        value(freq2count(freq));  
        
        control(control() | (prescale << TIMER_PRESCALE_SHIFT));
        
        enable();
    }

    void handler(Handler * hand) {          
        _handler[_channel] = hand;        
    }

    Hertz frequency() { 
        if (_channel == 0)
            return CLOCK / load();
        else
            return FREQUENCY / load();
    }
    
    void frequency(const Hertz & f) {
        db<Timer>(TRC) << "Timer_"<<_channel<<"::frequency(f="<<f<<")\n";
        if (_channel == 0)
            load(CLOCK / f);
        else
            load(FREQUENCY / f);
    };

    static void init();

    static void int_handler(IC::Interrupt_Id id);

    void reset() {
        db<IC>(TRC) << "Timer_"<<_channel<<"::reset()\n";
        value(load());
    }
    
    void enable(){
        db<IC>(TRC) << "Timer_"<<_channel<<"::enable()\n";
        control(control() | TIMER_ENABLE);        
    }

    void disable() {
        db<IC>(TRC) << "Timer_"<<_channel<<"::disable()\n";
        control(control() & ~(TIMER_ENABLE));
    }

    Tick read() {
        return value();
    }
    
    static void value(CPU::Reg32 val) {
        CPU::out32(PRIVATE_TIMER_BASE + PTCTR, val);
    }

    static CPU::Reg32 value() {
        return CPU::in32(PRIVATE_TIMER_BASE + PTCTR);
    }

private:
    static Hertz count2freq(const Count & c) {
	    return c ? CLOCK / c : 0;
    }

    static Count freq2count(const Hertz & f) { 
	    return f ? CLOCK / f : 0;
    }


};
*/

__END_SYS

#endif

\end{lstlisting}
