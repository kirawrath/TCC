\begin{lstlisting}
#ifndef __zynq_uart_h__
#define __zynq_uart_h__

#include <cpu.h>
#include <uart.h>

__BEGIN_SYS

class Zynq_UART: public UART_Common
{
    typedef CPU::Reg32 Reg32;
public:
    Zynq_UART(unsigned int baud, unsigned int data_bits, unsigned int parity,
            unsigned int stop_bits, unsigned int unit = 0) {}

    // The default unit is 1 because Serial_Display instantiate it without
    // parameters and we want it to use UART 1
    Zynq_UART(unsigned int unit = Traits<Zynq_UART>::DEFAULT_UART){
        if(unit == 0)
            _base = UART0_BASE;
        else if(unit == 1)
            _base = UART1_BASE;

		if(unit==0)
		{
			init();
			return;
		}
        // Divide baud rate by 6+1
        reg(BAUD_RATE_DIVIDER_REG0, 0x6);

        // Set baud rate clock divisor to 62
        reg(BAUD_RATE_GEN_REG0, 0x3E);

        // Enable and reset RX/TX data paths
        reg(CONTROL_REG0, reg(CONTROL_REG0) | 1<<UART1::RXRES | 1<<UART1::TXRES |
				1<<UART1::RXEN | 1<<UART1::TXEN);

        // Set 1 stop bit, no parity and 8 data bits
        reg(MODE_REG0, reg(MODE_REG0) | 0x0<<UART1::CHRL | 0x4<<UART1::PAR |
				0x0<<UART1::NBSTOP);
    }
private:
	class UART1{
		public:
		// CONTROL_REG0 bits
		enum {
			RXRES   = 0,
			TXRES   = 1,
			RXEN    = 2,
			RXDIS   = 3,
			TXEN    = 4,
			TXDIS   = 5,
			RSTTO   = 6,
			STTBRK  = 7,
			STPBRK  = 8
		};

		// MODE_REG0 bits
		enum {
			CLKS    = 0,
			CHRL    = 1,
			PAR     = 3,
			NBSTOP  = 6,
			CHMODE  = 8
		};

		// CHANNEL_STS_REG0 bis
		enum {
			RTRIG   = 0,
			REMPTY  = 1,
			RFUL    = 2,
			TEMPTY  = 3,
			TFUL    = 4,
			RACTIVE = 10,
			TACTIVE = 11,
			FDELT   = 12,
			TTRIG   = 13,
			TNFUL   = 14
		};
	};

	void init()
	{
		//Unlocking SLCR registers (p. 1500)
		out32(SLCR::SLCR_UNLOCK, 0xDF0D);
		set_pll_clock(26);//33MHZ*26

        //Reset Controller (using option 2, p. 558)
		out32(SLCR::UART_RST_CTRL, 0);
		if((Reg32)_base == UART0_BASE){
			/*Configure I/O signal routing*/
			out32(SLCR::MIO_PIN_46, 0x12E1);
			out32(SLCR::MIO_PIN_47, 0x12E0);
        /*	Configure UART_Ref_Clk (assuming I/O PPL = 667mhz.
			Dividing it by 17 to get 50mhz), based on p. 628.*/
			out32(SLCR::UART_CLK_CTRL, SLCR::CLKACT0 | (17<<SLCR::DIVISOR_SHIFT));
		}
		else if((Reg32)_base == UART1_BASE){
			/*Configure I/O signal routing*/
			out32(SLCR::MIO_PIN_49, 0x12E1);
			out32(SLCR::MIO_PIN_48, 0x12E0);

			out32(SLCR::UART_CLK_CTRL, SLCR::CLKACT1 | (17<<SLCR::DIVISOR_SHIFT));
		}
        //Configure controller functions
		write_bit(MODE_REG0, 0x3FF, 0x20);

		write_bit(CONTROL_REG0, RXEN, 0);
		write_bit(CONTROL_REG0, RXDIS, 1);
		write_bit(CONTROL_REG0, TXEN, 0);
		write_bit(CONTROL_REG0, TXDIS, 1);

		/* The input clock for UART is the UART_REF_CLK.
		 * The UART_REF_CLK is sourced from IO_PLL,
		 * which in turn mutiplies (by default) PS_CLK (33.33MHz) by 26.
		 * We can divide this 866MHz clock from the IO_PLL to 50MHz by
		 * writing 17 in the UART_CLK_CTRL, as we did above.
		 * Therefore, the input clock for UART (sel_clk) is of 50MHz.
		 * The baud rate is calculated as follows:
		 * baud_rate = sel_clk/(CD * (BDIV+1))
		 * Keeping BDIV (which is used for over sample) in its default value (15),
		 * we end up with a CD = 325 (if the target baud rate is 9600).
		 * */
		if(Traits<Zynq_UART>::BAUD_RATE == 9600)
		{
			write_bit(BAUD_RATE_GEN_REG0, CD, 325);
			write_bit(BAUD_RATE_DIVIDER_REG0, BDIV, 15); //default value
		}
		else if(Traits<Zynq_UART>::BAUD_RATE == 115200)
		{
			write_bit(BAUD_RATE_DIVIDER_REG0, BDIV, 6);
			write_bit(BAUD_RATE_GEN_REG0, CD, 62);
		}
		else //the general case
		{	//CD should not be bigger than 2^16.
			write_bit(BAUD_RATE_GEN_REG0,
					50000000/(16*Traits<Zynq_UART>::BAUD_RATE), 325);
			write_bit(BAUD_RATE_DIVIDER_REG0, BDIV, 15);
		}
		write_bit(CONTROL_REG0, TXRES, 1);
		write_bit(CONTROL_REG0, RXRES, 1);

		write_bit(CONTROL_REG0, RXEN, 1);
		write_bit(CONTROL_REG0, RXDIS, 0);
		write_bit(CONTROL_REG0, TXEN, 1);
		write_bit(CONTROL_REG0, TXDIS, 0);

		//Disabling FIFO interrupts (option b)
		write_bit(RCVR_FIFO_TRIGGER_LEVEL0, RCVR_RTRIG, 0);
		write_bit(RCVR_TIMEOUT_REG0, RTO, 0);

		out32(((unsigned int)_base) | CONTROL_REG0, 0x117);


		//Disabling UART Interrupts
		write_bit(RCVR_FIFO_TRIGGER_LEVEL0, RCVR_RTRIG, 0);

		write_bit(INTRPT_DIS_REG0, RTRIG, 1);
		write_bit(INTRPT_EN_REG0, RTRIG, 0);
	}
public:
    void put(unsigned char ch) {
        while((reg(CHANNEL_STS_REG0) & 1<<TFUL) != 0);
        reg(TX_RX_FIFO0, (Reg32)ch);
    }

