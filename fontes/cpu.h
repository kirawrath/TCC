\begin{lstlisting}
// EPOS-- ARMV7 CPU Mediator Declarations

#ifndef __armv7_h
#define __armv7_h

#include <system/config.h>
#include <cpu.h>

__BEGIN_SYS

class ARMV7: public CPU_Common
{
private:
    typedef Traits<CPU> _Traits;
    static const unsigned int CLOCK = Traits<Machine>::CLOCK;

public:
    // CPU Flags
    typedef Reg32 Flags;

    // CPU Context
    class Context
    {
    public:

        Context(Log_Addr entry, Log_Addr stack_bottom, Log_Addr exit) :
            _r0(0xA), _r1(0xB), _r2(0xC), _r3(0xD), _r4(0xE), _r5(0xF), _r6(0),
            _r7(0), _r8(0), _r9(0), _r10(0), _r11(0), _r12(0),
            _sp(stack_bottom), _lr(exit), _pc(entry),
            _cpsr(0x60000013) {} // all interrupts enabled by default. 0x13 is SVC mode.

        Context() {}

        void save() volatile;
        void load() const volatile;

        friend Debug & operator << (Debug & db, const Context & c) {
            db << hex;
            db << "{r0="    << c._r0
               << ",r1="    << c._r1
               << ",r2="    << c._r2
               << ",r3="    << c._r3
               << ",r4="    << c._r4
               << ",r5="    << c._r5
               << ",r6="    << c._r6
               << ",r7="    << c._r7
               << ",r8="    << c._r8
               << ",r9="    << c._r9
               << ",r10="    << c._r10
               << ",r11="    << c._r11
               << ",r12="    << c._r12
               << ",SP="    << c._sp
               << ",LR="    << c._lr
               << ",PC="    << c._pc
               << ",CPSR="    << c._cpsr
               << "}" ;
            db << dec;
            return db;
        }

    public:
        Reg32 _r0;
        Reg32 _r1;
        Reg32 _r2;
        Reg32 _r3;
        Reg32 _r4;
        Reg32 _r5;
        Reg32 _r6;
        Reg32 _r7;
        Reg32 _r8;
        Reg32 _r9;
        Reg32 _r10;
        Reg32 _r11;
        Reg32 _r12;
        Reg32 _sp;
        Reg32 _lr;
        Reg32 _pc;
        Reg32 _cpsr; // Current Program Status Register  
    };

public:
    ARMV7() {}

    static void init();

    static void int_enable() {
        irq_enable();
        //fiq_enable();
	}

    static void int_disable() {
        irq_disable();
        fiq_disable();
    }

    static void irq_enable() {
        Reg32 flags;
        ASMV("mrs %0, cpsr\n"
            "bic %0, %0, #0x80\n"
            "msr cpsr_c, %0\n":  "=r"(flags) : : "cc");
    }

    static void irq_disable() {
        Reg32 flags;
        ASMV("mrs %0, cpsr\n"
            "orr %0, %0, #0x80\n"
            "msr cpsr_c, %0\n":  "=r"(flags) : : "cc");
	}

    static void fiq_enable() {
        Reg32 flags;
        ASMV("mrs %0, cpsr\n"
            "bic %0, %0, #0x40\n"
            "msr cpsr_c, %0\n":  "=r"(flags) : : "cc");
	
	}
    static void fiq_disable() {
        Reg32 flags;
        ASMV("mrs %0, cpsr\n"
            "orr %0, %0, #0x40\n"
            "msr cpsr_c, %0\n":  "=r"(flags) : : "cc");
	}

    static void halt() {
        int_enable();
        //power(DOZE);
        while(true);
    }

    static void reboot() {
        ASMV("b _init\n");//Should do more than that...TODO
    }

    static void switch_context(Context * volatile * o, Context * volatile n);

    static Flags flags() { 
        register Reg32 result;
        ASMV("mrs %0, cpsr" : "=r"(result) ::); 
        return result;
    } 
    static void flags(Flags flags) {
        ASMV("msr cpsr_c, %0" : : "r"(flags) :);
    }

    static void sp(Reg32 sp) {
        ASMV("mov sp, %0" : : "r"(sp) : "sp");
    }

    static Reg32 fr() {
        Reg32 return_value;
        ASMV("mov %0, r0" : "=r" (return_value) : : "r0");
        return return_value;
    }

    static void fr(Reg32 fr) {	
        ASMV("mov r0, %0" : : "r" (fr) : "r0");
    } 

    static Reg32 sp() {
        Reg32 return_value;
        ASMV("mov %0, sp" : "=r" (return_value) : : );
        return return_value;
    }
    
    static Reg32 pdp() {
		//Translation table 0 base address
        Reg32 return_value;
        ASMV("mrc p15,0,%0,c2,c0,0" : "=r" (return_value) : : );
        return return_value;
	}
    static void pdp(Reg32 value) {
		//Translation table 0 base address
        ASMV("mcr p15,0,%0,c2,c0,0" : : "r"(value) : );
	}
    
    // PC is read with a +8 offset
    static Log_Addr ip()
    { 
        register Reg32 result;
        ASMV("mov %0, pc" : "=r"(result) : :);
        return result;
    } 

