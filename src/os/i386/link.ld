/* Linker script for the bare-metal i386 kernel.
 *
 * QEMU only finds the multiboot header if it lies in the first 8 KB of the
 * file, so it is the first section and nothing the toolchain likes to emit
 * ahead of .text (build-id, GNU property, CET and unwind notes) may precede
 * it.  The PVH note is kept as a second way for QEMU to boot the kernel. */
ENTRY(_start)
SECTIONS
{
	. = 1M;
	.multiboot : { KEEP(*(.multiboot)) }
	.note.Xen : { KEEP(*(.note.Xen)) }
	.text : { *(.text .text.*) }
	.rodata : ALIGN(4K) { *(.rodata .rodata.*) }
	.data : ALIGN(4K) { *(.data .data.*) }
	.bss : ALIGN(4K) { *(COMMON) *(.bss .bss.*) }
	/DISCARD/ : {
		*(.note.gnu.*)
		*(.note.ABI-tag)
		*(.eh_frame .eh_frame_hdr)
		*(.comment)
	}
}
