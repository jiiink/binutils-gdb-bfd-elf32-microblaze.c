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
  for (size_t i = 0; i < NUM_ELEM (microblaze_elf_howto_raw); ++i)
    {
      const unsigned int type = microblaze_elf_howto_raw[i].type;

      BFD_ASSERT (type < NUM_ELEM (microblaze_elf_howto_table));

      microblaze_elf_howto_table[type] = &microblaze_elf_howto_raw[i];
    }
}

static reloc_howto_type *
microblaze_elf_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
                                  bfd_reloc_code_real_type code)
{
  static const struct
  {
    bfd_reloc_code_real_type bfd_code;
    enum elf_microblaze_reloc_type elf_code;
  } reloc_map[] = {
    {BFD_RELOC_NONE, R_MICROBLAZE_NONE},
    {BFD_RELOC_MICROBLAZE_32_NONE, R_MICROBLAZE_32_NONE},
    {BFD_RELOC_MICROBLAZE_64_NONE, R_MICROBLAZE_64_NONE},
    {BFD_RELOC_32, R_MICROBLAZE_32},
    {BFD_RELOC_RVA, R_MICROBLAZE_32},
    {BFD_RELOC_32_PCREL, R_MICROBLAZE_32_PCREL},
    {BFD_RELOC_64_PCREL, R_MICROBLAZE_64_PCREL},
    {BFD_RELOC_MICROBLAZE_32_LO_PCREL, R_MICROBLAZE_32_PCREL_LO},
    {BFD_RELOC_64, R_MICROBLAZE_64},
    {BFD_RELOC_MICROBLAZE_32_LO, R_MICROBLAZE_32_LO},
    {BFD_RELOC_MICROBLAZE_32_ROSDA, R_MICROBLAZE_SRO32},
    {BFD_RELOC_MICROBLAZE_32_RWSDA, R_MICROBLAZE_SRW32},
    {BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM, R_MICROBLAZE_32_SYM_OP_SYM},
    {BFD_RELOC_VTABLE_INHERIT, R_MICROBLAZE_GNU_VTINHERIT},
    {BFD_RELOC_VTABLE_ENTRY, R_MICROBLAZE_GNU_VTENTRY},
    {BFD_RELOC_MICROBLAZE_64_GOTPC, R_MICROBLAZE_GOTPC_64},
    {BFD_RELOC_MICROBLAZE_64_GOT, R_MICROBLAZE_GOT_64},
    {BFD_RELOC_MICROBLAZE_64_TEXTPCREL, R_MICROBLAZE_TEXTPCREL_64},
    {BFD_RELOC_MICROBLAZE_64_TEXTREL, R_MICROBLAZE_TEXTREL_64},
    {BFD_RELOC_MICROBLAZE_64_PLT, R_MICROBLAZE_PLT_64},
    {BFD_RELOC_MICROBLAZE_64_GOTOFF, R_MICROBLAZE_GOTOFF_64},
    {BFD_RELOC_MICROBLAZE_32_GOTOFF, R_MICROBLAZE_GOTOFF_32},
    {BFD_RELOC_MICROBLAZE_64_TLSGD, R_MICROBLAZE_TLSGD},
    {BFD_RELOC_MICROBLAZE_64_TLSLD, R_MICROBLAZE_TLSLD},
    {BFD_RELOC_MICROBLAZE_32_TLSDTPREL, R_MICROBLAZE_TLSDTPREL32},
    {BFD_RELOC_MICROBLAZE_64_TLSDTPREL, R_MICROBLAZE_TLSDTPREL64},
    {BFD_RELOC_MICROBLAZE_32_TLSDTPMOD, R_MICROBLAZE_TLSDTPMOD32},
    {BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL, R_MICROBLAZE_TLSGOTTPREL32},
    {BFD_RELOC_MICROBLAZE_64_TLSTPREL, R_MICROBLAZE_TLSTPREL32},
    {BFD_RELOC_MICROBLAZE_COPY, R_MICROBLAZE_COPY}
  };

  for (size_t i = 0; i < sizeof (reloc_map) / sizeof (reloc_map[0]); ++i)
    {
      if (reloc_map[i].bfd_code == code)
	{
	  if (!microblaze_elf_howto_table[R_MICROBLAZE_32])
	    {
	      microblaze_elf_howto_init ();
	    }
	  return microblaze_elf_howto_table[(int) reloc_map[i].elf_code];
	}
    }

  return (reloc_howto_type *) NULL;
};

static reloc_howto_type *
microblaze_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				  const char *r_name)
{
  if (r_name == NULL)
    {
      return NULL;
    }

  for (unsigned int i = 0; i < NUM_ELEM (microblaze_elf_howto_raw); ++i)
    {
      reloc_howto_type *howto = &microblaze_elf_howto_raw[i];
      if (howto->name != NULL && strcasecmp (howto->name, r_name) == 0)
	{
	  return howto;
	}
    }

  return NULL;
}

/* Set the howto pointer for a RCE ELF reloc.  */

static bool
microblaze_elf_info_to_howto (bfd *abfd,
                              arelent *cache_ptr,
                              Elf_Internal_Rela *dst)
{
  if (!microblaze_elf_howto_table[R_MICROBLAZE_32])
    {
      microblaze_elf_howto_init ();
    }

  const unsigned int r_type = ELF32_R_TYPE (dst->r_info);

  if (r_type >= R_MICROBLAZE_max)
    {
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
                          abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
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

static bool
microblaze_elf_new_section_hook (bfd *abfd, asection *sec)
{
  struct _microblaze_elf_section_data *sdata =
    bfd_zalloc (abfd, sizeof (*sdata));
  if (sdata == NULL)
    {
      return false;
    }

  sec->used_by_bfd = sdata;
  return _bfd_elf_new_section_hook (abfd, sec);
}

/* Microblaze ELF local labels start with 'L.' or '$L', not '.L'.  */

static bool
microblaze_elf_is_local_label_name (bfd *abfd, const char *name)
{
  if (!name)
    {
      return false;
    }

  return (name[0] == 'L' && name[1] == '.')
         || (name[0] == '$' && name[1] == 'L')
         || _bfd_elf_is_local_label_name (abfd, name);
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
link_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table,
		   const char *string)
{
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf32_mb_link_hash_entry));
      if (entry == NULL)
	{
	  return NULL;
	}
    }

  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry == NULL)
    {
      return NULL;
    }

  struct elf32_mb_link_hash_entry *eh =
    (struct elf32_mb_link_hash_entry *) entry;
  eh->tls_mask = 0;

  return entry;
}

/* Create a mb ELF linker hash table.  */

static struct bfd_link_hash_table *
microblaze_elf_link_hash_table_create (bfd *abfd)
{
  struct elf32_mb_link_hash_table *ret = bfd_zmalloc (sizeof (*ret));
  if (!ret)
    {
      return NULL;
    }

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd, link_hash_newfunc,
				      sizeof (struct elf32_mb_link_hash_entry)))
    {
      free (ret);
      return NULL;
    }

  return &ret->elf.root;
}

/* Set the values of the small data pointers.  */

static void
update_small_data_pointer (struct bfd_link_info *info,
                           const char *anchor_name,
                           bfd_vma *target_pointer)
{
  struct bfd_link_hash_entry *h =
    bfd_link_hash_lookup (info->hash, anchor_name, false, false, true);

  if (h != NULL && h->type == bfd_link_hash_defined)
    {
      asection *sec = h->u.def.section;
      *target_pointer = (h->u.def.value
                         + sec->output_section->vma
                         + sec->output_offset);
    }
}

static void
microblaze_elf_final_sdp (struct bfd_link_info *info)
{
  update_small_data_pointer (info, RO_SDA_ANCHOR_NAME, &ro_small_data_pointer);
  update_small_data_pointer (info, RW_SDA_ANCHOR_NAME, &rw_small_data_pointer);
}

