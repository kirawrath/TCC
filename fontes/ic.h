\begin{lstlisting}
#ifndef __panda_ic_h
#define __panda_ic_h

#include <ic.h>
#include <utility/debug.h>
#include <cpu.h>
#include <system/memory_map.h>

__BEGIN_SYS

/*
extern "C" {
extern void _irq_handler() __attribute__ ((naked));
};
*/

class Zynq_IC :public IC_Common
{
    static const unsigned int INTERRUPT_MASK = 0x000003FF;
    static const unsigned int SPURIOUS_INTERRUPT = 1023;
    
    //Distributor Register Description
    enum {
        ICDDCR      = 0x000, //Distributor Control Register
        ICDICTR     = 0x004, //Interrupt Controller Type Register    
        ICDIIDR     = 0x008, //Distributor Implementer Identification Registers
        ICDISRN     = 0x080, //0x080 - 0x09C - Interrupt Security Registers
        ICDISERN    = 0x100, //0x104-0x11C Interrupt Set-Enable Registers
        ICDICERN    = 0x180, //0x184-0x19C Interrupt Clear-Enable Registers
        ICDISPRN    = 0x200, //0x200-0x27C Interrupt Set-Pending Registers
        ICDICPRN    = 0x280, //0x280-0x29C Interrupt Clear-Pending Registers
        ICDABRN     = 0x300, //0x300-0x31C Active Bit registers
        ICDIPRN     = 0x400, //0x400-0x4FC Interrupt Priority Registers
        ICDIPTRN    = 0x800, //0x800-0x8FC Interrupt Processor Targets Registers
        ICDICFRN    = 0xC00, //0xC00, 0xC08, 0xC08-0xC3C Interrupt Configuration Registers
        ICPPISR     = 0xD00, //PPI Status Register
        ICSPISRN    = 0xD04, //0xD04-0xD1C SPI Status Registers
        ICDSGIR     = 0xF00, //Software Generated Interrupt Register
        ICPIDR0     = 0xFD0, //Peripheral ID0 register   
        ICPIDR1     = 0xFD4, //Peripheral ID1 register
        ICPIDR2     = 0xFD8, //Peripheral ID2 register
        ICPIDR3     = 0xFDC, //Peripheral ID3 register
        ICPIDR4     = 0xFE0, //Peripheral ID4 register
        ICPIDR5     = 0xFE4, //Peripheral ID5 register
        ICPIDR6     = 0xFE8, //Peripheral ID6 register
        ICPIDR7     = 0xFEC, //Peripheral ID7 register
        ICCIDR0     = 0xFF0, //Component ID0 register
        ICCIDR1     = 0xFF4, //Component ID1 register
        ICCIDR2     = 0xFF8, //Component ID2 register
        ICCIDR3     = 0xFFC, //Component ID3 register
    };        
    
    //Processor Interface Register
    enum {
        ICCICR      = 0x000, //CPU Interface Control Register
        ICCPMR      = 0x004, //Interrupt Priority Mask Register
        ICCBPR      = 0x008, //Binary Point Register
        ICCIAR      = 0x00C, //Interrupt Ack Register
        ICCEOI      = 0x010, //End Of Interrupt Register
        ICCRPR      = 0x014, //Running Priority Register
        ICCHPIR     = 0x018, //Highest Pending Interrupt Register
        ICCABPR     = 0x01C, //Aliased Non-Secure Binary Point Register
        ICCIDR      = 0x0FC, //CPU Interface Implementer Identification Register
    };        
	public:
	static const unsigned int INTS = 96;
    static const Interrupt_Id TIMERINT0 = 29;
	private:
    static Interrupt_Handler _vector[INTS];
	static void default_handler(Interrupt_Id id);
	public:
    static void int_handler();
    static const unsigned int GIC_PROC_INTERFACE = 0xF8F00100;
    static const unsigned int GIC_INTR_DISTRIBUTOR = 0xF8F01000;
	static const Interrupt_Id INT_RESCHEDULER = 42;
	static const Interrupt_Id INT_TIMER = 29;
    static void init();

    static void interrupt_distributor_init(void)
	{
		//disable the whole distributor controller
        disable_distributor();
        
        //configure every shared peripheral interrupt
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICFRN + 8, 0x555D5757);
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICFRN + 12, 0x5555D5D5);
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICFRN + 16, 0x75555555);
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICFRN + 20, 0xFF555555);
		
            
		//32*(n+1) interrupts
		unsigned int n_regs = 1 + (CPU::in32(GIC_INTR_DISTRIBUTOR + ICDICTR)&0x1f);

        //reset interrupt priorities of every shared peripheral interrupt
		//Temporarily setting everybody to the highest (0) priority
		for(unsigned char i = 0; i < n_regs*8; i++)
            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDIPRN + (i * 4), 0x00);
            
        //set CPU target of every shared peripheral interrupt
        for(unsigned char i = 0; i < n_regs; i++)
		{
			if(Traits<System>::multicore)
	            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDIPTRN + (i * 4), 0x03);
			else
	            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDIPTRN + (i * 4), 0x01);
		}
            
        //enable PPIs
		//enabling everybody \o/
        for(unsigned char i = 0; i < n_regs; i++)
			CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISERN + i*4, 0xffffffff);//banked register
        
        //set all interrupts to non-secure
        for(unsigned int i = 0; i < n_regs; i++)
            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISRN + (i * 4), 0xffffffff);
            
		interrupt_interface_init();
        enable_distributor();
	}
    static void interrupt_interface_init(void)
	{
        //disable the priority filter
        CPU::out32(GIC_PROC_INTERFACE + ICCPMR, 0xF0);
        
        //disable preemption of interrupt handling by interrupts
        CPU::out32(GIC_PROC_INTERFACE + ICCBPR, 0x07);
        
        //enable device
        enable_cpu_interface();       
	}
	Zynq_IC()
	{
		//Nothing here, all initialization is done in init()
	}
	static void ipi_send(unsigned int cpu, Interrupt_Id int_id)
	{kout<<"ipi_send"<<endl;}
    static void disable_distributor() {
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDDCR, 0x00000000);
    }
    
    static void enable_distributor() {
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDDCR, 0x00000001);
        //kout << "enable distributor = " << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDDCR) << "\n";
    }

    static void disable_cpu_interface() {
        CPU::out32(GIC_PROC_INTERFACE + ICCICR, 0x00000000);
    }
	
    static void enable_cpu_interface() {
        CPU::out32(GIC_PROC_INTERFACE + ICCICR, 0x00000001);
		kout << "Enabling CPU (IC) interface\n";
        // retorna 1, nao adianta escrever 0x7, significa que nao implementa as extensoes de seguranca
    }
    static void int_vector(Interrupt_Id id,Interrupt_Handler func) {
        db<Zynq_IC>(TRC) << "IC::int_vector(id=" << id << ",hdr="<< (void*)func <<")\n";
        _vector[id] = func;
    }
	static void enable(){
		//init() already did everything
	}
	static void enable(Interrupt_Id id){
        unsigned int word = id / 32;
        id %= 32;
        id = 1 << id;
        
        //disable_distributor();
        //disable_cpu_interface();
        
        //kout << "ICDICFRN0 = " << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDICFRN) << "\n";
        //kout << "ICDICFRN1 = " << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDICFRN + 4) << "\n";
        
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISERN + (word * 4), CPU::in32(GIC_INTR_DISTRIBUTOR + ICDISERN + (word * 4)) | id);
        
        //enable_cpu_interface();
        //enable_distributor();                
    }
	
};





