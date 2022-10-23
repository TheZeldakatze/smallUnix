#include <pmm.h>

#include <main.h>

#define BITMAP_SIZE 32768
#define BITMAP_EMPTY b11111111111111111111111111111111
#define BITMAP_FULL 0
static unsigned long bitmap[BITMAP_SIZE]; // 1=free, 0=used

void pmm_install(struct multiboot *mb) {


	// first, set every entry of the bitmap as used
	for(int i = 0; i<BITMAP_SIZE; i++) {
		bitmap[i] = BITMAP_FULL;
	}

	// mark free entries on the multiboot memmap as free
	struct multiboot_memmap* memmap = mb->memmap_addr;
	struct multiboot_memmap* memmap_end = mb->memmap_addr + mb->memmap_length;
	while(memmap < memmap_end) {

		// check if the entry marks a free memory region
		if(memmap->type == 1) {
			// get the memory range
			void* addr = (void*) ((unsigned long) memmap->base);
			void* end_addr = addr + memmap->length;

			// set the entries
			while(addr < end_addr) {
				pmm_free(addr);
				addr +=4096;
			}
		}

		// advance to the next entry
		memmap++;
	}

	// mark the kernel as used again
	void* addr = (void*) &kernel_start;
	void* addr_end = (void*) &kernel_end;
	while(addr < addr_end) {
		pmm_mark_used((void*) addr);
		addr += 4096;
	}

	// mark the multiboot structure as used
	pmm_mark_used(mb);

	// mark the multiboot modules as used
	struct multiboot_module* mod = mb->mods_addr;

	// iterate through the module list
	for(int i = 0; i<mb->mods_count; i++) {
		void* addr = (void*) mod[i].start;
		while(addr < ((void*) mod[i].end)) {
			pmm_mark_used(addr);
			addr +=4096;
		}
	}

	pmm_mark_used((void*) 0);
}

void* pmm_alloc() {
	// iterate through the bitmap
	for(int i = 0 ; i<BITMAP_SIZE; i++) {
		// if the entry is not zero, there's at least one free page
		if(bitmap[i] != BITMAP_FULL) {
			// iterate through the entry
			for(int j = 0; j<32;j++) {
				// found a free page
				if((bitmap[i] & (1 << j))) {
					// get the address
					bitmap[i] &= ~(1 << j);
					return (void*) ((i*32 + j) * 4096);
				}
			}
		}
	}

	kputs("pmm_alloc(): no free space found");
	return (void*) 0;
}

void* pmm_alloc_range(int pages) {
	if(pages <= 0) {
		kputs("pmm_alloc_range(): pages <= 0");
	}

	// iterate through the bitmap
	void* freeStartCandidate = (void*) 0;
	int pageCount = 0;

	// iterate through the bitmap
	for(int i = 0 ; i<BITMAP_SIZE; i++) {
		// if the entry is not zero, there's at least one free page
		if(bitmap[i] != BITMAP_FULL) {
			// iterate through the entry
			for(int j = 0; j<32;j++) {
				// found a free page
				if((bitmap[i] & (1 << j))) {
					if(freeStartCandidate == (void*) 0) {
						freeStartCandidate = ((i*32 + j) * 4096);
					}

					pageCount++;

					// free page range found, mark it as used
					if(pageCount >= pages) {
						for(int k = 0; k<pageCount; k++) {
							pmm_mark_used((void*) (k*4096 + ((long) freeStartCandidate)));
						}

						return freeStartCandidate;
					}
				} else { // if the page is not free, reset the counter
					freeStartCandidate = (void*) 0;
					pageCount = 0;
				}
			}
		}
	}

	kputs("pmm_alloc_range(): no free space range found");
	return (void*) 0;
}

void pmm_clear_page(void* addr) {
	void *start = addr;
	void *end = (void*) (((int) addr) +4096);
	for(; start<end; start++) {
		start = 0;
	}
}

void pmm_mark_used(void* addr) {
	unsigned long i = (unsigned long) addr / 4096;
	bitmap[i / 32] &= ~(1 << (i % 32));
}

void pmm_free(void* addr) {
	unsigned long i = (unsigned long) addr / 4096;
	bitmap[i / 32] |= (1 << (i % 32));
}