    unsigned char get() {
        while((reg(CHANNEL_STS_REG0) & 1<<RTRIG) != 1);
        return reg(TX_RX_FIFO0);
    }

private:
	//Auxiliary functions.
	static void out32(const Reg32 reg, const Reg32 value){
		(*(volatile Reg32 *)reg) = value;
	}

	/*Writes to the specified register (using _base), matching the 
	 * mask to write the value to the corresponding area.
	 * It uses a read-modify strategy (non-destructive).*/
	inline void write_bit(Reg32 reg, Reg32 mask, Reg32 value){
		int n=0;
		//Matching mask with the value.
		while((mask & (1<<n)) == 0)
			++n;
		Reg32* p = (Reg32*)(_base + reg/sizeof(Reg32));
		*p = ((*p)&(~mask)) | (mask & (value << n));
	}
	
	inline Reg32 get_value(Reg32 reg){
		return *((Reg32*)reg);
	}
	static inline Reg32 in32(Reg32 reg){
		return (*((Reg32*)reg));
	}
	enum{
		IO_PLL_CTRL=0xf8000108,
		IO_PLL_CFG=0xf8000118,
		PLL_STATUS= 0xf800010c,

		FDIV_SHIFT = 12,
		PLL_BYPASS_FORCE = 1<<4,
		PLL_RESET = 1,

		IO_PLL_LOCK = 1<<2,
		
		LOCK_CNT_SHIFT = 12,//10 bits
		PLL_CP_SHIFT = 8,//4 bits
		PLL_RES_SHIFT=4,//4 bits
	};
    void set_pll_clock(unsigned int fdiv)
	{
		out32(IO_PLL_CTRL, (in32(IO_PLL_CTRL)&(0x7f<<FDIV_SHIFT)) |
				(fdiv<<FDIV_SHIFT));
		out32(IO_PLL_CFG, (2<<PLL_CP_SHIFT) | (12<<PLL_RES_SHIFT) |
				(375<<LOCK_CNT_SHIFT));
		out32(IO_PLL_CTRL, in32(IO_PLL_CTRL) | PLL_BYPASS_FORCE);
		
		out32(IO_PLL_CTRL, in32(IO_PLL_CTRL) | PLL_RESET);
		out32(IO_PLL_CTRL, in32(IO_PLL_CTRL) & 0xfffffffe);

		while((in32(PLL_STATUS) & IO_PLL_LOCK) == 0)
			;
		out32(IO_PLL_CTRL, in32(IO_PLL_CTRL) & 0xffffffef);
	}

    // Base address of each UART
    enum {
        UART0_BASE = 0xE0000000,
        UART1_BASE = 0xE0001000
    };

