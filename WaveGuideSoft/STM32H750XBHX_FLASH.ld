/*
******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Author      : STM32CubeIDE
**
**  Abstract    : Linker script for STM32H7 series
**                128Kbytes FLASH and 1056Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** Copyright (c) 2023 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(ITCMRAM) + LENGTH(ITCMRAM);    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Stack_Size = 0x400; /* required amount of stack */
_Min_Heap_Size  = 4096; /* required amount of heap  */
_Min_Dma_Size   = 65536;  /* required amount of DMA */
_Min_BMDA_Size  = 4096;   /* required amount of BMDA */

/* Specify the memory areas */
MEMORY
{
    FLASH (rx)     : ORIGIN = 0x08000000, LENGTH = 2048K
    ITCMRAM (rwx)  : ORIGIN = 0x00000000, LENGTH = 64K
    TCMRAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 128K
    SRAMD1 (rwx)   : ORIGIN = 0x24000000, LENGTH = 512K
    SRAMD2 (rwx)   : ORIGIN = 0x30000000, LENGTH = 288K
    SRAMD3 (rwx)   : ORIGIN = 0x38000000, LENGTH = 64K
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  .itcmram(NOLOAD) :
  {
    . = ALIGN(4);
    _itcmram_start = .;
    *(.itcmram) /* Startup code */
    _itcmram_end = .;
    . = ALIGN(4);
  } >TCMRAM

  .specify_sramd1_bss(NOLOAD) : 
  {
    . = ALIGN(4);     /* DMA sections   */
    _sramd1_dma_start = .;
    *(.sramd1.dma)
    . = _sramd1_dma_start + _Min_Dma_Size;

    . = ALIGN(4);
    *(.sramd1.noinit) /* NOINIT section */

    . = ALIGN(4);     /* BSS sections*/
    _specify_sramd1_bss_start = .;
    *(.sramd1.zero)
    . = ALIGN(4);
    _specify_sramd1_bss_end = .;

    . = ALIGN(4);    /* HEAP section */
    PROVIDE(_heap_begin = .);
    . = . + _Min_Heap_Size;
    PROVIDE(_heap_end = .);
    . = ALIGN(4);

  } >SRAMD1

  .specify_sramd2_bss(NOLOAD) : 
  {
    *(.sramd2.noinit)
    . = ALIGN(4);
    _specify_sramd2_bss_start = .;
    *(.sramd2.zero)
    . = ALIGN(4);
    _specify_sramd2_bss_end = .;
  } >SRAMD2

  .specify_sramd3_bss(NOLOAD) :
  {
    . = ALIGN(4);     /* BDMA sections   */
    _sramd3_bdma_start = .;
    *(.sramd3.dma)
    . = _sramd3_bdma_start + _Min_BMDA_Size;

    *(.sramd3.noinit)
    
    . = ALIGN(4);
    _specify_sramd3_bss_start = .;
    *(.sramd3.zero)
    . = ALIGN(4);
    _specify_sramd3_bss_end = .;
  } >SRAMD3

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data :
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >ITCMRAM AT> FLASH

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >ITCMRAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}


