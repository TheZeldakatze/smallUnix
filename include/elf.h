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
	unsigned short short type, opcode_set;
	unsigned long elf_version, entry_posititon;
};

#endif /* INCLUDE_ELF_H_ */
