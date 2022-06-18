// See LICENSE.Sifive for license details.
#include <stdint.h>
#include <stddef.h>
#include <platform.h>
#include "testConfig.h"

#include "common.h"

#define DEBUG
#include "kprintf.h"

// Total payload in B
#define PAYLOAD_SIZE_B (30 << 20) // default: 30MiB
// A sector is 512 bytes, so (1 << 11) * 512B = 1 MiB
#define SECTOR_SIZE_B 512
// Payload size in # of sectors
#define PAYLOAD_SIZE (PAYLOAD_SIZE_B / SECTOR_SIZE_B)

// The sector at which the BBL partition starts
#define BBL_PARTITION_START_SECTOR 34

#ifndef TL_CLK
#error Must define TL_CLK
#endif

#define F_CLK TL_CLK

static volatile uint32_t * const spi = (void *)(SPI_CTRL_ADDR);

static inline uint8_t spi_xfer(uint8_t d)
{
	int32_t r;

	REG32(spi, SPI_REG_TXFIFO) = d;
	do {
		r = REG32(spi, SPI_REG_RXFIFO);
	} while (r < 0);
	return r;
}

static inline uint8_t sd_dummy(void)
{
	return spi_xfer(0xFF);
}

static uint8_t sd_cmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	unsigned long n;
	uint8_t r;

	REG32(spi, SPI_REG_CSMODE) = SPI_CSMODE_HOLD;
	sd_dummy();
	spi_xfer(cmd);
	spi_xfer(arg >> 24);
	spi_xfer(arg >> 16);
	spi_xfer(arg >> 8);
	spi_xfer(arg);
	spi_xfer(crc);

	n = 1000;
	do {
		r = sd_dummy();
		if (!(r & 0x80)) {
//			dprintf("sd:cmd: %hx\r\n", r);
			goto done;
		}
	} while (--n > 0);
	kputs("sd_cmd: timeout");
done:
	return r;
}

static inline void sd_cmd_end(void)
{
	sd_dummy();
	REG32(spi, SPI_REG_CSMODE) = SPI_CSMODE_AUTO;
}


static void sd_poweron(void)
{
	long i;
	REG32(spi, SPI_REG_SCKDIV) = (F_CLK / 300000UL);
	REG32(spi, SPI_REG_CSMODE) = SPI_CSMODE_OFF;
	for (i = 10; i > 0; i--) {
		sd_dummy();
	}
	REG32(spi, SPI_REG_CSMODE) = SPI_CSMODE_AUTO;
}

static int sd_cmd0(void)
{
	int rc;
	dputs("CMD0");
	rc = (sd_cmd(0x40, 0, 0x95) != 0x01);
	sd_cmd_end();
	return rc;
}

static int sd_cmd8(void)
{
	int rc;
	dputs("CMD8");
	rc = (sd_cmd(0x48, 0x000001AA, 0x87) != 0x01);
	sd_dummy(); /* command version; reserved */
	sd_dummy(); /* reserved */
	rc |= ((sd_dummy() & 0xF) != 0x1); /* voltage */
	rc |= (sd_dummy() != 0xAA); /* check pattern */
	sd_cmd_end();
	return rc;
}

static void sd_cmd55(void)
{
	sd_cmd(0x77, 0, 0x65);
	sd_cmd_end();
}

static int sd_acmd41(void)
{
	uint8_t r;
	dputs("ACMD41");
	do {
		sd_cmd55();
		r = sd_cmd(0x69, 0x40000000, 0x77); /* HCS = 1 */
	} while (r == 0x01);
	return (r != 0x00);
}

static int sd_cmd58(void)
{
	int rc;
	dputs("CMD58");
	rc = (sd_cmd(0x7A, 0, 0xFD) != 0x00);
	rc |= ((sd_dummy() & 0x80) != 0x80); /* Power up status */
	sd_dummy();
	sd_dummy();
	sd_dummy();
	sd_cmd_end();
	return rc;
}

static int sd_cmd16(void)
{
	int rc;
	dputs("CMD16");
	rc = (sd_cmd(0x50, 0x200, 0x15) != 0x00);
	sd_cmd_end();
	return rc;
}

static uint16_t crc16_round(uint16_t crc, uint8_t data) {
	crc = (uint8_t)(crc >> 8) | (crc << 8);
	crc ^= data;
	crc ^= (uint8_t)(crc >> 4) & 0xf;
	crc ^= crc << 12;
	crc ^= (crc & 0xff) << 5;
	return crc;
}

#define SPIN_SHIFT	6
#define SPIN_UPDATE(i)	(!((i) & ((1 << SPIN_SHIFT)-1)))
#define SPIN_INDEX(i)	(((i) >> SPIN_SHIFT) & 0x3)

static const char spinner[] = { '-', '/', '|', '\\' };