/*
 *
 *
 *
 *
 *
 *
 *
 *
 *
 * Panda IC
 *
 * 
 *
 *
 *
 *
 *
 *
 *
 *
 */





class PandaBoard_IC: public IC_Common
{
protected:

    //PandaBoard has 160 interrupts
    static const unsigned int INTS = 160;
    
    static const CPU::Reg32 PIC_IRQ_STATUS    = 0x14000000; // RO
    static const CPU::Reg32 PIC_IRQ_RAWSTAT   = 0x14000004; // RO
    static const CPU::Reg32 PIC_IRQ_ENABLESET = 0x14000008; // RW
    static const CPU::Reg32 PIC_IRQ_ENABLECLR = 0x1400000C; // WO
    static const CPU::Reg32 PIC_INT_SOFTSET   = 0x14000010; // RW
    static const CPU::Reg32 PIC_INT_SOFTCLR   = 0x14000014; // WO
    static const CPU::Reg32 PIC_FIQ_STATUS    = 0x14000020;
    static const CPU::Reg32 PIC_FIQ_ENABLESET = 0x14000028;
    
    // FIQ/IRQ routing 
    // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0184b/CHDGGDHE.html
    
    static const CPU::Reg32 CM_FIQ_ENSET      = 0x10000068;
    static const CPU::Reg32 CM_IRQ_ENSET      = 0x10000048;
    
    static const unsigned int GIC_PROC_INTERFACE = 0x48240100;
    static const unsigned int GIC_PROC_INTERFACE_SIZE = 0xFF; 
    static const unsigned int GIC_INTR_DISTRIBUTOR = 0x48241000;
    static const unsigned int GIC_INTR_DISTRIBUTOR_SIZE = 0x1000;
    
    static const unsigned int INTERRUPT_MASK = 0x000003FF;
    static const unsigned int SPURIOUS_INTERRUPT = 1023;
    
    //Distributor Register Description
    enum {
        ICDDCR      = 0x000, //Distributor Control Register
        ICDICTR     = 0x004, //Interrupt Controller Type Register    
        ICDIIDR     = 0x008, //Distributor Implementer Identification Registers
        ICDISRN     = 0x080, //0x080 - 0x09C - Interrupt Security Registers
        ICDISERN    = 0x100, //0x104-0x11C Interrupt Set-Enable Registers
        ICDICERN    = 0x180, //0x184-0x19C Interrupt Clear-Enable Registers
        ICDISPRN    = 0x200, //0x200-0x27C Interrupt Set-Pending Registers
        ICDICPRN    = 0x280, //0x280-0x29C Interrupt Clear-Pending Registers
        ICDABRN     = 0x300, //0x300-0x31C Active Bit registers
        ICDIPRN     = 0x400, //0x400-0x4FC Interrupt Priority Registers
        ICDIPTRN    = 0x800, //0x800-0x8FC Interrupt Processor Targets Registers
        ICDICFRN    = 0xC00, //0xC00, 0xC08, 0xC08-0xC3C Interrupt Configuration Registers
        ICPPISR     = 0xD00, //PPI Status Register
        ICSPISRN    = 0xD04, //0xD04-0xD1C SPI Status Registers
        ICDSGIR     = 0xF00, //Software Generated Interrupt Register
        ICPIDR0     = 0xFD0, //Peripheral ID0 register   
        ICPIDR1     = 0xFD4, //Peripheral ID1 register
        ICPIDR2     = 0xFD8, //Peripheral ID2 register
        ICPIDR3     = 0xFDC, //Peripheral ID3 register
        ICPIDR4     = 0xFE0, //Peripheral ID4 register
        ICPIDR5     = 0xFE4, //Peripheral ID5 register
        ICPIDR6     = 0xFE8, //Peripheral ID6 register
        ICPIDR7     = 0xFEC, //Peripheral ID7 register
        ICCIDR0     = 0xFF0, //Component ID0 register
        ICCIDR1     = 0xFF4, //Component ID1 register
        ICCIDR2     = 0xFF8, //Component ID2 register
        ICCIDR3     = 0xFFC, //Component ID3 register
    };        
    
    //Processor Interface Register
    enum {
        ICCICR      = 0x000, //CPU Interface Control Register
        ICCPMR      = 0x004, //Interrupt Priority Mask Register
        ICCBPR      = 0x008, //Binary Point Register
        ICCIAR      = 0x00C, //Interrupt Ack Register
        ICCEOI      = 0x010, //End Of Interrupt Register
        ICCRPR      = 0x014, //Running Priority Register
        ICCHPIR     = 0x018, //Highest Pending Interrupt Register
        ICCABPR     = 0x01C, //Aliased Non-Secure Binary Point Register
        ICCIDR      = 0x0FC, //CPU Interface Implementer Identification Register
    };        
    