    static bool tsl(volatile bool & lock) {
        register bool result;
		register unsigned int value = 1;
		static volatile bool old=lock;

	    ASMV("1: ldrexb %0, [%1]      \n"
	         "   strexb r4, %2, [%1]  \n"
	         "   cmp r4, #0          \n"
	         "   bne 1b              \n"
	       : "=&r" (result)
	       : "r"(&lock), "r"(value)
	       : "r4" );
        
        return old;
    }

	template <typename T>
    static T finc(volatile T & value) {
        register int result;
		volatile register T old = value;
        ASMV("1: ldrex %0, [%1]     \n"
             "   add %0, %0, #1     \n"
             "   strex r1, %0, [%1] \n"
             "   cmp r1, #0         \n"
             "   bne 1b             \n"
           : "=&r"(result)
           : "r" (&value)
           : "r1");    
        
        return old;
    }

	template <typename T>
    static int fdec(volatile T & value) {
        register int result;
		volatile register T old = value;
        
        ASMV("1: ldrex %0, [%1]     \n"
             "   sub %0, %0, #1     \n"
             "   strex r1, %0, [%1] \n"
             "   cmp r1, #0         \n"
             "   bne 1b             \n"
           : "=&r"(result)
           : "r" (&value)
           : "r1");   
        
        return old;
   }
	static void init_cpu1()
	{
		unsigned int addr=0xfffffff0, init;
		//Write the first instruction to be 
		//executed by CPU1 at the position 0xfffffff0
		ASMV("ldr %0, =boot_return \n"
			"str %0, [%1] \n"
			"dsb \n"
			"SEV \n"
			: "=r"(init)
			: "r"(addr)
			:"cc");
		kout << "Starting cpu 1!\n";
		//kout << "boot_return addr = " << init << endl;
		kout << "0xfffffff0 = " << *((unsigned int*)0xfffffff0) << endl;
	}

    static Reg32 htonl(Reg32 v) { return swap32(v); }
    static Reg16 htons(Reg16 v) { return swap16(v); }
    static Reg32 ntohl(Reg32 v) { return swap32(v); }
    static Reg16 ntohs(Reg16 v) { return swap16(v); }

    static Context * init_stack(Log_Addr stack, unsigned int size,
                                void (* exit)(), int (* entry)()) 
    {
            Log_Addr sp = stack + size;
            sp -= sizeof(Context); //stack bottom
            return new(sp) Context(entry, sp - sizeof(unsigned int), Log_Addr(exit));
    }

    // Registers r0 - r3 are used for parameter passing (ARM calling convention)
    template<typename T1>
    static Context * init_stack(Log_Addr stack, unsigned int size,
                                void (* exit)(), 
                                int (* entry)(T1 a1), 
                                T1 a1)
    {
            Log_Addr sp = stack + size;
            sp -= sizeof(Context); //stack bottom
            Context * ctx = new(sp) Context(entry, 
                                            sp - sizeof(unsigned int),
                                            Log_Addr(exit));
            ctx->_r0 = (Reg32)(a1);
            return ctx;
    }

    template<typename T1,typename T2>
    static Context * init_stack(Log_Addr stack, unsigned int size,
                                void (* exit)(), 
                                int (* entry)(T1 a1, T2 a2), 
                                T1 a1, T2 a2)
    {
            Log_Addr sp = stack + size;
            sp -= sizeof(Context); //stack bottom
            Context * ctx = new(sp) Context(entry,
                                            sp - sizeof(unsigned int),
                                            Log_Addr(exit));
            ctx->_r0 = (Reg32)(a1);
            ctx->_r1 = (Reg32)(a2);
            return ctx;
    }
    
    template<typename T1,typename T2,typename T3>
    static Context * init_stack(Log_Addr stack, unsigned int size,
                                void (* exit)(), 
                                int (* entry)(T1 a1, T2 a2, T3 a3), 
                                T1 a1, T2 a2, T3 a3)
    {
            Log_Addr sp = stack + size;
            sp -= sizeof(Context); //stack bottom
            Context * ctx = new(sp) Context(entry, sp - sizeof(unsigned int), Log_Addr(exit));
            ctx->_r0 = (Reg32)(a1);
            ctx->_r1 = (Reg32)(a2);
            ctx->_r2 = (Reg32)(a3);
            return ctx;
    }

    // ARMV7 specific methods

    static Reg8 in8(const Reg32 port) {
        return (*(volatile Reg8 *)port);
    }

    static Reg16 in16(const Reg32 port) {
        return (*(volatile Reg16 *)port);
    }

    static Reg32 in32(const Reg32 port) {
        return (*(volatile Reg32 *)port);
    }

    static void out8(const Reg32 port, const Reg8 value) {
        (*(volatile Reg8 *)port) = value;
    }

    static void out16(const Reg32 port, const Reg16 value) {
        (*(volatile Reg16 *)port) = value;
    }  

    static void out32(const Reg32 port, const Reg32 value) {
        (*(volatile Reg32 *)port) = value;
    }
	static void masked_out32(const Reg32 port, const Reg32 value, const Reg32 mask) {
        (*(volatile Reg32 *)port) = value | (in32(port) & mask);
	}

    typedef char OP_Mode;
    enum {
        OFF = 0,
        HIBERNATE = 1,
        DOZE = 2,
        FULL = 3,
        STANDBY = HIBERNATE,
        LIGHT = DOZE

    };
    static OP_Mode power() { return _mode; }
    static void power(OP_Mode mode);

private:
    static OP_Mode _mode;
};

__END_SYS

#endif
\end{lstlisting}
