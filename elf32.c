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

static void
microblaze_elf_howto_init (void)
{
  unsigned int i;

  for (i = NUM_ELEM (microblaze_elf_howto_raw); i--;)
    {
      unsigned int type = microblaze_elf_howto_raw[i].type;
      BFD_ASSERT (type < NUM_ELEM (microblaze_elf_howto_table));
      microblaze_elf_howto_table [type] = & microblaze_elf_howto_raw [i];
    }
}

static enum elf_microblaze_reloc_type
map_bfd_reloc_to_microblaze(bfd_reloc_code_real_type code)
{
  switch (code)
    {
    case BFD_RELOC_NONE:
      return R_MICROBLAZE_NONE;
    case BFD_RELOC_MICROBLAZE_32_NONE:
      return R_MICROBLAZE_32_NONE;
    case BFD_RELOC_MICROBLAZE_64_NONE:
      return R_MICROBLAZE_64_NONE;
    case BFD_RELOC_32:
    case BFD_RELOC_RVA:
      return R_MICROBLAZE_32;
    case BFD_RELOC_32_PCREL:
      return R_MICROBLAZE_32_PCREL;
    case BFD_RELOC_64_PCREL:
      return R_MICROBLAZE_64_PCREL;
    case BFD_RELOC_MICROBLAZE_32_LO_PCREL:
      return R_MICROBLAZE_32_PCREL_LO;
    case BFD_RELOC_64:
      return R_MICROBLAZE_64;
    case BFD_RELOC_MICROBLAZE_32_LO:
      return R_MICROBLAZE_32_LO;
    case BFD_RELOC_MICROBLAZE_32_ROSDA:
      return R_MICROBLAZE_SRO32;
    case BFD_RELOC_MICROBLAZE_32_RWSDA:
      return R_MICROBLAZE_SRW32;
    case BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM:
      return R_MICROBLAZE_32_SYM_OP_SYM;
    case BFD_RELOC_VTABLE_INHERIT:
      return R_MICROBLAZE_GNU_VTINHERIT;
    case BFD_RELOC_VTABLE_ENTRY:
      return R_MICROBLAZE_GNU_VTENTRY;
    case BFD_RELOC_MICROBLAZE_64_GOTPC:
      return R_MICROBLAZE_GOTPC_64;
    case BFD_RELOC_MICROBLAZE_64_GOT:
      return R_MICROBLAZE_GOT_64;
    case BFD_RELOC_MICROBLAZE_64_TEXTPCREL:
      return R_MICROBLAZE_TEXTPCREL_64;
    case BFD_RELOC_MICROBLAZE_64_TEXTREL:
      return R_MICROBLAZE_TEXTREL_64;
    case BFD_RELOC_MICROBLAZE_64_PLT:
      return R_MICROBLAZE_PLT_64;
    case BFD_RELOC_MICROBLAZE_64_GOTOFF:
      return R_MICROBLAZE_GOTOFF_64;
    case BFD_RELOC_MICROBLAZE_32_GOTOFF:
      return R_MICROBLAZE_GOTOFF_32;
    case BFD_RELOC_MICROBLAZE_64_TLSGD:
      return R_MICROBLAZE_TLSGD;
    case BFD_RELOC_MICROBLAZE_64_TLSLD:
      return R_MICROBLAZE_TLSLD;
    case BFD_RELOC_MICROBLAZE_32_TLSDTPREL:
      return R_MICROBLAZE_TLSDTPREL32;
    case BFD_RELOC_MICROBLAZE_64_TLSDTPREL:
      return R_MICROBLAZE_TLSDTPREL64;
    case BFD_RELOC_MICROBLAZE_32_TLSDTPMOD:
      return R_MICROBLAZE_TLSDTPMOD32;
    case BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL:
      return R_MICROBLAZE_TLSGOTTPREL32;
    case BFD_RELOC_MICROBLAZE_64_TLSTPREL:
      return R_MICROBLAZE_TLSTPREL32;
    case BFD_RELOC_MICROBLAZE_COPY:
      return R_MICROBLAZE_COPY;
    default:
      return R_MICROBLAZE_NONE;
    }
}

static void
ensure_howto_table_initialized(void)
{
  if (!microblaze_elf_howto_table[R_MICROBLAZE_32])
    microblaze_elf_howto_init();
}

static reloc_howto_type *
microblaze_elf_reloc_type_lookup(bfd * abfd ATTRIBUTE_UNUSED,
                                 bfd_reloc_code_real_type code)
{
  enum elf_microblaze_reloc_type microblaze_reloc = map_bfd_reloc_to_microblaze(code);
  
  if (microblaze_reloc == R_MICROBLAZE_NONE && code != BFD_RELOC_NONE)
    return (reloc_howto_type *) NULL;
    
  ensure_howto_table_initialized();
  
  return microblaze_elf_howto_table[(int) microblaze_reloc];
};

static reloc_howto_type *
microblaze_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				  const char *r_name)
{
  unsigned int i;

  for (i = 0; i < NUM_ELEM (microblaze_elf_howto_raw); i++)
    {
      if (microblaze_elf_howto_raw[i].name == NULL)
        continue;
      
      if (strcasecmp (microblaze_elf_howto_raw[i].name, r_name) == 0)
        return &microblaze_elf_howto_raw[i];
    }

  return NULL;
}

/* Set the howto pointer for a RCE ELF reloc.  */