    static void install_interrupt_handler(void) {
        //unsigned int branch_offset;                
        
        //set up the raw interrupt handler by creating a branch instruction at the IRQ vector 0x18
        //branch_offset = ((unsigned) &_irq_handler);
        //branch_offset -= Memory_Map<PandaBoard>::BASE + 0x18 + 0x08; //VECTOR_BASE = 0x82000000;
        //CPU::out32(Memory_Map<PandaBoard>::BASE + 0x18, 0xEA000000 | (branch_offset >> 2));
        
        //clean_dcache_line_vma(0x18);
    }
    
    static void interrupt_distributor_init(void) {                 
        //disable the whole distributor controller
        disable_distributor();
        
        //configure every shared peripheral interrupt
        for(unsigned int i = 2; i < INTS / 16; i++)
            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICFRN + (i * 4), 0x00000000);
            
        //reset interrupt priorities of every shared peripheral interrupt
        for(unsigned char i = 8; i < INTS / 4; i++)
            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDIPRN + (i * 4), 0x00);
            
        //set CPU target of every shared peripheral interrupt
        for(unsigned char i = 8; i < INTS / 4; i++)
            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDIPTRN + (i * 4), 0xFF);  
            
        //enable PPIs
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISERN, 0xffff0000);
        
        //set all interrupts to non-secure
        for(unsigned int i = 0; i < INTS / 32; i++)
            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISRN + (i * 4), 0xffffffff);
            
        //disable the priority filter
        CPU::out32(GIC_PROC_INTERFACE + ICCPMR, 0xFF);
        
        //disable preemption of interrupt handling by interrupts
        CPU::out32(GIC_PROC_INTERFACE + ICCBPR, 0x07);
        
        //enable device
        enable_cpu_interface();       
        enable_distributor();
        
        /*unsigned int ints = CPU::in32(GIC_INTR_DISTRIBUTOR + ICDICTR);
        kout << "Supported IRQs = " << ((ints & 0x1f)+ 1) * 32 << "\n"; //output is 160*/
       
        //disable all interrupts
        //for(unsigned int i = 0; i < INTS / 32; i++)
        //    CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICERN + (i * 4), 0xFFFFFFFF);
            
        //clear all interrupts
        //for(unsigned int i = 0; i < INTS / 32; i++)
        //    CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICPRN + (i * 4), 0xFFFFFFFF);
        
        //reset interrupt priorities
        //for(unsigned int i = 0; i < INTS / 4; i++)
        //    CPU::out32(GIC_INTR_DISTRIBUTOR + ICDIPRN + (i * 4), 0x00000000);
        
        //reset interrupt targets
        //for(unsigned int i = 0; i < INTS / 4; i++)
        //    CPU::out32(GIC_INTR_DISTRIBUTOR + ICDIPTRN + (i * 4), 0x00000000);
            
        /* From OMAP4430 TRM page 3407 - CAUTION: Although the hardware interrupts
           can be configured to be rising-edge or high-level sensitive in the INTC,
           the device supports only active-high, level-sensitive configuration.
         */
        //set interrupt configuration to level sensitive
        //for(unsigned int i = 2; i < INTS / 16; i++)
        //    CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICFRN + (i * 4), 0x00000000);
        
        //verify which interrupt is supported    
        /*for(unsigned int i = 0; i < INTS / 32; i++) {
           //kout << "Writing and reading ICDISER[" << i << "] = ";
            CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISERN + (i * 4), 0xFFFFFFFF);
           //kout << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDISERN + (i * 4)) << "\n";
        }*/
        
        //verify secutiry extension
        //unsigned int security = CPU::in32(GIC_INTR_DISTRIBUTOR + ICDICTR) & (1 << 10);
        //kout << "security = " << security << "\n"; //output is 1024, it has security extensions
        //for(unsigned int i = 0; i < INTS / 32; i++) {
            //ouput is 0x00000000, which means that all interrupts are secure 
            //kout << "Interrupt security registers ICDISR[" << i << "] = " << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDISRN + (i * 4)) << "\n";                        
        //}
        
        
        //from interrupt_interface_init
        //for(unsigned char i = 0; i < INTS / 4; i++) 
        //    CPU::out8(GIC_INTR_DISTRIBUTOR + ICDIPRN + i, 0x01); 
            
        //enable PPIs
        //CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISERN, 0xffff0000);
            
        //set the target for the PPIs 0x01 = CPU0
        //for(unsigned char i = 16; i < 32; i++)
        //    CPU::out8(GIC_INTR_DISTRIBUTOR + ICDIPTRN + i, 0x01);
        
        //CPU interface configuration
        
        //set priority mask to 256
        //CPU::out32(GIC_PROC_INTERFACE + ICCPMR, 0x0000000FF);                                                          
        
        //disable preemptions on the ICCBPR register
        //CPU::out32(GIC_PROC_INTERFACE + ICCBPR, 0x07);
        
        
        //install_interrupt_handler();        
            
        //enable the distributor controller
        //enable_distributor();        
        
        //enable_cpu_interface();       
        //enable_distributor();
    }
    
    static void interrupt_interface_init(void) {        
        //disable the distributor and cpu interface controllers
        //disable_distributor();
        //disable_cpu_interface();
        
        //PPIs configuration
        
        //configure SGIs to edge sensitive
        //SGIs is read only always 0xaaaaaaaa
        //CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICFRN, 0x55555555);
        
        //configure PPIs to edge sensitive - maybe it is read only - returns 0x7dc00000
        //CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICFRN + 4, 0x55555555);   
        
        //set the priorities for the SGIs and PPIs as 1
        //I guess when multicore, Machine::cpu_id() should be used GIC_INTR_DISTRIBUTOR + ICDIPRN + (Machine::cpu_id() * 4)
        /*for(unsigned char i = 0; i < INTS / 4; i++) 
            CPU::out8(GIC_INTR_DISTRIBUTOR + ICDIPRN + i, 0x01); 
            
        //enable PPIs
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISERN, 0xffff0000);
            
        //set the target for the PPIs 0x01 = CPU0
        for(unsigned char i = 16; i < 32; i++)
            CPU::out8(GIC_INTR_DISTRIBUTOR + ICDIPTRN + i, 0x01);
        
        //CPU interface configuration
        
        //set priority mask to 256
        CPU::out32(GIC_PROC_INTERFACE + ICCPMR, 0x0000000FF);                                                          
        
        //disable preemptions on the ICCBPR register
        CPU::out32(GIC_PROC_INTERFACE + ICCBPR, 0x07);
        
        //enable the CPU interface and distributor
        enable_cpu_interface();       
        enable_distributor();*/
    }
    
    static void disable_distributor() {
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDDCR, 0x00000000);
    }
    
    static void enable_distributor() {
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDDCR, 0x00000001);
        //kout << "enable distributor = " << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDDCR) << "\n";
    }
    
    static void disable_cpu_interface() {
        CPU::out32(GIC_PROC_INTERFACE + ICCICR, 0x00000000);
    }
    
    static void enable_cpu_interface() {
        CPU::out32(GIC_PROC_INTERFACE + ICCICR, 0x00000001);
        // retorna 1, nao adianta escrever 0x7, significa que nao implementa as extensoes de seguranca
        //kout << "\nenable_cpu_interface() = " << CPU::in32(GIC_PROC_INTERFACE + ICCICR) << "\n"; 
    }
        
    
    PandaBoard_IC() {}