static bfd_vma
dtprel_base (struct bfd_link_info *info)
{
  struct bfd_section * const tls_sec = elf_hash_table (info)->tls_sec;
  return tls_sec ? tls_sec->vma : 0;
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
  if (!output_bfd || !sreloc || !sreloc->contents)
    {
      return;
    }

  Elf_Internal_Rela rel;
  Elf32_External_Rela *rel_ext = (Elf32_External_Rela *) sreloc->contents;

  rel.r_info = ELF32_R_INFO (indx, r_type);
  rel.r_offset = offset;
  rel.r_addend = addend;

  bfd_elf32_swap_reloca_out (output_bfd, &rel, &rel_ext[reloc_index]);
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

#define INST_WORD_SIZE 4

static void
microblaze_elf_howto_init (void);

static bfd_vma
dtprel_base (struct bfd_link_info *info);

static void
microblaze_elf_output_dynamic_relocation (bfd *output_bfd,
					  asection *srel,
					  unsigned long rel_count,
					  unsigned long symindx,
					  int r_type,
					  bfd_vma r_offset,
					  bfd_vma r_addend);

static void
microblaze_elf_final_sdp (struct bfd_link_info *info);

extern bfd_vma ro_small_data_pointer;
extern bfd_vma rw_small_data_pointer;

static void
init_howto_table_if_needed (void)
{
  if (!microblaze_elf_howto_table[R_MICROBLAZE_max - 1])
    microblaze_elf_howto_init ();
}

static void
microblaze_bfd_put_64_split (bfd *abfd, bfd_vma val, bfd_byte *loc,
			     unsigned int imm_offset)
{
  bfd_put_16 (abfd, (val >> 16) & 0xffff, loc + imm_offset);
  bfd_put_16 (abfd, val & 0xffff, loc + imm_offset + INST_WORD_SIZE);
}

static bfd_reloc_status_type
handle_small_data_reloc (struct bfd_link_info *info,
			 bfd *input_bfd,
			 asection *input_section,
			 reloc_howto_type *howto,
			 bfd_byte *contents,
			 bfd_vma offset,
			 bfd_vma relocation,
			 bfd_vma addend,
			 asection *sec,
			 const char *sym_name,
			 bfd_vma *sdp,
			 const char *sdata_sec,
			 const char *sbss_sec,
			 bool *success)
{
  if (!sec)
    return bfd_reloc_ok;

  const char *name = bfd_section_name (sec);

  if (strcmp (name, sdata_sec) == 0 || strcmp (name, sbss_sec) == 0)
    {
      if (*sdp == 0)
	microblaze_elf_final_sdp (info);
      if (*sdp == 0)
	return bfd_reloc_undefined;

      relocation -= *sdp;
      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, offset, relocation, addend);
    }
  else
    {
      _bfd_error_handler (_("%pB: the target (%s) of an %s relocation"
			    " is in the wrong section (%pA)"),
			  input_bfd, sym_name, howto->name, sec);
      *success = false;
      return bfd_reloc_ok;
    }
}

static bool
should_create_dynamic_reloc (struct bfd_link_info *info,
			     struct elf_link_hash_entry *h,
			     const reloc_howto_type *howto,
			     bool resolved_to_zero)
{
  if (bfd_link_pic (info)
      && (h == NULL
	  || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	      && !resolved_to_zero)
	  || h->root.type != bfd_link_hash_undefweak)
      && (!howto->pc_relative
	  || (h != NULL
	      && h->dynindx != -1
	      && (!info->symbolic
		  || !h->def_regular))))
    return true;

  if (!bfd_link_pic (info)
      && h != NULL
      && h->dynindx != -1
      && !h->non_got_ref
      && ((h->def_dynamic && !h->def_regular)
	  || h->root.type == bfd_link_hash_undefweak
	  || h->root.type == bfd_link_hash_undefined))
    return true;

  return false;
}

static void
create_dynamic_reloc (bfd *output_bfd,
		      struct bfd_link_info *info,
		      bfd *input_bfd,
		      asection *input_section,
		      asection *sreloc,
		      struct elf_link_hash_entry *h,
		      const Elf_Internal_Rela *rel,
		      int r_type,
		      bfd_vma addend,
		      bfd_vma relocation)
{
  Elf_Internal_Rela outrel;

  outrel.r_offset = _bfd_elf_section_offset (output_bfd, info, input_section,
					     rel->r_offset);

  if (outrel.r_offset == (bfd_vma) -1 || outrel.r_offset == (bfd_vma) -2)
    return;

  outrel.r_offset += (input_section->output_section->vma
		      + input_section->output_offset);

  if (h != NULL && ((!info->symbolic && h->dynindx != -1) || !h->def_regular))
    {
      BFD_ASSERT (h->dynindx != -1);
      outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
      outrel.r_addend = addend;
    }
  else
    {
      if (r_type == R_MICROBLAZE_32)
	{
	  outrel.r_info = ELF32_R_INFO (0, R_MICROBLAZE_REL);
	  outrel.r_addend = relocation + addend;
	}
      else
	{
	  BFD_FAIL ();
	  _bfd_error_handler (_("%pB: probably compiled without -fPIC?"),
			      input_bfd);
	  bfd_set_error (bfd_error_bad_value);
	  return;
	}
    }

  bfd_elf32_swap_reloca_out (output_bfd, &outrel,
			     sreloc->contents
			     + sreloc->reloc_count++
			     * sizeof (Elf32_External_Rela));
}

static void
perform_static_reloc (bfd *input_bfd,
		      int r_type,
		      bfd_vma relocation,
		      bfd_vma addend,
		      asection *input_section,
		      bfd_byte *contents,
		      bfd_vma offset,
		      unsigned int imm_offset)
{
  relocation += addend;

  if (r_type == R_MICROBLAZE_32)
    {
      bfd_put_32 (input_bfd, relocation, contents + offset);
      return;
    }

  if (r_type == R_MICROBLAZE_64_PCREL)
    relocation -= (input_section->output_section->vma
		   + input_section->output_offset
		   + offset + INST_WORD_SIZE);
  else if (r_type == R_MICROBLAZE_TEXTREL_64
	   || r_type == R_MICROBLAZE_TEXTREL_32_LO)
    relocation -= input_section->output_section->vma;

  if (r_type == R_MICROBLAZE_TEXTREL_32_LO)
    bfd_put_16 (input_bfd, relocation & 0xffff,
		contents + offset + imm_offset);
  else
    microblaze_bfd_put_64_split (input_bfd, relocation, contents + offset,
				 imm_offset);
}

static void
report_reloc_error (bfd_reloc_status_type r,
		    struct bfd_link_info *info,
		    struct elf_link_hash_entry *h,
		    Elf_Internal_Sym *sym,
		    asection *sec,
		    bfd *input_bfd,
		    Elf_Internal_Shdr *symtab_hdr,
		    const char *howto_name,
		    asection *input_section,
		    bfd_vma offset)
{
  const char *name;
  const char *errmsg = NULL;

  if (h != NULL)
    name = h->root.root.string;
  else
    {
      name = (bfd_elf_string_from_elf_section
	      (input_bfd, symtab_hdr->sh_link, sym->st_name));
      if (name == NULL || *name == '\0')
	name = bfd_section_name (sec);
    }

  switch (r)
    {
    case bfd_reloc_overflow:
      (*info->callbacks->reloc_overflow)
	(info, (h ? &h->root : NULL), name, howto_name,
	 (bfd_vma) 0, input_bfd, input_section, offset);
      break;
    case bfd_reloc_undefined:
      (*info->callbacks->undefined_symbol)
	(info, name, input_bfd, input_section, offset, true);
      break;
    case bfd_reloc_outofrange:
      errmsg = _("internal error: out of range error");
      break;
    case bfd_reloc_notsupported:
      errmsg = _("internal error: unsupported relocation error");
      break;
    case bfd_reloc_dangerous:
      errmsg = _("internal error: dangerous error");
      break;
    default:
      errmsg = _("internal error: unknown error");
      break;
    }

  if (errmsg)
    (*info->callbacks->warning) (info, errmsg, name, input_bfd,
				 input_section, offset);
}