static bool
microblaze_elf_info_to_howto (bfd * abfd,
			      arelent * cache_ptr,
			      Elf_Internal_Rela * dst)
{
  if (!microblaze_elf_howto_table [R_MICROBLAZE_32])
    microblaze_elf_howto_init ();

  unsigned int r_type = ELF32_R_TYPE (dst->r_info);
  
  if (r_type >= R_MICROBLAZE_max)
    {
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  cache_ptr->howto = microblaze_elf_howto_table [r_type];
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

static bool
microblaze_elf_new_section_hook (bfd *abfd, asection *sec)
{
  struct _microblaze_elf_section_data *sdata;

  sdata = bfd_zalloc (abfd, sizeof (*sdata));
  if (sdata == NULL)
    return false;
  sec->used_by_bfd = sdata;

  return _bfd_elf_new_section_hook (abfd, sec);
}

/* Microblaze ELF local labels start with 'L.' or '$L', not '.L'.  */

static bool
microblaze_elf_is_local_label_name (bfd *abfd, const char *name)
{
  #define LOCAL_LABEL_PREFIX_L "L."
  #define LOCAL_LABEL_PREFIX_DOLLAR "$L"
  #define LOCAL_LABEL_PREFIX_L_LEN 2
  #define LOCAL_LABEL_PREFIX_DOLLAR_LEN 2

  if (strncmp(name, LOCAL_LABEL_PREFIX_L, LOCAL_LABEL_PREFIX_L_LEN) == 0)
    return true;

  if (strncmp(name, LOCAL_LABEL_PREFIX_DOLLAR, LOCAL_LABEL_PREFIX_DOLLAR_LEN) == 0)
    return true;

  return _bfd_elf_is_local_label_name (abfd, name);
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

static struct bfd_hash_entry *
allocate_entry_if_needed(struct bfd_hash_table *table, struct bfd_hash_entry *entry)
{
  if (entry != NULL)
    return entry;
  
  return bfd_hash_allocate(table, sizeof(struct elf32_mb_link_hash_entry));
}

static void
initialize_mb_link_hash_entry(struct bfd_hash_entry *entry)
{
  struct elf32_mb_link_hash_entry *eh;
  eh = (struct elf32_mb_link_hash_entry *) entry;
  eh->tls_mask = 0;
}

static struct bfd_hash_entry *
link_hash_newfunc(struct bfd_hash_entry *entry,
                  struct bfd_hash_table *table,
                  const char *string)
{
  entry = allocate_entry_if_needed(table, entry);
  if (entry == NULL)
    return entry;

  entry = _bfd_elf_link_hash_newfunc(entry, table, string);
  if (entry == NULL)
    return entry;

  initialize_mb_link_hash_entry(entry);
  return entry;
}

/* Create a mb ELF linker hash table.  */

static struct bfd_link_hash_table *
microblaze_elf_link_hash_table_create (bfd *abfd)
{
  struct elf32_mb_link_hash_table *ret;
  size_t amt = sizeof (struct elf32_mb_link_hash_table);

  ret = (struct elf32_mb_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd, link_hash_newfunc,
				      sizeof (struct elf32_mb_link_hash_entry)))
    {
      free (ret);
      return NULL;
    }

  return &ret->elf.root;
}

/* Set the values of the small data pointers.  */

static bfd_vma
calculate_pointer_value(struct bfd_link_hash_entry *h)
{
  return h->u.def.value
         + h->u.def.section->output_section->vma
         + h->u.def.section->output_offset;
}

static void
set_small_data_pointer(struct bfd_link_info *info, 
                      const char *anchor_name,
                      bfd_vma *pointer_var)
{
  struct bfd_link_hash_entry *h;
  
  h = bfd_link_hash_lookup(info->hash, anchor_name, false, false, true);
  if (h != NULL && h->type == bfd_link_hash_defined)
    *pointer_var = calculate_pointer_value(h);
}

static void
microblaze_elf_final_sdp(struct bfd_link_info *info)
{
  set_small_data_pointer(info, RO_SDA_ANCHOR_NAME, &ro_small_data_pointer);
  set_small_data_pointer(info, RW_SDA_ANCHOR_NAME, &rw_small_data_pointer);
}

static bfd_vma
dtprel_base (struct bfd_link_info *info)
{
  struct elf_link_hash_table *hash_table = elf_hash_table (info);
  
  if (hash_table->tls_sec == NULL)
    return 0;
    
  return hash_table->tls_sec->vma;
}

/* The size of the thread control block.  */
#define TCB_SIZE	8

/* Output a simple dynamic relocation into SRELOC.  */

static void
microblaze_elf_output_dynamic_relocation (bfd *output_bfd,
					  asection *sreloc,
					  unsigned long reloc_index,
					  unsigned long indx,
					  int r_type,
					  bfd_vma offset,
					  bfd_vma addend)
{
  Elf_Internal_Rela rel;

  rel.r_info = ELF32_R_INFO (indx, r_type);
  rel.r_offset = offset;
  rel.r_addend = addend;

  bfd_elf32_swap_reloca_out (output_bfd, &rel,
	      (sreloc->contents + reloc_index * sizeof (Elf32_External_Rela)));
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

#define MASK_16BIT 0xffff
#define INST_WORD_SIZE 4
#define TLS_TLS 1
#define TLS_GD 2
#define TLS_LD 4
#define IS_TLS_LD(x) ((x) & TLS_LD)
#define IS_TLS_GD(x) ((x) & TLS_GD)

static bool
validate_relocation_type(bfd *input_bfd, int r_type)
{
  if (r_type < 0 || r_type >= (int) R_MICROBLAZE_max)
    {
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
                         input_bfd, (int) r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  return true;
}

static bool
handle_relocatable_link(bfd *input_bfd, asection **local_sections,
                       Elf_Internal_Sym *local_syms, Elf_Internal_Rela *rel,
                       bfd_byte *contents, Elf_Internal_Shdr *symtab_hdr,
                       reloc_howto_type *howto)
{
  unsigned long r_symndx = ELF32_R_SYM (rel->r_info);
  
  if (r_symndx >= symtab_hdr->sh_info)
    return true;

  Elf_Internal_Sym *sym = local_syms + r_symndx;
  
  if (ELF_ST_TYPE (sym->st_info) != STT_SECTION)
    return true;

  asection *sec = local_sections[r_symndx];
  bfd_vma addend = rel->r_addend + sec->output_offset + sym->st_value;

#ifdef USE_REL
  if (howto->partial_inplace)
    {
      _bfd_relocate_contents (howto, input_bfd, addend,
                             contents + rel->r_offset);
    }
#endif
  return true;
}

static void
get_symbol_info(bfd *input_bfd, Elf_Internal_Rela *rel,
               Elf_Internal_Shdr *symtab_hdr, Elf_Internal_Sym *local_syms,
               asection **local_sections, struct elf_link_hash_entry **sym_hashes,
               struct bfd_link_info *info, Elf_Internal_Sym **sym_out,
               asection **sec_out, struct elf_link_hash_entry **h_out,
               const char **sym_name_out, bfd_vma *relocation_out,
               bool *unresolved_reloc)
{
  unsigned long r_symndx = ELF32_R_SYM (rel->r_info);
  
  if (r_symndx < symtab_hdr->sh_info)
    {
      *sym_out = local_syms + r_symndx;
      *sec_out = local_sections[r_symndx];
      *h_out = NULL;
      *sym_name_out = "<local symbol>";
      if (*sec_out != 0)
        *relocation_out = _bfd_elf_rela_local_sym (input_bfd, *sym_out, sec_out, rel);
    }
  else
    {
      bool warned = false;
      bool ignored = false;
      RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, NULL, rel,
                              r_symndx, symtab_hdr, sym_hashes,
                              *h_out, *sec_out, *relocation_out,
                              *unresolved_reloc, warned, ignored);
      if (*h_out)
        *sym_name_out = (*h_out)->root.root.string;
    }
}

static bool
check_small_data_section(const char *name, const char *expected1, const char *expected2)
{
  return (strcmp(name, expected1) == 0 || strcmp(name, expected2) == 0);
}

static bfd_reloc_status_type
handle_small_data_relocation(bfd *input_bfd, asection *input_section,
                            bfd_byte *contents, bfd_vma offset,
                            bfd_vma relocation, bfd_vma addend,
                            bfd_vma small_data_pointer, reloc_howto_type *howto)
{
  relocation -= small_data_pointer;
  return _bfd_final_link_relocate(howto, input_bfd, input_section,
                                 contents, offset, relocation, addend);
}

static bool
process_sro32_relocation(bfd *input_bfd, asection *input_section,
                        asection *sec, bfd_byte *contents, bfd_vma offset,
                        bfd_vma relocation, bfd_vma addend, reloc_howto_type *howto,
                        const char *sym_name, struct bfd_link_info *info)
{
  if (!sec)
    return true;
    
  const char *name = bfd_section_name(sec);
  
  if (!check_small_data_section(name, ".sdata2", ".sbss2"))
    {
      _bfd_error_handler(_("%pB: the target (%s) of an %s relocation"
                          " is in the wrong section (%pA)"),
                        input_bfd, sym_name, howto->name, sec);
      return false;
    }
    
  if (ro_small_data_pointer == 0)
    {
      microblaze_elf_final_sdp(info);
      if (ro_small_data_pointer == 0)
        return false;
    }
    
  handle_small_data_relocation(input_bfd, input_section, contents, offset,
                              relocation, addend, ro_small_data_pointer, howto);
  return true;
}

static bool
process_srw32_relocation(bfd *input_bfd, asection *input_section,
                        asection *sec, bfd_byte *contents, bfd_vma offset,
                        bfd_vma relocation, bfd_vma addend, reloc_howto_type *howto,
                        const char *sym_name, struct bfd_link_info *info)
{
  if (!sec)
    return true;
    
  const char *name = bfd_section_name(sec);
  
  if (!check_small_data_section(name, ".sdata", ".sbss"))
    {
      _bfd_error_handler(_("%pB: the target (%s) of an %s relocation"
                          " is in the wrong section (%pA)"),
                        input_bfd, sym_name, howto->name, sec);
      return false;
    }
    
  if (rw_small_data_pointer == 0)
    {
      microblaze_elf_final_sdp(info);
      if (rw_small_data_pointer == 0)
        return false;
    }
    
  handle_small_data_relocation(input_bfd, input_section, contents, offset,
                              relocation, addend, rw_small_data_pointer, howto);
  return true;
}

static void
write_64bit_value(bfd *input_bfd, bfd_byte *contents, bfd_vma offset,
                 bfd_vma value, int endian)
{
  bfd_put_16(input_bfd, (value >> 16) & MASK_16BIT, contents + offset + endian);
  bfd_put_16(input_bfd, value & MASK_16BIT, contents + offset + endian + INST_WORD_SIZE);
}

static void
compute_pc_relative_offset(asection *input_section, bfd_vma *relocation,
                          bfd_vma offset, bfd_vma addend)
{
  *relocation -= (input_section->output_section->vma +
                 input_section->output_offset + offset + INST_WORD_SIZE);
  *relocation += addend;
}

static bfd_vma *
get_got_offset_pointer(struct elf32_mb_link_hash_table *htab,
                      struct elf_link_hash_entry *h,
                      bfd_vma *local_got_offsets, unsigned long r_symndx,
                      unsigned int tls_type)
{
  if (IS_TLS_LD(tls_type))
    return &htab->tlsld_got.offset;
  else if (h != NULL)
    {
      if (htab->elf.sgotplt != NULL && h->got.offset != (bfd_vma) -1)
        return &h->got.offset;
    }
  else if (local_got_offsets != NULL)
    return &local_got_offsets[r_symndx];
    
  return NULL;
}

static unsigned long
get_symbol_index(struct bfd_link_info *info, struct elf_link_hash_entry *h)
{
  if (!h)
    return 0;
    
  bool dyn = elf_hash_table(info)->dynamic_sections_created;
  
  if (WILL_CALL_FINISH_DYNAMIC_SYMBOL(dyn, bfd_link_pic(info), h) &&
      (!bfd_link_pic(info) || !SYMBOL_REFERENCES_LOCAL(info, h)))
    return h->dynindx;
    
  return 0;
}

static void
process_tls_module_id(bfd *output_bfd, struct elf32_mb_link_hash_table *htab,
                     struct bfd_link_info *info, bfd_vma off,
                     unsigned int tls_type, unsigned long indx,
                     bfd_vma static_value, bool need_relocs)
{
  bfd_vma got_offset = htab->elf.sgot->output_section->vma +
                      htab->elf.sgot->output_offset + off;
                      
  if (IS_TLS_LD(tls_type))
    {
      if (!bfd_link_pic(info))
        bfd_put_32(output_bfd, 1, htab->elf.sgot->contents + off);
      else
        microblaze_elf_output_dynamic_relocation(output_bfd, htab->elf.srelgot,
                                                htab->elf.srelgot->reloc_count++,
                                                0, R_MICROBLAZE_TLSDTPMOD32,
                                                got_offset, 0);
    }
  else if (IS_TLS_GD(tls_type))
    {
      if (!need_relocs)
        bfd_put_32(output_bfd, 1, htab->elf.sgot->contents + off);
      else
        microblaze_elf_output_dynamic_relocation(output_bfd, htab->elf.srelgot,
                                                htab->elf.srelgot->reloc_count++,
                                                indx, R_MICROBLAZE_TLSDTPMOD32,
                                                got_offset, indx ? 0 : static_value);
    }
}

static void
process_tls_offset(bfd *output_bfd, struct elf32_mb_link_hash_table *htab,
                  struct bfd_link_info *info, bfd_vma off2,
                  unsigned int tls_type, unsigned long indx,
                  bfd_vma static_value, bool need_relocs, bfd_vma *offp)
{
  bfd_vma got_offset = htab->elf.sgot->output_section->vma +
                      htab->elf.sgot->output_offset + off2;
                      
  if (IS_TLS_LD(tls_type))
    {
      *offp |= 1;
      bfd_put_32(output_bfd, 0, htab->elf.sgot->contents + off2);
    }
  else if (IS_TLS_GD(tls_type))
    {
      *offp |= 1;
      static_value -= dtprel_base(info);
      if (need_relocs)
        microblaze_elf_output_dynamic_relocation(output_bfd, htab->elf.srelgot,
                                                htab->elf.srelgot->reloc_count++,
                                                indx, R_MICROBLAZE_TLSDTPREL32,
                                                got_offset, indx ? 0 : static_value);
      else
        bfd_put_32(output_bfd, static_value, htab->elf.sgot->contents + off2);
    }
  else
    {
      bfd_put_32(output_bfd, static_value, htab->elf.sgot->contents + off2);
      if (bfd_link_pic(info) && !indx)
        {
          *offp |= 1;
          microblaze_elf_output_dynamic_relocation(output_bfd, htab->elf.srelgot,
                                                  htab->elf.srelgot->reloc_count++,
                                                  indx, R_MICROBLAZE_REL,
                                                  got_offset, static_value);
        }
    }
}

static bool
needs_dynamic_relocation(struct bfd_link_info *info,
                        struct elf_link_hash_entry *h,
                        bool resolved_to_zero, reloc_howto_type *howto)
{
  if (bfd_link_pic(info))
    {
      if (h == NULL || (ELF_ST_VISIBILITY(h->other) == STV_DEFAULT && !resolved_to_zero) ||
          h->root.type != bfd_link_hash_undefweak)
        {
          if (!howto->pc_relative || (h && h->dynindx != -1 &&
              (!info->symbolic || !h->def_regular)))
            return true;
        }
    }
  else if (h && h->dynindx != -1 && !h->non_got_ref)
    {
      if ((h->def_dynamic && !h->def_regular) ||
          h->root.type == bfd_link_hash_undefweak ||
          h->root.type == bfd_link_hash_undefined)
        return true;
    }
  return false;
}

static void
output_dynamic_relocation(bfd *output_bfd, asection *sreloc, asection *input_section,
                         Elf_Internal_Rela *rel, struct elf_link_hash_entry *h,
                         bfd_vma relocation, bfd_vma addend, int r_type,
                         struct bfd_link_info *info)
{
  Elf_Internal_Rela outrel;
  bfd_byte *loc;

  outrel.r_offset = _bfd_elf_section_offset(output_bfd, info, input_section, rel->r_offset);
  if (outrel.r_offset == (bfd_vma) -1 || outrel.r_offset == (bfd_vma) -2)
    {
      memset(&outrel, 0, sizeof outrel);
      return;
    }

  outrel.r_offset += (input_section->output_section->vma + input_section->output_offset);

  if (h != NULL && ((!info->symbolic && h->dynindx != -1) || !h->def_regular))
    {
      outrel.r_info = ELF32_R_INFO(h->dynindx, r_type);
      outrel.r_addend = addend;
    }
  else if (r_type == R_MICROBLAZE_32)
    {
      outrel.r_info = ELF32_R_INFO(0, R_MICROBLAZE_REL);
      outrel.r_addend = relocation + addend;
    }
  else
    {
      BFD_FAIL();
      _bfd_error_handler(_("%pB: probably compiled without -fPIC?"), output_bfd);
      bfd_set_error(bfd_error_bad_value);
      return;
    }

  loc = sreloc->contents;
  loc += sreloc->reloc_count++ * sizeof(Elf32_External_Rela);
  bfd_elf32_swap_reloca_out(output_bfd, &outrel, loc);
}

static void
write_textrel_relocation(bfd *input_bfd, bfd_byte *contents, bfd_vma offset,
                        bfd_vma relocation, asection *input_section, int r_type, int endian)
{
  if (r_type == R_MICROBLAZE_64_PCREL)
    relocation -= (input_section->output_section->vma +
                  input_section->output_offset + offset + INST_WORD_SIZE);
  else if (r_type == R_MICROBLAZE_TEXTREL_64 || r_type == R_MICROBLAZE_TEXTREL_32_LO)
    relocation -= input_section->output_section->vma;

  if (r_type == R_MICROBLAZE_TEXTREL_32_LO)
    bfd_put_16(input_bfd, relocation & MASK_16BIT, contents + offset + endian);
  else
    write_64bit_value(input_bfd, contents, offset, relocation, endian);
}

static void
handle_relocation_error(struct bfd_link_info *info, struct elf_link_hash_entry *h,
                       Elf_Internal_Sym *sym, asection *sec, bfd *input_bfd,
                       asection *input_section, bfd_vma offset,
                       Elf_Internal_Shdr *symtab_hdr, bfd_reloc_status_type r,
                       reloc_howto_type *howto, const char *errmsg)
{
  const char *name;

  if (h != NULL)
    name = h->root.root.string;
  else
    {
      name = bfd_elf_string_from_elf_section(input_bfd, symtab_hdr->sh_link, sym->st_name);
      if (name == NULL || *name == '\0')
        name = bfd_section_name(sec);
    }

  if (errmsg != NULL)
    {
      (*info->callbacks->warning)(info, errmsg, name, input_bfd, input_section, offset);
      return;
    }

  switch (r)
    {
    case bfd_reloc_overflow:
      (*info->callbacks->reloc_overflow)(info, (h ? &h->root : NULL), name, howto->name,
                                        (bfd_vma) 0, input_bfd, input_section, offset);
      break;
    case bfd_reloc_undefined:
      (*info->callbacks->undefined_symbol)(info, name, input_bfd, input_section, offset, true);
      break;
    case bfd_reloc_outofrange:
      (*info->callbacks->warning)(info, _("internal error: out of range error"),
                                 name, input_bfd, input_section, offset);
      break;
    case bfd_reloc_notsupported:
      (*info->callbacks->warning)(info, _("internal error: unsupported relocation error"),
                                 name, input_bfd, input_section, offset);
      break;
    case bfd_reloc_dangerous:
      (*info->callbacks->warning)(info, _("internal error: dangerous error"),
                                 name, input_bfd, input_section, offset);
      break;
    default:
      (*info->callbacks->warning)(info, _("internal error: unknown error"),
                                 name, input_bfd, input_section, offset);
      break;
    }
}

static int
microblaze_elf_relocate_section(bfd *output_bfd, struct bfd_link_info *info,
                               bfd *input_bfd, asection *input_section,
                               bfd_byte *contents, Elf_Internal_Rela *relocs,
                               Elf_Internal_Sym *local_syms, asection **local_sections)
{
  struct elf32_mb_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata(input_bfd)->symtab_hdr;
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes(input_bfd);
  Elf_Internal_Rela *rel, *relend;
  int endian = bfd_little_endian(output_bfd) ? 0 : 2;
  bool ret = true;
  asection *sreloc;
  bfd_vma *local_got_offsets;
  unsigned int tls_type;

  if (!microblaze_elf_howto_table[R_MICROBLAZE_max-1])
    microblaze_elf_howto_init();

  htab = elf32_mb_hash_table(info);
  if (htab == NULL)
    return false;

  local_got_offsets = elf_local_got_offsets(input_bfd);
  sreloc = elf_section_data(input_section)->sreloc;

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  
  for (; rel < relend; rel++)
    {
      int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      bfd_vma addend = rel->r_addend;
      bfd_vma offset = rel->r_offset;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *sym;
      asection *sec;
      const char *sym_name;
      bfd_reloc_status_type r = bfd_reloc_ok;
      const char *errmsg = NULL;
      bool unresolved_reloc = false;

      h = NULL;
      r_type = ELF32_R_TYPE(rel->r_info);
      tls_type = 0;

      if (!validate_relocation_type(input_bfd, r_type))
        {
          ret = false;
          continue;
        }

      howto = microblaze_elf_howto_table[r_type];
      r_symndx = ELF32_R_SYM(rel->r_info);

      if (bfd_link_relocatable(info))
        {
          if (!handle_relocatable_link(input_bfd, local_sections, local_syms,
                                      rel, contents, symtab_hdr, howto))
            continue;
        }
      else
        {
          bfd_vma relocation;
          bool resolved_to_zero;

          sym = NULL;
          sec = NULL;
          unresolved_reloc = false;

          get_symbol_info(input_bfd, rel, symtab_hdr, local_syms, local_sections,
                        sym_hashes, info, &sym, &sec, &h, &sym_name, &relocation,
                        &unresolved_reloc);

          if (r_symndx < symtab_hdr->sh_info && sec == 0)
            continue;

          if (offset > bfd_get_section_limit(input_bfd, input_section))
            {
              r = bfd_reloc_outofrange;
              goto check_reloc;
            }

          resolved_to_zero = (h != NULL && UNDEFWEAK_NO_DYNAMIC_RELOC(info, h));

          switch ((int) r_type)
            {
            case (int) R_MICROBLAZE_SRO32:
              if (!process_sro32_relocation(input_bfd, input_section, sec, contents,
                                           offset, relocation, addend, howto, sym_name, info))
                {
                  ret = false;
                  continue;
                }
              break;

            case (int) R_MICROBLAZE_SRW32:
              if (!process_srw32_relocation(input_bfd, input_section, sec, contents,
                                           offset, relocation, addend, howto, sym_name, info))
                {
                  ret = false;
                  continue;
                }
              break;

            case (int) R_MICROBLAZE_32_SYM_OP_SYM:
              break;

            case (int) R_MICROBLAZE_GOTPC_64:
              relocation = (htab->elf.sgotplt->output_section->vma +
                          htab->elf.sgotplt->output_offset);
              compute_pc_relative_offset(input_section, &relocation, offset, addend);
              write_64bit_value(input_bfd, contents, offset, relocation, endian);
              break;

            case (int) R_MICROBLAZE_TEXTPCREL_64:
              relocation = input_section->output_section->vma;
              compute_pc_relative_offset(input_section, &relocation, offset, addend);
              write_64bit_value(input_bfd, contents, offset, relocation, endian);
              break;

            case (int) R_MICROBLAZE_PLT_64:
              {
                bfd_vma immediate;
                if (htab->elf.splt != NULL && h != NULL && h->plt.offset != (bfd_vma) -1)
                  {
                    relocation = (htab->elf.splt->output_section->vma +
                                htab->elf.splt->output_offset + h->plt.offset);
                    unresolved_reloc = false;
                  }
                immediate = relocation - (input_section->output_section->vma +
                                        input_section->output_offset + offset + INST_WORD_SIZE);
                write_64bit_value(input_bfd, contents, offset, immediate, endian);
              }
              break;

            case (int) R_MICROBLAZE_TLSGD:
              tls_type = (TLS_TLS | TLS_GD);
              goto dogot;
            case (int) R_MICROBLAZE_TLSLD:
              tls_type = (TLS_TLS | TLS_LD);
            dogot:
            case (int) R_MICROBLAZE_GOT_64:
              {
                bfd_vma *offp;
                bfd_vma off, off2;
                unsigned long indx;
                bfd_vma static_value;
                bool need_relocs = false;

                if (htab->elf.sgot == NULL)
                  abort();

                offp = get_got_offset_pointer(htab, h, local_got_offsets, r_symndx, tls_type);
                if (!offp)
                  abort();

                off = (*offp) & ~1;
                off2 = (IS_TLS_LD(tls_type) || IS_TLS_GD(tls_type)) ? off + 4 : off;

                indx = get_symbol_index(info, h);

                if ((bfd_link_pic(info) || indx != 0) &&
                    (h == NULL || (ELF_ST_VISIBILITY(h->other) == STV_DEFAULT && !resolved_to_zero) ||
                     h->root.type != bfd_link_hash_undefweak))
                  need_relocs = true;

                static_value = relocation + addend;

                if (!((*offp) & 1))
                  {
                    process_tls_module_id(output_bfd, htab, info, off, tls_type,
                                        indx, static_value, need_relocs);

                    if (htab->elf.srelgot == NULL)
                      abort();

                    process_tls_offset(output_bfd, htab, info, off2, tls_type,
                                     indx, static_value, need_relocs, offp);
                  }

                relocation = htab->elf.sgot->output_section->vma +
                           htab->elf.sgot->output_offset + off -
                           htab->elf.sgotplt->output_section->vma -
                           htab->elf.sgotplt->output_offset;

                write_64bit_value(input_bfd, contents, offset, relocation, endian);
                unresolved_reloc = false;
              }
              break;

            case (int) R_MICROBLAZE_GOTOFF_64:
              {
                bfd_vma immediate;
                relocation += addend;
                relocation -= (htab->elf.sgotplt->output_section->vma +
                             htab->elf.sgotplt->output_offset);
                immediate = relocation;
                bfd_put_16(input_bfd, (immediate >> 16) & MASK_16BIT, contents + offset + endian);
                bfd_put_16(input_bfd, immediate & MASK_16BIT,
                         contents + offset + INST_WORD_SIZE + endian);
              }
              break;

            case (int) R_MICROBLAZE_GOTOFF_32:
              relocation += addend;
              relocation -= (htab->elf.sgotplt->output_section->vma +
                           htab->elf.sgotplt->output_offset);
              bfd_put_32(input_bfd, relocation, contents + offset);
              break;

            case (int) R_MICROBLAZE_TLSDTPREL64:
              relocation += addend;
              relocation -= dtprel_base(info);
              write_64bit_value(input_bfd, contents, offset, relocation, endian);
              break;

            case (int) R_MICROBLAZE_TEXTREL_64:
            case (int) R_MICROBLAZE_TEXTREL_32_LO:
            case (int) R_MICROBLAZE_64_PCREL:
            case (int) R_MICROBLAZE_64:
            case (int) R_MICROBLAZE_32:
              {
                if (r_symndx == STN_UNDEF || (input_section->flags & SEC_ALLOC) == 0)
                  {
                    relocation += addend;
                    if (r_type == R_MICROBLAZE_32)
                      bfd_put_32(input_bfd, relocation, contents + offset);
                    else
                      write_textrel_relocation(input_bfd, contents, offset, relocation,
                                             input_section, r_type, endian);
                    break;
                  }

                if (needs_dynamic_relocation(info, h, resolved_to_zero, howto))
                  {
                    BFD_ASSERT(sreloc != NULL);
                    output_dynamic_relocation(output_bfd, sreloc, input_section, rel,
                                            h, relocation, addend, r_type, info);
                    break;
                  }
                else
                  {
                    relocation += addend;
                    if (r_type == R_MICROBLAZE_32)
                      bfd_put_32(input_b

/* Calculate fixup value for reference.  */

static size_t
calc_fixup (bfd_vma start, bfd_vma size, asection *sec)
{
  struct _microblaze_elf_section_data *sdata;
  bfd_vma end = start + size;
  size_t fixup = 0;
  size_t i;

  if (sec == NULL)
    return 0;

  sdata = microblaze_elf_section_data (sec);
  if (sdata == NULL)
    return 0;

  for (i = 0; i < sdata->relax_count; i++)
    {
      if (end <= sdata->relax[i].addr)
        break;
      
      if (end == start || start <= sdata->relax[i].addr)
        fixup += sdata->relax[i].size;
    }

  return fixup;
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   a 32-bit instruction.  */
static void
microblaze_bfd_write_imm_value_32 (bfd *abfd, bfd_byte *bfd_addr, bfd_vma val)
{
    #define IMM_VALUE_MASK 0x0000ffff
    
    unsigned long instr = bfd_get_32 (abfd, bfd_addr);
    instr &= ~IMM_VALUE_MASK;
    instr |= (val & IMM_VALUE_MASK);
    bfd_put_32 (abfd, instr, bfd_addr);
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   two consecutive 32-bit instructions.  */
static void
microblaze_bfd_write_imm_value_64 (bfd *abfd, bfd_byte *bfd_addr, bfd_vma val)
{
    #define IMM_MASK 0x0000ffff
    #define IMM_SHIFT 16
    
    unsigned long instr_hi;
    unsigned long instr_lo;

    instr_hi = bfd_get_32 (abfd, bfd_addr);
    instr_hi &= ~IMM_MASK;
    instr_hi |= ((val >> IMM_SHIFT) & IMM_MASK);
    bfd_put_32 (abfd, instr_hi, bfd_addr);

    instr_lo = bfd_get_32 (abfd, bfd_addr + INST_WORD_SIZE);
    instr_lo &= ~IMM_MASK;
    instr_lo |= (val & IMM_MASK);
    bfd_put_32 (abfd, instr_lo, bfd_addr + INST_WORD_SIZE);
}

#define INST_WORD_SIZE 4
#define SYMBOL_FIXUP_MASK 0xffff8000

static bool is_section_relaxable(asection *sec, struct bfd_link_info *link_info)
{
    struct _microblaze_elf_section_data *sdata = microblaze_elf_section_data(sec);
    
    return !bfd_link_relocatable(link_info)
        && (sec->flags & SEC_RELOC) != 0
        && (sec->flags & SEC_CODE) != 0
        && sec->reloc_count != 0
        && sdata != NULL;
}

static bool is_reloc_type_relaxable(int r_type)
{
    return r_type == R_MICROBLAZE_64_PCREL
        || r_type == R_MICROBLAZE_64
        || r_type == R_MICROBLAZE_TEXTREL_64;
}

static bfd_byte *get_section_contents(bfd *abfd, asection *sec, bfd_byte **free_contents)
{
    bfd_byte *contents;
    
    if (elf_section_data(sec)->this_hdr.contents != NULL) {
        return elf_section_data(sec)->this_hdr.contents;
    }
    
    contents = (bfd_byte *)bfd_malloc(sec->size);
    if (contents == NULL) {
        return NULL;
    }
    
    *free_contents = contents;
    
    if (!bfd_get_section_contents(abfd, sec, contents, (file_ptr)0, sec->size)) {
        return NULL;
    }
    
    elf_section_data(sec)->this_hdr.contents = contents;
    return contents;
}

static bfd_byte *get_other_section_contents(bfd *abfd, asection *o)
{
    bfd_byte *ocontents;
    
    if (elf_section_data(o)->this_hdr.contents != NULL) {
        return elf_section_data(o)->this_hdr.contents;
    }
    
    if (o->rawsize == 0) {
        o->rawsize = o->size;
    }
    
    ocontents = (bfd_byte *)bfd_malloc(o->rawsize);
    if (ocontents == NULL) {
        return NULL;
    }
    
    if (!bfd_get_section_contents(abfd, o, ocontents, (file_ptr)0, o->rawsize)) {
        return NULL;
    }
    
    elf_section_data(o)->this_hdr.contents = ocontents;
    return ocontents;
}

static asection *get_symbol_section(bfd *abfd, Elf_Internal_Sym *isym)
{
    if (isym->st_shndx == SHN_UNDEF) {
        return bfd_und_section_ptr;
    }
    if (isym->st_shndx == SHN_ABS) {
        return bfd_abs_section_ptr;
    }
    if (isym->st_shndx == SHN_COMMON) {
        return bfd_com_section_ptr;
    }
    return bfd_section_from_elf_index(abfd, isym->st_shndx);
}

static bfd_vma get_local_symbol_value(bfd *abfd, Elf_Internal_Sym *isym, 
                                      Elf_Internal_Rela *irel)
{
    asection *sym_sec = get_symbol_section(abfd, isym);
    return _bfd_elf_rela_local_sym(abfd, isym, &sym_sec, irel);
}

static bfd_vma get_global_symbol_value(bfd *abfd, Elf_Internal_Rela *irel,
                                       Elf_Internal_Shdr *symtab_hdr)
{
    unsigned long indx = ELF32_R_SYM(irel->r_info) - symtab_hdr->sh_info;
    struct elf_link_hash_entry *h = elf_sym_hashes(abfd)[indx];
    
    BFD_ASSERT(h != NULL);
    
    if (h->root.type != bfd_link_hash_defined &&
        h->root.type != bfd_link_hash_defweak) {
        return 0;
    }
    
    return h->root.u.def.value
         + h->root.u.def.section->output_section->vma
         + h->root.u.def.section->output_offset;
}

static bfd_vma calculate_symbol_value(bfd *abfd, Elf_Internal_Rela *irel, 
                                      asection *sec, Elf_Internal_Sym *isymbuf,
                                      Elf_Internal_Shdr *symtab_hdr)
{
    bfd_vma symval;
    int r_type = ELF32_R_TYPE(irel->r_info);
    
    if (ELF32_R_SYM(irel->r_info) < symtab_hdr->sh_info) {
        symval = get_local_symbol_value(abfd, isymbuf + ELF32_R_SYM(irel->r_info), irel);
    } else {
        symval = get_global_symbol_value(abfd, irel, symtab_hdr);
        if (symval == 0) {
            return ULONG_MAX;
        }
    }
    
    if (r_type == R_MICROBLAZE_64_PCREL) {
        symval = symval + irel->r_addend
               - (irel->r_offset + sec->output_section->vma + sec->output_offset);
    } else if (r_type == R_MICROBLAZE_TEXTREL_64) {
        symval = symval + irel->r_addend - sec->output_section->vma;
    } else {
        symval += irel->r_addend;
    }
    
    return symval;
}

static void rewrite_relocation_type(Elf_Internal_Rela *irel)
{
    int new_type;
    
    switch (ELF32_R_TYPE(irel->r_info)) {
    case R_MICROBLAZE_64_PCREL:
        new_type = R_MICROBLAZE_32_PCREL_LO;
        break;
    case R_MICROBLAZE_64:
        new_type = R_MICROBLAZE_32_LO;
        break;
    case R_MICROBLAZE_TEXTREL_64:
        new_type = R_MICROBLAZE_TEXTREL_32_LO;
        break;
    default:
        BFD_ASSERT(false);
        return;
    }
    
    irel->r_info = ELF32_R_INFO(ELF32_R_SYM(irel->r_info), new_type);
}

static bool process_relaxable_reloc(bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
                                    struct _microblaze_elf_section_data *sdata,
                                    bfd_byte **contents, bfd_byte **free_contents,
                                    Elf_Internal_Sym *isymbuf, Elf_Internal_Shdr *symtab_hdr)
{
    bfd_vma symval;
    
    if (*contents == NULL) {
        *contents = get_section_contents(abfd, sec, free_contents);
        if (*contents == NULL) {
            return false;
        }
    }
    
    symval = calculate_symbol_value(abfd, irel, sec, isymbuf, symtab_hdr);
    if (symval == ULONG_MAX) {
        return true;
    }
    
    if ((symval & SYMBOL_FIXUP_MASK) == 0 || 
        (symval & SYMBOL_FIXUP_MASK) == SYMBOL_FIXUP_MASK) {
        sdata->relax[sdata->relax_count].addr = irel->r_offset;
        sdata->relax[sdata->relax_count].size = INST_WORD_SIZE;
        sdata->relax_count++;
        rewrite_relocation_type(irel);
    }
    
    return true;
}

static void handle_none_reloc(bfd *abfd, bfd_byte *contents, Elf_Internal_Rela *irel,
                              asection *sec, int r_type)
{
    size_t sfix, efix;
    bfd_vma target_address;
    
    if (r_type == R_MICROBLAZE_64_NONE) {
        target_address = irel->r_addend + irel->r_offset + INST_WORD_SIZE;
        sfix = calc_fixup(irel->r_offset + INST_WORD_SIZE, 0, sec);
        efix = calc_fixup(target_address, 0, sec);
        irel->r_addend -= (efix - sfix);
        microblaze_bfd_write_imm_value_32(abfd, contents + irel->r_offset + INST_WORD_SIZE,
                                          irel->r_addend);
    } else {
        target_address = irel->r_addend + irel->r_offset;
        sfix = calc_fixup(irel->r_offset, 0, sec);
        efix = calc_fixup(target_address, 0, sec);
        irel->r_addend -= (efix - sfix);
        microblaze_bfd_write_imm_value_32(abfd, contents + irel->r_offset, irel->r_addend);
    }
}

static void update_reloc_in_section(bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
                                    bfd_byte *contents, Elf_Internal_Sym *isymbuf,
                                    Elf_Internal_Shdr *symtab_hdr, unsigned int shndx)
{
    bfd_vma nraddr = irel->r_offset - calc_fixup(irel->r_offset, 0, sec);
    int r_type = ELF32_R_TYPE(irel->r_info);
    
    switch (r_type) {
    case R_MICROBLAZE_TEXTREL_64:
    case R_MICROBLAZE_TEXTREL_32_LO:
    case R_MICROBLAZE_64:
    case R_MICROBLAZE_32_LO:
        if (ELF32_R_SYM(irel->r_info) < symtab_hdr->sh_info) {
            Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM(irel->r_info);
            if (isym->st_shndx == shndx && ELF32_ST_TYPE(isym->st_info) == STT_SECTION) {
                irel->r_addend -= calc_fixup(irel->r_addend, 0, sec);
            }
        }
        break;
    case R_MICROBLAZE_NONE:
    case R_MICROBLAZE_32_NONE:
    case R_MICROBLAZE_64_NONE:
        handle_none_reloc(abfd, contents, irel, sec, r_type);
        break;
    }
    
    irel->r_offset = nraddr;
}

static bool handle_r32_reloc(bfd *abfd, asection *o, asection *sec,
                             Elf_Internal_Rela *irelscan, bfd_byte **ocontents,
                             Elf_Internal_Sym *isymbuf, unsigned int shndx)
{
    Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM(irelscan->r_info);
    
    if (isym->st_shndx == shndx && ELF32_ST_TYPE(isym->st_info) == STT_SECTION) {
        if (*ocontents == NULL) {
            *ocontents = get_other_section_contents(abfd, o);
            if (*ocontents == NULL) {
                return false;
            }
        }
        irelscan->r_addend -= calc_fixup(irelscan->r_addend, 0, sec);
    }
    return true;
}

static bool handle_r32_sym_op_sym_reloc(bfd *abfd, asection *o, asection *sec,
                                        Elf_Internal_Rela *irelscan, bfd_byte **ocontents,
                                        Elf_Internal_Sym *isymbuf)
{
    Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM(irelscan->r_info);
    
    if (*ocontents == NULL) {
        *ocontents = get_other_section_contents(abfd, o);
        if (*ocontents == NULL) {
            return false;
        }
    }
    irelscan->r_addend -= calc_fixup(irelscan->r_addend + isym->st_value, 0, sec);
    return true;
}

static bool handle_lo_reloc(bfd *abfd, asection *o, asection *sec,
                            Elf_Internal_Rela *irelscan, bfd_byte **ocontents,
                            Elf_Internal_Sym *isymbuf, unsigned int shndx)
{
    Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM(irelscan->r_info);
    
    if (isym->st_shndx == shndx && ELF32_ST_TYPE(isym->st_info) == STT_SECTION) {
        if (*ocontents == NULL) {
            *ocontents = get_other_section_contents(abfd, o);
            if (*ocontents == NULL) {
                return false;
            }
        }
        
        unsigned long instr = bfd_get_32(abfd, *ocontents + irelscan->r_offset);
        bfd_vma immediate = instr & 0x0000ffff;
        size_t offset = calc_fixup(immediate, 0, sec);
        irelscan->r_addend -= offset;
        microblaze_bfd_write_imm_value_32(abfd, *ocontents + irelscan->r_offset,
                                          irelscan->r_addend);
    }
    return true;
}

static bool handle_64_reloc(bfd *abfd, asection *o, asection *sec,
                           Elf_Internal_Rela *irelscan, bfd_byte **ocontents,
                           Elf_Internal_Sym *isymbuf, unsigned int shndx)
{
    Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM(irelscan->r_info);
    
    if (isym->st_shndx == shndx && ELF32_ST_TYPE(isym->st_info) == STT_SECTION) {
        if (*ocontents == NULL) {
            *ocontents = get_other_section_contents(abfd, o);
            if (*ocontents == NULL) {
                return false;
            }
        }
        size_t offset = calc_fixup(irelscan->r_addend, 0, sec);
        irelscan->r_addend -= offset;
    }
    return true;
}

static bool handle_64_pcrel_reloc(bfd *abfd, asection *o, asection *sec,
                                  Elf_Internal_Rela *irelscan, bfd_byte **ocontents,
                                  Elf_Internal_Sym *isymbuf, unsigned int shndx)
{
    Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM(irelscan->r_info);
    
    if (isym->st_shndx == shndx && ELF32_ST_TYPE(isym->st_info) == STT_SECTION) {
        if (*ocontents == NULL) {
            *ocontents = get_other_section_contents(abfd, o);
            if (*ocontents == NULL) {
                return false;
            }
        }
        
        unsigned long instr_hi = bfd_get_32(abfd, *ocontents + irelscan->r_offset);
        unsigned long instr_lo = bfd_get_32(abfd, *ocontents + irelscan->r_offset + INST_WORD_SIZE);
        bfd_vma immediate = ((instr_hi & 0x0000ffff) << 16) | (instr_lo & 0x0000ffff);
        size_t offset = calc_fixup(immediate, 0, sec);
        immediate -= offset;
        irelscan->r_addend -= offset;
        microblaze_bfd_write_imm_value_64(abfd, *ocontents + irelscan->r_offset, immediate);
    }
    return true;
}

static bool process_other_section_relocs(bfd *abfd, asection *o, asection *sec,
                                         Elf_Internal_Sym *isymbuf, unsigned int shndx)
{
    Elf_Internal_Rela *irelocs, *irelscan, *irelscanend;
    bfd_byte *ocontents = NULL;
    
    if (o == sec || (o->flags & SEC_RELOC) == 0 || o->reloc_count == 0) {
        return true;
    }
    
    irelocs = _bfd_elf_link_read_relocs(abfd, o, NULL, NULL, true);
    if (irelocs == NULL) {
        return false;
    }
    
    irelscanend = irelocs + o->reloc_count;
    for (irelscan = irelocs; irelscan < irelscanend; irelscan++) {
        int r_type = ELF32_R_TYPE(irelscan->r_info);
        
        if (r_type == R_MICROBLAZE_32 || r_type == R_MICROBLAZE_32_NONE) {
            if (!handle_r32_reloc(abfd, o, sec, irelscan, &ocontents, isymbuf, shndx)) {
                return false;
            }
        } else if (r_type == R_MICROBLAZE_32_SYM_OP_SYM) {
            if (!handle_r32_sym_op_sym_reloc(abfd, o, sec, irelscan, &ocontents, isymbuf)) {
                return false;
            }
        } else if (r_type == R_MICROBLAZE_32_PCREL_LO ||
                   r_type == R_MICROBLAZE_32_LO ||
                   r_type == R_MICROBLAZE_TEXTREL_32_LO) {
            if (!handle_lo_reloc(abfd, o, sec, irelscan, &ocontents, isymbuf, shndx)) {
                return false;
            }
        } else if (r_type == R_MICROBLAZE_64 || r_type == R_MICROBLAZE_TEXTREL_64) {
            if (!handle_64_reloc(abfd, o, sec, irelscan, &ocontents, isymbuf, shndx)) {
                return false;
            }
        } else if (r_type == R_MICROBLAZE_64_PCREL) {
            if (!handle_64_pcrel_reloc(abfd, o, sec, irelscan, &ocontents, isymbuf, shndx)) {
                return false;
            }
        }
    }
    
    return true;
}

static void adjust_local_symbols(Elf_Internal_Sym *isymbuf, Elf_Internal_Shdr *symtab_hdr,
                                 unsigned int shndx, asection *sec)
{
    Elf_Internal_Sym *isym, *isymend;
    
    isymend = isymbuf + symtab_hdr->sh_info;
    for (isym = isymbuf; isym < isymend; isym++) {
        if (isym->st_shndx == shndx) {
            isym->st_value -= calc_fixup(isym->st_value, 0, sec);
            if (isym->st_size) {
                isym->st_size -= calc_fixup(isym->st_value, isym->st_size, sec);
            }
        }
    }
}

static void adjust_global_symbols(bfd *abfd, Elf_Internal_Shdr *symtab_hdr, asection *sec)
{
    size_t symcount, sym_index;
    struct elf_link_hash_entry *sym_hash;
    
    symcount = (symtab_hdr->sh_size / sizeof(Elf32_External_Sym)) - symtab_hdr->sh_info;
    
    for (sym_index = 0; sym_index < symcount; sym_index++) {
        sym_hash = elf_sym_hashes(abfd)[sym_index];
        if ((sym_hash->root.type == bfd_link_hash_defined ||
             sym_hash->root.type == bfd_link_hash_defweak) &&
            sym_hash->root.u.def.section == sec) {
            sym_hash->root.u.def.value -= calc_fixup(sym_hash->root.u.def.value, 0, sec);
            if (sym_hash->size) {
                sym_hash->size -= calc_fixup(sym_hash->root.u.def.value, sym_hash->size, sec);
            }
        }
    }
}

static void physically_move_code(bfd_byte *contents, asection *sec,
                                 struct _microblaze_elf_section_data *sdata)
{
    bfd_vma dest = sdata->relax[0].addr;
    size_t i;
    
    for (i = 0; i < sdata->relax_count; i++) {
        size_t len;
        bfd_vma src = sdata->relax[i].addr + sdata->relax[i].size;
        len = sdata->relax[i + 1].addr - sdata->relax[i].addr - sdata->relax[i].size;
        
        memmove(contents + dest, contents + src, len);
        sec->size -= sdata->relax[i].size;
        dest += len;
    }
}

static bool
microblaze_elf_relax_section(bfd *abfd,
                             asection *sec,
                             struct bfd_link_info *link_info,
                             bool *again)
{
    Elf_Internal_Shdr *symtab_hdr;
    Elf_Internal_Rela *internal_relocs;
    Elf_Internal_Rela *free_relocs = NULL;
    Elf_Internal_Rela *irel, *irelend;
    bfd_byte *contents = NULL;
    bfd_byte *free_contents = NULL;
    unsigned int shndx;
    Elf_Internal_Sym *isymbuf;
    size_t symcount;
    struct _microblaze_elf_section_data *sdata;
    asection *o;
    
    *again = false;
    
    if (!is_section_relaxable(sec, link_info)) {
        return true;
    }
    
    sdata = microblaze_elf_section_data(sec);
    BFD_ASSERT((sec->size > 0) || (sec->rawsize > 0));
    
    if (sec->size == 0) {
        sec->size = sec->rawsize;
    }
    
    symtab_hdr = &elf_tdata(abfd)->symtab_hdr;
    isymbuf = (Elf_Internal_Sym *)symtab_hdr->contents;
    symcount = symtab_hdr->sh_size / sizeof(Elf32_External_Sym);
    
    if (isymbuf == NULL) {
        isymbuf = bfd_elf_get_elf_syms(abfd, symtab_hdr, symcount, 0, NULL, NULL, NULL);
    }
    BFD_ASSERT(isymbuf != NULL);
    
    internal_relocs = _bfd_elf_link_read_relocs(abfd, sec, NULL, NULL, link_info->keep_memory);
    if (internal_relocs == NULL) {
        goto error_return;
    }
    
    if (!link_info->keep_memory) {
        free_relocs = internal_relocs;
    }
    
    sdata->relax_count = 0;
    sdata->relax = (struct relax_table *)bfd_malloc((sec->reloc_count + 1) * sizeof(*sdata->relax));
    if (sdata->relax == NULL) {
        goto error_return;
    }
    
    irelend = internal_relocs + sec->reloc_count;
    for (irel = internal_relocs; irel < irelend; irel++) {
        if (!is_reloc_type_relaxable(ELF32_R_TYPE(irel->r_info))) {
            continue;
        }
        
        if (!process_relaxable_reloc(abfd, sec, irel, sdata, &contents, 
                                     &free_contents, isymbuf, symtab_hdr)) {
            goto error_return;
        }
    }
    
    if (sdata->relax_count > 0) {
        shndx = _bfd_elf_section_from_bfd_section(abfd, sec);
        sdata->relax[sdata->relax_count].addr = sec->size;
        
        for (irel = internal_relocs; irel < irelend; irel++) {
            update_reloc_in_section(abfd, sec, irel, contents, isymbuf, symtab_hdr, shndx);
        }
        
        for (o = abfd->sections; o != NULL; o = o->next) {
            if (!process_other_section_relocs(abfd, o, sec, isymbuf, shndx)) {
                goto error_return;
            }
        }
        
        adjust_local_symbols(isymbuf, symtab_hdr, shndx, sec);
        adjust_global_symbols(abfd, symtab_hdr, sec);
        physically_move_code(contents, sec, sdata);
        
        elf_section_data(sec)->relocs = internal_relocs;
        free_relocs = NULL;
        elf_section_data(sec)->this_hdr.contents = contents;
        free_contents = NULL;
        symtab_hdr->contents = (bfd_byte *)isymbuf;
    }
    
    free(free_relocs);
    free_relocs = NULL;
    
    if (free_contents != NULL) {
        if (!link_info->keep_memory) {
            free(free_contents);
        } else {
            elf_section_data(sec)->this_hdr.contents = contents;
        }
        free_contents = NULL;
    }
    
    if (sdata->relax_count == 0) {
        *again = false;
        free(sdata->relax);
        sdata->relax = NULL;
    } else {
        *again = true;
    }
    
    return true;
    
error_return:
    free(free_relocs);
    free(free_contents);
    free(sdata->relax);
    sdata->relax = NULL;
    sdata->relax_count = 0;
    return false;
}

/* Return the section that should be marked against GC for a given
   relocation.  */

static asection *
microblaze_elf_gc_mark_hook (asection *sec,
			     struct bfd_link_info * info,
			     Elf_Internal_Rela * rel,
			     struct elf_link_hash_entry * h,
			     Elf_Internal_Sym * sym)
{
  if (h != NULL)
    {
      unsigned int r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type == R_MICROBLAZE_GNU_VTINHERIT || r_type == R_MICROBLAZE_GNU_VTENTRY)
	return NULL;
    }

  return _bfd_elf_gc_mark_hook (sec, info, rel, h, sym);
}

/* PIC support.  */

#define PLT_ENTRY_SIZE 16

#define PLT_ENTRY_WORD_0  0xb0000000	      /* "imm 0".  */
#define PLT_ENTRY_WORD_1  0xe9940000	      /* "lwi r12,r20,0" - relocated to lwi r12,r20,func@GOT.  */
#define PLT_ENTRY_WORD_1_NOPIC	0xe9800000    /* "lwi r12,r0,0" - non-PIC object.  */
#define PLT_ENTRY_WORD_2  0x98186000	      /* "brad r12".  */
#define PLT_ENTRY_WORD_3  0x80000000	      /* "nop".  */

static bool allocate_local_got_arrays(bfd *abfd, Elf_Internal_Shdr *symtab_hdr)
{
    bfd_size_type size = symtab_hdr->sh_info;
    size *= (sizeof(bfd_signed_vma) + sizeof(unsigned char));
    
    bfd_signed_vma *local_got_refcounts = bfd_zalloc(abfd, size);
    if (local_got_refcounts == NULL)
        return false;
    
    elf_local_got_refcounts(abfd) = local_got_refcounts;
    return true;
}

static unsigned char* get_local_got_tls_masks(bfd *abfd, Elf_Internal_Shdr *symtab_hdr)
{
    bfd_signed_vma *local_got_refcounts = elf_local_got_refcounts(abfd);
    return (unsigned char *)(local_got_refcounts + symtab_hdr->sh_info);
}

static bool update_local_sym_info(bfd *abfd,
                                  Elf_Internal_Shdr *symtab_hdr,
                                  unsigned long r_symndx,
                                  unsigned int tls_type)
{
    bfd_signed_vma *local_got_refcounts = elf_local_got_refcounts(abfd);
    
    if (local_got_refcounts == NULL)
    {
        if (!allocate_local_got_arrays(abfd, symtab_hdr))
            return false;
        local_got_refcounts = elf_local_got_refcounts(abfd);
    }
    
    unsigned char *local_got_tls_masks = get_local_got_tls_masks(abfd, symtab_hdr);
    local_got_tls_masks[r_symndx] |= tls_type;
    local_got_refcounts[r_symndx] += 1;
    
    return true;
}
/* Look through the relocs for a section during the first phase.  */

static bool
is_indirect_or_warning_symbol(struct elf_link_hash_entry *h)
{
  return h->root.type == bfd_link_hash_indirect ||
         h->root.type == bfd_link_hash_warning;
}

static struct elf_link_hash_entry *
resolve_indirect_symbol(struct elf_link_hash_entry *h)
{
  while (is_indirect_or_warning_symbol(h))
    h = (struct elf_link_hash_entry *) h->root.u.i.link;
  return h;
}

static struct elf_link_hash_entry *
get_hash_entry(struct elf_link_hash_entry **sym_hashes,
              unsigned long r_symndx,
              Elf_Internal_Shdr *symtab_hdr)
{
  if (r_symndx < symtab_hdr->sh_info)
    return NULL;
  
  struct elf_link_hash_entry *h = sym_hashes[r_symndx - symtab_hdr->sh_info];
  return resolve_indirect_symbol(h);
}

static bool
create_got_section_if_needed(struct elf32_mb_link_hash_table *htab,
                             bfd *abfd,
                             struct bfd_link_info *info)
{
  if (htab->elf.sgot != NULL)
    return true;
    
  if (htab->elf.dynobj == NULL)
    htab->elf.dynobj = abfd;
    
  return _bfd_elf_create_got_section(htab->elf.dynobj, info);
}

static bool
handle_vtable_inheritance(bfd *abfd,
                          asection *sec,
                          struct elf_link_hash_entry *h,
                          const Elf_Internal_Rela *rel)
{
  return bfd_elf_gc_record_vtinherit(abfd, sec, h, rel->r_offset);
}

static bool
handle_vtable_entry(bfd *abfd,
                   asection *sec,
                   struct elf_link_hash_entry *h,
                   const Elf_Internal_Rela *rel)
{
  return bfd_elf_gc_record_vtentry(abfd, sec, h, rel->r_addend);
}

static void
handle_plt_relocation(struct elf_link_hash_entry *h)
{
  if (h != NULL)
  {
    h->needs_plt = 1;
    h->plt.refcount += 1;
  }
}

static bool
handle_got_relocation(struct elf32_mb_link_hash_table *htab,
                      bfd *abfd,
                      struct bfd_link_info *info,
                      asection *sec,
                      struct elf_link_hash_entry *h,
                      unsigned long r_symndx,
                      Elf_Internal_Shdr *symtab_hdr,
                      unsigned char tls_type)
{
  if (tls_type & TLS_TLS)
    sec->has_tls_reloc = 1;
    
  if (!create_got_section_if_needed(htab, abfd, info))
    return false;
    
  if (h != NULL)
  {
    h->got.refcount += 1;
    elf32_mb_hash_entry(h)->tls_mask |= tls_type;
  }
  else
  {
    if (!update_local_sym_info(abfd, symtab_hdr, r_symndx, tls_type))
      return false;
  }
  
  return true;
}

static bool
handle_gotoff_relocation(struct elf32_mb_link_hash_table *htab,
                         bfd *abfd,
                         struct bfd_link_info *info)
{
  return create_got_section_if_needed(htab, abfd, info);
}

static void
update_hash_entry_for_direct_reloc(struct elf_link_hash_entry *h,
                                   struct bfd_link_info *info,
                                   unsigned int r_type)
{
  if (h == NULL || bfd_link_pic(info))
    return;
    
  h->non_got_ref = 1;
  h->plt.refcount += 1;
  
  if (r_type != R_MICROBLAZE_64_PCREL)
    h->pointer_equality_needed = 1;
}

static bool
needs_dynamic_relocation(struct bfd_link_info *info,
                         asection *sec,
                         unsigned int r_type,
                         struct elf_link_hash_entry *h)
{
  if ((sec->flags & SEC_ALLOC) == 0)
    return false;
    
  if (bfd_link_pic(info))
  {
    if (r_type == R_MICROBLAZE_64_PCREL && h == NULL)
      return false;
      
    if (h != NULL && info->symbolic && 
        h->root.type != bfd_link_hash_defweak && h->def_regular)
      return false;
      
    return true;
  }
  
  return h != NULL && 
         (h->root.type == bfd_link_hash_defweak || !h->def_regular);
}

static asection *
create_dynamic_reloc_section(struct elf32_mb_link_hash_table *htab,
                             bfd *abfd,
                             asection *sec)
{
  if (htab->elf.dynobj == NULL)
    htab->elf.dynobj = abfd;
    
  bfd *dynobj = htab->elf.dynobj;
  return _bfd_elf_make_dynamic_reloc_section(sec, dynobj, 2, abfd, 1);
}

static struct elf_dyn_relocs **
get_dynrelocs_head_for_local(struct elf32_mb_link_hash_table *htab,
                             bfd *abfd,
                             unsigned long r_symndx)
{
  Elf_Internal_Sym *isym = bfd_sym_from_r_symndx(&htab->elf.sym_cache,
                                                  abfd, r_symndx);
  if (isym == NULL)
    return NULL;
    
  asection *s = bfd_section_from_elf_index(abfd, isym->st_shndx);
  if (s == NULL)
    return NULL;
    
  void *vpp = &elf_section_data(s)->local_dynrel;
  return (struct elf_dyn_relocs **) vpp;
}

static struct elf_dyn_relocs *
allocate_dynrelocs_entry(struct elf32_mb_link_hash_table *htab,
                         asection *sec)
{
  size_t amt = sizeof(struct elf_dyn_relocs);
  struct elf_dyn_relocs *p = bfd_alloc(htab->elf.dynobj, amt);
  
  if (p != NULL)
  {
    p->sec = sec;
    p->count = 0;
    p->pc_count = 0;
  }
  
  return p;
}

static bool
add_dynamic_relocation(struct elf32_mb_link_hash_table *htab,
                      struct elf_dyn_relocs **head,
                      asection *sec,
                      unsigned int r_type)
{
  struct elf_dyn_relocs *p = *head;
  
  if (p == NULL || p->sec != sec)
  {
    p = allocate_dynrelocs_entry(htab, sec);
    if (p == NULL)
      return false;
      
    p->next = *head;
    *head = p;
  }
  
  p->count += 1;
  if (r_type == R_MICROBLAZE_64_PCREL)
    p->pc_count += 1;
    
  return true;
}

static bool
handle_direct_relocation(struct elf32_mb_link_hash_table *htab,
                         bfd *abfd,
                         struct bfd_link_info *info,
                         asection *sec,
                         struct elf_link_hash_entry *h,
                         unsigned long r_symndx,
                         unsigned int r_type,
                         asection **sreloc)
{
  update_hash_entry_for_direct_reloc(h, info, r_type);
  
  if (!needs_dynamic_relocation(info, sec, r_type, h))
    return true;
    
  if (*sreloc == NULL)
  {
    *sreloc = create_dynamic_reloc_section(htab, abfd, sec);
    if (*sreloc == NULL)
      return false;
  }
  
  struct elf_dyn_relocs **head;
  
  if (h != NULL)
  {
    head = &h->dyn_relocs;
  }
  else
  {
    head = get_dynrelocs_head_for_local(htab, abfd, r_symndx);
    if (head == NULL)
      return false;
  }
  
  return add_dynamic_relocation(htab, head, sec, r_type);
}

static bool
process_relocation(struct elf32_mb_link_hash_table *htab,
                   bfd *abfd,
                   struct bfd_link_info *info,
                   asection *sec,
                   const Elf_Internal_Rela *rel,
                   Elf_Internal_Shdr *symtab_hdr,
                   struct elf_link_hash_entry **sym_hashes,
                   asection **sreloc)
{
  unsigned long r_symndx = ELF32_R_SYM(rel->r_info);
  unsigned int r_type = ELF32_R_TYPE(rel->r_info);
  struct elf_link_hash_entry *h = get_hash_entry(sym_hashes, r_symndx, symtab_hdr);
  unsigned char tls_type = 0;
  
  switch (r_type)
  {
    case R_MICROBLAZE_GNU_VTINHERIT:
      return handle_vtable_inheritance(abfd, sec, h, rel);
      
    case R_MICROBLAZE_GNU_VTENTRY:
      return handle_vtable_entry(abfd, sec, h, rel);
      
    case R_MICROBLAZE_PLT_64:
      handle_plt_relocation(h);
      break;
      
    case R_MICROBLAZE_TLSGD:
      tls_type = TLS_TLS | TLS_GD;
      return handle_got_relocation(htab, abfd, info, sec, h, r_symndx,
                                   symtab_hdr, tls_type);
      
    case R_MICROBLAZE_TLSLD:
      tls_type = TLS_TLS | TLS_LD;
      return handle_got_relocation(htab, abfd, info, sec, h, r_symndx,
                                   symtab_hdr, tls_type);
      
    case R_MICROBLAZE_GOT_64:
      return handle_got_relocation(htab, abfd, info, sec, h, r_symndx,
                                   symtab_hdr, tls_type);
      
    case R_MICROBLAZE_GOTOFF_64:
    case R_MICROBLAZE_GOTOFF_32:
      return handle_gotoff_relocation(htab, abfd, info);
      
    case R_MICROBLAZE_64:
    case R_MICROBLAZE_64_PCREL:
    case R_MICROBLAZE_32:
      return handle_direct_relocation(htab, abfd, info, sec, h, r_symndx,
                                      r_type, sreloc);
  }
  
  return true;
}

static bool
microblaze_elf_check_relocs(bfd *abfd,
                            struct bfd_link_info *info,
                            asection *sec,
                            const Elf_Internal_Rela *relocs)
{
  if (bfd_link_relocatable(info))
    return true;
    
  struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
  if (htab == NULL)
    return false;
    
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata(abfd)->symtab_hdr;
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes(abfd);
  asection *sreloc = NULL;
  
  const Elf_Internal_Rela *rel_end = relocs + sec->reloc_count;
  
  for (const Elf_Internal_Rela *rel = relocs; rel < rel_end; rel++)
  {
    if (!process_relocation(htab, abfd, info, sec, rel, symtab_hdr,
                           sym_hashes, &sreloc))
      return false;
  }
  
  return true;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
microblaze_elf_copy_indirect_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *dir,
				     struct elf_link_hash_entry *ind)
{
  struct elf32_mb_link_hash_entry *edir, *eind;

  edir = (struct elf32_mb_link_hash_entry *) dir;
  eind = (struct elf32_mb_link_hash_entry *) ind;

  edir->tls_mask |= eind->tls_mask;

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

static bool
should_skip_plt_creation(struct bfd_link_info *info,
                        struct elf_link_hash_entry *h)
{
    return h->plt.refcount <= 0
           || SYMBOL_CALLS_LOCAL(info, h)
           || (ELF_ST_VISIBILITY(h->other) != STV_DEFAULT
               && h->root.type == bfd_link_hash_undefweak);
}

static void
reset_plt_entry(struct elf_link_hash_entry *h)
{
    h->plt.offset = (bfd_vma) -1;
    h->needs_plt = 0;
}

static bool
handle_function_symbol(struct bfd_link_info *info,
                       struct elf_link_hash_entry *h)
{
    if (should_skip_plt_creation(info, h)) {
        reset_plt_entry(h);
    }
    return true;
}

static bool
handle_weak_alias(struct elf_link_hash_entry *h)
{
    struct elf_link_hash_entry *def = weakdef(h);
    BFD_ASSERT(def->root.type == bfd_link_hash_defined);
    h->root.u.def.section = def->root.u.def.section;
    h->root.u.def.value = def->root.u.def.value;
    return true;
}

static bool
should_skip_copy_reloc(struct bfd_link_info *info,
                       struct elf_link_hash_entry *h)
{
    if (bfd_link_pic(info))
        return true;
    
    if (!h->non_got_ref)
        return true;
    
    if (info->nocopyreloc) {
        h->non_got_ref = 0;
        return true;
    }
    
    if (!_bfd_elf_readonly_dynrelocs(h)) {
        h->non_got_ref = 0;
        return true;
    }
    
    return false;
}

static void
select_dynbss_sections(struct elf32_mb_link_hash_table *htab,
                       struct elf_link_hash_entry *h,
                       asection **s,
                       asection **srel)
{
    if ((h->root.u.def.section->flags & SEC_READONLY) != 0) {
        *s = htab->elf.sdynrelro;
        *srel = htab->elf.sreldynrelro;
    } else {
        *s = htab->elf.sdynbss;
        *srel = htab->elf.srelbss;
    }
}

static void
update_reloc_section(struct elf_link_hash_entry *h,
                    asection *srel)
{
    if ((h->root.u.def.section->flags & SEC_ALLOC) != 0) {
        srel->size += sizeof(Elf32_External_Rela);
        h->needs_copy = 1;
    }
}

#define MAX_ALIGNMENT_POWER 3

static unsigned int
calculate_alignment_power(bfd_vma size)
{
    unsigned int power_of_two = bfd_log2(size);
    if (power_of_two > MAX_ALIGNMENT_POWER)
        power_of_two = MAX_ALIGNMENT_POWER;
    return power_of_two;
}

static bool
allocate_dynbss_symbol(asection *s,
                       struct elf_link_hash_entry *h)
{
    unsigned int power_of_two = calculate_alignment_power(h->size);
    
    s->size = BFD_ALIGN(s->size, (bfd_size_type)(1 << power_of_two));
    if (!bfd_link_align_section(s, power_of_two))
        return false;
    
    h->root.u.def.section = s;
    h->root.u.def.value = s->size;
    s->size += h->size;
    
    return true;
}

static bool
microblaze_elf_adjust_dynamic_symbol(struct bfd_link_info *info,
                                    struct elf_link_hash_entry *h)
{
    struct elf32_mb_link_hash_table *htab;
    asection *s, *srel;
    
    htab = elf32_mb_hash_table(info);
    if (htab == NULL)
        return false;
    
    if (h->type == STT_FUNC || h->needs_plt) {
        return handle_function_symbol(info, h);
    }
    
    h->plt.offset = (bfd_vma) -1;
    
    if (h->is_weakalias) {
        return handle_weak_alias(h);
    }
    
    if (should_skip_copy_reloc(info, h)) {
        return true;
    }
    
    select_dynbss_sections(htab, h, &s, &srel);
    update_reloc_section(h, srel);
    
    return allocate_dynbss_symbol(s, h);
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

#define PLT_ENTRY_SIZE_VALUE 12
#define GOT_ENTRY_SIZE 4
#define RELA_SIZE sizeof(Elf32_External_Rela)
#define TLS_ENTRY_SIZE 8
#define INVALID_OFFSET ((bfd_vma) -1)

static bool
ensure_dynamic_symbol(struct bfd_link_info *info, struct elf_link_hash_entry *h)
{
  if (h->dynindx == -1 && !h->forced_local)
    {
      if (!bfd_elf_link_record_dynamic_symbol(info, h))
        return false;
    }
  return true;
}

static void
reset_plt_entry(struct elf_link_hash_entry *h)
{
  h->plt.offset = INVALID_OFFSET;
  h->needs_plt = 0;
}

static bool
allocate_plt_entry(struct elf_link_hash_entry *h,
                   struct bfd_link_info *info,
                   struct elf32_mb_link_hash_table *htab)
{
  asection *s = htab->elf.splt;
  
  if (s->size == 0)
    s->size = PLT_ENTRY_SIZE_VALUE;
  
  h->plt.offset = s->size;
  
  if (!bfd_link_pic(info) && !h->def_regular)
    {
      h->root.u.def.section = s;
      h->root.u.def.value = h->plt.offset;
    }
  
  s->size += PLT_ENTRY_SIZE_VALUE;
  htab->elf.sgotplt->size += GOT_ENTRY_SIZE;
  htab->elf.srelplt->size += RELA_SIZE;
  
  return true;
}

static bool
process_plt_allocation(struct elf_link_hash_entry *h,
                       struct bfd_link_info *info,
                       struct elf32_mb_link_hash_table *htab)
{
  if (htab->elf.dynamic_sections_created && h->plt.refcount > 0)
    {
      if (!ensure_dynamic_symbol(info, h))
        return false;
      
      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL(1, bfd_link_pic(info), h))
        return allocate_plt_entry(h, info, htab);
      else
        reset_plt_entry(h);
    }
  else
    {
      reset_plt_entry(h);
    }
  return true;
}

static unsigned int
calculate_tls_need(struct elf32_mb_link_hash_entry *eh,
                   struct elf32_mb_link_hash_table *htab)
{
  unsigned int need = 0;
  
  if ((eh->tls_mask & TLS_LD) != 0)
    {
      if (!eh->elf.def_dynamic)
        htab->tlsld_got.refcount += 1;
      else
        need += TLS_ENTRY_SIZE;
    }
  if ((eh->tls_mask & TLS_GD) != 0)
    need += TLS_ENTRY_SIZE;
  
  return need;
}

static bool
allocate_got_entry(struct elf_link_hash_entry *h,
                   struct bfd_link_info *info,
                   struct elf32_mb_link_hash_table *htab)
{
  struct elf32_mb_link_hash_entry *eh = (struct elf32_mb_link_hash_entry *) h;
  unsigned int need;
  asection *s;
  
  if (!ensure_dynamic_symbol(info, h))
    return false;
  
  if ((eh->tls_mask & TLS_TLS) != 0)
    need = calculate_tls_need(eh, htab);
  else
    need = GOT_ENTRY_SIZE;
  
  if (need == 0)
    {
      h->got.offset = INVALID_OFFSET;
    }
  else
    {
      s = htab->elf.sgot;
      h->got.offset = s->size;
      s->size += need;
      htab->elf.srelgot->size += need * (RELA_SIZE / GOT_ENTRY_SIZE);
    }
  
  return true;
}

static bool
process_got_allocation(struct elf_link_hash_entry *h,
                       struct bfd_link_info *info,
                       struct elf32_mb_link_hash_table *htab)
{
  if (h->got.refcount > 0)
    return allocate_got_entry(h, info, htab);
  else
    h->got.offset = INVALID_OFFSET;
  
  return true;
}

static void
discard_pc_relative_relocs(struct elf_link_hash_entry *h)
{
  struct elf_dyn_relocs **pp;
  struct elf_dyn_relocs *p;
  
  for (pp = &h->dyn_relocs; (p = *pp) != NULL;)
    {
      p->count -= p->pc_count;
      p->pc_count = 0;
      if (p->count == 0)
        *pp = p->next;
      else
        pp = &p->next;
    }
}

static bool
should_discard_shared_relocs(struct elf_link_hash_entry *h,
                             struct bfd_link_info *info)
{
  return h->def_regular && (h->forced_local || info->symbolic);
}

static bool
should_keep_non_shared_relocs(struct elf_link_hash_entry *h,
                              struct elf32_mb_link_hash_table *htab)
{
  return !h->non_got_ref &&
         ((h->def_dynamic && !h->def_regular) ||
          (htab->elf.dynamic_sections_created &&
           (h->root.type == bfd_link_hash_undefweak ||
            h->root.type == bfd_link_hash_undefined)));
}

static bool
process_non_shared_dynrelocs(struct elf_link_hash_entry *h,
                             struct bfd_link_info *info,
                             struct elf32_mb_link_hash_table *htab)
{
  if (should_keep_non_shared_relocs(h, htab))
    {
      if (!ensure_dynamic_symbol(info, h))
        return false;
      
      if (h->dynindx == -1)
        h->dyn_relocs = NULL;
    }
  else
    {
      h->dyn_relocs = NULL;
    }
  
  return true;
}

static bool
process_dynrelocs(struct elf_link_hash_entry *h,
                  struct bfd_link_info *info,
                  struct elf32_mb_link_hash_table *htab)
{
  if (h->dyn_relocs == NULL)
    return true;
  
  if (bfd_link_pic(info))
    {
      if (should_discard_shared_relocs(h, info))
        discard_pc_relative_relocs(h);
      else if (UNDEFWEAK_NO_DYNAMIC_RELOC(info, h))
        h->dyn_relocs = NULL;
    }
  else
    {
      if (!process_non_shared_dynrelocs(h, info, htab))
        return false;
    }
  
  return true;
}

static void
allocate_dynreloc_space(struct elf_link_hash_entry *h)
{
  struct elf_dyn_relocs *p;
  
  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data(p->sec)->sreloc;
      sreloc->size += p->count * RELA_SIZE;
    }
}

static bool
allocate_dynrelocs(struct elf_link_hash_entry *h, void *dat)
{
  struct bfd_link_info *info;
  struct elf32_mb_link_hash_table *htab;
  
  if (h->root.type == bfd_link_hash_indirect)
    return true;
  
  info = (struct bfd_link_info *) dat;
  htab = elf32_mb_hash_table(info);
  if (htab == NULL)
    return false;
  
  if (!process_plt_allocation(h, info, htab))
    return false;
  
  if (!process_got_allocation(h, info, htab))
    return false;
  
  if (!process_dynrelocs(h, info, htab))
    return false;
  
  allocate_dynreloc_space(h);
  
  return true;
}

/* Set the sizes of the dynamic sections.  */

static bool
process_input_bfd_sections(bfd *ibfd, struct bfd_link_info *info)
{
    asection *s;
    
    for (s = ibfd->sections; s != NULL; s = s->next)
    {
        struct elf_dyn_relocs *p;
        
        for (p = ((struct elf_dyn_relocs *)
                  elf_section_data (s)->local_dynrel);
             p != NULL;
             p = p->next)
        {
            if (bfd_is_abs_section (p->sec)
                && !bfd_is_abs_section (p->sec->output_section)
                && p->count != 0)
            {
                asection *srel = elf_section_data (p->sec)->sreloc;
                srel->size += p->count * sizeof (Elf32_External_Rela);
                if ((p->sec->output_section->flags & SEC_READONLY) != 0)
                    info->flags |= DF_TEXTREL;
            }
        }
    }
    return true;
}

static unsigned int
calculate_tls_need(unsigned char lgot_mask)
{
    unsigned int need = 0;
    
    if ((lgot_mask & TLS_TLS) != 0)
    {
        if ((lgot_mask & TLS_GD) != 0)
            need += 8;
    }
    else
    {
        need += 4;
    }
    
    return need;
}

static void
process_local_got_entry(bfd_signed_vma *local_got, unsigned char lgot_mask,
                       asection *s, asection *srel, 
                       struct elf32_mb_link_hash_table *htab,
                       struct bfd_link_info *info)
{
    if (*local_got <= 0)
    {
        *local_got = (bfd_vma) -1;
        return;
    }
    
    unsigned int need = calculate_tls_need(lgot_mask);
    
    if ((lgot_mask & TLS_TLS) != 0 && (lgot_mask & TLS_LD) != 0)
        htab->tlsld_got.refcount += 1;
    
    if (need == 0)
    {
        *local_got = (bfd_vma) -1;
    }
    else
    {
        *local_got = s->size;
        s->size += need;
        if (bfd_link_pic (info))
            srel->size += need * (sizeof (Elf32_External_Rela) / 4);
    }
}

static void
process_local_got_offsets(bfd *ibfd, struct elf32_mb_link_hash_table *htab,
                         struct bfd_link_info *info)
{
    bfd_signed_vma *local_got;
    bfd_signed_vma *end_local_got;
    bfd_size_type locsymcount;
    Elf_Internal_Shdr *symtab_hdr;
    unsigned char *lgot_masks;
    asection *s, *srel;
    
    local_got = elf_local_got_refcounts (ibfd);
    if (!local_got)
        return;
    
    symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
    locsymcount = symtab_hdr->sh_info;
    end_local_got = local_got + locsymcount;
    lgot_masks = (unsigned char *) end_local_got;
    s = htab->elf.sgot;
    srel = htab->elf.srelgot;
    
    for (; local_got < end_local_got; ++local_got, ++lgot_masks)
    {
        process_local_got_entry(local_got, *lgot_masks, s, srel, htab, info);
    }
}

static void
setup_local_syms_and_relocs(struct bfd_link_info *info,
                           struct elf32_mb_link_hash_table *htab)
{
    bfd *ibfd;
    
    for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
        if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
            continue;
        
        process_input_bfd_sections(ibfd, info);
        process_local_got_offsets(ibfd, htab, info);
    }
}

static void
allocate_tlsld_got(struct elf32_mb_link_hash_table *htab,
                  struct bfd_link_info *info)
{
    if (htab->tlsld_got.refcount > 0)
    {
        htab->tlsld_got.offset = htab->elf.sgot->size;
        htab->elf.sgot->size += 8;
        if (bfd_link_pic (info))
            htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
    }
    else
    {
        htab->tlsld_got.offset = (bfd_vma) -1;
    }
}

static void
add_plt_trailing_nop(struct elf32_mb_link_hash_table *htab)
{
    if (elf_hash_table (htab)->dynamic_sections_created)
    {
        if (htab->elf.splt->size > 0)
            htab->elf.splt->size += 4;
    }
}

static bool
is_special_section(asection *s, struct elf32_mb_link_hash_table *htab)
{
    return (s == htab->elf.splt
            || s == htab->elf.sgot
            || s == htab->elf.sgotplt
            || s == htab->elf.sdynbss
            || s == htab->elf.sdynrelro);
}

static bool
should_strip_rela_section(asection *s)
{
    const char *name = bfd_section_name (s);
    return (startswith (name, ".rela") && s->size == 0);
}

static bool
allocate_section_contents(asection *s, bfd *dynobj)
{
    s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
    if (s->contents == NULL && s->size != 0)
        return false;
    s->alloced = 1;
    return true;
}

static bool
allocate_dynamic_sections(bfd *dynobj, struct elf32_mb_link_hash_table *htab)
{
    asection *s;
    
    for (s = dynobj->sections; s != NULL; s = s->next)
    {
        if ((s->flags & SEC_LINKER_CREATED) == 0)
            continue;
        
        const char *name = bfd_section_name (s);
        
        if (startswith (name, ".rela"))
        {
            if (should_strip_rela_section(s))
            {
                s->flags |= SEC_EXCLUDE;
                continue;
            }
            s->reloc_count = 0;
        }
        else if (!is_special_section(s, htab))
        {
            continue;
        }
        
        if (!allocate_section_contents(s, dynobj))
            return false;
    }
    
    return true;
}

static bool
microblaze_elf_late_size_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
                                  struct bfd_link_info *info)
{
    struct elf32_mb_link_hash_table *htab;
    bfd *dynobj;
    
    htab = elf32_mb_hash_table (info);
    if (htab == NULL)
        return false;
    
    dynobj = htab->elf.dynobj;
    if (dynobj == NULL)
        return true;
    
    setup_local_syms_and_relocs(info, htab);
    
    elf_link_hash_traverse (elf_hash_table (info), allocate_dynrelocs, info);
    
    allocate_tlsld_got(htab, info);
    
    add_plt_trailing_nop(htab);
    
    if (!allocate_dynamic_sections(dynobj, htab))
        return false;
    
    info->flags |= DF_BIND_NOW;
    return _bfd_elf_add_dynamic_tags (output_bfd, info, true);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static void
fill_plt_entry(bfd *output_bfd, asection *splt, bfd_vma plt_offset, 
               bfd_vma got_addr, bool is_pic)
{
    bfd_put_32(output_bfd, PLT_ENTRY_WORD_0 + ((got_addr >> 16) & 0xffff),
               splt->contents + plt_offset);
    
    bfd_vma word1 = is_pic ? PLT_ENTRY_WORD_1 : PLT_ENTRY_WORD_1_NOPIC;
    bfd_put_32(output_bfd, word1 + (got_addr & 0xffff),
               splt->contents + plt_offset + 4);
    
    bfd_put_32(output_bfd, (bfd_vma) PLT_ENTRY_WORD_2,
               splt->contents + plt_offset + 8);
    bfd_put_32(output_bfd, (bfd_vma) PLT_ENTRY_WORD_3,
               splt->contents + plt_offset + 12);
}

static void
create_plt_relocation(bfd *output_bfd, asection *sgotplt, asection *srela,
                     bfd_vma got_offset, bfd_vma plt_index, 
                     struct elf_link_hash_entry *h)
{
    Elf_Internal_Rela rela;
    bfd_byte *loc;
    
    rela.r_offset = sgotplt->output_section->vma + sgotplt->output_offset + got_offset;
    rela.r_info = ELF32_R_INFO(h->dynindx, R_MICROBLAZE_JUMP_SLOT);
    rela.r_addend = 0;
    
    loc = srela->contents + plt_index * sizeof(Elf32_External_Rela);
    bfd_elf32_swap_reloca_out(output_bfd, &rela, loc);
}

static bool
process_plt_entry(bfd *output_bfd, struct bfd_link_info *info,
                 struct elf_link_hash_entry *h, Elf_Internal_Sym *sym,
                 struct elf32_mb_link_hash_table *htab)
{
    if (h->plt.offset == (bfd_vma) -1)
        return false;
        
    BFD_ASSERT(h->dynindx != -1);
    
    asection *splt = htab->elf.splt;
    asection *srela = htab->elf.srelplt;
    asection *sgotplt = htab->elf.sgotplt;
    BFD_ASSERT(splt != NULL && srela != NULL && sgotplt != NULL);
    
    bfd_vma plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
    bfd_vma got_offset = (plt_index + 3) * 4;
    bfd_vma got_addr = got_offset;
    
    if (!bfd_link_pic(info))
        got_addr += sgotplt->output_section->vma + sgotplt->output_offset;
    
    fill_plt_entry(output_bfd, splt, h->plt.offset, got_addr, bfd_link_pic(info));
    create_plt_relocation(output_bfd, sgotplt, srela, got_offset, plt_index, h);
    
    if (!h->def_regular) {
        sym->st_shndx = SHN_UNDEF;
        sym->st_value = 0;
    }
    
    return true;
}

static bfd_vma
calculate_got_value(struct elf_link_hash_entry *h)
{
    asection *sec = h->root.u.def.section;
    bfd_vma value = h->root.u.def.value;
    
    if (sec->output_section != NULL)
        value += sec->output_section->vma + sec->output_offset;
    
    return value;
}

static bool
should_emit_relative_reloc(struct bfd_link_info *info, 
                          struct elf_link_hash_entry *h)
{
    return bfd_link_pic(info) && 
           ((info->symbolic && h->def_regular) || h->dynindx == -1);
}

static bool
process_got_entry(bfd *output_bfd, struct bfd_link_info *info,
                 struct elf_link_hash_entry *h, 
                 struct elf32_mb_link_hash_entry *eh,
                 struct elf32_mb_link_hash_table *htab)
{
    if (h->got.offset == (bfd_vma) -1)
        return false;
        
    if ((h->got.offset & 1) || IS_TLS_LD(eh->tls_mask) || IS_TLS_GD(eh->tls_mask))
        return false;
    
    asection *sgot = htab->elf.sgot;
    asection *srela = htab->elf.srelgot;
    BFD_ASSERT(sgot != NULL && srela != NULL);
    
    bfd_vma offset = sgot->output_section->vma + sgot->output_offset + 
                    (h->got.offset &~ (bfd_vma) 1);
    
    if (should_emit_relative_reloc(info, h)) {
        bfd_vma value = calculate_got_value(h);
        microblaze_elf_output_dynamic_relocation(output_bfd, srela, 
                                                srela->reloc_count++, 0,
                                                R_MICROBLAZE_REL, offset, value);
    } else {
        microblaze_elf_output_dynamic_relocation(output_bfd, srela,
                                                srela->reloc_count++, h->dynindx,
                                                R_MICROBLAZE_GLOB_DAT, offset, 0);
    }
    
    bfd_put_32(output_bfd, (bfd_vma) 0, 
              sgot->contents + (h->got.offset &~ (bfd_vma) 1));
    
    return true;
}

static bool
process_copy_reloc(bfd *output_bfd, struct elf_link_hash_entry *h,
                  struct elf32_mb_link_hash_table *htab)
{
    if (!h->needs_copy)
        return false;
    
    BFD_ASSERT(h->dynindx != -1);
    
    Elf_Internal_Rela rela;
    rela.r_offset = h->root.u.def.value + 
                   h->root.u.def.section->output_section->vma +
                   h->root.u.def.section->output_offset;
    rela.r_info = ELF32_R_INFO(h->dynindx, R_MICROBLAZE_COPY);
    rela.r_addend = 0;
    
    asection *s = (h->root.u.def.section == htab->elf.sdynrelro) ?
                 htab->elf.sreldynrelro : htab->elf.srelbss;
    
    bfd_byte *loc = s->contents + s->reloc_count++ * sizeof(Elf32_External_Rela);
    bfd_elf32_swap_reloca_out(output_bfd, &rela, loc);
    
    return true;
}

static void
mark_special_symbols(struct elf_link_hash_entry *h, Elf_Internal_Sym *sym,
                    struct elf32_mb_link_hash_table *htab)
{
    if (h == htab->elf.hdynamic || h == htab->elf.hgot || h == htab->elf.hplt)
        sym->st_shndx = SHN_ABS;
}

static bool
microblaze_elf_finish_dynamic_symbol(bfd *output_bfd,
                                    struct bfd_link_info *info,
                                    struct elf_link_hash_entry *h,
                                    Elf_Internal_Sym *sym)
{
    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    struct elf32_mb_link_hash_entry *eh = elf32_mb_hash_entry(h);
    
    process_plt_entry(output_bfd, info, h, sym, htab);
    process_got_entry(output_bfd, info, h, eh, htab);
    process_copy_reloc(output_bfd, h, htab);
    mark_special_symbols(h, sym, htab);
    
    return true;
}


/* Finish up the dynamic sections.  */

static bool
get_dynamic_section_info(Elf_Internal_Dyn *dyn, struct elf32_mb_link_hash_table *htab,
                         asection **section, bool *is_size)
{
    switch (dyn->d_tag)
    {
    case DT_PLTGOT:
        *section = htab->elf.sgotplt;
        *is_size = false;
        return true;

    case DT_PLTRELSZ:
        *section = htab->elf.srelplt;
        *is_size = true;
        return true;

    case DT_JMPREL:
        *section = htab->elf.srelplt;
        *is_size = false;
        return true;

    default:
        return false;
    }
}

static void
update_dynamic_entry(bfd *output_bfd, Elf_Internal_Dyn *dyn,
                    asection *s, bool is_size, Elf32_External_Dyn *dyncon)
{
    if (s == NULL)
    {
        dyn->d_un.d_val = 0;
    }
    else if (!is_size)
    {
        dyn->d_un.d_ptr = s->output_section->vma + s->output_offset;
    }
    else
    {
        dyn->d_un.d_val = s->size;
    }
    bfd_elf32_swap_dyn_out(output_bfd, dyn, dyncon);
}

static void
process_dynamic_entries(bfd *output_bfd, bfd *dynobj, asection *sdyn,
                       struct elf32_mb_link_hash_table *htab)
{
    Elf32_External_Dyn *dyncon = (Elf32_External_Dyn *) sdyn->contents;
    Elf32_External_Dyn *dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
    
    for (; dyncon < dynconend; dyncon++)
    {
        Elf_Internal_Dyn dyn;
        asection *s;
        bool is_size;

        bfd_elf32_swap_dyn_in(dynobj, dyncon, &dyn);
        
        if (get_dynamic_section_info(&dyn, htab, &s, &is_size))
        {
            update_dynamic_entry(output_bfd, &dyn, s, is_size, dyncon);
        }
    }
}

#define PLT_ENTRY_SIZE 16
#define NOP_INSTRUCTION 0x80000000
#define ENTSIZE_VALUE 4

static void
initialize_plt_section(bfd *output_bfd, asection *splt)
{
    if (splt->size > 0)
    {
        memset(splt->contents, 0, PLT_ENTRY_SIZE);
        bfd_put_32(output_bfd, (bfd_vma) NOP_INSTRUCTION,
                  splt->contents + splt->size - 4);

        if (splt->output_section != bfd_abs_section_ptr)
        {
            elf_section_data(splt->output_section)->this_hdr.sh_entsize = ENTSIZE_VALUE;
        }
    }
}

static void
initialize_got_section(bfd *output_bfd, asection *sgot, asection *sdyn)
{
    if (sgot && sgot->size > 0)
    {
        bfd_vma value = 0;
        if (sdyn != NULL)
        {
            value = sdyn->output_section->vma + sdyn->output_offset;
        }
        bfd_put_32(output_bfd, value, sgot->contents);
        elf_section_data(sgot->output_section)->this_hdr.sh_entsize = ENTSIZE_VALUE;
    }
}

static void
set_section_entsize(asection *section)
{
    if (section && section->size > 0)
    {
        elf_section_data(section->output_section)->this_hdr.sh_entsize = ENTSIZE_VALUE;
    }
}

static bool
microblaze_elf_finish_dynamic_sections(bfd *output_bfd,
                                       struct bfd_link_info *info)
{
    struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table(info);
    if (htab == NULL)
        return false;

    bfd *dynobj = htab->elf.dynobj;
    asection *sdyn = bfd_get_linker_section(dynobj, ".dynamic");

    if (htab->elf.dynamic_sections_created)
    {
        process_dynamic_entries(output_bfd, dynobj, sdyn, htab);
        
        asection *splt = htab->elf.splt;
        BFD_ASSERT(splt != NULL && sdyn != NULL);
        
        initialize_plt_section(output_bfd, splt);
    }

    initialize_got_section(output_bfd, htab->elf.sgotplt, sdyn);
    set_section_entsize(htab->elf.sgot);

    return true;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */

static bool
is_small_common_symbol(Elf_Internal_Sym *sym, struct bfd_link_info *info, bfd *abfd)
{
    return sym->st_shndx == SHN_COMMON
           && !bfd_link_relocatable(info)
           && sym->st_size <= elf_gp_size(abfd);
}

static bool
create_sbss_section(bfd *abfd, asection **secp, bfd_vma size)
{
    *secp = bfd_make_section_old_way(abfd, ".sbss");
    if (*secp == NULL)
        return false;
    
    if (!bfd_set_section_flags(*secp, SEC_IS_COMMON | SEC_SMALL_DATA))
        return false;
    
    return true;
}

static bool
microblaze_elf_add_symbol_hook(bfd *abfd,
                               struct bfd_link_info *info,
                               Elf_Internal_Sym *sym,
                               const char **namep ATTRIBUTE_UNUSED,
                               flagword *flagsp ATTRIBUTE_UNUSED,
                               asection **secp,
                               bfd_vma *valp)
{
    if (!is_small_common_symbol(sym, info, abfd))
        return true;
    
    if (!create_sbss_section(abfd, secp, sym->st_size))
        return false;
    
    *valp = sym->st_size;
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
