/* Copyright (C) 1994, 1997 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/*  entry.s
 *
 *  This file contains the entry point for the application.
 *  The name of this entry point is compiler dependent.
 *  It jumps to the BSP which is responsible for performing
 *  all initialization.
 *
 */

	.data
	.global  _Do_Load_IDT
	.global  _Do_Load_GDT

	.text
	      .global  start                  # GNU default entry point
	.global  _establish_stack

	.global   _bsp_start
	.global   _load_segments
	.global   __exit

start:
	nop
	cli                             # DISABLE INTERRUPTS!!!
#
#  Load the segment registers
#
#  NOTE: Upon return, gs will contain the segment descriptor for
#        a segment which maps directly to all of physical memory.
#
	jmp     _load_segments          # load board dependent segments

#
#  Set up the stack
#

_establish_stack:

	movl    $stack_end,%esp         # set stack pointer
	movl    $stack_end,%ebp         # set base pointer

#
#  Zero out the BSS segment
#
zero_bss:
	cld                             # make direction flag count up
	movl    $_end,%ecx              # find end of .bss
	movl    $_bss_start,%edi        # edi = beginning of .bss
	subl    %edi,%ecx               # ecx = size of .bss in bytes
	shrl    $2,%ecx                 # size of .bss in longs
	xorl    %eax,%eax               # value to clear out memory
	repne                           # while ecx != 0
	stosl                           #   clear a long in the bss

#
#  Set the C heap information for malloc
#
	movl    $heap_size,___C_heap_size    # set ___C_heap_size
	movl    $heap_memory,___C_heap_start # set ___C_heap_start

#
#  Copy the Global Descriptor Table to our space
#

	sgdt    _Original_GDTR          # save original GDT
	movzwl  _Original_GDTR_limit,%ecx # size of GDT in bytes; limit
					  #   is 8192 entries * 8 bytes per

	# make ds:esi point to the original GDT

	movl    _Original_GDTR_base,%esi
	push    %ds                     # save ds
	movw    %gs,%ax
	movw    %ax,%ds

	# make es:edi point to the new (our copy) GDT
	movl    $_Global_descriptor_table,%edi

	rep
	movsb                            # copy the GDT (ds:esi -> es:edi)

	pop     %ds                      # restore ds

	# Build and load new contents of GDTR
	movw    _Original_GDTR_limit,%ecx # set new limit
	movw    %cx,_New_GDTR_limit

	push    $_Global_descriptor_table
	push    %es
	call    _Logical_to_physical
	addl    $6,%esp
	movl    %eax,_New_GDTR_base      # set new base

	cmpb    $0,_Do_Load_GDT          # Should the new GDT be loaded?
	je      no_gdt_load              # NO, then branch
	lgdt    _New_GDTR                # load the new GDT
no_gdt_load:

#
#  Copy the Interrupt Descriptor Table to our space
#

	sidt    _Original_IDTR          # save original IDT
	movzwl  _Original_IDTR_limit,%ecx # size of IDT in bytes; limit
					  #   is 256 entries * 8 bytes per


	# make ds:esi point to the original IDT
	movl    _Original_IDTR_base,%esi

	push    %ds                     # save ds
	movw    %gs,%ax
	movw    %ax,%ds

	# make es:edi point to the new (our copy) IDT
	movl    $_Interrupt_descriptor_table,%edi

	rep
	movsb                            # copy the IDT (ds:esi -> es:edi)
	pop     %ds                      # restore ds

	# Build and load new contents of IDTR
	movw    _Original_IDTR_limit,%ecx # set new limit
	movw    %cx,_New_IDTR_limit

	push    $_Interrupt_descriptor_table
	push    %es
	call    _Logical_to_physical
	addl    $6,%esp
	movl    %eax,_New_IDTR_base      # set new base

	cmpb    $0,_Do_Load_IDT          # Should the new IDT be loaded?
	je      no_idt_load              # NO, then branch
	lidt    _New_IDTR                # load the new IDT
no_idt_load:

#
#  Initialize the i387.
#
#  Using the NO WAIT form of the instruction insures that if
#  it is not present the board will not lock up or get an
#  exception.
#

	fninit                           # MUST USE NO-WAIT FORM

	call    __Board_Initialize       # initialize the board

	pushl   $0                       # envp = NULL
	pushl   $0                       # argv = NULL
	pushl   $0                       # argc = NULL
	call    ___libc_init             # initialize the library and
					 #   call main
	addl    $12,%esp

	pushl   $0                       # argc = NULL
	call    __exit                   # call the Board specific exit
	addl     $4,%esp