static int
microblaze_elf_relocate_section (bfd *output_bfd,
				 struct bfd_link_info *info,
				 bfd *input_bfd,
				 asection *input_section,
				 bfd_byte *contents,
				 Elf_Internal_Rela *relocs,
				 Elf_Internal_Sym *local_syms,
				 asection **local_sections)
{
  struct elf32_mb_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (input_bfd);
  const unsigned int imm_offset = bfd_big_endian (output_bfd) ? 2 : 0;
  bool ret = true;

  init_howto_table_if_needed ();

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  bfd_vma *local_got_offsets = elf_local_got_offsets (input_bfd);
  asection *sreloc = elf_section_data (input_section)->sreloc;

  for (Elf_Internal_Rela *rel = relocs, *relend = relocs + input_section->reloc_count;
       rel < relend;
       rel++)
    {
      const int r_type = ELF32_R_TYPE (rel->r_info);
      const unsigned long r_symndx = ELF32_R_SYM (rel->r_info);
      bfd_vma addend = rel->r_addend;
      const bfd_vma offset = rel->r_offset;
      bfd_reloc_status_type r = bfd_reloc_ok;

      struct elf_link_hash_entry *h = NULL;
      Elf_Internal_Sym *sym = NULL;
      asection *sec = NULL;
      const char *sym_name = NULL;

      if (r_type < 0 || r_type >= (int) R_MICROBLAZE_max)
	{
	  _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			      input_bfd, r_type);
	  bfd_set_error (bfd_error_bad_value);
	  ret = false;
	  continue;
	}

      reloc_howto_type *howto = microblaze_elf_howto_table[r_type];

      if (bfd_link_relocatable (info))
	{
	  if (r_symndx < symtab_hdr->sh_info)
	    {
	      sym = local_syms + r_symndx;
	      if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		{
		  sec = local_sections[r_symndx];
		  addend += sec->output_offset + sym->st_value;
#ifdef USE_REL
		  if (howto->partial_inplace)
		    r = _bfd_relocate_contents (howto, input_bfd, addend,
						contents + offset);
#endif
		}
	    }
	}
      else
	{
	  bfd_vma relocation = 0;
	  bool unresolved_reloc = false;
	  bool resolved_to_zero = false;
	  unsigned int tls_type = 0;

	  if (r_symndx < symtab_hdr->sh_info)
	    {
	      sym = local_syms + r_symndx;
	      sec = local_sections[r_symndx];
	      if (sec == NULL)
		continue;
	      sym_name = "<local symbol>";
	      relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	      addend = rel->r_addend;
	    }
	  else
	    {
	      RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				       r_symndx, symtab_hdr, sym_hashes,
				       h, sec, relocation,
				       unresolved_reloc, ret, ret);
	      sym_name = h->root.root.string;
	    }

	  if (offset > bfd_get_section_limit (input_bfd, input_section))
	    {
	      r = bfd_reloc_outofrange;
	      goto handle_error;
	    }

	  resolved_to_zero = (h != NULL && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));

	  switch ((int) r_type)
	    {
	    case R_MICROBLAZE_SRO32:
	      r = handle_small_data_reloc (info, input_bfd, input_section,
					   howto, contents, offset, relocation,
					   addend, sec, sym_name,
					   &ro_small_data_pointer, ".sdata2",
					   ".sbss2", &ret);
	      break;

	    case R_MICROBLAZE_SRW32:
	      r = handle_small_data_reloc (info, input_bfd, input_section,
					   howto, contents, offset, relocation,
					   addend, sec, sym_name,
					   &rw_small_data_pointer, ".sdata",
					   ".sbss", &ret);
	      break;

	    case R_MICROBLAZE_32_SYM_OP_SYM:
	      break;

	    case R_MICROBLAZE_GOTPC_64:
	      relocation = (htab->elf.sgotplt->output_section->vma
			    + htab->elf.sgotplt->output_offset);
	      relocation -= (input_section->output_section->vma
			     + input_section->output_offset
			     + offset + INST_WORD_SIZE);
	      relocation += addend;
	      microblaze_bfd_put_64_split (input_bfd, relocation,
					   contents + offset, imm_offset);
	      break;

	    case R_MICROBLAZE_TEXTPCREL_64:
	      relocation = input_section->output_section->vma;
	      relocation -= (input_section->output_section->vma
			     + input_section->output_offset
			     + offset + INST_WORD_SIZE);
	      relocation += addend;
	      microblaze_bfd_put_64_split (input_bfd, relocation,
					   contents + offset, imm_offset);
	      break;

	    case R_MICROBLAZE_PLT_64:
	      if (htab->elf.splt != NULL && h != NULL
		  && h->plt.offset != (bfd_vma) -1)
		{
		  relocation = (htab->elf.splt->output_section->vma
				+ htab->elf.splt->output_offset
				+ h->plt.offset);
		}
	      relocation -= (input_section->output_section->vma
			     + input_section->output_offset
			     + offset + INST_WORD_SIZE);
	      microblaze_bfd_put_64_split (input_bfd, relocation,
					   contents + offset, imm_offset);
	      break;

	    case R_MICROBLAZE_TLSGD:
	      tls_type = (TLS_TLS | TLS_GD);
	      goto dogot;
	    case R_MICROBLAZE_TLSLD:
	      tls_type = (TLS_TLS | TLS_LD);
	    dogot:
	    case R_MICROBLAZE_GOT_64:
	      {
		bfd_vma *offp;
		bfd_vma static_value;
		unsigned long indx = 0;

		if (IS_TLS_LD (tls_type))
		  offp = &htab->tlsld_got.offset;
		else if (h != NULL)
		  offp = &h->got.offset;
		else
		  offp = &local_got_offsets[r_symndx];

		bfd_vma off = (*offp) & ~1;
		bfd_vma off2 = IS_TLS_LD(tls_type) || IS_TLS_GD(tls_type) ? off + 4 : off;

		bool dyn = elf_hash_table (info)->dynamic_sections_created;
		if (h != NULL
		    && WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h)
		    && (!bfd_link_pic (info) || !SYMBOL_REFERENCES_LOCAL (info, h)))
		  indx = h->dynindx;

		bool need_relocs = (bfd_link_pic (info) || indx != 0)
				   && (h == NULL
				       || (ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
					   && !resolved_to_zero)
				       || h->root.type != bfd_link_hash_undefweak);

		static_value = relocation + addend;

		if (!((*offp) & 1))
		  {
		    bfd_vma got_offset = (htab->elf.sgot->output_section->vma
					  + htab->elf.sgot->output_offset
					  + off);
		    if (IS_TLS_LD (tls_type))
		      {
			if (!bfd_link_pic (info))
			  bfd_put_32 (output_bfd, 1, htab->elf.sgot->contents + off);
			else
			  microblaze_elf_output_dynamic_relocation
			    (output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++,
			     0, R_MICROBLAZE_TLSDTPMOD32, got_offset, 0);
		      }
		    else if (IS_TLS_GD (tls_type))
		      {
			if (!need_relocs)
			  bfd_put_32 (output_bfd, 1, htab->elf.sgot->contents + off);
			else
			  microblaze_elf_output_dynamic_relocation
			    (output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++,
			     indx, R_MICROBLAZE_TLSDTPMOD32,
			     got_offset, indx ? 0 : static_value);
		      }

		    got_offset = (htab->elf.sgot->output_section->vma
				  + htab->elf.sgot->output_offset
				  + off2);
		    if (IS_TLS_LD (tls_type))
		      {
			*offp |= 1;
			bfd_put_32 (output_bfd, 0, htab->elf.sgot->contents + off2);
		      }
		    else if (IS_TLS_GD (tls_type))
		      {
			*offp |= 1;
			static_value -= dtprel_base (info);
			if (need_relocs)
			  microblaze_elf_output_dynamic_relocation
			    (output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++,
			     indx, R_MICROBLAZE_TLSDTPREL32,
			     got_offset, indx ? 0 : static_value);
			else
			  bfd_put_32 (output_bfd, static_value, htab->elf.sgot->contents + off2);
		      }
		    else
		      {
			bfd_put_32 (output_bfd, static_value, htab->elf.sgot->contents + off2);
			if (bfd_link_pic (info) && h == NULL)
			  {
			    *offp |= 1;
			    microblaze_elf_output_dynamic_relocation
			      (output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++,
			       indx, R_MICROBLAZE_REL, got_offset, static_value);
			  }
		      }
		  }
		relocation = htab->elf.sgot->output_section->vma
			     + htab->elf.sgot->output_offset
			     + off
			     - htab->elf.sgotplt->output_section->vma
			     - htab->elf.sgotplt->output_offset;

		microblaze_bfd_put_64_split (input_bfd, relocation,
					   contents + offset, imm_offset);
	      }
	      break;

	    case R_MICROBLAZE_GOTOFF_64:
	      relocation += addend;
	      relocation -= (htab->elf.sgotplt->output_section->vma
			     + htab->elf.sgotplt->output_offset);
	      bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
			  contents + offset + imm_offset);
	      bfd_put_16 (input_bfd, relocation & 0xffff,
			  contents + offset + INST_WORD_SIZE + imm_offset);
	      break;

	    case R_MICROBLAZE_GOTOFF_32:
	      relocation += addend;
	      relocation -= (htab->elf.sgotplt->output_section->vma
			     + htab->elf.sgotplt->output_offset);
	      bfd_put_32 (input_bfd, relocation, contents + offset);
	      break;

	    case R_MICROBLAZE_TLSDTPREL64:
	      relocation += addend;
	      relocation -= dtprel_base (info);
	      microblaze_bfd_put_64_split (input_bfd, relocation,
					   contents + offset, imm_offset);
	      break;

	    case R_MICROBLAZE_TEXTREL_64:
	    case R_MICROBLAZE_TEXTREL_32_LO:
	    case R_MICROBLAZE_64_PCREL:
	    case R_MICROBLAZE_64:
	    case R_MICROBLAZE_32:
	      if (r_symndx == STN_UNDEF
		  || (input_section->flags & SEC_ALLOC) == 0)
		{
		  perform_static_reloc (input_bfd, r_type, relocation, addend,
					input_section, contents, offset,
					imm_offset);
		}
	      else if (should_create_dynamic_reloc (info, h, howto,
						    resolved_to_zero))
		{
		  create_dynamic_reloc (output_bfd, info, input_bfd,
					input_section, sreloc, h, rel,
					r_type, addend, relocation);
		}
	      else
		{
		  perform_static_reloc (input_bfd, r_type, relocation, addend,
					input_section, contents, offset,
					imm_offset);
		}
	      break;

	    default:
	      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					    contents, offset,
					    relocation, addend);
	      break;
	    }
	}
    handle_error:
      if (r != bfd_reloc_ok)
	{
	  report_reloc_error (r, info, h, sym, sec, input_bfd,
			      symtab_hdr, howto->name, input_section,
			      offset);
	  ret = false;
	}
    }
  return ret;
}

