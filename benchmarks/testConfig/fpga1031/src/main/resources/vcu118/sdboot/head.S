#include <platform.h>
#include <smp.h>
#include "common.h"
#include "testConfig.h"

  .section .text.init
  .option norvc
  .globl _prog_start
_prog_start:
  // initialize
  li sp, (PAYLOAD_DEST + 0xffff000)
  li s1, 0 // assume hartid 0 not pause
  smp_pause(s1, s2)
  call init_uart
  smp_resume(s1, s2)

  li s3, CORE_NUM // Core Num  
  li s4, 0  
single_core:
  mv s1, s4
  smp_pause(s1, s2)
  csrr a0, mhartid
  call singlecore_mem_wr_B2B
  csrr a0, mhartid
  call singlecore_mem_wr_nonB2B
  csrr a0, mhartid
  call singlecore_mem_Bit_wr_B2B
  csrr a0, mhartid
  call singlecore_mem_Bit_wr_nonB2B
  smp_resume(s1, s2)
  addi s4, s4, 1
  blt s4, s3, single_core

  li s4, 0
multi_core:
  li s5, 0  
multi_core_1:
  beq s4, s5, increase
  // hart s4 -- For notice and write
  mv s1, s4
  smp_pause(s1, s2)
  mv a0, s4
  mv a1, s5
  call print_multi_core_info
  csrr a0, mhartid
  call multicore_mem_w
  smp_resume(s1, s2)
  // hart s5 -- For read and check
  mv s1, s5
  smp_pause(s1, s2)
  csrr a0, mhartid
  call multicore_mem_r
  smp_resume(s1, s2)
increase:    
  addi s5, s5, 1
  blt s5, s3, multi_core_1
  addi s4, s4, 1
  blt s4, s3, multi_core

  li s1, 0
  smp_pause(s1, s2)
  li sp, (PAYLOAD_DEST + 0xffff000)
  call main
  smp_resume(s1, s2)
  csrr a0, mhartid // hartid for next level bootloader
  la a1, dtb // dtb address for next level bootloader
  li s1, PAYLOAD_DEST
  jr s1
  
  .section .dtb
  .align 3
dtb:

