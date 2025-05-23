MEMORY
{
    FLASH (rx)     : ORIGIN = 0x08000000 + 128K * 10, LENGTH = 128K * 6
    ITCMRAM (rwx)  : ORIGIN = 0x00000000, LENGTH = 64K
    TCMRAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 128K
    SRAMD1 (rwx)   : ORIGIN = 0x24000000, LENGTH = 512K
    SRAMD2 (rwx)   : ORIGIN = 0x30000000, LENGTH = 288K
    SRAMD3 (rwx)   : ORIGIN = 0x38000000, LENGTH = 64K
}

SECTIONS
{
  /* The startup code goes first into FLASH */
  .font :
  {
    KEEP(*(.font)) /* Startup code */
  } >FLASH
}