/* Calculate fixup value for reference.  */

static size_t
calc_fixup (bfd_vma start, bfd_vma size, asection *sec)
{
  struct _microblaze_elf_section_data *sdata;
  if (sec == NULL || (sdata = microblaze_elf_section_data (sec)) == NULL)
    return 0;

  size_t fixup = 0;
  const bfd_vma range_end = (size == 0) ? start : start + size;

  for (size_t i = 0; i < sdata->relax_count; i++)
    {
      const bfd_vma current_addr = sdata->relax[i].addr;
      if (current_addr >= range_end)
        break;

      if (size == 0 || current_addr >= start)
        fixup += sdata->relax[i].size;
    }
  return fixup;
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   a 32-bit instruction.  */
static void
microblaze_bfd_write_imm_value_32 (bfd *abfd, bfd_byte *bfd_addr, bfd_vma val)
{
    const bfd_vma IMM_MASK = 0xffff;
    const bfd_vma original_instruction = bfd_get_32 (abfd, bfd_addr);
    const bfd_vma value_to_insert = val & IMM_MASK;
    const bfd_vma modified_instruction = (original_instruction & ~IMM_MASK) | value_to_insert;

    bfd_put_32 (abfd, modified_instruction, bfd_addr);
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   two consecutive 32-bit instructions.  */
static void
microblaze_bfd_write_imm_value_64(bfd *abfd, bfd_byte *bfd_addr, bfd_vma val)
{
    const unsigned long IMM_MASK = 0xFFFF;
    const int IMM_SHIFT = 16;
    bfd_byte *second_instr_addr = bfd_addr + INST_WORD_SIZE;

    unsigned long first_instr = bfd_get_32(abfd, bfd_addr);
    first_instr = (first_instr & ~IMM_MASK) | ((val >> IMM_SHIFT) & IMM_MASK);
    bfd_put_32(abfd, first_instr, bfd_addr);

    unsigned long second_instr = bfd_get_32(abfd, second_instr_addr);
    second_instr = (second_instr & ~IMM_MASK) | (val & IMM_MASK);
    bfd_put_32(abfd, second_instr, second_instr_addr);
}

static Elf_Internal_Sym *
get_elf_symbols (bfd *abfd, Elf_Internal_Shdr *symtab_hdr)
{
  size_t symcount = symtab_hdr->sh_size / sizeof (Elf32_External_Sym);
  Elf_Internal_Sym *isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;

  if (isymbuf == NULL)
    isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr, symcount, 0, NULL, NULL, NULL);

  BFD_ASSERT (isymbuf != NULL);
  return isymbuf;
}

static bool
load_section_contents (bfd *abfd, asection *sec, bfd_byte **contents_p, bfd_byte **free_contents_p)
{
  *contents_p = elf_section_data (sec)->this_hdr.contents;
  if (*contents_p)
    return true;

  *contents_p = bfd_malloc (sec->size);
  if (!*contents_p)
    return false;

  if (!bfd_get_section_contents (abfd, sec, *contents_p, 0, sec->size))
    {
      free (*contents_p);
      *contents_p = NULL;
      return false;
    }

  *free_contents_p = *contents_p;
  elf_section_data (sec)->this_hdr.contents = *contents_p;
  return true;
}

static bool
get_reloc_symbol_value (bfd *abfd, const Elf_Internal_Rela *irel, const Elf_Internal_Sym *isymbuf,
			const Elf_Internal_Shdr *symtab_hdr, bfd_vma *symval_p)
{
  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
    {
      const Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irel->r_info);
      asection *sym_sec;
      switch (isym->st_shndx)
	{
	case SHN_UNDEF: sym_sec = bfd_und_section_ptr; break;
	case SHN_ABS:   sym_sec = bfd_abs_section_ptr; break;
	case SHN_COMMON:sym_sec = bfd_com_section_ptr; break;
	default:	sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx); break;
	}
      *symval_p = _bfd_elf_rela_local_sym (abfd, isym, &sym_sec, irel);
    }
  else
    {
      unsigned long indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
      struct elf_link_hash_entry *h = elf_sym_hashes (abfd)[indx];
      BFD_ASSERT (h != NULL);

      if (h->root.type != bfd_link_hash_defined && h->root.type != bfd_link_hash_defweak)
	return false;

      *symval_p = (h->root.u.def.value
		   + h->root.u.def.section->output_section->vma
		   + h->root.u.def.section->output_offset);
    }
  return true;
}

static bfd_vma
adjust_symval_for_reloc (bfd_vma symval, const Elf_Internal_Rela *irel, const asection *sec)
{
  switch ((enum elf_microblaze_reloc_type) ELF32_R_TYPE (irel->r_info))
    {
    case R_MICROBLAZE_64_PCREL:
      return symval + irel->r_addend - (irel->r_offset
					+ sec->output_section->vma
					+ sec->output_offset);
    case R_MICROBLAZE_TEXTREL_64:
      return symval + irel->r_addend - sec->output_section->vma;
    case R_MICROBLAZE_64:
      return symval + irel->r_addend;
    default:
      return symval;
    }
}

static inline bool
can_be_shortened (bfd_vma val)
{
  return (val & 0xffff8000) == 0 || (val & 0xffff8000) == 0xffff8000;
}

static void
rewrite_reloc_for_shortening (Elf_Internal_Rela *irel)
{
  unsigned long sym_idx = ELF32_R_SYM (irel->r_info);
  enum elf_microblaze_reloc_type new_type;

  switch ((enum elf_microblaze_reloc_type) ELF32_R_TYPE (irel->r_info))
    {
    case R_MICROBLAZE_64_PCREL:     new_type = R_MICROBLAZE_32_PCREL_LO; break;
    case R_MICROBLAZE_64:           new_type = R_MICROBLAZE_32_LO; break;
    case R_MICROBLAZE_TEXTREL_64:   new_type = R_MICROBLAZE_TEXTREL_32_LO; break;
    default: BFD_ASSERT (false); return;
    }
  irel->r_info = ELF32_R_INFO (sym_idx, (int) new_type);
}

static void
adjust_local_symbols (asection *sec, Elf_Internal_Sym *isymbuf,
		      const Elf_Internal_Shdr *symtab_hdr, unsigned int shndx)
{
  Elf_Internal_Sym *isym, *isymend;
  isymend = isymbuf + symtab_hdr->sh_info;
  for (isym = isymbuf; isym < isymend; isym++)
    {
      if (isym->st_shndx == shndx)
	{
	  isym->st_value -= calc_fixup (isym->st_value, 0, sec);
	  if (isym->st_size)
	    isym->st_size -= calc_fixup (isym->st_value, isym->st_size, sec);
	}
    }
}

static void
adjust_global_symbols (bfd *abfd, asection *sec, const Elf_Internal_Shdr *symtab_hdr)
{
  size_t symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)) - symtab_hdr->sh_info;
  for (size_t sym_index = 0; sym_index < symcount; sym_index++)
    {
      struct elf_link_hash_entry *sym_hash = elf_sym_hashes (abfd)[sym_index];
      if ((sym_hash->root.type == bfd_link_hash_defined
	   || sym_hash->root.type == bfd_link_hash_defweak)
	  && sym_hash->root.u.def.section == sec)
	{
	  sym_hash->root.u.def.value -= calc_fixup (sym_hash->root.u.def.value, 0, sec);
	  if (sym_hash->size)
	    sym_hash->size -= calc_fixup (sym_hash->root.u.def.value, sym_hash->size, sec);
	}
    }
}

