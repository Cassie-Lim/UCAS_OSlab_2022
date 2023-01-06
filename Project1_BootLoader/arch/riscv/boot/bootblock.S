#include <asm/biosdef.h>

// os size location (os_size could be sectors [p1-task3] or bytes [p1-task4])
.equ os_size_loc, 0x502001fc

// kernel address (move kernel to here, and jmp here to start kernel)
.equ kernel, 0x50201000

// BIOS function entry (jump here to use BIOS APIs)
.equ bios_func_entry, 0x50150000

// start address for task info is stored in this loacation, size of array is in following 4 bytes
.equ app_info_addr_loc, 0x502001f4

.equ app_seq_addr_loc, 0x502001f0

.text
.global main

main:
	// fence on all memory and I/O
	fence

	// TODO: [p1-task1] call BIOS to print string "It's bootblock!"
	li a0, 9		// number for func bios_putstr
	la a1, msg		// addr of string
	call bios_func_entry

	// TODO: [p1-task2] call BIOS to read kernel in SD card
	li a0, 11		// number for func bios_sdread
	la a1, kernel	// load kernel to given address
	
	la x5, os_size_loc
	lh a2, (x5)		// load in block_nums

	li a3, 1		// start from 2nd block , index is 1
	call bios_func_entry


	// TODO: [p1-task4] load task-related arguments and pass them to kernel
	la x5, app_info_addr_loc	
	lw a0, (x5)		// pass the location for task info as parameter 1
	lw a1, 4(x5)	// pass the size as parameter 2
	lw a2, -4(x5)	// pass the end location for seq as param 3
	lw a3, -8(x5)	// pass the start location for seq as param 3
	// TODO: [p1-task2] jump to kernel to start UCAS-OS
	j kernel

// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's a bootloader...\n\r"

