#pragma once

#define MEM_BSS_SRAMD1 __attribute__ ((section (".sramd1.zero")))
#define MEM_BSS_SRAMD2 __attribute__ ((section (".sramd2.zero")))
#define MEM_BSS_SRAMD3 __attribute__ ((section (".sramd3.zero")))

#define MEM_BSS_ITCM __attribute__ ((section (".itcmram")))

#define MEM_DATA_SRAMD1 __attribute__ ((section (".sramd1.nonzero")))
#define MEM_DATA_SRAMD2 __attribute__ ((section (".sramd2.nonzero")))
#define MEM_DATA_SRAMD3 __attribute__ ((section (".sramd3.nonzero")))

#define MEM_NOINIT_SRAMD1 __attribute__ ((section (".sramd1.noinit")))
#define MEM_NOINIT_SRAMD2 __attribute__ ((section (".sramd2.noinit")))
#define MEM_NOINIT_SRAMD3 __attribute__ ((section (".sramd3.noinit")))

#define MEM_DMA_SRAMD1 __attribute__ ((section (".sramd1.dma")))
#define MEM_DMA_SRAMD2 __attribute__ ((section (".sramd2.dma")))
#define MEM_DMA_SRAMD3 __attribute__ ((section (".sramd3.dma")))