static void
compact_section_code (asection *sec, bfd_byte *contents)
{
  struct _microblaze_elf_section_data *sdata = microblaze_elf_section_data (sec);
  bfd_vma dest = sdata->relax[0].addr;
  for (size_t i = 0; i < sdata->relax_count; i++)
    {
      bfd_vma src = sdata->relax[i].addr + sdata->relax[i].size;
      size_t len = sdata->relax[i+1].addr - src;
      memmove (contents + dest, contents + src, len);
      sec->size -= sdata->relax[i].size;
      dest += len;
    }
}

static bool
microblaze_elf_relax_section (bfd *abfd,
			      asection *sec,
			      struct bfd_link_info *link_info,
			      bool *again)
{
  struct _microblaze_elf_section_data *sdata;
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs = NULL;
  Elf_Internal_Rela *free_relocs = NULL;
  bfd_byte *contents = NULL;
  bfd_byte *free_contents = NULL;
  Elf_Internal_Sym *isymbuf;
  bool success = false;

  *again = false;

  if (bfd_link_relocatable (link_info)
      || (sec->flags & (SEC_RELOC | SEC_CODE)) != (SEC_RELOC | SEC_CODE)
      || sec->reloc_count == 0
      || (sdata = microblaze_elf_section_data (sec)) == NULL)
    return true;

  BFD_ASSERT ((sec->size > 0) || (sec->rawsize > 0));
  if (sec->size == 0)
    sec->size = sec->rawsize;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  isymbuf = get_elf_symbols (abfd, symtab_hdr);
  if (!isymbuf)
    goto cleanup;

  internal_relocs = _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL, link_info->keep_memory);
  if (!internal_relocs)
    goto cleanup;
  if (!link_info->keep_memory)
    free_relocs = internal_relocs;

  sdata->relax = bfd_malloc ((sec->reloc_count + 1) * sizeof (*sdata->relax));
  if (!sdata->relax)
    goto cleanup;
  sdata->relax_count = 0;

  Elf_Internal_Rela *irel, *irelend = internal_relocs + sec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      unsigned int r_type = ELF32_R_TYPE (irel->r_info);
      if (r_type != R_MICROBLAZE_64_PCREL
	  && r_type != R_MICROBLAZE_64
	  && r_type != R_MICROBLAZE_TEXTREL_64)
	continue;

      if (!contents && !load_section_contents (abfd, sec, &contents, &free_contents))
	goto cleanup;

      bfd_vma symval;
      if (!get_reloc_symbol_value (abfd, irel, isymbuf, symtab_hdr, &symval))
	continue;

      symval = adjust_symval_for_reloc (symval, irel, sec);

      if (can_be_shortened (symval))
	{
	  sdata->relax[sdata->relax_count].addr = irel->r_offset;
	  sdata->relax[sdata->relax_count].size = INST_WORD_SIZE;
	  sdata->relax_count++;
	  rewrite_reloc_for_shortening (irel);
	}
    }

  if (sdata->relax_count > 0)
    {
      unsigned int shndx = _bfd_elf_section_from_bfd_section (abfd, sec);
      sdata->relax[sdata->relax_count].addr = sec->size;

      for (irel = internal_relocs; irel < irelend; irel++)
	{
	  irel->r_offset -= calc_fixup (irel->r_offset, 0, sec);
	  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
	    {
	      Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irel->r_info);
	      if (isym->st_shndx == shndx && ELF32_ST_TYPE (isym->st_info) == STT_SECTION)
		irel->r_addend -= calc_fixup (irel->r_addend, 0, sec);
	    }
	}

      for (asection *o = abfd->sections; o != NULL; o = o->next)
	{
	  if (o == sec || (o->flags & SEC_RELOC) == 0 || o->reloc_count == 0)
	    continue;

	  Elf_Internal_Rela *irelocs = _bfd_elf_link_read_relocs (abfd, o, NULL, NULL, true);
	  if (!irelocs)
	    goto cleanup;

	  Elf_Internal_Rela *irelscan, *irelscanend = irelocs + o->reloc_count;
	  for (irelscan = irelocs; irelscan < irelscanend; irelscan++)
	    {
	      Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irelscan->r_info);
	      if (isym->st_shndx == shndx && ELF32_ST_TYPE (isym->st_info) == STT_SECTION)
		irelscan->r_addend -= calc_fixup (irelscan->r_addend, 0, sec);
	    }
	}

      adjust_local_symbols (sec, isymbuf, symtab_hdr, shndx);
      adjust_global_symbols (abfd, sec, symtab_hdr);
      compact_section_code (sec, contents);

      elf_section_data (sec)->relocs = internal_relocs;
      free_relocs = NULL;
      elf_section_data (sec)->this_hdr.contents = contents;
      free_contents = NULL;
      symtab_hdr->contents = (bfd_byte *) isymbuf;
      *again = true;
    }

  success = true;

cleanup:
  free (free_relocs);
  if (free_contents && !link_info->keep_memory)
    free (free_contents);

  if (!*again)
    {
      free (sdata->relax);
      sdata->relax = NULL;
    }
  if (!success && sdata)
    {
      free (sdata->relax);
      sdata->relax = NULL;
      sdata->relax_count = 0;
    }

  return success;
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
      const unsigned long r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type == R_MICROBLAZE_GNU_VTINHERIT
	  || r_type == R_MICROBLAZE_GNU_VTENTRY)
	{
	  return NULL;
	}
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

static bool
update_local_sym_info (bfd *abfd,
		       Elf_Internal_Shdr *symtab_hdr,
		       unsigned long r_symndx,
		       unsigned int tls_type)
{
  const bfd_size_type num_symbols = symtab_hdr->sh_info;
  bfd_signed_vma *local_got_refcounts;

  if (r_symndx >= num_symbols)
    return false;

  local_got_refcounts = elf_local_got_refcounts (abfd);

  if (local_got_refcounts == NULL)
    {
      bfd_size_type total_size;
      const size_t combined_size = sizeof (bfd_signed_vma) + sizeof (unsigned char);

      if (num_symbols > 0
	  && ((bfd_size_type) -1) / num_symbols < combined_size)
	return false;

      total_size = num_symbols * combined_size;
      local_got_refcounts = bfd_zalloc (abfd, total_size);
      if (local_got_refcounts == NULL)
	return false;

      elf_local_got_refcounts (abfd) = local_got_refcounts;
    }

  unsigned char *local_got_tls_masks =
    (unsigned char *) (local_got_refcounts + num_symbols);

  local_got_tls_masks[r_symndx] |= (unsigned char) tls_type;
  local_got_refcounts[r_symndx] += 1;

  return true;
}
/* Look through the relocs for a section during the first phase.  */