    // Register addresses relative to base
    enum {
        CONTROL_REG0            = 0x00,
        MODE_REG0               = 0x04,
        INTRPT_EN_REG0          = 0x08,
        INTRPT_DIS_REG0         = 0x0C,
        INTRPT_MASK_REG0        = 0x10,
        CHNL_INT_STS_REG0       = 0x14,
        BAUD_RATE_GEN_REG0      = 0x18,
		RCVR_TIMEOUT_REG0		= 0x1C,
        RCVR_FIFO_TRIGGER_LEVEL0= 0x20,
        MODEM_CTRL_REG0         = 0x24,
        MODEM_STS_REG0          = 0x28,
        CHANNEL_STS_REG0        = 0x2C,
        TX_RX_FIFO0             = 0x30,
        BAUD_RATE_DIVIDER_REG0  = 0x34,
        FLOW_DELAY_REG0         = 0x38,
        TX_FIFO_TRIGGER_LEVEL0  = 0x44,
	};
	// System Level Control Registers (slcr)
	class SLCR{
		public:
		enum{
			UART_RST_CTRL	= 0xF8000228,
			UART_CLK_CTRL	= 0xF8000154,
			SLCR_UNLOCK		= 0xF8000008,
			MIO_PIN_46		= 0xF80007B8,
			MIO_PIN_47		= 0xF80007BC,
			MIO_PIN_48		= 0xF80007C0,
			MIO_PIN_49		= 0xF80007C4,
		};
		//UART_RST_CTRL bits
		enum{
			UART1_REF_RST		= 1<<3,
			UART0_REF_RST		= 1<<2,
			UART1_CPU1X_RST		= 1<<1,
			UART0_CPU1X_RST		= 1,
		};
		//UART_CLK_CTRL bits
		enum{
			DIVISOR			= 0x1f00,
			SRCSEL			= 0x30,
			CLKACT1			= 1<<1,
			CLKACT0			= 1,
			SRCSEL_SHIFT	= 4,
			DIVISOR_SHIFT	= 8,
		};
		//SLCR_UNLOCK bits
		enum{
			UNLOCK_KEY		= 0xffff,
		};
	};
    //Control_reg0 bits
    enum{
        STOPBRK     = 1<<8,
        STARTBRK    = 1<<7,
        RSTTO       = 1<<6,
        TXDIS       = 1<<5,
        TXEN        = 1<<4,
        RXDIS       = 1<<3,
        RXEN        = 1<<2,
        TXRES       = 1<<1,
        RXRES       = 1,
    };

    //MODE_REG0 bits
    enum{
        CHMODE      = (1<<9)|(1<<8),
        NBSTOP      = (1<<7)|(1<<6),
        PAR         = (1<<5)|(1<<4)|(1<<3),
        CHRL        = (1<<2)|(1<<1),
        CLKS        = 1,
    };
    //Intrpt_en_reg0, Intrpt_dis_reg0, 
    //Intrpt_mask_reg0 and Chnl_int_sts_reg0 bits
    enum{
        TOVR        = 1<<12,
        TNFUL       = 1<<11,
        TTRIG       = 1<<10,
        DMSI        = 1<<9,
        TIMEOUT     = 1<<8,
        PARE        = 1<<7,
        FRAME       = 1<<6,
        ROVR        = 1<<5,
        TFUL        = 1<<4,
        TEMPTY      = 1<<3,
        RFUL        = 1<<2,
        REMPTY      = 1<<1,
        RTRIG       = 1,
    };
    //Baud_rate_gen_reg0 bits
    enum{
        CD          = 0xffff,
    };
    //Rcvr_timeout_reg0 bits
    enum{
        RTO         = 0xff,
    };
    //Rcvr_FIFO_trigger_level0 bits
    enum{
        RCVR_RTRIG  = 0x3f,
    };
    //Modem_ctrl_reg0 bits
    enum{
        FCM         = 1<<5,
        RTS         = 1<<1,
        DTR         = 1,
    };
    //Modem_sts_reg0 bis
    enum{
        FCMS        = 1<<8,
        DCD         = 1<<7,
		RI			= 1<<6,
		DSR			= 1<<5,
		CTS			= 1<<4,
		DDCD		= 1<<3,
		TERI		= 1<<2,
		DDSR		= 1<<1,
        //DTR        = 1, defined above
    };
    //Channel_sts_reg0 bis
	class STS{ //just a namespace due to name conflicts.
		public:
		enum{
			TNFUL       = 1<<14,
			TTRIG       = 1<<13,
			FDELT		= 1<<12,
			TACTIVE		= 1<<11,
			RACTIVE		= 1<<10,
			TFUL		= 1<<4,
			TEMPTY		= 1<<3,
			RFUL		= 1<<2,
			REMPTY		= 1<<1,
			RTRIG       = 1,
		};
	};
	//TX_RX_FIFO0 bits
	enum{
		FIFO		= 0xff,
	};
	//Baud_rate_divider_reg0 bits
	enum{
		BDIV		= 0xff,
	};
private:
    Reg32 reg(Reg32 addr) { return CPU::in32(_base + addr); }
    void reg(Reg32 addr, Reg32 value) { CPU::out32(_base + addr, value); }

    Reg32 _base;
};

__END_SYS

#endif
\end{lstlisting}
