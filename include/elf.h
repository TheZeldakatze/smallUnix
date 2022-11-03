/*
 * elf.h
 *
 *  Created on: 21.10.2022
 *      Author: maite
 */

#ifndef INCLUDE_ELF_H_
#define INCLUDE_ELF_H_

#define ELF_MAGIC 0x464C457F

struct elf_header {
	unsigned long magic;
	unsigned char bits, endianess, elf_header_version, abi_version;
	unsigned long long unused;
	unsigned short type, opcode_set;
	unsigned long elf_version, entry_posititon, program_header_tbl_offset, section_header_tbl_offset;
	unsigned long flags;
	unsigned short header_size, program_header_entry_size, program_header_entry_count, section_header_size, section_header_count, section_header_name_index;

} __attribute__((packed));

struct elf_program_header_entry {
	unsigned long type, p_offset, vaddr, reserved, filesize, memsize, flags, alignment;

} __attribute__((packed));

#endif /* INCLUDE_ELF_H_ */