static struct elf_link_hash_entry *
get_link_hash_entry (struct elf_link_hash_entry **sym_hashes,
                     const Elf_Internal_Shdr *symtab_hdr,
                     unsigned long r_symndx)
{
  if (r_symndx < symtab_hdr->sh_info)
    return NULL;

  struct elf_link_hash_entry *h;
  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
  while (h->root.type == bfd_link_hash_indirect
         || h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  return h;
}

static bool
ensure_got_section (struct elf32_mb_link_hash_table *htab, bfd *abfd,
                    struct bfd_link_info *info)
{
  if (htab->elf.sgot == NULL)
    {
      if (htab->elf.dynobj == NULL)
        htab->elf.dynobj = abfd;
      if (!_bfd_elf_create_got_section (htab->elf.dynobj, info))
        return false;
    }
  return true;
}

static bool
handle_got_reloc (bfd *abfd, struct bfd_link_info *info, asection *sec,
                  const Elf_Internal_Rela *rel, struct elf_link_hash_entry *h,
                  const Elf_Internal_Shdr *symtab_hdr,
                  unsigned long r_symndx)
{
  struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table (info);
  unsigned char tls_type = 0;
  unsigned int r_type = ELF32_R_TYPE (rel->r_info);

  if (r_type == R_MICROBLAZE_TLSGD)
    tls_type |= (TLS_TLS | TLS_GD);
  else if (r_type == R_MICROBLAZE_TLSLD)
    tls_type |= (TLS_TLS | TLS_LD);

  if (tls_type != 0)
    sec->has_tls_reloc = 1;

  if (!ensure_got_section (htab, abfd, info))
    return false;

  if (h != NULL)
    {
      h->got.refcount += 1;
      elf32_mb_hash_entry (h)->tls_mask |= tls_type;
    }
  else
    {
      if (!update_local_sym_info (abfd, symtab_hdr, r_symndx, tls_type))
        return false;
    }
  return true;
}

static bool
record_dynamic_reloc (bfd *abfd, struct bfd_link_info *info, asection *sec,
                      struct elf_link_hash_entry *h,
                      unsigned long r_symndx, unsigned int r_type,
                      asection **sreloc_ptr)
{
  struct elf_dyn_relocs **head;
  struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table (info);

  if (*sreloc_ptr == NULL)
    {
      bfd *dynobj = htab->elf.dynobj;
      if (dynobj == NULL)
        {
          dynobj = abfd;
          htab->elf.dynobj = dynobj;
        }
      *sreloc_ptr =
        _bfd_elf_make_dynamic_reloc_section (sec, dynobj, 2, abfd, 1);
      if (*sreloc_ptr == NULL)
        return false;
    }

  if (h != NULL)
    head = &h->dyn_relocs;
  else
    {
      Elf_Internal_Sym *isym =
        bfd_sym_from_r_symndx (&htab->elf.sym_cache, abfd, r_symndx);
      if (isym == NULL)
        return false;

      asection *s = bfd_section_from_elf_index (abfd, isym->st_shndx);
      if (s == NULL)
        return false;

      void *vpp = &elf_section_data (s)->local_dynrel;
      head = (struct elf_dyn_relocs **) vpp;
    }

  struct elf_dyn_relocs *p = *head;
  if (p == NULL || p->sec != sec)
    {
      p = bfd_alloc (htab->elf.dynobj, sizeof (*p));
      if (p == NULL)
        return false;
      p->next = *head;
      *head = p;
      p->sec = sec;
      p->count = 0;
      p->pc_count = 0;
    }

  p->count += 1;
  if (r_type == R_MICROBLAZE_64_PCREL)
    p->pc_count += 1;

  return true;
}

static bool
handle_dynamic_reloc (bfd *abfd, struct bfd_link_info *info, asection *sec,
                      const Elf_Internal_Rela *rel,
                      struct elf_link_hash_entry *h,
                      unsigned long r_symndx, asection **sreloc_ptr)
{
  unsigned int r_type = ELF32_R_TYPE (rel->r_info);
  bool is_pic = bfd_link_pic (info);

  if (h != NULL && !is_pic)
    {
      h->non_got_ref = 1;
      h->plt.refcount += 1;
      if (r_type != R_MICROBLAZE_64_PCREL)
        h->pointer_equality_needed = 1;
    }

  if ((sec->flags & SEC_ALLOC) == 0)
    return true;

  bool needs_dyn_reloc = false;
  if (is_pic)
    {
      bool sym_needs_copy = (h != NULL
                             && (!info->symbolic
                                 || h->root.type == bfd_link_hash_defweak
                                 || !h->def_regular));
      if (r_type != R_MICROBLAZE_64_PCREL || sym_needs_copy)
        needs_dyn_reloc = true;
    }
  else
    {
      if (h != NULL
          && (h->root.type == bfd_link_hash_defweak || !h->def_regular))
        needs_dyn_reloc = true;
    }

  if (needs_dyn_reloc)
    return record_dynamic_reloc (abfd, info, sec, h, r_symndx, r_type,
                                 sreloc_ptr);

  return true;
}

static bool
microblaze_elf_check_relocs (bfd * abfd,
			     struct bfd_link_info * info,
			     asection * sec,
			     const Elf_Internal_Rela * relocs)
{
  if (bfd_link_relocatable (info))
    return true;

  struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  struct elf_link_hash_entry **sym_hashes = elf_sym_hashes (abfd);
  asection *sreloc = NULL;
  const Elf_Internal_Rela *rel_end = relocs + sec->reloc_count;

  for (const Elf_Internal_Rela *rel = relocs; rel < rel_end; rel++)
    {
      unsigned long r_symndx = ELF32_R_SYM (rel->r_info);
      unsigned int r_type = ELF32_R_TYPE (rel->r_info);
      struct elf_link_hash_entry *h =
        get_link_hash_entry (sym_hashes, symtab_hdr, r_symndx);
      bool ok = true;

      switch (r_type)
        {
        case R_MICROBLAZE_GNU_VTINHERIT:
          ok = bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset);
          break;

        case R_MICROBLAZE_GNU_VTENTRY:
          ok = bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend);
          break;

        case R_MICROBLAZE_PLT_64:
          if (h != NULL)
            {
              h->needs_plt = 1;
              h->plt.refcount += 1;
            }
          break;

        case R_MICROBLAZE_TLSGD:
        case R_MICROBLAZE_TLSLD:
        case R_MICROBLAZE_GOT_64:
          ok = handle_got_reloc (abfd, info, sec, rel, h, symtab_hdr,
                                 r_symndx);
          break;

        case R_MICROBLAZE_GOTOFF_64:
        case R_MICROBLAZE_GOTOFF_32:
          ok = ensure_got_section (htab, abfd, info);
          break;

        case R_MICROBLAZE_64:
        case R_MICROBLAZE_64_PCREL:
        case R_MICROBLAZE_32:
          ok = handle_dynamic_reloc (abfd, info, sec, rel, h, r_symndx,
                                     &sreloc);
          break;
        }

      if (!ok)
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
  if (!dir || !ind)
    {
      return;
    }

  struct elf32_mb_link_hash_entry *edir = (struct elf32_mb_link_hash_entry *) dir;
  struct elf32_mb_link_hash_entry *eind = (struct elf32_mb_link_hash_entry *) ind;

  edir->tls_mask |= eind->tls_mask;

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

static bool
microblaze_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				      struct elf_link_hash_entry *h)
{
  struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  if (h->type == STT_FUNC || h->needs_plt)
    {
      bool can_avoid_plt = (h->plt.refcount <= 0
			    || SYMBOL_CALLS_LOCAL (info, h)
			    || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
				&& h->root.type == bfd_link_hash_undefweak));
      if (can_avoid_plt)
	{
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}
      return true;
    }

  h->plt.offset = (bfd_vma) -1;

  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      h->root.u.def.section = def->root.u.def.section;
      h->root.u.def.value = def->root.u.def.value;
      return true;
    }

  if (bfd_link_pic (info) || !h->non_got_ref)
    return true;

  if (info->nocopyreloc || !_bfd_elf_readonly_dynrelocs (h))
    {
      h->non_got_ref = 0;
      return true;
    }

  bool is_readonly = (h->root.u.def.section->flags & SEC_READONLY) != 0;
  asection *s = is_readonly ? htab->elf.sdynrelro : htab->elf.sdynbss;
  asection *srel = is_readonly ? htab->elf.sreldynrelro : htab->elf.srelbss;

  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  unsigned int power_of_two = bfd_log2 (h->size);
  if (power_of_two > 3)
    power_of_two = 3;

  s->size = BFD_ALIGN (s->size, (bfd_size_type) (1 << power_of_two));
  if (!bfd_link_align_section (s, power_of_two))
    return false;

  h->root.u.def.section = s;
  h->root.u.def.value = s->size;

  s->size += h->size;

  return true;
}

/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bool
ensure_dynamic_symbol (struct elf_link_hash_entry *h,
		       struct bfd_link_info *info)
{
  if (h->dynindx == -1 && !h->forced_local)
    return bfd_elf_link_record_dynamic_symbol (info, h);
  return true;
}

static bool
handle_plt_allocation (struct elf_link_hash_entry *h,
		       struct bfd_link_info *info,
		       struct elf32_mb_link_hash_table *htab)
{
  if (!htab->elf.dynamic_sections_created || h->plt.refcount <= 0)
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
      return true;
    }

  if (!ensure_dynamic_symbol (h, info))
    return false;

  if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, bfd_link_pic (info), h))
    {
      asection *s = htab->elf.splt;

      if (s->size == 0)
	s->size = PLT_ENTRY_SIZE;

      h->plt.offset = s->size;

      if (!bfd_link_pic (info) && !h->def_regular)
	{
	  h->root.u.def.section = s;
	  h->root.u.def.value = h->plt.offset;
	}

      s->size += PLT_ENTRY_SIZE;
      htab->elf.sgotplt->size += 4;
      htab->elf.srelplt->size += sizeof (Elf32_External_Rela);
    }
  else
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
    }

  return true;
}

static bool
handle_got_allocation (struct elf_link_hash_entry *h,
		       struct bfd_link_info *info,
		       struct elf32_mb_link_hash_table *htab)
{
  struct elf32_mb_link_hash_entry *eh = (struct elf32_mb_link_hash_entry *) h;
  unsigned int need = 0;

  if (h->got.refcount <= 0)
    {
      h->got.offset = (bfd_vma) -1;
      return true;
    }

  if (!ensure_dynamic_symbol (h, info))
    return false;

