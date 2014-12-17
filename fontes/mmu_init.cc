\begin{lstlisting}
// EPOS-- ARMV7 MMU Mediator Initialization

#include <mmu.h>
#include <system.h>

//extern "C" unsigned __bss_end__;
//extern "C" unsigned MMUTable;

__BEGIN_SYS


void ARMV7_MMU::init()
{
	db<Init, ARMV7_MMU>(TRC) << "ARMV7_MMU::init()\n";

	/* L1 PT: 4096 entries 4 bytes long.
	 * L2 PT: 256 entries 4 bytes long. There is one L2 PT for each entry in L1 (4096).
	 * Therefore, MMU page tables size = 4*4096 + 4096*4*256 = 0x404000
	 * However, the current implementation maps the 512mb:1023mb into 0:511mb, saving 512*1024 bytes.
	 * */

	ARMV7_MMU::build_PTs(); //Build page tables.

	 /*unsigned int* p = (unsigned int*)0x1ffffffc;
	 unsigned int* q = (unsigned int*)(0x1ffffffc + 0x20000000);
	 *p = 0xabcd;
	 kout << "*p=" << *p << endl;
	 kout << "*q=" << *q << endl;

	db<Init, ARMV7_MMU>(TRC)<<"Enabling MMU...";
	ARMV7_MMU::enable();
	//ARMV7_MMU::enable2();
	db<Init, ARMV7_MMU>(TRC)<<"done.\n";

	kout << "*p=" << *p << endl;
	kout << "*q=" << *q << endl;
	*/
	// let our stack breath!
	const unsigned int limit = 
		Memory_Map<Machine>::MEM_TOP - Traits<Machine>::APPLICATION_STACK_SIZE;
	
	//Getting the memory position right after the end of the MMU Table
	CPU::Phy_Addr base = 0x104000+0x404000-0x80000;
	if(base % 0x4000 != 0)
		base = base + (4 - base % 4);

	kout << "mem base = " << base << " limit = " << limit << "\n";
	ARMV7_MMU::free(base, pages(limit - base));
	kout << "Memory freed!" << endl;
}