public:

    //16 interprocessor interrupts (IPI) ID[15:0]
    //2 private Timer/Watchdog interrupts ID[30:29]
    //2 legacy nFIQ & nIRQ one per CPU ID28 and ID31
    //128 hardware interrupts ID[159:32]
    /*static const Interrupt_Id MA_IRQ_127        = 127; //Reserved
    static const Interrupt_Id MA_IRQ_126        = 126; //Reserved
    static const Interrupt_Id MA_IRQ_125        = 125; //Reserved
    static const Interrupt_Id MA_IRQ_124        = 124; //Reserved
    static const Interrupt_Id MA_IRQ_123        = 123; //Reserved
    static const Interrupt_Id MA_IRQ_122        = 122; //Reserved
    static const Interrupt_Id MA_IRQ_121        = 121; //Reserved
    static const Interrupt_Id KBD_CTL_IRQ       = 120; //Keyboard controller interrupt
    static const Interrupt_Id SYS_NIRQ2         = 119; //External interrupt 2 (active low)
    static const Interrupt_Id MA_IRQ_118        = 118; //Reserved
    static const Interrupt_Id MA_IRQ_117        = 117; //Reserved
    static const Interrupt_Id MA_IRQ_116        = 116; //Reserved
    static const Interrupt_Id MA_IRQ_115        = 115; //Reserved
    static const Interrupt_Id DMIC_IRQ          = 114; //DMIC interrupt
    static const Interrupt_Id DMM_IRQ           = 113; //DMM interrupt
    static const Interrupt_Id MCPDM_IRQ         = 112; //MCPDM interrupt
    static const Interrupt_Id EMIF2_IRQ         = 111; //EMIF2 interrupt
    static const Interrupt_Id EMIF1_IRQ         = 110; //EMIF1 interrupt
    static const Interrupt_Id MCASP1_AXINT      = 109; //McASP1 transmit interrupt
    static const Interrupt_Id MA_IRQ_108        = 108; //Reserved
    static const Interrupt_Id IVAHD_MAIL_IRQ0   = 107; //IVAHD mailbox interrupt 0
    static const Interrupt_Id MA_IRQ_106        = 106; //Reserved
    static const Interrupt_Id MA_IRQ_105        = 105; //Reserved    
    static const Interrupt_Id IVAHD_IRQ1        = 104; //Sync interrupt from iCONT1
    static const Interrupt_Id IVAHD_IRQ2        = 103; //Sync interrupt from iCONT2 (vDMA)
    static const Interrupt_Id SR_IVA_IRQ        = 102; //SmartReflex IVA interrupt
    static const Interrupt_Id DSS_HDMI_IRQ      = 101; //Display subsystem HDMI interrupt
    static const Interrupt_Id CORTEXM3_MMU_IRQ  = 100; //Cortex-M3 MMU interrupt
    static const Interrupt_Id ABE_MPU_IRQ       = 99;  //Audio back-end interrupt
    static const Interrupt_Id SLIMBUS2_IRQ      = 98;  //SLIMBUS2 interrupt
    static const Interrupt_Id SLIMBUS1_IRQ      = 97;  //SLIMBUS1 interrupt
    static const Interrupt_Id MMC4_IRQ          = 96;  //MMC4 interrupt
	static const Interrupt_Id MA_IRQ_95         = 95;  //Reserved
	static const Interrupt_Id MMC3_IRQ          = 94;  //MMC3 interrupt
	static const Interrupt_Id HSUSB_OTG_DMA_IRQ = 93;  //HSUSB OTG DMA interrupt
	static const Interrupt_Id HSUSB_OTG_IRQ     = 92;  //HSUSB OTG controller interrupt
	static const Interrupt_Id MCSPI3_IRQ        = 91;  //MCSPI3 interrupt
	static const Interrupt_Id FSUSB_SMI_IRQ     = 90;  //FS-USB host controller SMI interrupt	
	static const Interrupt_Id FSUSB_IRQ         = 89;  //FS-USB host controller interrupt
	static const Interrupt_Id C2C_SSCM_IRQ1     = 88;  //C2C GPI interrupt
	static const Interrupt_Id MPU_ICR_IRQ       = 87;  //ICR interrupt
	static const Interrupt_Id MMC2_IRQ          = 86;  //MMC2 interrupt
	static const Interrupt_Id MA_IRQ_85         = 85;  //Reserved
	static const Interrupt_Id DSS_DSI2_IRQ      = 84;  //Display subsystem DSI2 interrupt
	static const Interrupt_Id MMC1_IRQ          = 83;  //MMC1 interrupt
	static const Interrupt_Id MA_IRQ_82         = 82;  //Reserved
	static const Interrupt_Id MA_IRQ_81         = 81;  //Reserved
	static const Interrupt_Id WDT2_IRQ          = 80;  //WDTIMER2 interrupt
	static const Interrupt_Id MA_IRQ_79         = 79;  //Reserved
	static const Interrupt_Id HSUSB_TLL_IRQ     = 78;  //HSUSB MP TLL interrupt
	static const Interrupt_Id HSUSB_EHCI_IRQ    = 77;  //HSUSB MP host interrupt EHCI controller
	static const Interrupt_Id HSUSB_OHCI_IRQ    = 76;  //HSUSB MP host interrupt OHCI controller
	static const Interrupt_Id PBIAS_IRQ         = 75;  //Merged interrupt for PBIASlite1 and 2
	static const Interrupt_Id UAR3_IRQ          = 74;  //UART module 3 interrupt
	static const Interrupt_Id UAR2_IRQ          = 73;  //UART module 2 interrupt
	static const Interrupt_Id UAR1_IRQ          = 72;  //UART module 1 interrupt
	static const Interrupt_Id HSI_DMA_MPU_IRQ   = 71;  //HSI DMA engine MPU request
	static const Interrupt_Id UAR4_IRQ          = 70;  //UART module 4 interrupt
	static const Interrupt_Id FDIF_IRQ_3        = 69;  //Face detect interrupt 3
	static const Interrupt_Id HSI_P2_MPU_IRQ    = 68;  //HSI port 2 interrupt
	static const Interrupt_Id HSI_P1_MPU_IRQ    = 67;  //HSI port 1 interrupt
	static const Interrupt_Id MCSPI2_IRQ        = 66;  //MCSPI2 interrupt
	static const Interrupt_Id MCSPI1_IRQ        = 65;  //MCSPI1 interrupt
	static const Interrupt_Id MA_IRQ_64         = 64;  //Reserved
	static const Interrupt_Id MA_IRQ_63         = 63;  //Reserved
	static const Interrupt_Id I2C4_IRQ          = 62;  //I2C4 interrupt
	static const Interrupt_Id I2C3_IRQ          = 61;  //I2C3 interrupt
	static const Interrupt_Id MA_IRQ_60         = 60;  //Reserved
	static const Interrupt_Id MMC5_IRQ          = 59;  //MMC5 interrupt
	static const Interrupt_Id HDQ_IRQ           = 58;  //HDQ/1 wire interrupt
	static const Interrupt_Id I2C2_IRQ          = 57;  //I2C2 interrupt
	static const Interrupt_Id I2C1_IRQ          = 56;  //I2C1 interrupt
	static const Interrupt_Id MA_IRQ_55         = 55;  //Reserved
	static const Interrupt_Id MA_IRQ_54         = 54;  //Reserved
	static const Interrupt_Id DSS_SDI1_IRQ      = 53;  //Display subsystem DSI1 interrupt
	static const Interrupt_Id MA_IRQ_52         = 52;  //Reserved
	static const Interrupt_Id MA_IRQ_51         = 51;  //Reserved
	static const Interrupt_Id MA_IRQ_50         = 50;  //Reserved
	static const Interrupt_Id MA_IRQ_49         = 49;  //Reserved
	static const Interrupt_Id MCSPI4_IRQ        = 48;  //MCSPI4 interrupt
	static const Interrupt_Id GPT11_IRQ         = 47;  //GPTIMER11 interrupt
	static const Interrupt_Id GPT10_IRQ         = 46;  //GPTIMER10 interrupt
	static const Interrupt_Id GPT9_IRQ          = 45;  //GPTIMER9 interrupt
	static const Interrupt_Id GPT8_IRQ          = 44;  //GPTIMER8 interrupt
	static const Interrupt_Id GPT7_IRQ          = 43;  //GPTIMER7 interrupt
	static const Interrupt_Id GPT6_IRQ          = 42;  //GPTIMER6 interrupt
	static const Interrupt_Id GPT5_IRQ          = 41;  //GPTIMER5 interrupt
	static const Interrupt_Id GPT4_IRQ          = 40;  //GPTIMER4 interrupt
	static const Interrupt_Id GPT3_IRQ          = 39;  //GPTIMER3 interrupt
	static const Interrupt_Id GPT2_IRQ          = 38;  //GPTIMER2 interrupt
	static const Interrupt_Id GPT1_IRQ          = 37;  //GPTIMER1 interrupt
	static const Interrupt_Id WDT3_IRQ          = 36;  //WDTIMER3 overflow
	static const Interrupt_Id MA_IRQ_35         = 35;  //Reserved
	static const Interrupt_Id GPIO6_MPU_IRQ     = 34;  //GPIO6 MPU interrupt
	static const Interrupt_Id GPIO5_MPU_IRQ     = 33;  //GPIO5 MPU interrupt
	static const Interrupt_Id GPIO4_MPU_IRQ     = 32;  //GPIO4 MPU interrupt
	static const Interrupt_Id GPIO3_MPU_IRQ     = 31;  //GPIO3 MPU interrupt
	static const Interrupt_Id GPIO2_MPU_IRQ     = 30;  //GPIO2 MPU interrupt
	static const Interrupt_Id GPIO1_MPU_IRQ     = 29;  //GPIO1 MPU interrupt
	static const Interrupt_Id DSP_MMU_IRQ       = 28;  //DSP MMU interrupt
	static const Interrupt_Id C2C_SSCM_IRQ0     = 27;  //C2C status interrupt
	static const Interrupt_Id MAIL_U0_MPU_IRQ   = 26;  //Mailbox user 0 interrupt
	static const Interrupt_Id DSS_DISPC_IRQ     = 25;  //Display controller interrupt
	static const Interrupt_Id ISS_IRQ5          = 24;  //Imaging subsystem interrupt 5
	static const Interrupt_Id MCBSP3_IRQ        = 23;  //MCBSP3 interrupt
	static const Interrupt_Id MCBSP2_IRQ        = 22;  //MCBSP2 interrupt
	static const Interrupt_Id SGX_IRQ           = 21;  //2D/3D graphics module interrupt
	static const Interrupt_Id GPMC_IRQ          = 20;  //GPMC interrupt
	static const Interrupt_Id SR_CORE_IRQ       = 19;  //SmartReflex Core interrupt
	static const Interrupt_Id SR_MPU_IRQ        = 18;  //SmartReflex MPU interrupt
	static const Interrupt_Id MCBSP1_IRQ        = 17;  //MCBSP1 interrupt
	static const Interrupt_Id MCBSP4_IRQ        = 16;  //MCBSP4 interrupt
	static const Interrupt_Id SDMA_IRQ_3        = 15;  //sDMA interrupt 3
	static const Interrupt_Id SDMA_IRQ_2        = 14;  //sDMA interrupt 2
	static const Interrupt_Id SDMA_IRQ_1        = 13;  //sDMA interrupt 1
	static const Interrupt_Id SDMA_IRQ_0        = 12;  //sDMA interrupt 0
	static const Interrupt_Id PRCM_MPU_IRQ      = 11;  //PRCM interrupt
	static const Interrupt_Id L3_APP_IRQ        = 10;  //L3 interconnect application error
	static const Interrupt_Id L3_DBG_IRQ        = 9;   //L3 interconnect debug error
	static const Interrupt_Id MA_IRQ_8          = 8;   //Reserved
	static const Interrupt_Id SYS_NIRQ1         = 7;   //External interrupt 1 (active low)
	static const Interrupt_Id MA_IRQ_6          = 6;   //Reserved
	static const Interrupt_Id MA_IRQ_5          = 5;   //Reserved
	static const Interrupt_Id ELM_IRQ           = 4;   //Error location process completion
	static const Interrupt_Id MA_IRQ_3          = 3;   //Reserved
	static const Interrupt_Id CTI_IRQ_1         = 2;   //Cross-trigger module 1 (CTI1) interrupt
    static const Interrupt_Id CTI_IRQ_0         = 1;   //Cross-trigger module 0 (CTI0) interrupt
    static const Interrupt_Id L2_CACHE_IRQ      = 0;   //L2 cache controller interrupt*/
   
    static const Interrupt_Id MA_IRQ_127        = 159; //Reserved
    static const Interrupt_Id MA_IRQ_126        = 158; //Reserved
    static const Interrupt_Id MA_IRQ_125        = 157; //Reserved
    static const Interrupt_Id MA_IRQ_124        = 156; //Reserved
    static const Interrupt_Id MA_IRQ_123        = 155; //Reserved
    static const Interrupt_Id MA_IRQ_122        = 154; //Reserved
    static const Interrupt_Id MA_IRQ_121        = 153; //Reserved
    static const Interrupt_Id KBD_CTL_IRQ       = 152; //Keyboard controller interrupt
    static const Interrupt_Id SYS_NIRQ2         = 151; //External interrupt 2 (active low)
    static const Interrupt_Id MA_IRQ_118        = 150; //Reserved
    static const Interrupt_Id MA_IRQ_117        = 149; //Reserved
    static const Interrupt_Id MA_IRQ_116        = 148; //Reserved
    static const Interrupt_Id MA_IRQ_115        = 147; //Reserved
    static const Interrupt_Id DMIC_IRQ          = 146; //DMIC interrupt
    static const Interrupt_Id DMM_IRQ           = 145; //DMM interrupt
    static const Interrupt_Id MCPDM_IRQ         = 144; //MCPDM interrupt
    static const Interrupt_Id EMIF2_IRQ         = 143; //EMIF2 interrupt
    static const Interrupt_Id EMIF1_IRQ         = 142; //EMIF1 interrupt
    static const Interrupt_Id MCASP1_AXINT      = 141; //McASP1 transmit interrupt
    static const Interrupt_Id MA_IRQ_108        = 140; //Reserved
    static const Interrupt_Id IVAHD_MAIL_IRQ0   = 139; //IVAHD mailbox interrupt 0
    static const Interrupt_Id MA_IRQ_106        = 138; //Reserved
    static const Interrupt_Id MA_IRQ_105        = 137; //Reserved    
    static const Interrupt_Id IVAHD_IRQ1        = 136; //Sync interrupt from iCONT1
    static const Interrupt_Id IVAHD_IRQ2        = 135; //Sync interrupt from iCONT2 (vDMA)
    static const Interrupt_Id SR_IVA_IRQ        = 134; //SmartReflex IVA interrupt
    static const Interrupt_Id DSS_HDMI_IRQ      = 133; //Display subsystem HDMI interrupt
    static const Interrupt_Id CORTEXM3_MMU_IRQ  = 132; //Cortex-M3 MMU interrupt
    static const Interrupt_Id ABE_MPU_IRQ       = 131; //Audio back-end interrupt
    static const Interrupt_Id SLIMBUS2_IRQ      = 130; //SLIMBUS2 interrupt
    static const Interrupt_Id SLIMBUS1_IRQ      = 129; //SLIMBUS1 interrupt
    static const Interrupt_Id MMC4_IRQ          = 128; //MMC4 interrupt
	static const Interrupt_Id MA_IRQ_95         = 127; //Reserved
	static const Interrupt_Id MMC3_IRQ          = 126; //MMC3 interrupt
	static const Interrupt_Id HSUSB_OTG_DMA_IRQ = 125; //HSUSB OTG DMA interrupt
	static const Interrupt_Id HSUSB_OTG_IRQ     = 124; //HSUSB OTG controller interrupt
	static const Interrupt_Id MCSPI3_IRQ        = 123; //MCSPI3 interrupt
	static const Interrupt_Id FSUSB_SMI_IRQ     = 122; //FS-USB host controller SMI interrupt	
	static const Interrupt_Id FSUSB_IRQ         = 121; //FS-USB host controller interrupt
	static const Interrupt_Id C2C_SSCM_IRQ1     = 120; //C2C GPI interrupt
	static const Interrupt_Id MPU_ICR_IRQ       = 119; //ICR interrupt
	static const Interrupt_Id MMC2_IRQ          = 118; //MMC2 interrupt
	static const Interrupt_Id MA_IRQ_85         = 117; //Reserved
	static const Interrupt_Id DSS_DSI2_IRQ      = 116; //Display subsystem DSI2 interrupt
	static const Interrupt_Id MMC1_IRQ          = 115; //MMC1 interrupt
	static const Interrupt_Id MA_IRQ_82         = 114; //Reserved
	static const Interrupt_Id MA_IRQ_81         = 113; //Reserved
	static const Interrupt_Id WDT2_IRQ          = 112; //WDTIMER2 interrupt
	static const Interrupt_Id MA_IRQ_79         = 111; //Reserved
	static const Interrupt_Id HSUSB_TLL_IRQ     = 110; //HSUSB MP TLL interrupt
	static const Interrupt_Id HSUSB_EHCI_IRQ    = 109; //HSUSB MP host interrupt EHCI controller
	static const Interrupt_Id HSUSB_OHCI_IRQ    = 108; //HSUSB MP host interrupt OHCI controller
	static const Interrupt_Id PBIAS_IRQ         = 107; //Merged interrupt for PBIASlite1 and 2
	static const Interrupt_Id UAR3_IRQ          = 106; //UART module 3 interrupt
	static const Interrupt_Id UAR2_IRQ          = 105; //UART module 2 interrupt
	static const Interrupt_Id UAR1_IRQ          = 104; //UART module 1 interrupt
	static const Interrupt_Id HSI_DMA_MPU_IRQ   = 103; //HSI DMA engine MPU request
	static const Interrupt_Id UAR4_IRQ          = 102; //UART module 4 interrupt
	static const Interrupt_Id FDIF_IRQ_3        = 101; //Face detect interrupt 3
	static const Interrupt_Id HSI_P2_MPU_IRQ    = 100; //HSI port 2 interrupt
	static const Interrupt_Id HSI_P1_MPU_IRQ    = 99;  //HSI port 1 interrupt
	static const Interrupt_Id MCSPI2_IRQ        = 98;  //MCSPI2 interrupt
	static const Interrupt_Id MCSPI1_IRQ        = 97;  //MCSPI1 interrupt
	static const Interrupt_Id MA_IRQ_64         = 96;  //Reserved
	static const Interrupt_Id MA_IRQ_63         = 95;  //Reserved
	static const Interrupt_Id I2C4_IRQ          = 94;  //I2C4 interrupt
	static const Interrupt_Id I2C3_IRQ          = 93;  //I2C3 interrupt
	static const Interrupt_Id MA_IRQ_60         = 92;  //Reserved
	static const Interrupt_Id MMC5_IRQ          = 91;  //MMC5 interrupt
	static const Interrupt_Id HDQ_IRQ           = 90;  //HDQ/1 wire interrupt
	static const Interrupt_Id I2C2_IRQ          = 89;  //I2C2 interrupt
	static const Interrupt_Id I2C1_IRQ          = 88;  //I2C1 interrupt
	static const Interrupt_Id MA_IRQ_55         = 87;  //Reserved
	static const Interrupt_Id MA_IRQ_54         = 86;  //Reserved
	static const Interrupt_Id DSS_SDI1_IRQ      = 85;  //Display subsystem DSI1 interrupt
	static const Interrupt_Id MA_IRQ_52         = 84;  //Reserved
	static const Interrupt_Id MA_IRQ_51         = 83;  //Reserved
	static const Interrupt_Id MA_IRQ_50         = 82;  //Reserved
	static const Interrupt_Id MA_IRQ_49         = 81;  //Reserved
	static const Interrupt_Id MCSPI4_IRQ        = 80;  //MCSPI4 interrupt
	static const Interrupt_Id GPT11_IRQ         = 79;  //GPTIMER11 interrupt
	static const Interrupt_Id GPT10_IRQ         = 78;  //GPTIMER10 interrupt
	static const Interrupt_Id GPT9_IRQ          = 77;  //GPTIMER9 interrupt
	static const Interrupt_Id GPT8_IRQ          = 76;  //GPTIMER8 interrupt
	static const Interrupt_Id GPT7_IRQ          = 75;  //GPTIMER7 interrupt
	static const Interrupt_Id GPT6_IRQ          = 74;  //GPTIMER6 interrupt
	static const Interrupt_Id GPT5_IRQ          = 73;  //GPTIMER5 interrupt
	static const Interrupt_Id GPT4_IRQ          = 72;  //GPTIMER4 interrupt
	static const Interrupt_Id GPT3_IRQ          = 71;  //GPTIMER3 interrupt
	static const Interrupt_Id GPT2_IRQ          = 70;  //GPTIMER2 interrupt
	static const Interrupt_Id GPT1_IRQ          = 69;  //GPTIMER1 interrupt
	static const Interrupt_Id WDT3_IRQ          = 68;  //WDTIMER3 overflow
	static const Interrupt_Id MA_IRQ_35         = 67;  //Reserved
	static const Interrupt_Id GPIO6_MPU_IRQ     = 66;  //GPIO6 MPU interrupt
	static const Interrupt_Id GPIO5_MPU_IRQ     = 65;  //GPIO5 MPU interrupt
	static const Interrupt_Id GPIO4_MPU_IRQ     = 64;  //GPIO4 MPU interrupt
	static const Interrupt_Id GPIO3_MPU_IRQ     = 63;  //GPIO3 MPU interrupt
	static const Interrupt_Id GPIO2_MPU_IRQ     = 62;  //GPIO2 MPU interrupt
	static const Interrupt_Id GPIO1_MPU_IRQ     = 61;  //GPIO1 MPU interrupt
	static const Interrupt_Id DSP_MMU_IRQ       = 60;  //DSP MMU interrupt
	static const Interrupt_Id C2C_SSCM_IRQ0     = 59;  //C2C status interrupt
	static const Interrupt_Id MAIL_U0_MPU_IRQ   = 58;  //Mailbox user 0 interrupt
	static const Interrupt_Id DSS_DISPC_IRQ     = 57;  //Display controller interrupt
	static const Interrupt_Id ISS_IRQ5          = 56;  //Imaging subsystem interrupt 5
	static const Interrupt_Id MCBSP3_IRQ        = 55;  //MCBSP3 interrupt
	static const Interrupt_Id MCBSP2_IRQ        = 54;  //MCBSP2 interrupt
	static const Interrupt_Id SGX_IRQ           = 53;  //2D/3D graphics module interrupt
	static const Interrupt_Id GPMC_IRQ          = 52;  //GPMC interrupt
	static const Interrupt_Id SR_CORE_IRQ       = 51;  //SmartReflex Core interrupt
	static const Interrupt_Id SR_MPU_IRQ        = 50;  //SmartReflex MPU interrupt
	static const Interrupt_Id MCBSP1_IRQ        = 49;  //MCBSP1 interrupt
	static const Interrupt_Id MCBSP4_IRQ        = 48;  //MCBSP4 interrupt
	static const Interrupt_Id SDMA_IRQ_3        = 47;  //sDMA interrupt 3
	static const Interrupt_Id SDMA_IRQ_2        = 46;  //sDMA interrupt 2
	static const Interrupt_Id SDMA_IRQ_1        = 45;  //sDMA interrupt 1
	static const Interrupt_Id SDMA_IRQ_0        = 44;  //sDMA interrupt 0
	static const Interrupt_Id PRCM_MPU_IRQ      = 43;  //PRCM interrupt
	static const Interrupt_Id L3_APP_IRQ        = 42;  //L3 interconnect application error
	static const Interrupt_Id L3_DBG_IRQ        = 41;  //L3 interconnect debug error
	static const Interrupt_Id MA_IRQ_8          = 40;  //Reserved
	static const Interrupt_Id SYS_NIRQ1         = 39;  //External interrupt 1 (active low)
	static const Interrupt_Id MA_IRQ_6          = 38;  //Reserved
	static const Interrupt_Id MA_IRQ_5          = 37;  //Reserved
	static const Interrupt_Id ELM_IRQ           = 36;  //Error location process completion
	static const Interrupt_Id MA_IRQ_3          = 35;  //Reserved
	static const Interrupt_Id CTI_IRQ_1         = 34;  //Cross-trigger module 1 (CTI1) interrupt
    static const Interrupt_Id CTI_IRQ_0         = 33;  //Cross-trigger module 0 (CTI0) interrupt
    static const Interrupt_Id L2_CACHE_IRQ      = 32;  //L2 cache controller interrupt
    static const Interrupt_Id NIRQ              = 31;  //NIRQ interrupt
    static const Interrupt_Id PRIVATE_WATCHDOG  = 30;  //Each core has its own private watchdog interrupt
    static const Interrupt_Id PRIVATE_TIMER     = 29;  //Each core has its own private timer interrupt
    static const Interrupt_Id NFIQ              = 28;  //NFIQ interrupt
    static const Interrupt_Id GLOBAL_TIMER      = 27;  //Global Timer interrupt
    static const Interrupt_Id RESERVED_IRQ_10   = 26;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_9    = 25;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_8    = 24;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_7    = 23;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_6    = 22;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_5    = 21;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_4    = 20;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_3    = 19;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_2    = 18;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_1    = 17;  //Reserved
    static const Interrupt_Id RESERVED_IRQ_0    = 16;  //Reserved
    static const Interrupt_Id SGI15             = 15;  //Software Generated Interrupt 15
    static const Interrupt_Id SGI14             = 14;  //Software Generated Interrupt 14
    static const Interrupt_Id SGI13             = 13;  //Software Generated Interrupt 13
    static const Interrupt_Id SGI12             = 12;  //Software Generated Interrupt 12
    static const Interrupt_Id SGI11             = 11;  //Software Generated Interrupt 11
    static const Interrupt_Id SGI10             = 10;  //Software Generated Interrupt 10
    static const Interrupt_Id SGI9              = 9;   //Software Generated Interrupt 9
    static const Interrupt_Id SGI8              = 8;   //Software Generated Interrupt 8
    static const Interrupt_Id SGI7              = 7;   //Software Generated Interrupt 7
    static const Interrupt_Id SGI6              = 6;   //Software Generated Interrupt 6
    static const Interrupt_Id SGI5              = 5;   //Software Generated Interrupt 5
    static const Interrupt_Id SGI4              = 4;   //Software Generated Interrupt 4
    static const Interrupt_Id SGI3              = 3;   //Software Generated Interrupt 3
    static const Interrupt_Id SGI2              = 2;   //Software Generated Interrupt 2
    static const Interrupt_Id SGI1              = 1;   //Software Generated Interrupt 1
    static const Interrupt_Id SGI0              = 0;   //Software Generated Interrupt 0


    //export the same interface for the Timer class
    static const Interrupt_Id TIMERINT0 = PRIVATE_TIMER;
    
    static void enable() {
        db<PandaBoard_IC>(TRC) << "IC::enable()\n";
        enable_cpu_interface();
        enable_distributor();
        //CPU::out32(PIC_IRQ_ENABLESET, _mask);
    }

	//enables a single interrupt
    static void enable(Interrupt_Id id) {
        db<PandaBoard_IC>(TRC) << "IC::enable(id=" << id << ")\n";
        unsigned int word = id / 32;
        //unsigned int m = id / 4;
        //unsigned int offset = id % 4;
        id %= 32;
        id = 1 << id;
        //kout << "IC::enable(id=" << (void *) id << ") word = " << (void *) word << 
        //" into " << (void *) (GIC_INTR_DISTRIBUTOR + ICDISERN + (word * 4)) << "\n";
        
        //disable_distributor();
        //disable_cpu_interface();
        
        //kout << "ICDICFRN0 = " << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDICFRN) << "\n";
        //kout << "ICDICFRN1 = " << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDICFRN + 4) << "\n";
        
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDISERN + (word * 4), CPU::in32(GIC_INTR_DISTRIBUTOR + ICDISERN + (word * 4)) | id);
        
       //kout << "IC::enable() set enable register = " << (void *) CPU::in32(GIC_INTR_DISTRIBUTOR + ICDISERN) << "\n";
       //kout << "IC::enable() reading active bit register = " << (void *)  CPU::in32(GIC_INTR_DISTRIBUTOR + ICDABRN) << "\n";                
        
        //enable_cpu_interface();
        //enable_distributor();                
    }

    static void disable() {
        db<PandaBoard_IC>(TRC) << "IC::disable()\n";
        _mask = CPU::in32(PIC_IRQ_ENABLESET);
        //CPU::out32(PIC_IRQ_ENABLESET, 0);
        disable_distributor();
        disable_cpu_interface();
    }

    static void disable(Interrupt_Id id) { 
        db<PandaBoard_IC>(TRC) << "IC::disable(id=" << id << ")\n";
        unsigned int word = id / 32;
        id %= 32;
        id = 1 << id; 
        CPU::out32(GIC_INTR_DISTRIBUTOR + ICDICERN + (word * 4), id);
    }

    static void int_vector(Interrupt_Id id,Interrupt_Handler func) {
        db<PandaBoard_IC>(TRC) << "IC::int_vector(id=" << id << ",hdr="<< (void*)func <<")\n";
        kout << "IC::int_vector(id=" << id << ",hdr="<< (void*)func <<")\n";
        _vector[id] = func;
    }

    static void int_handler();
    static void default_handler(Interrupt_Id id);
    
    static void init();
    
private:
    static Interrupt_Handler _vector[INTS];
    static CPU::Reg32 _mask;
};



__END_SYS

#endif
\end{lstlisting}