  if ((eh->tls_mask & TLS_TLS) != 0)
    {
      if ((eh->tls_mask & TLS_LD) != 0)
	{
	  if (!eh->elf.def_dynamic)
	    htab->tlsld_got.refcount += 1;
	  else
	    need += 8;
	}
      if ((eh->tls_mask & TLS_GD) != 0)
	need += 8;
    }
  else
    {
      need += 4;
    }

  if (need == 0)
    {
      h->got.offset = (bfd_vma) -1;
    }
  else
    {
      asection *s = htab->elf.sgot;
      h->got.offset = s->size;
      s->size += need;
      htab->elf.srelgot->size += need * (sizeof (Elf32_External_Rela) / 4);
    }

  return true;
}

static bool
cleanup_dynamic_relocs_non_pic (struct elf_link_hash_entry *h,
				struct bfd_link_info *info,
				struct elf32_mb_link_hash_table *htab)
{
  bool keep_relocs = false;
  if (!h->non_got_ref
      && ((h->def_dynamic && !h->def_regular)
	  || (htab->elf.dynamic_sections_created
	      && (h->root.type == bfd_link_hash_undefweak
		  || h->root.type == bfd_link_hash_undefined))))
    {
      if (!ensure_dynamic_symbol (h, info))
	return false;

      if (h->dynindx != -1)
	keep_relocs = true;
    }

  if (!keep_relocs)
    h->dyn_relocs = NULL;

  return true;
}

static void
cleanup_dynamic_relocs_pic (struct elf_link_hash_entry *h,
			    struct bfd_link_info *info)
{
  if (h->def_regular && (h->forced_local || info->symbolic))
    {
      struct elf_dyn_relocs **pp;
      for (pp = &h->dyn_relocs; *pp != NULL;)
	{
	  struct elf_dyn_relocs *p = *pp;
	  p->count -= p->pc_count;
	  p->pc_count = 0;
	  if (p->count == 0)
	    *pp = p->next;
	  else
	    pp = &p->next;
	}
    }
  else if (UNDEFWEAK_NO_DYNAMIC_RELOC (info, h))
    {
      h->dyn_relocs = NULL;
    }
}

static bool
cleanup_dynamic_relocs (struct elf_link_hash_entry *h,
			struct bfd_link_info *info,
			struct elf32_mb_link_hash_table *htab)
{
  if (h->dyn_relocs == NULL)
    return true;

  if (bfd_link_pic (info))
    {
      cleanup_dynamic_relocs_pic (h, info);
      return true;
    }
  return cleanup_dynamic_relocs_non_pic (h, info, htab);
}

static void
allocate_reloc_space (struct elf_link_hash_entry *h)
{
  for (struct elf_dyn_relocs *p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }
}

static bool
allocate_dynrelocs (struct elf_link_hash_entry *h, void *dat)
{
  struct bfd_link_info *info;
  struct elf32_mb_link_hash_table *htab;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) dat;
  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  if (!handle_plt_allocation (h, info, htab))
    return false;

  if (!handle_got_allocation (h, info, htab))
    return false;

  if (!cleanup_dynamic_relocs (h, info, htab))
    return false;

  allocate_reloc_space (h);

  return true;
}

/* Set the sizes of the dynamic sections.  */

static void
process_section_dynamic_relocs (asection *s, struct bfd_link_info *info)
{
  struct elf_dyn_relocs *p;

  for (p = (struct elf_dyn_relocs *) elf_section_data (s)->local_dynrel;
       p != NULL;
       p = p->next)
    {
      if (p->count == 0)
	continue;

      if (!bfd_is_abs_section (p->sec)
	  && bfd_is_abs_section (p->sec->output_section))
	continue;

      asection *srel = elf_section_data (p->sec)->sreloc;
      srel->size += p->count * sizeof (Elf32_External_Rela);
      if ((p->sec->output_section->flags & SEC_READONLY) != 0)
	info->flags |= DF_TEXTREL;
    }
}

static void
process_local_got_entries (bfd *ibfd,
			   struct bfd_link_info *info,
			   struct elf32_mb_link_hash_table *htab)
{
  bfd_signed_vma *local_got = elf_local_got_refcounts (ibfd);
  if (!local_got)
    return;

  Elf_Internal_Shdr *symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
  bfd_size_type locsymcount = symtab_hdr->sh_info;
  bfd_signed_vma *end_local_got = local_got + locsymcount;
  unsigned char *lgot_masks = (unsigned char *) end_local_got;

  asection *sgot = htab->elf.sgot;
  asection *srelgot = htab->elf.srelgot;

  for (; local_got < end_local_got; ++local_got, ++lgot_masks)
    {
      if (*local_got <= 0)
	{
	  *local_got = (bfd_vma) -1;
	  continue;
	}

      unsigned int need = 0;
      if ((*lgot_masks & TLS_TLS) != 0)
	{
	  if ((*lgot_masks & TLS_GD) != 0)
	    need += 8;
	  if ((*lgot_masks & TLS_LD) != 0)
	    htab->tlsld_got.refcount += 1;
	}
      else
	{
	  need += 4;
	}

      if (need == 0)
	{
	  *local_got = (bfd_vma) -1;
	}
      else
	{
	  *local_got = sgot->size;
	  sgot->size += need;
	  if (bfd_link_pic (info))
	    srelgot->size += need * (sizeof (Elf32_External_Rela) / 4);
	}
    }
}

static void
process_input_bfd (bfd *ibfd,
		   struct bfd_link_info *info,
		   struct elf32_mb_link_hash_table *htab)
{
  asection *s;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
    return;

  for (s = ibfd->sections; s != NULL; s = s->next)
    process_section_dynamic_relocs (s, info);

  process_local_got_entries (ibfd, info, htab);
}

static bool
allocate_dynamic_sections_memory (bfd *dynobj,
				  struct elf32_mb_link_hash_table *htab)
{
  asection *s;

  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      const char *name = bfd_section_name (s);
      if (startswith (name, ".rela"))
	{
	  if (s->size == 0)
	    {
	      s->flags |= SEC_EXCLUDE;
	      continue;
	    }
	  s->reloc_count = 0;
	}
      else if (s != htab->elf.splt
	       && s != htab->elf.sgot
	       && s != htab->elf.sgotplt
	       && s != htab->elf.sdynbss
	       && s != htab->elf.sdynrelro)
	{
	  continue;
	}

      if (s->size > 0)
	{
	  s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
	  if (s->contents == NULL)
	    return false;
	}
      s->alloced = 1;
    }

  return true;
}

static bool
microblaze_elf_late_size_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				   struct bfd_link_info *info)
{
  struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  bfd *dynobj = htab->elf.dynobj;
  if (dynobj == NULL)
    return true;

  bfd *ibfd;
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    process_input_bfd (ibfd, info, htab);

  elf_link_hash_traverse (elf_hash_table (info), allocate_dynrelocs, info);

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

  if (elf_hash_table (info)->dynamic_sections_created
      && htab->elf.splt->size > 0)
    htab->elf.splt->size += 4;

  if (!allocate_dynamic_sections_memory (dynobj, htab))
    return false;

  info->flags |= DF_BIND_NOW;
  return _bfd_elf_add_dynamic_tags (output_bfd, info, true);
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static void
handle_plt_entry (bfd *output_bfd, struct bfd_link_info *info,
                  struct elf_link_hash_entry *h,
                  struct elf32_mb_link_hash_table *htab,
                  Elf_Internal_Sym *sym)
{
  asection *splt = htab->elf.splt;
  asection *srela = htab->elf.srelplt;
  asection *sgotplt = htab->elf.sgotplt;
  Elf_Internal_Rela rela;
  bfd_byte *loc;
  bfd_vma plt_index, got_offset, got_addr;
  bool is_pic = bfd_link_pic (info);

  BFD_ASSERT (h->dynindx != -1);
  BFD_ASSERT (splt != NULL && srela != NULL && sgotplt != NULL);

  /* First PLT entry is reserved. */
  plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;

  /* First three GOT entries are reserved for the PLT. */
  got_offset = (plt_index + 3) * 4;
  got_addr = got_offset;

  if (!is_pic)
    got_addr += sgotplt->output_section->vma + sgotplt->output_offset;

  loc = splt->contents + h->plt.offset;
  bfd_put_32 (output_bfd, PLT_ENTRY_WORD_0 + ((got_addr >> 16) & 0xffff), loc);
  if (is_pic)
    bfd_put_32 (output_bfd, PLT_ENTRY_WORD_1 + (got_addr & 0xffff), loc + 4);
  else
    bfd_put_32 (output_bfd, PLT_ENTRY_WORD_1_NOPIC + (got_addr & 0xffff), loc + 4);
  bfd_put_32 (output_bfd, (bfd_vma) PLT_ENTRY_WORD_2, loc + 8);
  bfd_put_32 (output_bfd, (bfd_vma) PLT_ENTRY_WORD_3, loc + 12);

  rela.r_offset = (sgotplt->output_section->vma
		   + sgotplt->output_offset
		   + got_offset);
  rela.r_info = ELF32_R_INFO (h->dynindx, R_MICROBLAZE_JUMP_SLOT);
  rela.r_addend = 0;
  loc = srela->contents + plt_index * sizeof (Elf32_External_Rela);
  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

  if (!h->def_regular)
    {
      sym->st_shndx = SHN_UNDEF;
      sym->st_value = 0;
    }
}

static void
handle_got_entry (bfd *output_bfd, struct bfd_link_info *info,
                  struct elf_link_hash_entry *h,
                  struct elf32_mb_link_hash_table *htab)
{
  asection *sgot = htab->elf.sgot;
  asection *srela = htab->elf.srelgot;
  BFD_ASSERT (sgot != NULL && srela != NULL);

  bfd_vma got_entry_offset = h->got.offset &~ (bfd_vma) 1;
  bfd_vma reloc_offset = (sgot->output_section->vma
			+ sgot->output_offset
			+ got_entry_offset);

  bool is_pic = bfd_link_pic (info);
  bool is_symbolic_link = info->symbolic && h->def_regular;
  bool is_forced_local = h->dynindx == -1;

  if (is_pic && (is_symbolic_link || is_forced_local))
    {
      asection *sec = h->root.u.def.section;
      bfd_vma value = h->root.u.def.value;

      if (sec->output_section != NULL)
	value += sec->output_section->vma + sec->output_offset;

      microblaze_elf_output_dynamic_relocation (output_bfd, srela,
						srela->reloc_count++, 0,
						R_MICROBLAZE_REL, reloc_offset,
						value);
    }
  else
    {
      microblaze_elf_output_dynamic_relocation (output_bfd, srela,
						srela->reloc_count++, h->dynindx,
						R_MICROBLAZE_GLOB_DAT,
						reloc_offset, 0);
    }

  bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents + got_entry_offset);
}

