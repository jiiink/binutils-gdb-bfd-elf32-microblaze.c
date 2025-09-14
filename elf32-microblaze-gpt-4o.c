/* Xilinx MicroBlaze-specific support for 32-bit ELF

   Copyright (C) 2009-2025 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
   Boston, MA 02110-1301, USA.  */


#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/microblaze.h"
#include <assert.h>

#define	USE_RELA	/* Only USE_REL is actually significant, but this is
			   here are a reminder...  */
#define INST_WORD_SIZE 4

static int ro_small_data_pointer = 0;
static int rw_small_data_pointer = 0;

static reloc_howto_type * microblaze_elf_howto_table [(int) R_MICROBLAZE_max];

static reloc_howto_type microblaze_elf_howto_raw[] =
{
   /* This reloc does nothing.  */
   HOWTO (R_MICROBLAZE_NONE,	/* Type.  */
	  0,			/* Rightshift.  */
	  0,			/* Size.  */
	  0,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont,  /* Complain on overflow.  */
	  NULL,			 /* Special Function.  */
	  "R_MICROBLAZE_NONE",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0,			/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* A standard 32 bit relocation.  */
   HOWTO (R_MICROBLAZE_32,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  32,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_32",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0xffffffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* A standard PCREL 32 bit relocation.  */
   HOWTO (R_MICROBLAZE_32_PCREL,/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  32,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_32_PCREL",	/* Name.  */
	  true,			/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0xffffffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* A 64 bit PCREL relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_64_PCREL,/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_64_PCREL",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* The low half of a PCREL 32 bit relocation.  */
   HOWTO (R_MICROBLAZE_32_PCREL_LO,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_signed, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,	/* Special Function.  */
	  "R_MICROBLAZE_32_PCREL_LO",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* A 64 bit relocation.  Table entry not really used.  */
   HOWTO (R_MICROBLAZE_64,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_64",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* The low half of a 32 bit relocation.  */
   HOWTO (R_MICROBLAZE_32_LO,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_signed, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_32_LO", /* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* Read-only small data section relocation.  */
   HOWTO (R_MICROBLAZE_SRO32,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_SRO32", /* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* Read-write small data area relocation.  */
   HOWTO (R_MICROBLAZE_SRW32,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_SRW32", /* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* This reloc does nothing.	Used for relaxation.  */
   HOWTO (R_MICROBLAZE_32_NONE,	/* Type.  */
	0,			/* Rightshift.  */
	2,			/* Size (0 = byte, 1 = short, 2 = long).  */
	32,			/* Bitsize.  */
	true,			/* PC_relative.  */
	0,			/* Bitpos.  */
	complain_overflow_bitfield, /* Complain on overflow.  */
	NULL,			/* Special Function.  */
	"R_MICROBLAZE_32_NONE", /* Name.  */
	false,			/* Partial Inplace.  */
	0,			/* Source Mask.  */
	0,			/* Dest Mask.  */
	false),		/* PC relative offset?  */

   /* This reloc does nothing.	Used for relaxation.  */
   HOWTO (R_MICROBLAZE_64_NONE,	/* Type.  */
	  0,			/* Rightshift.  */
	  0,			/* Size.  */
	  0,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  NULL,			 /* Special Function.  */
	  "R_MICROBLAZE_64_NONE",/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0,			/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* Symbol Op Symbol relocation.  */
   HOWTO (R_MICROBLAZE_32_SYM_OP_SYM,		/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  32,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_bitfield, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_32_SYM_OP_SYM",		/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0xffffffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* GNU extension to record C++ vtable hierarchy.  */
   HOWTO (R_MICROBLAZE_GNU_VTINHERIT, /* Type.  */
	  0,			 /* Rightshift.  */
	  4,			 /* Size.  */
	  0,			 /* Bitsize.  */
	  false,		 /* PC_relative.  */
	  0,			 /* Bitpos.  */
	  complain_overflow_dont,/* Complain on overflow.  */
	  NULL,			 /* Special Function.  */
	  "R_MICROBLAZE_GNU_VTINHERIT", /* Name.  */
	  false,		 /* Partial Inplace.  */
	  0,			 /* Source Mask.  */
	  0,			 /* Dest Mask.  */
	  false),		 /* PC relative offset?  */

   /* GNU extension to record C++ vtable member usage.  */
   HOWTO (R_MICROBLAZE_GNU_VTENTRY,   /* Type.  */
	  0,			 /* Rightshift.  */
	  4,			 /* Size.  */
	  0,			 /* Bitsize.  */
	  false,		 /* PC_relative.  */
	  0,			 /* Bitpos.  */
	  complain_overflow_dont,/* Complain on overflow.  */
	  _bfd_elf_rel_vtable_reloc_fn,	 /* Special Function.  */
	  "R_MICROBLAZE_GNU_VTENTRY", /* Name.  */
	  false,		 /* Partial Inplace.  */
	  0,			 /* Source Mask.  */
	  0,			 /* Dest Mask.  */
	  false),		 /* PC relative offset?  */

   /* A 64 bit GOTPC relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GOTPC_64,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,	/* Special Function.  */
	  "R_MICROBLAZE_GOTPC_64",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

     /* A 64 bit TEXTPCREL relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_TEXTPCREL_64,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,	/* Special Function.  */
	  "R_MICROBLAZE_TEXTPCREL_64",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* A 64 bit GOT relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GOT_64,  /* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_GOT_64",/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

    /* A 64 bit TEXTREL relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_TEXTREL_64,  /* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_TEXTREL_64",/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* A 64 bit PLT relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_PLT_64,  /* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_PLT_64",/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /*  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_REL,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_REL",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /*  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_JUMP_SLOT,/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_JUMP_SLOT",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /*  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GLOB_DAT,/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  true,			/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_GLOB_DAT",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  true),		/* PC relative offset?  */

   /* A 64 bit GOT relative relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GOTOFF_64,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_GOTOFF_64",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* A 32 bit GOT relative relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_GOTOFF_32,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,	/* Special Function.  */
	  "R_MICROBLAZE_GOTOFF_32",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* COPY relocation.  Table-entry not really used.  */
   HOWTO (R_MICROBLAZE_COPY,	/* Type.  */
	  0,			/* Rightshift.  */
	  4,			/* Size.  */
	  16,			/* Bitsize.  */
	  false,		/* PC_relative.  */
	  0,			/* Bitpos.  */
	  complain_overflow_dont, /* Complain on overflow.  */
	  bfd_elf_generic_reloc,/* Special Function.  */
	  "R_MICROBLAZE_COPY",	/* Name.  */
	  false,		/* Partial Inplace.  */
	  0,			/* Source Mask.  */
	  0x0000ffff,		/* Dest Mask.  */
	  false),		/* PC relative offset?  */

   /* Marker relocs for TLS.  */
   HOWTO (R_MICROBLAZE_TLS,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_MICROBLAZE_TLS",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,			/* dst_mask */
	 false),		/* pcrel_offset */

   HOWTO (R_MICROBLAZE_TLSGD,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSGD",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,			/* dst_mask */
	 false),		/* pcrel_offset */

   HOWTO (R_MICROBLAZE_TLSLD,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSLD",		/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes the load module index of the load module that contains the
      definition of its TLS sym.  */
   HOWTO (R_MICROBLAZE_TLSDTPMOD32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSDTPMOD32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes a dtv-relative displacement, the difference between the value
      of sym+add and the base address of the thread-local storage block that
      contains the definition of sym, minus 0x8000.  Used for initializing GOT */
   HOWTO (R_MICROBLAZE_TLSDTPREL32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSDTPREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes a dtv-relative displacement, the difference between the value
      of sym+add and the base address of the thread-local storage block that
      contains the definition of sym, minus 0x8000.  */
   HOWTO (R_MICROBLAZE_TLSDTPREL64,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSDTPREL64",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes a tp-relative displacement, the difference between the value of
      sym+add and the value of the thread pointer (r13).  */
   HOWTO (R_MICROBLAZE_TLSGOTTPREL32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSGOTTPREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

   /* Computes a tp-relative displacement, the difference between the value of
      sym+add and the value of the thread pointer (r13).  */
   HOWTO (R_MICROBLAZE_TLSTPREL32,
	 0,			/* rightshift */
	 4,			/* size */
	 32,			/* bitsize */
	 false,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_MICROBLAZE_TLSTPREL32",	/* name */
	 false,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 false),		/* pcrel_offset */

};

#ifndef NUM_ELEM
#define NUM_ELEM(a) (sizeof (a) / sizeof (a)[0])
#endif

/* Initialize the microblaze_elf_howto_table, so that linear accesses can be done.  */

#include <assert.h>

static void microblaze_elf_howto_init(void) {
    for (unsigned int i = 0; i < NUM_ELEM(microblaze_elf_howto_raw); i++) {
        unsigned int type = microblaze_elf_howto_raw[i].type;
        assert(type < NUM_ELEM(microblaze_elf_howto_table));
        microblaze_elf_howto_table[type] = &microblaze_elf_howto_raw[i];
    }
}

static reloc_howto_type *microblaze_elf_reloc_type_lookup(bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code) {
    static const enum elf_microblaze_reloc_type reloc_map[] = {
        [BFD_RELOC_NONE] = R_MICROBLAZE_NONE,
        [BFD_RELOC_MICROBLAZE_32_NONE] = R_MICROBLAZE_32_NONE,
        [BFD_RELOC_MICROBLAZE_64_NONE] = R_MICROBLAZE_64_NONE,
        [BFD_RELOC_32] = R_MICROBLAZE_32,
        [BFD_RELOC_RVA] = R_MICROBLAZE_32,
        [BFD_RELOC_32_PCREL] = R_MICROBLAZE_32_PCREL,
        [BFD_RELOC_64_PCREL] = R_MICROBLAZE_64_PCREL,
        [BFD_RELOC_MICROBLAZE_32_LO_PCREL] = R_MICROBLAZE_32_PCREL_LO,
        [BFD_RELOC_64] = R_MICROBLAZE_64,
        [BFD_RELOC_MICROBLAZE_32_LO] = R_MICROBLAZE_32_LO,
        [BFD_RELOC_MICROBLAZE_32_ROSDA] = R_MICROBLAZE_SRO32,
        [BFD_RELOC_MICROBLAZE_32_RWSDA] = R_MICROBLAZE_SRW32,
        [BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM] = R_MICROBLAZE_32_SYM_OP_SYM,
        [BFD_RELOC_VTABLE_INHERIT] = R_MICROBLAZE_GNU_VTINHERIT,
        [BFD_RELOC_VTABLE_ENTRY] = R_MICROBLAZE_GNU_VTENTRY,
        [BFD_RELOC_MICROBLAZE_64_GOTPC] = R_MICROBLAZE_GOTPC_64,
        [BFD_RELOC_MICROBLAZE_64_GOT] = R_MICROBLAZE_GOT_64,
        [BFD_RELOC_MICROBLAZE_64_TEXTPCREL] = R_MICROBLAZE_TEXTPCREL_64,
        [BFD_RELOC_MICROBLAZE_64_TEXTREL] = R_MICROBLAZE_TEXTREL_64,
        [BFD_RELOC_MICROBLAZE_64_PLT] = R_MICROBLAZE_PLT_64,
        [BFD_RELOC_MICROBLAZE_64_GOTOFF] = R_MICROBLAZE_GOTOFF_64,
        [BFD_RELOC_MICROBLAZE_32_GOTOFF] = R_MICROBLAZE_GOTOFF_32,
        [BFD_RELOC_MICROBLAZE_64_TLSGD] = R_MICROBLAZE_TLSGD,
        [BFD_RELOC_MICROBLAZE_64_TLSLD] = R_MICROBLAZE_TLSLD,
        [BFD_RELOC_MICROBLAZE_32_TLSDTPREL] = R_MICROBLAZE_TLSDTPREL32,
        [BFD_RELOC_MICROBLAZE_64_TLSDTPREL] = R_MICROBLAZE_TLSDTPREL64,
        [BFD_RELOC_MICROBLAZE_32_TLSDTPMOD] = R_MICROBLAZE_TLSDTPMOD32,
        [BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL] = R_MICROBLAZE_TLSGOTTPREL32,
        [BFD_RELOC_MICROBLAZE_64_TLSTPREL] = R_MICROBLAZE_TLSTPREL32,
        [BFD_RELOC_MICROBLAZE_COPY] = R_MICROBLAZE_COPY
    };

    if (code < 0 || code >= sizeof(reloc_map) / sizeof(reloc_map[0])) {
        return NULL;
    }
    
    enum elf_microblaze_reloc_type microblaze_reloc = reloc_map[code];
    
    if (!microblaze_elf_howto_table[R_MICROBLAZE_32]) {
        microblaze_elf_howto_init();
    }

    return microblaze_elf_howto_table[(int)microblaze_reloc];
};

static reloc_howto_type *
microblaze_elf_reloc_name_lookup(bfd *abfd, const char *r_name)
{
  unsigned int i;
  if (r_name == NULL) {
    return NULL;
  }

  for (i = 0; i < NUM_ELEM(microblaze_elf_howto_raw); i++) {
    const char *name = microblaze_elf_howto_raw[i].name;
    if (name != NULL && strcasecmp(name, r_name) == 0) {
      return &microblaze_elf_howto_raw[i];
    }
  }

  return NULL;
}

/* Set the howto pointer for a RCE ELF reloc.  */

static bool microblaze_elf_info_to_howto(bfd *abfd, arelent *cache_ptr, Elf_Internal_Rela *dst) {
  unsigned int r_type;

  if (!microblaze_elf_howto_table[R_MICROBLAZE_32]) {
    microblaze_elf_howto_init();
  }

  r_type = ELF32_R_TYPE(dst->r_info);
  if (r_type >= R_MICROBLAZE_max) {
    _bfd_error_handler(_("%pB: unsupported relocation type %#x"), abfd, r_type);
    bfd_set_error(bfd_error_bad_value);
    return false;
  }

  cache_ptr->howto = microblaze_elf_howto_table[r_type];
  return true;
}

/* Relax table contains information about instructions which can
   be removed by relaxation -- replacing a long address with a
   short address.  */
struct relax_table
{
  /* Address where bytes may be deleted.  */
  bfd_vma addr;

  /* Number of bytes to be deleted.  */
  size_t size;
};

struct _microblaze_elf_section_data
{
  struct bfd_elf_section_data elf;
  /* Count of used relaxation table entries.  */
  size_t relax_count;
  /* Relaxation table.  */
  struct relax_table *relax;
};

#define microblaze_elf_section_data(sec) \
  ((struct _microblaze_elf_section_data *) elf_section_data (sec))

static bool microblaze_elf_new_section_hook(bfd *abfd, asection *sec) {
    struct _microblaze_elf_section_data *sdata = bfd_zalloc(abfd, sizeof(*sdata));
    if (!sdata) return false;
    
    sec->used_by_bfd = sdata;
    return _bfd_elf_new_section_hook(abfd, sec);
}

/* Microblaze ELF local labels start with 'L.' or '$L', not '.L'.  */

bool microblaze_elf_is_local_label_name (bfd *abfd, const char *name) {
    if ((name[0] == 'L' && name[1] == '.') || 
        (name[0] == '$' && name[1] == 'L')) {
        return true;
    }
    return _bfd_elf_is_local_label_name(abfd, name);
}

/* ELF linker hash entry.  */

struct elf32_mb_link_hash_entry
{
  struct elf_link_hash_entry elf;

  /* TLS Reference Types for the symbol; Updated by check_relocs */
#define TLS_GD     1  /* GD reloc. */
#define TLS_LD     2  /* LD reloc. */
#define TLS_TPREL  4  /* TPREL reloc, => IE. */
#define TLS_DTPREL 8  /* DTPREL reloc, => LD. */
#define TLS_TLS    16 /* Any TLS reloc.  */
  unsigned char tls_mask;

};

#define IS_TLS_GD(x)     (x == (TLS_TLS | TLS_GD))
#define IS_TLS_LD(x)     (x == (TLS_TLS | TLS_LD))
#define IS_TLS_DTPREL(x) (x == (TLS_TLS | TLS_DTPREL))
#define IS_TLS_NONE(x)   (x == 0)

#define elf32_mb_hash_entry(ent) ((struct elf32_mb_link_hash_entry *)(ent))

/* ELF linker hash table.  */

struct elf32_mb_link_hash_table
{
  struct elf_link_hash_table elf;

  /* TLS Local Dynamic GOT Entry */
  union {
    bfd_signed_vma refcount;
    bfd_vma offset;
  } tlsld_got;
};

/* Nonzero if this section has TLS related relocations.  */
#define has_tls_reloc sec_flg0

/* Get the ELF linker hash table from a link_info structure.  */

#define elf32_mb_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == MICROBLAZE_ELF_DATA)	\
   ? (struct elf32_mb_link_hash_table *) (p)->hash : NULL)

/* Create an entry in a microblaze ELF linker hash table.  */

static struct bfd_hash_entry *link_hash_newfunc(struct bfd_hash_entry *entry, struct bfd_hash_table *table, const char *string) {
    if (!entry) {
        entry = bfd_hash_allocate(table, sizeof(struct elf32_mb_link_hash_entry));
        if (!entry) return NULL;
    }

    entry = _bfd_elf_link_hash_newfunc(entry, table, string);
    if (entry) {
        ((struct elf32_mb_link_hash_entry *)entry)->tls_mask = 0;
    }

    return entry;
}

/* Create a mb ELF linker hash table.  */

struct bfd_link_hash_table *microblaze_elf_link_hash_table_create(bfd *abfd) {
    size_t amt = sizeof(struct elf32_mb_link_hash_table);
    struct elf32_mb_link_hash_table *ret = (struct elf32_mb_link_hash_table *) bfd_zmalloc(amt);
    if (!ret) 
        return NULL;

    if (!_bfd_elf_link_hash_table_init(&ret->elf, abfd, link_hash_newfunc, sizeof(struct elf32_mb_link_hash_entry))) {
        free(ret);
        return NULL;
    }

    return &ret->elf.root;
}

/* Set the values of the small data pointers.  */

static void microblaze_elf_final_sdp(struct bfd_link_info *info) {
    struct bfd_link_hash_entry *h = bfd_link_hash_lookup(info->hash, RO_SDA_ANCHOR_NAME, false, false, true);
    if (h && h->type == bfd_link_hash_defined) {
        ro_small_data_pointer = h->u.def.value + h->u.def.section->output_section->vma + h->u.def.section->output_offset;
    }

    h = bfd_link_hash_lookup(info->hash, RW_SDA_ANCHOR_NAME, false, false, true);
    if (h && h->type == bfd_link_hash_defined) {
        rw_small_data_pointer = h->u.def.value + h->u.def.section->output_section->vma + h->u.def.section->output_offset;
    }
}

static bfd_vma dtprel_base(struct bfd_link_info *info) {
    struct elf_link_hash_table *hash_table = elf_hash_table(info);
    if (hash_table == NULL || hash_table->tls_sec == NULL) {
        return 0;
    }
    return hash_table->tls_sec->vma;
}

/* The size of the thread control block.  */
#define TCB_SIZE	8

/* Output a simple dynamic relocation into SRELOC.  */

#include <stdbool.h>

static bool is_valid_reloc_index(unsigned long reloc_index, asection *sreloc) {
    return reloc_index * sizeof(Elf32_External_Rela) < sreloc->size;
}

static bool is_valid_parameters(bfd *output_bfd, asection *sreloc, unsigned long reloc_index) {
    return output_bfd != NULL && sreloc != NULL && is_valid_reloc_index(reloc_index, sreloc);
}

static void microblaze_elf_output_dynamic_relocation(bfd *output_bfd, asection *sreloc, unsigned long reloc_index, unsigned long indx, int r_type, bfd_vma offset, bfd_vma addend) {
    if (!is_valid_parameters(output_bfd, sreloc, reloc_index)) {
        return;
    }

    Elf_Internal_Rela rel;
    rel.r_info = ELF32_R_INFO(indx, r_type);
    rel.r_offset = offset;
    rel.r_addend = addend;

    bfd_elf32_swap_reloca_out(output_bfd, &rel, sreloc->contents + reloc_index * sizeof(Elf32_External_Rela));
}

/* This code is taken from elf32-m32r.c
   There is some attempt to make this function usable for many architectures,
   both USE_REL and USE_RELA ['twould be nice if such a critter existed],
   if only to serve as a learning tool.

   The RELOCATE_SECTION function is called by the new ELF backend linker
   to handle the relocations for a section.

   The relocs are always passed as Rela structures; if the section
   actually uses Rel structures, the r_addend field will always be
   zero.

   This function is responsible for adjust the section contents as
   necessary, and (if using Rela relocs and generating a
   relocatable output file) adjusting the reloc addend as
   necessary.

   This function does not have to worry about setting the reloc
   address or the reloc symbol index.

   LOCAL_SYMS is a pointer to the swapped in local symbols.

   LOCAL_SECTIONS is an array giving the section in the input file
   corresponding to the st_shndx field of each local symbol.

   The global hash table entry for the global symbols can be found
   via elf_sym_hashes (input_bfd).

   When generating relocatable output, this function must handle
   STB_LOCAL/STT_SECTION symbols specially.  The output symbol is
   going to be the section symbol corresponding to the output
   section, which means that the addend must be adjusted
   accordingly.  */

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int microblaze_elf_relocate_section(bfd *output_bfd, struct bfd_link_info *info, bfd *input_bfd, asection *input_section, bfd_byte *contents, Elf_Internal_Rela *relocs, Elf_Internal_Sym *local_syms, asection **local_sections) {
    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    if (htab == NULL) return false;

    Elf_Internal_Shdr *symtab_hdr = &elf_tdata(input_bfd)->symtab_hdr;
    struct elf_link_hash_entry **sym_hashes = elf_sym_hashes(input_bfd);
    int endian = (bfd_little_endian(output_bfd)) ? 0 : 2;
    bool ret = true;
    bfd_vma *local_got_offsets = elf_local_got_offsets(input_bfd);
    asection *sreloc = elf_section_data(input_section)->sreloc;
    bool unresolved_reloc = false;

    if (!microblaze_elf_howto_table[R_MICROBLAZE_max - 1]) {
        microblaze_elf_howto_init();
    }

    for (Elf_Internal_Rela *rel = relocs, *relend = relocs + input_section->reloc_count; rel < relend; ++rel) {
        int r_type = ELF32_R_TYPE(rel->r_info);
        reloc_howto_type *howto;
        unsigned long r_symndx = ELF32_R_SYM(rel->r_info);
        bfd_vma addend = rel->r_addend;
        bfd_vma offset = rel->r_offset;
        struct elf_link_hash_entry *h = NULL;
        Elf_Internal_Sym *sym;
        asection *sec;
        const char *sym_name;
        bfd_reloc_status_type r = bfd_reloc_ok;
        const char *errmsg = NULL;
        unsigned int tls_type = 0;

        if (r_type < 0 || r_type >= (int)R_MICROBLAZE_max) {
            _bfd_error_handler(_("%pB: unsupported relocation type %#x"), input_bfd, (int)r_type);
            bfd_set_error(bfd_error_bad_value);
            ret = false;
            continue;
        }

        howto = microblaze_elf_howto_table[r_type];

        if (bfd_link_relocatable(info)) {
            if (r_symndx >= symtab_hdr->sh_info) continue;

            sym = local_syms + r_symndx;
            sym_name = "<local symbol>";
            if (ELF_ST_TYPE(sym->st_info) != STT_SECTION) continue;

            sec = local_sections[r_symndx];
            addend += sec->output_offset + sym->st_value;
#ifndef USE_REL
            continue;
#else /* USE_REL */
            if (!howto->partial_inplace) continue;
            r = _bfd_relocate_contents(howto, input_bfd, addend, contents + offset);
#endif /* USE_REL */
        } else {
            bfd_vma relocation;
            bool resolved_to_zero;

            if (r_symndx < symtab_hdr->sh_info) {
                sym = local_syms + r_symndx;
                sec = local_sections[r_symndx];
                if (!sec) continue;
                sym_name = "<local symbol>";
                relocation = _bfd_elf_rela_local_sym(output_bfd, sym, &sec, rel);
                addend = rel->r_addend;
            } else {
                bool warned ATTRIBUTE_UNUSED, ignored ATTRIBUTE_UNUSED;

                RELOC_FOR_GLOBAL_SYMBOL(info, input_bfd, input_section, rel, r_symndx, symtab_hdr, sym_hashes, h, sec, relocation, unresolved_reloc, warned, ignored);
                sym_name = h->root.root.string;
            }

            resolved_to_zero = (h != NULL && UNDEFWEAK_NO_DYNAMIC_RELOC(info, h));

            switch ((int)r_type) {
                case (int)R_MICROBLAZE_SRO32:
                case (int)R_MICROBLAZE_SRW32: {
                    const char *name;
                    if (sec) {
                        name = bfd_section_name(sec);
                        if ((r_type == (int)R_MICROBLAZE_SRO32 && (strcmp(name, ".sdata2") == 0 || strcmp(name, ".sbss2") == 0)) ||
                            (r_type == (int)R_MICROBLAZE_SRW32 && (strcmp(name, ".sdata") == 0 || strcmp(name, ".sbss") == 0))) {
                            if (r_type == (int)R_MICROBLAZE_SRO32) {
                                if (ro_small_data_pointer == 0) microblaze_elf_final_sdp(info);
                                if (ro_small_data_pointer == 0) {
                                    ret = false;
                                    r = bfd_reloc_undefined;
                                    goto check_reloc;
                                }
                                relocation -= ro_small_data_pointer;
                            } else {
                                if (rw_small_data_pointer == 0) microblaze_elf_final_sdp(info);
                                if (rw_small_data_pointer == 0) {
                                    ret = false;
                                    r = bfd_reloc_undefined;
                                    goto check_reloc;
                                }
                                relocation -= rw_small_data_pointer;
                            }
                            r = _bfd_final_link_relocate(howto, input_bfd, input_section, contents, offset, relocation, addend);
                        } else {
                            _bfd_error_handler(_("%pB: the target (%s) of an %s relocation is in the wrong section (%pA)"), input_bfd, sym_name, microblaze_elf_howto_table[(int)r_type]->name, sec);
                            ret = false;
                            continue;
                        }
                    }
                } break;

                case (int)R_MICROBLAZE_32_SYM_OP_SYM:
                    break;

                case (int)R_MICROBLAZE_GOTPC_64: 
                case (int)R_MICROBLAZE_TEXTPCREL_64: 
                case (int)R_MICROBLAZE_TLSDTPREL64: {
                    relocation += addend - (input_section->output_section->vma + input_section->output_offset + offset + INST_WORD_SIZE);
                    bfd_put_16(input_bfd, (relocation >> 16) & 0xffff, contents + offset + endian);
                    bfd_put_16(input_bfd, relocation & 0xffff, contents + offset + endian + INST_WORD_SIZE);
                } break;

                case (int)R_MICROBLAZE_PLT_64: {
                    bfd_vma immediate;
                    if (htab->elf.splt != NULL && h != NULL && h->plt.offset != (bfd_vma)-1) {
                        relocation = (htab->elf.splt->output_section->vma + htab->elf.splt->output_offset + h->plt.offset);
                        unresolved_reloc = false;
                        immediate = relocation - (input_section->output_section->vma + input_section->output_offset + offset + INST_WORD_SIZE);
                        bfd_put_16(input_bfd, (immediate >> 16) & 0xffff, contents + offset + endian);
                        bfd_put_16(input_bfd, immediate & 0xffff, contents + offset + endian + INST_WORD_SIZE);
                    } else {
                        relocation -= (input_section->output_section->vma + input_section->output_offset + offset + INST_WORD_SIZE);
                        immediate = relocation;
                        bfd_put_16(input_bfd, (immediate >> 16) & 0xffff, contents + offset + endian);
                        bfd_put_16(input_bfd, immediate & 0xffff, contents + offset + endian + INST_WORD_SIZE);
                    }
                } break;

                case (int)R_MICROBLAZE_TLSGD:
                case (int)R_MICROBLAZE_TLSLD: {
                    tls_type = (r_type == (int)R_MICROBLAZE_TLSGD ? (TLS_TLS | TLS_GD) : (TLS_TLS | TLS_LD));
                    goto dogot;
                }
                dogot:
                case (int)R_MICROBLAZE_GOT_64: {
                    bfd_vma *offp = NULL;
                    bfd_vma off = 0, off2 = 0;
                    unsigned long indx = 0;
                    bfd_vma static_value;
                    bool need_relocs = false;
                    if (htab->elf.sgot == NULL) abort();

                    if (IS_TLS_LD(tls_type)) {
                        offp = &htab->tlsld_got.offset;
                    } else if (h != NULL) {
                        if (htab->elf.sgotplt != NULL && h->got.offset != (bfd_vma)-1) {
                            offp = &h->got.offset;
                        } else {
                            abort();
                        }
                    } else {
                        if (local_got_offsets == NULL) abort();
                        offp = &local_got_offsets[r_symndx];
                    }

                    if (!offp) abort();

                    off = (*offp) & ~1;
                    off2 = IS_TLS_LD(tls_type) || IS_TLS_GD(tls_type) ? off + 4 : off;

                    if (h != NULL) {
                        bool dyn = elf_hash_table(info)->dynamic_sections_created;
                        if (WILL_CALL_FINISH_DYNAMIC_SYMBOL(dyn, bfd_link_pic(info), h) && (!bfd_link_pic(info) || !SYMBOL_REFERENCES_LOCAL(info, h))) {
                            indx = h->dynindx;
                        }
                    }

                    if ((bfd_link_pic(info) || indx != 0) && (h == NULL || (ELF_ST_VISIBILITY(h->other) == STV_DEFAULT && !resolved_to_zero) || h->root.type != bfd_link_hash_undefweak)) {
                        need_relocs = true;
                    }

                    static_value = relocation + addend;

                    if (!((*offp) & 1)) {
                        bfd_vma got_offset = (htab->elf.sgot->output_section->vma + htab->elf.sgot->output_offset + off);
                        if (IS_TLS_LD(tls_type)) {
                            if (!bfd_link_pic(info)) {
                                bfd_put_32(output_bfd, 1, htab->elf.sgot->contents + off);
                            } else {
                                microblaze_elf_output_dynamic_relocation(output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++, 0, R_MICROBLAZE_TLSDTPMOD32, got_offset, 0);
                            }
                        } else if (IS_TLS_GD(tls_type)) {
                            if (!need_relocs) {
                                bfd_put_32(output_bfd, 1, htab->elf.sgot->contents + off);
                            } else {
                                microblaze_elf_output_dynamic_relocation(output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++, indx, R_MICROBLAZE_TLSDTPMOD32, got_offset, indx ? 0 : static_value);
                            }
                        }

                        got_offset = (htab->elf.sgot->output_section->vma + htab->elf.sgot->output_offset + off2);

                        if (IS_TLS_LD(tls_type)) {
                            *offp |= 1;
                            bfd_put_32(output_bfd, 0, htab->elf.sgot->contents + off2);
                        } else if (IS_TLS_GD(tls_type)) {
                            *offp |= 1;
                            static_value -= dtprel_base(info);
                            if (need_relocs) {
                                microblaze_elf_output_dynamic_relocation(output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++, indx, R_MICROBLAZE_TLSDTPREL32, got_offset, indx ? 0 : static_value);
                            } else {
                                bfd_put_32(output_bfd, static_value, htab->elf.sgot->contents + off2);
                            }
                        } else {
                            bfd_put_32(output_bfd, static_value, htab->elf.sgot->contents + off2);
                            if (bfd_link_pic(info) && h == NULL) {
                                *offp |= 1;
                                microblaze_elf_output_dynamic_relocation(output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++, indx, R_MICROBLAZE_REL, got_offset, static_value);
                            }
                        }
                    }

                    relocation = htab->elf.sgot->output_section->vma + htab->elf.sgot->output_offset + off - htab->elf.sgotplt->output_section->vma - htab->elf.sgotplt->output_offset;

                    bfd_put_16(input_bfd, (relocation >> 16) & 0xffff, contents + offset + endian);
                    bfd_put_16(input_bfd, relocation & 0xffff, contents + offset + endian + INST_WORD_SIZE);
                    unresolved_reloc = false;
                } break;

                case (int)R_MICROBLAZE_GOTOFF_64: {
                    bfd_vma immediate = relocation += addend - (htab->elf.sgotplt->output_section->vma + htab->elf.sgotplt->output_offset);
                    bfd_put_16(input_bfd, (immediate >> 16) & 0xffff, contents + offset + endian);
                    bfd_put_16(input_bfd, immediate & 0xffff, contents + offset + INST_WORD_SIZE + endian);
                } break;

                case (int)R_MICROBLAZE_GOTOFF_32: {
                    relocation += addend - (htab->elf.sgotplt->output_section->vma + htab->elf.sgotplt->output_offset);
                    bfd_put_32(input_bfd, relocation, contents + offset);
                } break;

                case (int)R_MICROBLAZE_TEXTREL_64:
                case (int)R_MICROBLAZE_TEXTREL_32_LO:
                case (int)R_MICROBLAZE_64_PCREL:
                case (int)R_MICROBLAZE_64:
                case (int)R_MICROBLAZE_32: {
                    if (r_symndx == STN_UNDEF || (input_section->flags & SEC_ALLOC) == 0) {
                        relocation += addend;
                        if (r_type == R_MICROBLAZE_32) {
                            bfd_put_32(input_bfd, relocation, contents + offset);
                        } else {
                            if (r_type == R_MICROBLAZE_64_PCREL) {
                                relocation -= (input_section->output_section->vma + input_section->output_offset + offset + INST_WORD_SIZE);
                            } else if (r_type == R_MICROBLAZE_TEXTREL_64 || r_type == R_MICROBLAZE_TEXTREL_32_LO) {
                                relocation -= input_section->output_section->vma;
                            }
                            if (r_type == R_MICROBLAZE_TEXTREL_32_LO) {
                                bfd_put_16(input_bfd, relocation & 0xffff, contents + offset + endian);
                            } else {
                                bfd_put_16(input_bfd, (relocation >> 16) & 0xffff, contents + offset + endian);
                                bfd_put_16(input_bfd, relocation & 0xffff, contents + offset + endian + INST_WORD_SIZE);
                            }
                        }
                        break;
                    }

                    if ((bfd_link_pic(info) && (h == NULL || (ELF_ST_VISIBILITY(h->other) == STV_DEFAULT && !resolved_to_zero) || h->root.type != bfd_link_hash_undefweak) && (!howto->pc_relative || (h != NULL && h->dynindx != -1 && (!info->symbolic || !h->def_regular)))) || (!bfd_link_pic(info) && h != NULL && h->dynindx != -1 && !h->non_got_ref && ((h->def_dynamic && !h->def_regular) || h->root.type == bfd_link_hash_undefweak || h->root.type == bfd_link_hash_undefined))) {
                        Elf_Internal_Rela outrel;
                        bfd_byte *loc;
                        bool skip = false;

                        BFD_ASSERT(sreloc != NULL);

                        outrel.r_offset = _bfd_elf_section_offset(output_bfd, info, input_section, rel->r_offset) + (input_section->output_section->vma + input_section->output_offset);
                        if (outrel.r_offset == (bfd_vma)-1 || outrel.r_offset == (bfd_vma)-2) skip = true;
                        if (skip) memset(&outrel, 0, sizeof outrel);
                        else if (h != NULL && ((!info->symbolic && h->dynindx != -1) || !h->def_regular)) {
                            BFD_ASSERT(h->dynindx != -1);
                            outrel.r_info = ELF32_R_INFO(h->dynindx, r_type);
                            outrel.r_addend = addend;
                        } else {
                            if (r_type == R_MICROBLAZE_32) {
                                outrel.r_info = ELF32_R_INFO(0, R_MICROBLAZE_REL);
                                outrel.r_addend = relocation + addend;
                            } else {
                                BFD_FAIL();
                                _bfd_error_handler(_("%pB: probably compiled without -fPIC?"), input_bfd);
                                bfd_set_error(bfd_error_bad_value);
                                return false;
                            }
                        }

                        loc = sreloc->contents;
                        loc += sreloc->reloc_count++ * sizeof(Elf32_External_Rela);
                        bfd_elf32_swap_reloca_out(output_bfd, &outrel, loc);
                        break;
                    }
                    else {
                        relocation += addend;
                        if (r_type == R_MICROBLAZE_32) {
                            bfd_put_32(input_bfd, relocation, contents + offset);
                        } else {
                            if (r_type == R_MICROBLAZE_64_PCREL) {
                                relocation -= (input_section->output_section->vma + input_section->output_offset + offset + INST_WORD_SIZE);
                            } else if (r_type == R_MICROBLAZE_TEXTREL_64 || r_type == R_MICROBLAZE_TEXTREL_32_LO) {
                                relocation -= input_section->output_section->vma;
                            }
                            if (r_type == R_MICROBLAZE_TEXTREL_32_LO) {
                                bfd_put_16(input_bfd, relocation & 0xffff, contents + offset + endian);
                            } else {
                                bfd_put_16(input_bfd, (relocation >> 16) & 0xffff, contents + offset + endian);
                                bfd_put_16(input_bfd, relocation & 0xffff, contents + offset + endian + INST_WORD_SIZE);
                            }
                        }
                        break;
                    }
                } break;

                default:
                    r = _bfd_final_link_relocate(howto, input_bfd, input_section, contents, offset, relocation, addend);
                    break;
            }
        }

        check_reloc:
        if (r != bfd_reloc_ok) {
            const char *name;
            if (h != NULL) name = h->root.root.string;
            else {
                name = (bfd_elf_string_from_elf_section(input_bfd, symtab_hdr->sh_link, sym->st_name));
                if (name == NULL || *name == '\0') name = bfd_section_name(sec);
            }
            if (errmsg != NULL) goto common_error;

            switch (r) {
                case bfd_reloc_overflow:
                    (*info->callbacks->reloc_overflow)(info, (h ? &h->root : NULL), name, howto->name, (bfd_vma)0, input_bfd, input_section, offset);
                    break;
                case bfd_reloc_undefined:
                    (*info->callbacks->undefined_symbol)(info, name, input_bfd, input_section, offset, true);
                    break;
                case bfd_reloc_outofrange:
                case bfd_reloc_notsupported:
                case bfd_reloc_dangerous:
                    errmsg = (r == bfd_reloc_outofrange) ? _("internal error: out of range error") : (r == bfd_reloc_notsupported) ? _("internal error: unsupported relocation error") : _("internal error: dangerous error");
                    goto common_error;
                default:
                    errmsg = _("internal error: unknown error");
                    common_error:
                    (*info->callbacks->warning)(info, errmsg, name, input_bfd, input_section, offset);
                    break;
            }
        }
    }
    return ret;
}

/* Calculate fixup value for reference.  */

#include <stddef.h>

static size_t calc_fixup(bfd_vma start, bfd_vma size, asection *sec) {
    if (sec == NULL) {
        return 0;
    }

    struct _microblaze_elf_section_data *sdata = microblaze_elf_section_data(sec);
    if (sdata == NULL) {
        return 0;
    }

    bfd_vma end = start + size;
    size_t fixup = 0;

    for (size_t i = 0; i < sdata->relax_count; i++) {
        if (end <= sdata->relax[i].addr) {
            break;
        }
        if (start <= sdata->relax[i].addr && end > start) {
            fixup += sdata->relax[i].size;
        }
    }
    return fixup;
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   a 32-bit instruction.  */
#include <errno.h>

static int microblaze_bfd_write_imm_value_32(bfd *abfd, bfd_byte *bfd_addr, bfd_vma val) {
    if (!abfd || !bfd_addr) {
        return EINVAL;
    }

    unsigned long instr = bfd_get_32(abfd, bfd_addr);
    instr = (instr & 0xffff0000) | (val & 0x0000ffff);
    bfd_put_32(abfd, instr, bfd_addr);

    return 0;
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   two consecutive 32-bit instructions.  */
#include <stdbool.h>

static bool microblaze_bfd_write_imm_value_64(bfd *abfd, bfd_byte *bfd_addr, bfd_vma val) {
    unsigned long instr_hi = bfd_get_32(abfd, bfd_addr);
    unsigned long instr_lo = bfd_get_32(abfd, bfd_addr + INST_WORD_SIZE);
    
    if (!abfd || !bfd_addr) {
        return false;
    }

    instr_hi = (instr_hi & ~0x0000ffff) | ((val >> 16) & 0x0000ffff);
    instr_lo = (instr_lo & ~0x0000ffff) | (val & 0x0000ffff);

    bfd_put_32(abfd, instr_hi, bfd_addr);
    bfd_put_32(abfd, instr_lo, bfd_addr + INST_WORD_SIZE);

    return true;
}

static bool microblaze_elf_relax_section(bfd *abfd, asection *sec, struct bfd_link_info *link_info, bool *again) {
    if (bfd_link_relocatable(link_info) || (sec->flags & SEC_RELOC) == 0 || (sec->flags & SEC_CODE) == 0 || sec->reloc_count == 0) {
        return true;
    }

    struct _microblaze_elf_section_data *sdata = microblaze_elf_section_data(sec);
    if (sdata == NULL) {
        return true;
    }

    *again = false;
    if (sec->size == 0) {
        sec->size = sec->rawsize;
    }

    Elf_Internal_Shdr *symtab_hdr = &elf_tdata(abfd)->symtab_hdr;
    Elf_Internal_Sym *isymbuf = (Elf_Internal_Sym *)symtab_hdr->contents;
    size_t symcount = symtab_hdr->sh_size / sizeof(Elf32_External_Sym);
    if (isymbuf == NULL) {
        isymbuf = bfd_elf_get_elf_syms(abfd, symtab_hdr, symcount, 0, NULL, NULL, NULL);
    }
    if (isymbuf == NULL) {
        return false;
    }

    Elf_Internal_Rela *internal_relocs = _bfd_elf_link_read_relocs(abfd, sec, NULL, NULL, link_info->keep_memory);
    if (internal_relocs == NULL) {
        return false;
    }

    bfd_byte *contents = NULL;
    bfd_byte *free_contents = NULL;
    if (elf_section_data(sec)->this_hdr.contents != NULL) {
        contents = elf_section_data(sec)->this_hdr.contents;
    } else {
        contents = (bfd_byte *)bfd_malloc(sec->size);
        if (contents == NULL) {
            free(internal_relocs);
            return false;
        }
        free_contents = contents;
        if (!bfd_get_section_contents(abfd, sec, contents, 0, sec->size)) {
            free(internal_relocs);
            free(free_contents);
            return false;
        }
        elf_section_data(sec)->this_hdr.contents = contents;
    }

    sdata->relax = (struct relax_table *)bfd_malloc((sec->reloc_count + 1) * sizeof(*sdata->relax));
    if (sdata->relax == NULL) {
        free(internal_relocs);
        free(free_contents);
        return false;
    }

    sdata->relax_count = 0;
    Elf_Internal_Rela *irelend = internal_relocs + sec->reloc_count;
    for (Elf_Internal_Rela *irel = internal_relocs; irel < irelend; irel++) {
        int rtype = ELF32_R_TYPE(irel->r_info);
        if (rtype != R_MICROBLAZE_64_PCREL && rtype != R_MICROBLAZE_64 && rtype != R_MICROBLAZE_TEXTREL_64) {
            continue;
        }

        bfd_vma symval;
        if (ELF32_R_SYM(irel->r_info) < symtab_hdr->sh_info) {
            Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM(irel->r_info);
            asection *sym_sec = bfd_section_from_elf_index(abfd, isym->st_shndx);
            symval = _bfd_elf_rela_local_sym(abfd, isym, &sym_sec, irel);
        } else {
            unsigned long indx = ELF32_R_SYM(irel->r_info) - symtab_hdr->sh_info;
            struct elf_link_hash_entry *h = elf_sym_hashes(abfd)[indx];
            if (h == NULL || (h->root.type != bfd_link_hash_defined && h->root.type != bfd_link_hash_defweak)) {
                continue;
            }
            symval = h->root.u.def.value + h->root.u.def.section->output_section->vma + h->root.u.def.section->output_offset;
        }

        if (rtype == R_MICROBLAZE_64_PCREL) {
            symval = symval + irel->r_addend - (irel->r_offset + sec->output_section->vma + sec->output_offset);
        } else if (rtype == R_MICROBLAZE_TEXTREL_64) {
            symval = symval + irel->r_addend - sec->output_section->vma;
        } else {
            symval += irel->r_addend;
        }

        if ((symval & 0xffff8000) == 0 || (symval & 0xffff8000) == 0xffff8000) {
            sdata->relax[sdata->relax_count].addr = irel->r_offset;
            sdata->relax[sdata->relax_count].size = INST_WORD_SIZE;
            sdata->relax_count++;

            switch ((enum elf_microblaze_reloc_type)rtype) {
                case R_MICROBLAZE_64_PCREL:
                    irel->r_info = ELF32_R_INFO(ELF32_R_SYM(irel->r_info), R_MICROBLAZE_32_PCREL_LO);
                    break;
                case R_MICROBLAZE_64:
                    irel->r_info = ELF32_R_INFO(ELF32_R_SYM(irel->r_info), R_MICROBLAZE_32_LO);
                    break;
                case R_MICROBLAZE_TEXTREL_64:
                    irel->r_info = ELF32_R_INFO(ELF32_R_SYM(irel->r_info), R_MICROBLAZE_TEXTREL_32_LO);
                    break;
                default:
                    free(internal_relocs);
                    free(free_contents);
                    free(sdata->relax);
                    return false;
            }
        }
    }

    if (sdata->relax_count > 0) {
        bfd_vma dest = sdata->relax[0].addr;
        for (size_t i = 0; i < sdata->relax_count; i++) {
            bfd_vma src = sdata->relax[i].addr + sdata->relax[i].size;
            size_t len = sdata->relax[i + 1].addr - sdata->relax[i].addr - sdata->relax[i].size;
            memmove(contents + dest, contents + src, len);
            sec->size -= sdata->relax[i].size;
            dest += len;
        }

        *again = true;
    }

    elf_section_data(sec)->relocs = internal_relocs;
    free(free_relocs);
    free(free_contents);

    return true;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

asection *microblaze_elf_gc_mark_hook(asection *sec, struct bfd_link_info *info, Elf_Internal_Rela *rel, struct elf_link_hash_entry *h, Elf_Internal_Sym *sym) {
  if (h != NULL && (ELF32_R_TYPE(rel->r_info) == R_MICROBLAZE_GNU_VTINHERIT || ELF32_R_TYPE(rel->r_info) == R_MICROBLAZE_GNU_VTENTRY)) {
    return NULL;
  }
  return _bfd_elf_gc_mark_hook(sec, info, rel, h, sym);
}

/* PIC support.  */

#define PLT_ENTRY_SIZE 16

#define PLT_ENTRY_WORD_0  0xb0000000	      /* "imm 0".  */
#define PLT_ENTRY_WORD_1  0xe9940000	      /* "lwi r12,r20,0" - relocated to lwi r12,r20,func@GOT.  */
#define PLT_ENTRY_WORD_1_NOPIC	0xe9800000    /* "lwi r12,r0,0" - non-PIC object.  */
#define PLT_ENTRY_WORD_2  0x98186000	      /* "brad r12".  */
#define PLT_ENTRY_WORD_3  0x80000000	      /* "nop".  */

bool update_local_sym_info(bfd *abfd, Elf_Internal_Shdr *symtab_hdr, unsigned long r_symndx, unsigned int tls_type) {
    bfd_signed_vma *local_got_refcounts = elf_local_got_refcounts(abfd);
    unsigned char *local_got_tls_masks;

    if (!local_got_refcounts) {
        bfd_size_type size = symtab_hdr->sh_info * (sizeof(*local_got_refcounts) + sizeof(*local_got_tls_masks));
        local_got_refcounts = bfd_zalloc(abfd, size);
        if (!local_got_refcounts) {
            return false;
        }
        elf_local_got_refcounts(abfd) = local_got_refcounts;
    }

    local_got_tls_masks = (unsigned char *)(local_got_refcounts + symtab_hdr->sh_info);
    if (r_symndx < symtab_hdr->sh_info) {
        local_got_tls_masks[r_symndx] |= tls_type;
        local_got_refcounts[r_symndx]++;
    } else {
        return false;
    }

    return true;
}
/* Look through the relocs for a section during the first phase.  */

static bool microblaze_elf_check_relocs(bfd *abfd, struct bfd_link_info *info, asection *sec, const Elf_Internal_Rela *relocs) {
    if (bfd_link_relocatable(info)) return true;

    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    if (!htab) return false;

    Elf_Internal_Shdr *symtab_hdr = &elf_tdata(abfd)->symtab_hdr;
    struct elf_link_hash_entry **sym_hashes = elf_sym_hashes(abfd);
    const Elf_Internal_Rela *rel_end = relocs + sec->reloc_count;
    asection *sreloc = NULL;

    for (const Elf_Internal_Rela *rel = relocs; rel < rel_end; rel++) {
        unsigned int r_type = ELF32_R_TYPE(rel->r_info);
        unsigned long r_symndx = ELF32_R_SYM(rel->r_info);
        struct elf_link_hash_entry *h = NULL;
        unsigned char tls_type = 0;

        if (r_symndx >= symtab_hdr->sh_info) {
            h = sym_hashes[r_symndx - symtab_hdr->sh_info];
            while (h->root.type == bfd_link_hash_indirect || h->root.type == bfd_link_hash_warning) {
                h = (struct elf_link_hash_entry *)h->root.u.i.link;
            }
        }

        switch (r_type) {
            case R_MICROBLAZE_GNU_VTINHERIT:
                if (!bfd_elf_gc_record_vtinherit(abfd, sec, h, rel->r_offset)) return false;
                break;

            case R_MICROBLAZE_GNU_VTENTRY:
                if (!bfd_elf_gc_record_vtentry(abfd, sec, h, rel->r_addend)) return false;
                break;

            case R_MICROBLAZE_PLT_64:
                if (h) {
                    h->needs_plt = 1;
                    h->plt.refcount++;
                }
                break;

            case R_MICROBLAZE_TLSGD:
                tls_type |= (TLS_TLS | TLS_GD);
                sec->has_tls_reloc = 1;
                goto got_entry;
            case R_MICROBLAZE_TLSLD:
                tls_type |= (TLS_TLS | TLS_LD);
                sec->has_tls_reloc = 1;
                goto got_entry;
            case R_MICROBLAZE_GOT_64:
            got_entry:
                if (!htab->elf.sgot) {
                    if (!htab->elf.dynobj) htab->elf.dynobj = abfd;
                    if (!_bfd_elf_create_got_section(htab->elf.dynobj, info)) return false;
                }
                if (h) {
                    h->got.refcount++;
                    elf32_mb_hash_entry(h)->tls_mask |= tls_type;
                } else if (!update_local_sym_info(abfd, symtab_hdr, r_symndx, tls_type)) {
                    return false;
                }
                break;

            case R_MICROBLAZE_GOTOFF_64:
            case R_MICROBLAZE_GOTOFF_32:
                if (!htab->elf.sgot) {
                    if (!htab->elf.dynobj) htab->elf.dynobj = abfd;
                    if (!_bfd_elf_create_got_section(htab->elf.dynobj, info)) return false;
                }
                break;

            case R_MICROBLAZE_64:
            case R_MICROBLAZE_64_PCREL:
            case R_MICROBLAZE_32:
                if (h && !bfd_link_pic(info)) {
                    h->non_got_ref = 1;
                    h->plt.refcount++;
                    if (r_type != R_MICROBLAZE_64_PCREL) {
                        h->pointer_equality_needed = 1;
                    }
                }
                if ((bfd_link_pic(info) && (sec->flags & SEC_ALLOC) &&
                     (r_type != R_MICROBLAZE_64_PCREL || (h && (!info->symbolic ||
                      h->root.type == bfd_link_hash_defweak || !h->def_regular)))) ||
                    (!bfd_link_pic(info) && (sec->flags & SEC_ALLOC) && h && 
                    (h->root.type == bfd_link_hash_defweak || !h->def_regular))) {

                    if (!sreloc) {
                        if (!htab->elf.dynobj) htab->elf.dynobj = abfd;
                        sreloc = _bfd_elf_make_dynamic_reloc_section(sec, htab->elf.dynobj, 2, abfd, 1);
                        if (!sreloc) return false;
                    }

                    struct elf_dyn_relocs **head = (h) ? &h->dyn_relocs : NULL;
                    if (!h) {
                        asection *s = bfd_section_from_elf_index(abfd, bfd_sym_from_r_symndx(&htab->elf.sym_cache, abfd, r_symndx)->st_shndx);
                        if (!s) return false;
                        head = (struct elf_dyn_relocs **)&elf_section_data(s)->local_dynrel;
                    }

                    struct elf_dyn_relocs *p = *head;
                    if (!p || p->sec != sec) {
                        p = (struct elf_dyn_relocs *)bfd_alloc(htab->elf.dynobj, sizeof(*p));
                        if (!p) return false;
                        p->next = *head;
                        *head = p;
                        p->sec = sec;
                        p->count = 0;
                        p->pc_count = 0;
                    }

                    p->count++;
                    if (r_type == R_MICROBLAZE_64_PCREL) p->pc_count++;
                }
                break;
        }
    }
    return true;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void microblaze_elf_copy_indirect_symbol(struct bfd_link_info *info, struct elf_link_hash_entry *dir, struct elf_link_hash_entry *ind) {
    if (!info || !dir || !ind) {
        return; // Handle invalid input gracefully
    }

    struct elf32_mb_link_hash_entry *edir = (struct elf32_mb_link_hash_entry *)dir;
    struct elf32_mb_link_hash_entry *eind = (struct elf32_mb_link_hash_entry *)ind;

    edir->tls_mask |= eind->tls_mask;
    _bfd_elf_link_hash_copy_indirect(info, dir, ind);
}

static bool microblaze_elf_adjust_dynamic_symbol(struct bfd_link_info *info, struct elf_link_hash_entry *h) {
    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    if (!htab) return false;

    if ((h->type == STT_FUNC || h->needs_plt) && (h->plt.refcount <= 0 || SYMBOL_CALLS_LOCAL(info, h) ||
        (ELF_ST_VISIBILITY(h->other) != STV_DEFAULT && h->root.type == bfd_link_hash_undefweak))) {
        h->plt.offset = (bfd_vma)-1;
        h->needs_plt = 0;
        return true;
    }

    h->plt.offset = (bfd_vma)-1;

    if (h->is_weakalias) {
        struct elf_link_hash_entry *def = weakdef(h);
        BFD_ASSERT(def->root.type == bfd_link_hash_defined);
        h->root.u.def.section = def->root.u.def.section;
        h->root.u.def.value = def->root.u.def.value;
        return true;
    }

    if (bfd_link_pic(info)) return true;

    if (!h->non_got_ref || info->nocopyreloc || !_bfd_elf_readonly_dynrelocs(h)) {
        h->non_got_ref = 0;
        return true;
    }

    asection *s, *srel;
    if ((h->root.u.def.section->flags & SEC_READONLY) != 0) {
        s = htab->elf.sdynrelro;
        srel = htab->elf.sreldynrelro;
    } else {
        s = htab->elf.sdynbss;
        srel = htab->elf.srelbss;
    }

    if ((h->root.u.def.section->flags & SEC_ALLOC) != 0) {
        srel->size += sizeof(Elf32_External_Rela);
        h->needs_copy = 1;
    }

    unsigned int power_of_two = bfd_log2(h->size);
    power_of_two = (power_of_two > 3) ? 3 : power_of_two;

    s->size = BFD_ALIGN(s->size, (bfd_size_type)(1U << power_of_two));
    if (!bfd_link_align_section(s, power_of_two)) return false;

    h->root.u.def.section = s;
    h->root.u.def.value = s->size;
    s->size += h->size;
    
    return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool allocate_dynrelocs(struct elf_link_hash_entry *h, void *dat) {
    if (h->root.type == bfd_link_hash_indirect) return true;

    struct bfd_link_info *info = (struct bfd_link_info *)dat;
    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    if (htab == NULL) return false;

    if (htab->elf.dynamic_sections_created && h->plt.refcount > 0) {
        if (h->dynindx == -1 && !h->forced_local) {
            if (!bfd_elf_link_record_dynamic_symbol(info, h)) return false;
        }

        if (WILL_CALL_FINISH_DYNAMIC_SYMBOL(1, bfd_link_pic(info), h)) {
            asection *s = htab->elf.splt;
            if (s->size == 0) s->size = PLT_ENTRY_SIZE;

            h->plt.offset = s->size;

            if (!bfd_link_pic(info) && !h->def_regular) {
                h->root.u.def.section = s;
                h->root.u.def.value = h->plt.offset;
            }

            s->size += PLT_ENTRY_SIZE;
            htab->elf.sgotplt->size += 4;
            htab->elf.srelplt->size += sizeof(Elf32_External_Rela);
        } else {
            h->plt.offset = (bfd_vma)-1;
            h->needs_plt = 0;
        }
    } else {
        h->plt.offset = (bfd_vma)-1;
        h->needs_plt = 0;
    }

    struct elf32_mb_link_hash_entry *eh = (struct elf32_mb_link_hash_entry *)h;
    if (h->got.refcount > 0) {
        if (h->dynindx == -1 && !h->forced_local) {
            if (!bfd_elf_link_record_dynamic_symbol(info, h)) return false;
        }

        unsigned int need = 0;
        if ((eh->tls_mask & TLS_TLS) != 0) {
            if ((eh->tls_mask & TLS_LD) != 0) {
                if (!eh->elf.def_dynamic) htab->tlsld_got.refcount += 1;
                else need += 8;
            }
            if ((eh->tls_mask & TLS_GD) != 0) need += 8;
        } else {
            need += 4;
        }

        if (need == 0) {
            h->got.offset = (bfd_vma)-1;
        } else {
            asection *s = htab->elf.sgot;
            h->got.offset = s->size;
            s->size += need;
            htab->elf.srelgot->size += need * (sizeof(Elf32_External_Rela) / 4);
        }
    } else {
        h->got.offset = (bfd_vma)-1;
    }

    if (h->dyn_relocs == NULL) return true;

    if (bfd_link_pic(info)) {
        if (h->def_regular && (h->forced_local || info->symbolic)) {
            struct elf_dyn_relocs **pp;
            for (pp = &h->dyn_relocs; (p = *pp) != NULL;) {
                p->count -= p->pc_count;
                p->pc_count = 0;
                if (p->count == 0) *pp = p->next;
                else pp = &p->next;
            }
        } else if (UNDEFWEAK_NO_DYNAMIC_RELOC(info, h)) {
            h->dyn_relocs = NULL;
        }
    } else {
        if (!h->non_got_ref && 
            ((h->def_dynamic && !h->def_regular) || 
             (htab->elf.dynamic_sections_created && 
             (h->root.type == bfd_link_hash_undefweak || h->root.type == bfd_link_hash_undefined)))) {
            if (h->dynindx == -1 && !h->forced_local) {
                if (!bfd_elf_link_record_dynamic_symbol(info, h)) return false;
            }
            if (h->dynindx != -1) goto keep;
        }
        h->dyn_relocs = NULL;

    keep: ;
    }

    for (struct elf_dyn_relocs *p = h->dyn_relocs; p != NULL; p = p->next) {
        asection *sreloc = elf_section_data(p->sec)->sreloc;
        sreloc->size += p->count * sizeof(Elf32_External_Rela);
    }

    return true;
}

/* Set the sizes of the dynamic sections.  */

#include <stdbool.h>

static bool microblaze_elf_late_size_sections(bfd *output_bfd ATTRIBUTE_UNUSED, struct bfd_link_info *info) {
    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    if (htab == NULL) {
        return false;
    }

    bfd *dynobj = htab->elf.dynobj;
    if (dynobj == NULL) {
        return true;
    }

    for (bfd *ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next) {
        if (bfd_get_flavour(ibfd) != bfd_target_elf_flavour) {
            continue;
        }

        bfd_signed_vma *local_got = elf_local_got_refcounts(ibfd);
        if (!local_got) {
            continue;
        }

        Elf_Internal_Shdr *symtab_hdr = &elf_tdata(ibfd)->symtab_hdr;
        bfd_size_type locsymcount = symtab_hdr->sh_info;
        bfd_signed_vma *end_local_got = local_got + locsymcount;
        unsigned char *lgot_masks = (unsigned char *)end_local_got;
        asection *s = htab->elf.sgot;
        asection *srel = htab->elf.srelgot;

        for (; local_got < end_local_got; ++local_got, ++lgot_masks) {
            unsigned int need = 0;
            if (*local_got > 0) {
                if ((*lgot_masks & TLS_TLS) != 0) {
                    if ((*lgot_masks & TLS_GD) != 0) {
                        need += 8;
                    }
                    if ((*lgot_masks & TLS_LD) != 0) {
                        htab->tlsld_got.refcount += 1;
                    }
                } else {
                    need += 4;
                }
            }
            *local_got = (need == 0) ? (bfd_vma) -1 : s->size;
            s->size += need;
            if (bfd_link_pic(info)) {
                srel->size += need * (sizeof(Elf32_External_Rela) / 4);
            }
        }

        for (asection *sec = ibfd->sections; sec != NULL; sec = sec->next) {
            struct elf_dyn_relocs *p = (struct elf_dyn_relocs *)elf_section_data(sec)->local_dynrel;
            while (p != NULL) {
                if (p->count != 0 && (!bfd_is_abs_section(p->sec) || !bfd_is_abs_section(p->sec->output_section))) {
                    asection *srel = elf_section_data(p->sec)->sreloc;
                    srel->size += p->count * sizeof(Elf32_External_Rela);
                    if ((p->sec->output_section->flags & SEC_READONLY) != 0) {
                        info->flags |= DF_TEXTREL;
                    }
                }
                p = p->next;
            }
        }
    }

    elf_link_hash_traverse(elf_hash_table(info), allocate_dynrelocs, info);

    if (htab->tlsld_got.refcount > 0) {
        htab->tlsld_got.offset = htab->elf.sgot->size;
        htab->elf.sgot->size += 8;
        if (bfd_link_pic(info)) {
            htab->elf.srelgot->size += sizeof(Elf32_External_Rela);
        }
    } else {
        htab->tlsld_got.offset = (bfd_vma)-1;
    }

    if (elf_hash_table(info)->dynamic_sections_created && htab->elf.splt->size > 0) {
        htab->elf.splt->size += 4;
    }

    for (asection *s = dynobj->sections; s != NULL; s = s->next) {
        if ((s->flags & SEC_LINKER_CREATED) == 0) {
            continue;
        }

        const char *name = bfd_section_name(s);
        bool strip = false;

        if (startswith(name, ".rela")) {
            if (s->size == 0) {
                strip = true;
            } else {
                s->reloc_count = 0;
            }
        } else if (s != htab->elf.splt && s != htab->elf.sgot && s != htab->elf.sgotplt && s != htab->elf.sdynbss && s != htab->elf.sdynrelro) {
            continue;
        }

        if (strip) {
            s->flags |= SEC_EXCLUDE;
            continue;
        }

        s->contents = (bfd_byte *)bfd_zalloc(dynobj, s->size);
        if (s->contents == NULL && s->size != 0) {
            return false;
        }
        s->alloced = 1;
    }

    info->flags |= DF_BIND_NOW;
    return _bfd_elf_add_dynamic_tags(output_bfd, info, true);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

#include <stdbool.h>
#include <stddef.h>

static bool microblaze_elf_finish_dynamic_symbol(bfd *output_bfd,
                                                 struct bfd_link_info *info,
                                                 struct elf_link_hash_entry *h,
                                                 Elf_Internal_Sym *sym) {
    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    struct elf32_mb_link_hash_entry *eh = elf32_mb_hash_entry(h);

    if (h->plt.offset != (bfd_vma)-1) {
        BFD_ASSERT(h->dynindx != -1);

        asection *splt = htab->elf.splt;
        asection *srela = htab->elf.srelplt;
        asection *sgotplt = htab->elf.sgotplt;
        BFD_ASSERT(splt != NULL && srela != NULL && sgotplt != NULL);

        bfd_vma plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
        bfd_vma got_offset = (plt_index + 3) * 4;
        bfd_vma got_addr = got_offset + (bfd_link_pic(info) ? 0 : sgotplt->output_section->vma + sgotplt->output_offset);

        bfd_put_32(output_bfd, PLT_ENTRY_WORD_0 + ((got_addr >> 16) & 0xffff),
                   splt->contents + h->plt.offset);
        bfd_put_32(output_bfd, (bfd_link_pic(info) ? PLT_ENTRY_WORD_1 : PLT_ENTRY_WORD_1_NOPIC) + (got_addr & 0xffff),
                   splt->contents + h->plt.offset + 4);
        
        bfd_put_32(output_bfd, (bfd_vma)PLT_ENTRY_WORD_2, splt->contents + h->plt.offset + 8);
        bfd_put_32(output_bfd, (bfd_vma)PLT_ENTRY_WORD_3, splt->contents + h->plt.offset + 12);

        Elf_Internal_Rela rela = {
            .r_offset = sgotplt->output_section->vma + sgotplt->output_offset + got_offset,
            .r_info = ELF32_R_INFO(h->dynindx, R_MICROBLAZE_JUMP_SLOT),
            .r_addend = 0
        };
        
        bfd_byte *loc = srela->contents + plt_index * sizeof(Elf32_External_Rela);
        bfd_elf32_swap_reloca_out(output_bfd, &rela, loc);

        if (!h->def_regular) {
            sym->st_shndx = SHN_UNDEF;
            sym->st_value = 0;
        }
    }
    
    if ((h->got.offset != (bfd_vma)-1) && !((h->got.offset & 1) || IS_TLS_LD(eh->tls_mask) || IS_TLS_GD(eh->tls_mask))) {
        asection *sgot = htab->elf.sgot;
        asection *srela = htab->elf.srelgot;
        BFD_ASSERT(sgot != NULL && srela != NULL);

        bfd_vma offset = sgot->output_section->vma + sgot->output_offset + (h->got.offset & ~(bfd_vma)1);

        if (bfd_link_pic(info) && ((info->symbolic && h->def_regular) || h->dynindx == -1)) {
            asection *sec = h->root.u.def.section;
            bfd_vma value = h->root.u.def.value;
            if (sec->output_section != NULL)
                value += sec->output_section->vma + sec->output_offset;

            microblaze_elf_output_dynamic_relocation(output_bfd, srela, srela->reloc_count++, 0, R_MICROBLAZE_REL, offset, value);
        } else {
            microblaze_elf_output_dynamic_relocation(output_bfd, srela, srela->reloc_count++, h->dynindx, R_MICROBLAZE_GLOB_DAT, offset, 0);
        }
        
        bfd_put_32(output_bfd, (bfd_vma)0, sgot->contents + (h->got.offset & ~(bfd_vma)1));
    }

    if (h->needs_copy) {
        BFD_ASSERT(h->dynindx != -1);

        Elf_Internal_Rela rela = {
            .r_offset = h->root.u.def.value + h->root.u.def.section->output_section->vma + h->root.u.def.section->output_offset,
            .r_info = ELF32_R_INFO(h->dynindx, R_MICROBLAZE_COPY),
            .r_addend = 0
        };
        
        asection *s = (h->root.u.def.section == htab->elf.sdynrelro) ? htab->elf.sreldynrelro : htab->elf.srelbss;
        bfd_byte *loc = s->contents + s->reloc_count++ * sizeof(Elf32_External_Rela);
        bfd_elf32_swap_reloca_out(output_bfd, &rela, loc);
    }
    
    if (h == htab->elf.hdynamic || h == htab->elf.hgot || h == htab->elf.hplt) {
        sym->st_shndx = SHN_ABS;
    }
    
    return true;
}


/* Finish up the dynamic sections.  */

static bool
microblaze_elf_finish_dynamic_sections(bfd *output_bfd, struct bfd_link_info *info)
{
    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    if (!htab) return false;

    bfd *dynobj = htab->elf.dynobj;
    asection *sdyn = bfd_get_linker_section(dynobj, ".dynamic");

    if (htab->elf.dynamic_sections_created)
    {
        Elf32_External_Dyn *dyncon = (Elf32_External_Dyn *)sdyn->contents;
        Elf32_External_Dyn *dynconend = (Elf32_External_Dyn *)(sdyn->contents + sdyn->size);

        for (; dyncon < dynconend; ++dyncon)
        {
            Elf_Internal_Dyn dyn;
            asection *s = NULL;
            bool size = false;

            bfd_elf32_swap_dyn_in(dynobj, dyncon, &dyn);

            switch (dyn.d_tag)
            {
            case DT_PLTGOT:
                s = htab->elf.sgotplt;
                break;
            case DT_PLTRELSZ:
                s = htab->elf.srelplt;
                size = true;
                break;
            case DT_JMPREL:
                s = htab->elf.srelplt;
                break;
            }

            if (s)
            {
                if (size)
                    dyn.d_un.d_val = s->size;
                else
                    dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
            }
            else
            {
                dyn.d_un.d_val = 0;
            }

            bfd_elf32_swap_dyn_out(output_bfd, &dyn, dyncon);
        }

        asection *splt = htab->elf.splt;
        BFD_ASSERT(splt != NULL && sdyn != NULL);

        if (splt->size > 0)
        {
            memset(splt->contents, 0, PLT_ENTRY_SIZE);
            bfd_put_32(output_bfd, (bfd_vma)0x80000000, splt->contents + splt->size - 4);

            if (splt->output_section != bfd_abs_section_ptr)
                elf_section_data(splt->output_section)->this_hdr.sh_entsize = 4;
        }
    }

    asection *sgot = htab->elf.sgotplt;
    if (sgot && sgot->size > 0)
    {
        bfd_put_32(output_bfd, (sdyn ? sdyn->output_section->vma + sdyn->output_offset : 0), sgot->contents);
        elf_section_data(sgot->output_section)->this_hdr.sh_entsize = 4;
    }

    if (htab->elf.sgot && htab->elf.sgot->size > 0)
    {
        elf_section_data(htab->elf.sgot->output_section)->this_hdr.sh_entsize = 4;
    }

    return true;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */

#include <stdbool.h>
#include <stddef.h>
#include <bfd.h>
#include <elf.h>

static bool microblaze_elf_add_symbol_hook(bfd *abfd, struct bfd_link_info *info, Elf_Internal_Sym *sym, const char **namep ATTRIBUTE_UNUSED, flagword *flagsp ATTRIBUTE_UNUSED, asection **secp, bfd_vma *valp) {
    if (sym->st_shndx == SHN_COMMON && !bfd_link_relocatable(info) && sym->st_size <= elf_gp_size(abfd)) {
        *secp = bfd_make_section_old_way(abfd, ".sbss");
        if (*secp == NULL || !bfd_set_section_flags(*secp, SEC_IS_COMMON | SEC_SMALL_DATA)) {
            return false;
        }
        *valp = sym->st_size;
    }
    return true;
}

#define TARGET_LITTLE_SYM      microblaze_elf32_le_vec
#define TARGET_LITTLE_NAME     "elf32-microblazeel"

#define TARGET_BIG_SYM		microblaze_elf32_vec
#define TARGET_BIG_NAME		"elf32-microblaze"

#define ELF_ARCH		bfd_arch_microblaze
#define ELF_TARGET_ID		MICROBLAZE_ELF_DATA
#define ELF_MACHINE_CODE	EM_MICROBLAZE
#define ELF_MACHINE_ALT1	EM_MICROBLAZE_OLD
#define ELF_MAXPAGESIZE		0x1000
#define elf_info_to_howto	microblaze_elf_info_to_howto
#define elf_info_to_howto_rel	NULL

#define bfd_elf32_bfd_reloc_type_lookup		microblaze_elf_reloc_type_lookup
#define bfd_elf32_bfd_is_local_label_name	microblaze_elf_is_local_label_name
#define bfd_elf32_new_section_hook		microblaze_elf_new_section_hook
#define elf_backend_relocate_section		microblaze_elf_relocate_section
#define bfd_elf32_bfd_relax_section		microblaze_elf_relax_section
#define bfd_elf32_bfd_merge_private_bfd_data	_bfd_generic_verify_endian_match
#define bfd_elf32_bfd_reloc_name_lookup		microblaze_elf_reloc_name_lookup

#define elf_backend_gc_mark_hook		microblaze_elf_gc_mark_hook
#define elf_backend_check_relocs		microblaze_elf_check_relocs
#define elf_backend_copy_indirect_symbol	microblaze_elf_copy_indirect_symbol
#define bfd_elf32_bfd_link_hash_table_create	microblaze_elf_link_hash_table_create
#define elf_backend_can_gc_sections		1
#define elf_backend_can_refcount		1
#define elf_backend_want_got_plt		1
#define elf_backend_plt_readonly		1
#define elf_backend_got_header_size		12
#define elf_backend_want_dynrelro		1
#define elf_backend_rela_normal			1
#define elf_backend_dtrel_excludes_plt		1

#define elf_backend_adjust_dynamic_symbol	microblaze_elf_adjust_dynamic_symbol
#define elf_backend_create_dynamic_sections	_bfd_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections	microblaze_elf_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol	microblaze_elf_finish_dynamic_symbol
#define elf_backend_late_size_sections		microblaze_elf_late_size_sections
#define elf_backend_add_symbol_hook		microblaze_elf_add_symbol_hook

#include "elf32-target.h"
