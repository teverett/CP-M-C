/* boot.s - multiboot entry for the bare-metal i386 build (run by QEMU's
 * -kernel loader, which enters in 32-bit protected mode). */
	.set	MB_MAGIC, 0x1BADB002
	.set	MB_FLAGS, 0
	.set	MB_CHECK, -(MB_MAGIC + MB_FLAGS)

	.section .multiboot, "a"
	.align	4
	.long	MB_MAGIC
	.long	MB_FLAGS
	.long	MB_CHECK

	/* PVH entry note (XEN_ELFNOTE_PHYS32_ENTRY): QEMU can boot the kernel
	 * through this if it does not pick up the multiboot header */
	.section .note.Xen, "a", @note
	.align	4
	.long	2f - 1f
	.long	4f - 3f
	.long	18
1:	.asciz	"Xen"
2:	.align	4
3:	.long	_start
4:	.align	4

	.section .bss
	.align	16
stack_bottom:
	.skip	65536
stack_top:

	.section .text
	.global	_start
	.type	_start, @function
_start:
	cli
	movl	$stack_top, %esp
	xorl	%ebp, %ebp
	call	kmain
1:	hlt
	jmp	1b