static void
handle_copy_reloc (bfd *output_bfd, struct elf_link_hash_entry *h,
                   struct elf32_mb_link_hash_table *htab)
{
  Elf_Internal_Rela rela;
  asection *s;
  bfd_byte *loc;
  struct elf_link_hash_entry_def *def = &h->root.u.def;

  BFD_ASSERT (h->dynindx != -1);

  rela.r_offset = (def->value
		   + def->section->output_section->vma
		   + def->section->output_offset);
  rela.r_info = ELF32_R_INFO (h->dynindx, R_MICROBLAZE_COPY);
  rela.r_addend = 0;

  if (def->section == htab->elf.sdynrelro)
    s = htab->elf.sreldynrelro;
  else
    s = htab->elf.srelbss;

  loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
}

static bool
microblaze_elf_finish_dynamic_symbol (bfd *output_bfd,
				      struct bfd_link_info *info,
				      struct elf_link_hash_entry *h,
				      Elf_Internal_Sym *sym)
{
  const bfd_vma INVALID_OFFSET = (bfd_vma) -1;
  struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table (info);
  struct elf32_mb_link_hash_entry *eh = elf32_mb_hash_entry (h);

  if (h->plt.offset != INVALID_OFFSET)
    {
      handle_plt_entry (output_bfd, info, h, htab, sym);
    }

  bool needs_got_entry = (h->got.offset != INVALID_OFFSET)
			 && !(h->got.offset & 1)
			 && !IS_TLS_LD (eh->tls_mask)
			 && !IS_TLS_GD (eh->tls_mask);

  if (needs_got_entry)
    {
      handle_got_entry (output_bfd, info, h, htab);
    }

  if (h->needs_copy)
    {
      handle_copy_reloc (output_bfd, h, htab);
    }

  if (h == htab->elf.hdynamic
      || h == htab->elf.hgot
      || h == htab->elf.hplt)
    {
      sym->st_shndx = SHN_ABS;
    }

  return true;
}


/* Finish up the dynamic sections.  */

#define MICROBLAZE_NOP 0x80000000
#define PLT_NOP_OFFSET 4
#define ELF_ENTRY_SIZE_4 4

static void
process_dynamic_entry (struct elf32_mb_link_hash_table *htab,
		       bfd *output_bfd,
		       Elf_Internal_Dyn *dyn,
		       Elf32_External_Dyn *dyncon)
{
  asection *s = NULL;
  bool use_size = false;

  switch (dyn->d_tag)
    {
    case DT_PLTGOT:
      s = htab->elf.sgotplt;
      use_size = false;
      break;
    case DT_PLTRELSZ:
      s = htab->elf.srelplt;
      use_size = true;
      break;
    case DT_JMPREL:
      s = htab->elf.srelplt;
      use_size = false;
      break;
    default:
      return;
    }

  if (s != NULL && s->output_section != NULL)
    {
      if (use_size)
	dyn->d_un.d_val = s->size;
      else
	dyn->d_un.d_ptr = s->output_section->vma + s->output_offset;
    }
  else
    {
      dyn->d_un.d_val = 0;
    }

  bfd_elf32_swap_dyn_out (output_bfd, dyn, dyncon);
}

static bool
finish_dynamic_and_plt_sections (bfd *output_bfd,
				 struct elf32_mb_link_hash_table *htab,
				 asection *sdyn)
{
  bfd *dynobj = htab->elf.dynobj;
  asection *splt = htab->elf.splt;
  if (!splt || !sdyn || !sdyn->contents)
    return false;

  Elf32_External_Dyn *dyncon = (Elf32_External_Dyn *) sdyn->contents;
  Elf32_External_Dyn *dynconend =
    (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);

  for (; dyncon < dynconend; dyncon++)
    {
      Elf_Internal_Dyn dyn;
      bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);
      process_dynamic_entry (htab, output_bfd, &dyn, dyncon);
    }

  if (splt->size > 0)
    {
      if (!splt->contents)
	return false;

      memset (splt->contents, 0, PLT_ENTRY_SIZE);
      if (splt->size >= PLT_NOP_OFFSET)
	bfd_put_32 (output_bfd, (bfd_vma) MICROBLAZE_NOP,
		    splt->contents + splt->size - PLT_NOP_OFFSET);

      if (splt->output_section != bfd_abs_section_ptr)
	elf_section_data (splt->output_section)->this_hdr.sh_entsize
	  = ELF_ENTRY_SIZE_4;
    }

  return true;
}

static void
set_elf_section_entsize (asection *sec)
{
  if (sec && sec->size > 0 && sec->output_section)
    elf_section_data (sec->output_section)->this_hdr.sh_entsize
      = ELF_ENTRY_SIZE_4;
}

static void
finish_got_sections (bfd *output_bfd,
		     struct elf_link_hash_table *elf_htab,
		     asection *sdyn)
{
  asection *sgotplt = elf_htab->sgotplt;
  if (sgotplt && sgotplt->size > 0 && sgotplt->contents)
    {
      bfd_vma dyn_addr = 0;
      if (sdyn && sdyn->output_section)
	dyn_addr = sdyn->output_section->vma + sdyn->output_offset;

      bfd_put_32 (output_bfd, dyn_addr, sgotplt->contents);
    }

  set_elf_section_entsize (elf_htab->sgotplt);
  set_elf_section_entsize (elf_htab->sgot);
}

static bool
microblaze_elf_finish_dynamic_sections (bfd *output_bfd,
					struct bfd_link_info *info)
{
  struct elf32_mb_link_hash_table *htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  bfd *dynobj = htab->elf.dynobj;
  asection *sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      if (!finish_dynamic_and_plt_sections (output_bfd, htab, sdyn))
	return false;
    }

  finish_got_sections (output_bfd, &htab->elf, sdyn);

  return true;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */

static bool
microblaze_elf_add_symbol_hook (bfd *abfd,
				struct bfd_link_info *info,
				Elf_Internal_Sym *sym,
				const char **namep ATTRIBUTE_UNUSED,
				flagword *flagsp ATTRIBUTE_UNUSED,
				asection **secp,
				bfd_vma *valp)
{
  if (sym->st_shndx != SHN_COMMON
      || bfd_link_relocatable (info)
      || sym->st_size > elf_gp_size (abfd))
    {
      return true;
    }

  asection *const sbss_sec = bfd_make_section_old_way (abfd, ".sbss");
  if (sbss_sec == NULL
      || !bfd_set_section_flags (sbss_sec, SEC_IS_COMMON | SEC_SMALL_DATA))
    {
      return false;
    }

  *secp = sbss_sec;
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