#
#  Clean up
#


	.global  _Bsp_cleanup

	.global   _return_to_monitor

_Bsp_cleanup:
	cmpb    $0,_Do_Load_IDT          # Was the new IDT loaded?
	je      no_idt_restore           # NO, then branch
	lidt    _Original_IDTR           # restore the new IDT
no_idt_restore:

	cmpb    $0,_Do_Load_GDT          # Was the new GDT loaded?
	je      no_gdt_restore           # NO, then branch
	lgdt    _Original_GDTR           # restore the new GDT
no_gdt_restore:
	jmp     _return_to_monitor

#
#  void *Logical_to_physical(
#     rtems_unsigned16  segment,
#     void             *address
#  );
#
#  Returns thirty-two bit physical address for segment:address.
#

	.global  _Logical_to_physical

.set SEGMENT_ARG, 4
.set ADDRESS_ARG, 8

_Logical_to_physical:

	xorl    %eax,%eax                # clear eax
	movzwl  SEGMENT_ARG(%esp),%ecx   # ecx = segment value
	movl    $_Global_descriptor_table,%edx # edx = address of our GDT
	addl    %ecx,%edx                # edx = address of desired entry
	movb    7(%edx),%ah              # ah = base 31:24
	movb    4(%edx),%al              # al = base 23:16
	shll    $16,%eax                 # move ax into correct bits
	movw    2(%edx),%ax              # ax = base 0:15
	movl    ADDRESS_ARG(%esp),%ecx   # ecx = address to convert
	addl    %eax,%ecx                # ecx = physical address equivalent
	movl    %ecx,%eax                # eax = ecx
	ret

#
#  void *Physical_to_logical(
#     rtems_unsigned16  segment,
#     void             *address
#  );
#
#  Returns thirty-two bit physical address for segment:address.
#

	.global  _Physical_to_logical

#.set SEGMENT_ARG, 4
#.set ADDRESS_ARG, 8   -- use sets from above

_Physical_to_logical:

	xorl    %eax,%eax                # clear eax
	movzwl  SEGMENT_ARG(%esp),%ecx   # ecx = segment value
	movl    $_Global_descriptor_table,%edx # edx = address of our GDT
	addl    %ecx,%edx                # edx = address of desired entry
	movb    7(%edx),%ah              # ah = base 31:24
	movb    4(%edx),%al              # al = base 23:16
	shll    $16,%eax                 # move ax into correct bits
	movw    2(%edx),%ax              # ax = base 0:15
	movl    ADDRESS_ARG(%esp),%ecx   # ecx = address to convert
	subl    %eax,%ecx                # ecx = logical address equivalent
	movl    %ecx,%eax                # eax = ecx
	ret


/*
 *  Data Declarations.  Start with a macro which helps declare space.
 */

	.bss

#define DECLARE_SPACE(_name,_space,_align) \
	  .globl   _name ; \
	  .align   _align ; \
_name##:  .space _space

#define DECLARE_LABEL(_name) \
	  .globl   _name ; \
_name##:

#define DECLARE_PTR(_name) DECLARE_SPACE(_name,4,2)
#define DECLARE_U32(_name) DECLARE_SPACE(_name,4,2)
#define DECLARE_U16(_name) DECLARE_SPACE(_name,2,1)

/*
 *  Require environment stuff
 */

DECLARE_LABEL(_environ)
DECLARE_PTR(environ)

DECLARE_LABEL(_errno)
DECLARE_U32(errno)

/*
 *  Miscellaneous Variables used to restore the CPU state.
 *
 *  Start with a macro to declare the space for the contents of
 *  a Descriptor Table register.
 */

#define DECLARE_DTR_SPACE(_name) \
	  .global   _name ; \
	  .align    4 ; \
_name##:  ; \
_name##_limit:  .space 2  ; \
_name##_base:   .space 4

DECLARE_SPACE(_Interrupt_descriptor_table,256*8,4)
DECLARE_SPACE(_Global_descriptor_table,8192*8,4)

DECLARE_DTR_SPACE(_Original_IDTR)
DECLARE_DTR_SPACE(_New_IDTR)
DECLARE_DTR_SPACE(_Original_GDTR)
DECLARE_DTR_SPACE(_New_GDTR)

DECLARE_SPACE(_Physical_base_of_ds,4,4)
DECLARE_SPACE(_Physical_base_of_cs,4,4)

/*
 *  Stack Size and Space
 */

	.set stack_size, 0x20000

DECLARE_SPACE(stack_memory,stack_size,4)
DECLARE_LABEL(stack_end)
