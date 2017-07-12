/*
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (jdb@lartmaker.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
		__LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

extern void asm_read128(uint32_t *value, void *addr);
extern void asm_write128(uint32_t *value, void *addr);


uint32_t asm_read32(void* addr) {
	uint32_t ret;

	__asm ("ldr %[value], [%[address]]"
			: [value] "=r" (ret)
			: [address] "r" (addr)
		);

	return ret;
}

int main(int argc, char **argv) {
	int fd;
	void *map_base, *virt_addr;
	uint64_t read_result, writeval;
	uint32_t *value_ptr;
	off_t target;
	int access_type = 'w';
	int printbuffer = 0;
	int i;

	value_ptr = (uint32_t *)malloc(64);
	if (!value_ptr) {
		fprintf(stderr, "Failed to allocate memory for result buffer (64 bytes)\n");
		exit(1);
	}

	if(argc < 2) {
		fprintf(stderr, "\nUsage:\t%s { address } [ type [ data ] ]\n"
				"\taddress : memory address to act upon\n"
				"\ttype    : access operation type : [b]yte, [h]alfword, [w]ord\n"
				"\tdata    : data to be written\n\n",
				argv[0]);
		exit(1);
	}
	target = strtoul(argv[1], 0, 0);

	if(argc > 2)
		access_type = tolower(argv[2][0]);


	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
	printf("/dev/mem opened.\n");
	fflush(stdout);

	/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
	if(map_base == (void *) -1) FATAL;
	printf("Memory mapped at address %p.\n", map_base);
	fflush(stdout);

	virt_addr = map_base + (target & MAP_MASK);

	if (argc <=3 ) {
		switch(access_type) {
			case 'b':
				read_result = *((uint8_t *) virt_addr);
				break;
			case 'h':
				read_result = *((uint16_t *) virt_addr);
				break;
			case 'w':
				read_result = asm_read32(virt_addr);
				break;
			case 'l':
				read_result = *((uint64_t *) virt_addr);
				break;
			case 'q':
				asm_read128(value_ptr, virt_addr);
				printbuffer = 1;
				printf("Value at address 0x%X (%p):\n", (uint32_t)target, virt_addr);
				for (i = 0; i < 4; i++) {
					printf("\t%x\n", value_ptr[i]);
				}
				break;
			default:
				fprintf(stderr, "Illegal data type '%c'.\n", access_type);
				exit(2);
		}
		if (!printbuffer)
			printf("Value at address 0x%X (%p): 0x%llX\n", (uint32_t)target, virt_addr, read_result);
		fflush(stdout);
	}

	if(argc > 3) {
		if (argc > 4) {
			for (i = 3; i < argc; i++)
				value_ptr[i-3] = strtoul(argv[i], 0, 0);
		} else
			writeval = strtoull(argv[3], 0, 0);
		switch(access_type) {
			case 'b':
				*((uint8_t *) virt_addr) = writeval;
				break;
			case 'h':
				*((uint16_t *) virt_addr) = writeval;
				break;
			case 'w':
				*((uint32_t *) virt_addr) = writeval;
				break;
			case 'l':
				*((uint64_t *) virt_addr) = writeval;
				break;
			case 'q':
				asm_write128(value_ptr, virt_addr);
				printbuffer = 1;
				printf("The following values are written:\n");
				for (i = 0; i < 4; i++) {
					printf("\t%x\n", value_ptr[i]);
				}
				break;
		}
		if (!printbuffer)
			printf("Written 0x%llX\n", writeval);
		fflush(stdout);
	}

	if(munmap(map_base, MAP_SIZE) == -1) FATAL;
	close(fd);
	return 0;
}