static int copy(void)
{
	volatile uint8_t *p = (void *)(PAYLOAD_DEST);
	long i = PAYLOAD_SIZE;
	int rc = 0;

	dputs("CMD18");

	kprintf("LOADING 0x%xB PAYLOAD\r\n", PAYLOAD_SIZE_B);
	kprintf("LOADING  ");

	// TODO: Speed up SPI freq. (breaks between these two values)
	//REG32(spi, SPI_REG_SCKDIV) = (F_CLK / 16666666UL);
	REG32(spi, SPI_REG_SCKDIV) = (F_CLK / 5000000UL);
	if (sd_cmd(0x52, BBL_PARTITION_START_SECTOR, 0xE1) != 0x00) {
		sd_cmd_end();
		return 1;
	}
	do {
		uint16_t crc, crc_exp;
		long n;

		crc = 0;
		n = SECTOR_SIZE_B;
		while (sd_dummy() != 0xFE);
		do {
			uint8_t x = sd_dummy();
			*p++ = x;
			crc = crc16_round(crc, x);
		} while (--n > 0);

		crc_exp = ((uint16_t)sd_dummy() << 8);
		crc_exp |= sd_dummy();

		if (crc != crc_exp) {
			kputs("\b- CRC mismatch ");
			rc = 1;
			break;
		}

		if (SPIN_UPDATE(i)) {
			kputc('\b');
			kputc(spinner[SPIN_INDEX(i)]);
		}
	} while (--i > 0);
	sd_cmd_end();

	sd_cmd(0x4C, 0, 0x01);
	sd_cmd_end();
	kputs("\b ");
	return rc;
}

int main(void)
{
	kputs("INIT");
	sd_poweron();
	if (sd_cmd0() ||
	    sd_cmd8() ||
	    sd_acmd41() ||
	    sd_cmd58() ||
	    sd_cmd16() ||
	    copy()) {
		kputs("ERROR");
		return 1;
	}

	kputs("BOOT");

	__asm__ __volatile__ ("fence.i" : : : "memory");

	return 0;
}

typedef enum
{
    true = 1, false = 0
} bool;


int init_uart(void)
{
	REG32(uart, UART_REG_TXCTRL) = UART_TXEN;
	kprintf("[\033[32mSuccessful\033[0m] Initialize UART\r\n\r\n");
	return 0;
}

int print_multi_core_info(int core_num1, int core_num2)
{
	kprintf("\033[33m-----> CORE %u Write, CORE %u Read \033[0m\r\n", core_num1, core_num2);
	return 0;
}

static volatile DEFINED_TYPE * const mem_start_addr = (void *)(MEMORY_MEM_ADDR); // which is 0x80000000UL(32 bits)


int singlecore_mem_wr_B2B(int core_num)
{
	bool whetherError = false;
	kprintf("\033[33m--> CORE %u \033[0m\r\n", core_num);
	kputs("[\033[33mBegin\033[0m] Single-core Memory Back-to-back Write & Read ");
	kputs("Checking Address...");
	for (size_t i = 0; i < MEMORY_WR_SIZE; i++) {
		if (i % OUTPUT_SHIFT == 0)
			kprintf("\r%x", mem_start_addr + i);			
        
		mem_start_addr[i] = i % TYPE_RANGE;
        if (mem_start_addr[i] != i % TYPE_RANGE){
			whetherError = true;
			kprintf("\r[\033[31mError\033[0m] Mismatch in %x \r\n", mem_start_addr + i);
			kputs("Checking Address...");
		}
    }
	if (whetherError){
		kprintf("\r[\033[33mFinished\033[0m] Single-core Memory Back-to-back Write & Read \r\n\r\n");
	}else{
		kprintf("\r[\033[32mSuccessful\033[0m] Single-core Memory Back-to-back Write & Read \r\n\r\n");
	}
	__asm__ __volatile__ ("fence.i" : : : "memory");
	return 0;
}

int singlecore_mem_wr_nonB2B(int core_num)
{
	bool whetherError = false;
	kprintf("\033[33m--> CORE %u \033[0m\r\n", core_num);
	kputs("[\033[33mBegin\033[0m] Single-core Memory Non-back-to-back Write & Read");
	kputs("Writing to Address...");
	for (size_t i = 0; i < MEMORY_WR_SIZE; i++) {
		if (i % OUTPUT_SHIFT == 0)
	        kprintf("\r%x", mem_start_addr + i);	
		mem_start_addr[i] = i % TYPE_RANGE;
	}
	kprintf("\rFinished Writing \r\n");

	kputs("Reading and Checking Address...");
	for (size_t i = 0; i < MEMORY_WR_SIZE; i++) {
		if (i % OUTPUT_SHIFT == 0)
			kprintf("\r%x", mem_start_addr + i);
        if (mem_start_addr[i] != i % TYPE_RANGE){
			whetherError = true;
			kprintf("\r[\033[31mError\033[0m] Mismatch in %x \r\n", mem_start_addr + i);
			kputs("Reading and Checking Address...");
		}
    }
	if (whetherError){
		kprintf("\r[\033[33mFinished\033[0m] Single-core Memory Non-back-to-back Write & Read \r\n\r\n");
	}else{
		kprintf("\r[\033[32mSuccessful\033[0m] Single-core Memory Non-back-to-back Write & Read \r\n\r\n");
	}
	__asm__ __volatile__ ("fence.i" : : : "memory");
	return 0;
}

