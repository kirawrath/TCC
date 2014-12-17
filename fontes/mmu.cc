\begin{lstlisting}
// EPOS-- ARMV7 MMU Mediator Implementation

#include <mmu.h>
#include <machine.h>
#include <system/traits.h>

__BEGIN_SYS

// Class attributes
ARMV7_MMU::List ARMV7_MMU::_free;
/*
ARMV7_MMU::Phy_Addr ARMV7_MMU::alloc(unsigned int bytes) {
    Phy_Addr phy(false);
    if(bytes) {
        List::Element * e = _free.search_decrementing(bytes);
        if(e){
			//phy = e->object() + e->size();
            phy = (unsigned int)e->object() + (unsigned int)e->size();
			kout << "MMU::alloc: e->object()="<<(unsigned int)e->object()
				<< ", e->size()="<<(unsigned int)e->size()<<endl;
		}
        else {
            kout << "ARMV7_MMU::alloc() failed!\n";
            db<ARMV7_MMU>(WRN) << "ARMV7_MMU::alloc() failed!\n";
        }
    }
    db<ARMV7_MMU>(TRC) << "ARMV7_MMU::alloc(bytes=" << bytes << ") => "
                      << (void *)phy << "\n";
    //kout << "ARMV7_MMU::alloc(bytes=" << bytes << ") => " << (void *)phy << "\n";
    return phy;
}
*/
void ARMV7_MMU::free(Phy_Addr addr, int n) {
    db<ARMV7_MMU>(TRC) << "ARMV7_MMU::free(addr=" << (void *)addr
                      << ",n=" << n << ")\n";
                      
	if(addr % 4 != 0){
		db<ARMV7_MMU>(ERR) << "Unaligned address to be freed!\n";
		kout << "Unaligned address to be freed!\n";
		Machine::panic();
	}

    if(addr && n) {
       	List::Element * e = new (addr) List::Element(addr, n);
        List::Element * m1, * m2;
        _free.insert_merging(e, &m1, &m2);
    }
}
__END_SYS
\end{lstlisting}