void ARMV7_MMU::disable_domains()
{
	unsigned int x=0xffffffff; //all ones = manager (no domain check).
	ASMV("mcr p15,0,%0,c3,c0,0 \n"
			::"r"(x):);
}
void ARMV7_MMU::disable_access_flags()
{
	unsigned int old, flag = 1<<29;
	ASMV("mrc p15,0,%0,c1,c0,0 \n"
			"bic %1, %0, %1 \n"
			"mcr p15,0,%1,c1,c0,0 \n"
			:"=r"(old), "=r"(flag) : : );
}
void ARMV7_MMU::disable()
{
	unsigned int flag;
	ASMV("mrc p15, 0, %0, c1, c0, 0 \n"
			"bic %0, %0, #1\n"
			"mcr p15, 0, %0, c1, c0, 0 \n":
			"=r"(flag) : : "cc");
}
void ARMV7_MMU::set_ttbr()
{
	ASMV("mcr p15, 0, %0, c2, c0, 0 \n"
			"mcr p15, 0, %0, c2, c0, 1 \n"
			: : "r"(MMU_TABLE_ADDR) : "cc");
}
unsigned int ARMV7_MMU::check_ttbr()
{
	unsigned int ret;
	ASMV("mrc p15,0,%0,c2,c0,2 \n"
			:"=r"(ret) : :);
	if((ret&0x7)==0) kout<<"Using TTBR0 only!\n";

	ASMV("mrc p15, 0, %0, c2, c0, 0 \n"
			: "=r"(ret): :);
	return ret;
}
void ARMV7_MMU::enable2()
{
	unsigned long ttb_base = (unsigned long)MMU_TABLE_ADDR;
	unsigned long ulTemp;
	/* Disable MMU and Caches first of all */
	asm volatile (                                                                    \
			"mrc    p15, 0, r0, c1, c0, 0;"   /* Get MMU Control Register content */      \
			"bic    r0, r0, #0x1000;"         /* MMU_Control_I*/                          \
			"bic    r0, r0, #0x0005;"         /* MMU_Control_M | MMU_Control_C */         \
			"mcr    p15, 0, r0, c1, c0, 0;"   /* disable MMU, ICache, DCache */           \
			:                                                                             \
			:                                                                             \
			: "r0" /* clobber list */);
	/* Set the TTB register */
	asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base) /*:*/);

	/* Set the Domain Access Control Register */
	ulTemp = 0xffffffff;// or 0?
	asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(ulTemp) /*:*/);

	//ARMV7_MMU::build_PTs();

	/* Invalidate Caches, Activate Caches */
	kout << "Invalidating all" << endl;
	ARMV7_MMU::invalidate_all();
	kout << "enabling..."<<endl;
	asm volatile (                                                                      \
			//"mov    r0, #0;"                                                                  
			//"mcr    p15,0,r0,c7,c7,0;"        /* invalidate  i/d-cache */                   
			//"mcr    p15,0,r0,c8,c7,0;"        /* invalidate TLBs */                         
			
			"mrc    p15, 0, r1, c1, c0, 0;"   /* Get MMU Control Register content */        \
			"orr    r1, r1, #0x1000;"         /* MMU_Control_I*/                            \
			"orr    r1, r1, #0x0005;"         /* MMU_Control_M | MMU_Control_C */           \
			"ldr    r0, =VirtualStart;"                                                     \
			"cmp    r0, #0;"                  /* make sure no stall on "mov pc,r0" below */ \
			"dsb \n"
			"isb \n"
			"dmb \n"
			"mcr    p15, 0, r1, c1, c0, 0;"   /* enable MMU, ICache, DCache */              \
			"mov    pc, r0;"                  /* jump to new virtual address */             \
			"and r0, r0, r0;"                                                                          \
			"VirtualStart:"                                                                 \
			"dsb \n"
			"isb \n"
			"dmb \n"
			:                                                                               \
			:                                                                               \
			: "r0","r1","memory" /* clobber list */);



	kout << ">>>>>>>> MMU ENABLED <<<<<<<" << endl;

}
void ARMV7_MMU::enable()
{
	ARMV7_MMU::set_ttbr();
	ARMV7_MMU::disable_domains();
	ARMV7_MMU::disable_access_flags();
	unsigned int flag;

	//shouldn't I disable/clean caches first?
	//TLB?
	//ARMV7_MMU::invalidate_all(); //implicit cache_disable


	ASMV("mrc p15, 0, %0, c1, c0, 0 \n"
			:"=r"(flag) : :);
	//flag=(flag|1);
	flag=(flag|0x1005);
	//flag=(flag|0xc03807);
	kout << "Enabling MMU now!" << hex << flag << endl;;
	ASMV(//"orr %0, %0, #0x1 \n"
			"dsb \n"
			"isb \n"
			"dmb \n"
			"mcr p15, 0, %0, c1, c0, 0 \n"

			"dsb \n"
			"dsb \n"
			"dsb \n"
			"and r0, r0, r0 \n"
			"and r0, r0, r0 \n"
			"isb \n"
			::"r"(flag): );

	//enable_caches();
	kout << "Enabled!\n" << endl;
}
void ARMV7_MMU::enable_caches()
{   //setting bits 2 (data cache) and 12 (I-cache).
	unsigned int flag;//, val=0x1004;
	ASMV("mrc p15, 0, %0, c1, c0, 0 \n"
			:"=r"(flag) : : );
	flag=flag|0x1004;
	ASMV(
			"mcr p15, 0, %0, c1, c0, 0 \n"

			"and r0, r0, r0\n"
			"and r0, r0, r0\n"
			"dsb\n"
			"and r0, r0, r0\n"
			"and r0, r0, r0\n"
			"isb\n"
			::"r"(flag) : );
}
void ARMV7_MMU::disable_caches()
{   
	unsigned int flag;//, val=0x1004;
	ASMV("mrc p15, 0, %0, c1, c0, 0 \n"
			:"=r"(flag) : : );
	flag=flag & (~0x1004);
	ASMV(
			"mcr p15, 0, %0, c1, c0, 0 \n"

			"and r0, r0, r0\n"
			"and r0, r0, r0\n"
			"dsb\n"
			"and r0, r0, r0\n"
			"and r0, r0, r0\n"
			"isb\n"
			::"r"(flag) : );
}
__END_SYS

\end{lstlisting}