int singlecore_mem_Bit_wr_B2B(int core_num)
{
	bool whetherError = false;
	kprintf("\033[33m--> CORE %u \033[0m\r\n", core_num);
	kputs("[\033[33mBegin\033[0m] Single-core Memory Bits Back-to-back Write & Read");
	kputs("Checking the bits in Address...");
	for (size_t i = 0; i < MEMORY_WR_SIZE_BIT; i++) {
		for (int j = 0; j < TYPE_WIDTH; j++) {
			if (i % OUTPUT_SHIFT == 0)
				kprintf("\r%x", mem_start_addr + i);	
            mem_start_addr[i] = 1 << j;
			if (mem_start_addr[i] != 1 << j){
				whetherError = true;
				kprintf("\r[\033[31mError\033[0m] Mismatch in %x bit %u \r\n", mem_start_addr + i, j);
				kputs("Checking the bits in Address...");
			}
        }
	}
	if (whetherError){
		kprintf("\r[\033[33mFinished\033[0m] Single-core Memory Bits Back-to-back Write & Read \r\n\r\n");
	}else{
		kprintf("\r[\033[32mSuccessful\033[0m] Single-core Memory Bits Back-to-back Write & Read \r\n\r\n");
	}
	__asm__ __volatile__ ("fence.i" : : : "memory");
	return 0;
}

int singlecore_mem_Bit_wr_nonB2B(int core_num)
{
	bool whetherError = false;
	kprintf("\033[33m--> CORE %u \033[0m\r\n", core_num);
	kputs("[\033[33mBegin\033[0m] Single-core Memory Bits Non-back-to-back Write & Read");
	for (int j = 0; j < TYPE_WIDTH; j++) {
		kprintf("\rUsing bit %u to write Address...\r\n", j);
		for (size_t i = 0; i < MEMORY_WR_SIZE_BIT; i++) {
			if (i % OUTPUT_SHIFT == 0)
				kprintf("\r%x", mem_start_addr + i);	
			mem_start_addr[i] = 1 << j;
		}
		kprintf("\rUsing bit %u to read and check Address...\r\n", j);
		for (size_t i = 0; i < MEMORY_WR_SIZE_BIT; i++) {
			if (i % OUTPUT_SHIFT == 0)
				kprintf("\r%x", mem_start_addr + i);
			if (mem_start_addr[i] != 1 << j){
				whetherError = true;
				kprintf("\r[\033[31mError\033[0m] Mismatch in %x bit %u \r\n", mem_start_addr + i, j);
				kprintf("\rUsing bit %u to read and check Address...\r\n", j);
			}
		}
	}
	if (whetherError){
		kprintf("\r[\033[33mFinished\033[0m] Single-core Memory Bits Non-back-to-back Write & Read \r\n\r\n");
	}else{
		kprintf("\r[\033[32mSuccessful\033[0m] Single-core Memory Bits Non-back-to-back Write & Read \r\n\r\n");
	}
	__asm__ __volatile__ ("fence.i" : : : "memory");
	return 0;
}


int multicore_mem_w (int core_num)
{
	kprintf("\033[33m--> CORE %u \033[0m\r\n", core_num);
	kputs("[\033[33mBegin\033[0m] Multi-core Memory Write");
	kputs("Writing to Address...");
	for (size_t i = 0; i < MEMORY_WR_SIZE; i++){
		if (i % OUTPUT_SHIFT == 0)
			kprintf("\r%x", mem_start_addr + i);
		mem_start_addr[i] = i % TYPE_RANGE;
	}
	kprintf("\rFinished Multi-core Memory Write \r\n");
	__asm__ __volatile__ ("fence.i" : : : "memory");
	return 0;
}

int multicore_mem_r (int core_num)
{
	bool whetherError = false;
	kprintf("\033[33m--> CORE %u \033[0m\r\n", core_num);
	kputs("[\033[33mBegin\033[0m] Multi-core Memory Read and Check");
	kputs("Reading and Checking Address...");
	for (size_t i = 0; i < MEMORY_WR_SIZE; i++){
		if (i % OUTPUT_SHIFT == 0)
			kprintf("\r%x", mem_start_addr + i);
		if (mem_start_addr[i] != i % TYPE_RANGE){
			whetherError = true;
			kprintf("\r[\033[31mError\033[0m] Mismatch in %x\r\n", mem_start_addr + i);
			kputs("Reading and Checking Address...");
		}
	}
	if (whetherError){
		kprintf("\r[\033[33mFinished\033[0m] Multi-core Memory Read and Check \r\n\r\n");
	}else{
		kprintf("\r[\033[32mSuccessful\033[0m] Multi-core Memory Read and Check \r\n\r\n");
	}
	__asm__ __volatile__ ("fence.i" : : : "memory");
	return 0;
}
