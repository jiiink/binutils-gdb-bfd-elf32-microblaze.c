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

  for (i = 0; i < NUM_ELEM (microblaze_elf_howto_raw); i++)
    {
      unsigned int type = microblaze_elf_howto_raw[i].type;

      if (type >= NUM_ELEM (microblaze_elf_howto_table))
        {
          BFD_ASSERT (0);
          continue;
        }

      microblaze_elf_howto_table[type] = &microblaze_elf_howto_raw[i];
    }
}

static reloc_howto_type *
microblaze_elf_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
				  bfd_reloc_code_real_type code)
{
  static const struct {
    bfd_reloc_code_real_type bfd_code;
    enum elf_microblaze_reloc_type mb_reloc;
  } reloc_map[] = {
    { BFD_RELOC_NONE, R_MICROBLAZE_NONE },
    { BFD_RELOC_MICROBLAZE_32_NONE, R_MICROBLAZE_32_NONE },
    { BFD_RELOC_MICROBLAZE_64_NONE, R_MICROBLAZE_64_NONE },
    { BFD_RELOC_32, R_MICROBLAZE_32 },
    { BFD_RELOC_RVA, R_MICROBLAZE_32 },
    { BFD_RELOC_32_PCREL, R_MICROBLAZE_32_PCREL },
    { BFD_RELOC_64_PCREL, R_MICROBLAZE_64_PCREL },
    { BFD_RELOC_MICROBLAZE_32_LO_PCREL, R_MICROBLAZE_32_PCREL_LO },
    { BFD_RELOC_64, R_MICROBLAZE_64 },
    { BFD_RELOC_MICROBLAZE_32_LO, R_MICROBLAZE_32_LO },
    { BFD_RELOC_MICROBLAZE_32_ROSDA, R_MICROBLAZE_SRO32 },
    { BFD_RELOC_MICROBLAZE_32_RWSDA, R_MICROBLAZE_SRW32 },
    { BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM, R_MICROBLAZE_32_SYM_OP_SYM },
    { BFD_RELOC_VTABLE_INHERIT, R_MICROBLAZE_GNU_VTINHERIT },
    { BFD_RELOC_VTABLE_ENTRY, R_MICROBLAZE_GNU_VTENTRY },
    { BFD_RELOC_MICROBLAZE_64_GOTPC, R_MICROBLAZE_GOTPC_64 },
    { BFD_RELOC_MICROBLAZE_64_GOT, R_MICROBLAZE_GOT_64 },
    { BFD_RELOC_MICROBLAZE_64_TEXTPCREL, R_MICROBLAZE_TEXTPCREL_64 },
    { BFD_RELOC_MICROBLAZE_64_TEXTREL, R_MICROBLAZE_TEXTREL_64 },
    { BFD_RELOC_MICROBLAZE_64_PLT, R_MICROBLAZE_PLT_64 },
    { BFD_RELOC_MICROBLAZE_64_GOTOFF, R_MICROBLAZE_GOTOFF_64 },
    { BFD_RELOC_MICROBLAZE_32_GOTOFF, R_MICROBLAZE_GOTOFF_32 },
    { BFD_RELOC_MICROBLAZE_64_TLSGD, R_MICROBLAZE_TLSGD },
    { BFD_RELOC_MICROBLAZE_64_TLSLD, R_MICROBLAZE_TLSLD },
    { BFD_RELOC_MICROBLAZE_32_TLSDTPREL, R_MICROBLAZE_TLSDTPREL32 },
    { BFD_RELOC_MICROBLAZE_64_TLSDTPREL, R_MICROBLAZE_TLSDTPREL64 },
    { BFD_RELOC_MICROBLAZE_32_TLSDTPMOD, R_MICROBLAZE_TLSDTPMOD32 },
    { BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL, R_MICROBLAZE_TLSGOTTPREL32 },
    { BFD_RELOC_MICROBLAZE_64_TLSTPREL, R_MICROBLAZE_TLSTPREL32 },
    { BFD_RELOC_MICROBLAZE_COPY, R_MICROBLAZE_COPY }
  };

  enum elf_microblaze_reloc_type microblaze_reloc = R_MICROBLAZE_NONE;
  size_t i;
  
  for (i = 0; i < sizeof(reloc_map) / sizeof(reloc_map[0]); i++)
    {
      if (reloc_map[i].bfd_code == code)
        {
          microblaze_reloc = reloc_map[i].mb_reloc;
          break;
        }
    }
  
  if (i == sizeof(reloc_map) / sizeof(reloc_map[0]))
    return NULL;

  if (!microblaze_elf_howto_table[R_MICROBLAZE_32])
    microblaze_elf_howto_init();

  return microblaze_elf_howto_table[microblaze_reloc];
};

static reloc_howto_type *
microblaze_elf_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
                                  const char *r_name)
{
  if (r_name == NULL)
    return NULL;

  for (unsigned int i = 0; i < NUM_ELEM (microblaze_elf_howto_raw); i++)
    {
      const char *howto_name = microblaze_elf_howto_raw[i].name;
      if (howto_name != NULL && strcasecmp (howto_name, r_name) == 0)
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
  unsigned int r_type;

  if (abfd == NULL || cache_ptr == NULL || dst == NULL)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return false;
    }

  if (!microblaze_elf_howto_table [R_MICROBLAZE_32])
    microblaze_elf_howto_init ();

  r_type = ELF32_R_TYPE (dst->r_info);
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

  if (abfd == NULL || sec == NULL)
    return false;

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
  if (name == NULL)
    return false;

  if (name[0] == 'L' && name[1] == '.')
    return true;

  if (name[0] == '$' && name[1] == 'L')
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
link_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table,
		   const char *string)
{
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf32_mb_link_hash_entry));
      if (entry == NULL)
	return NULL;
    }

  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry == NULL)
    return NULL;

  struct elf32_mb_link_hash_entry *eh = (struct elf32_mb_link_hash_entry *) entry;
  eh->tls_mask = 0;

  return entry;
}

/* Create a mb ELF linker hash table.  */

static struct bfd_link_hash_table *
microblaze_elf_link_hash_table_create (bfd *abfd)
{
  struct elf32_mb_link_hash_table *ret;

  ret = bfd_zmalloc (sizeof (struct elf32_mb_link_hash_table));
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

static void
microblaze_elf_final_sdp (struct bfd_link_info *info)
{
  struct bfd_link_hash_entry *h;

  if (info == NULL || info->hash == NULL)
    return;

  h = bfd_link_hash_lookup (info->hash, RO_SDA_ANCHOR_NAME, false, false, true);
  if (h != NULL && h->type == bfd_link_hash_defined && h->u.def.section != NULL)
    {
      struct bfd_section *output_section = h->u.def.section->output_section;
      if (output_section != NULL)
        {
          ro_small_data_pointer = h->u.def.value
                                + output_section->vma
                                + h->u.def.section->output_offset;
        }
    }

  h = bfd_link_hash_lookup (info->hash, RW_SDA_ANCHOR_NAME, false, false, true);
  if (h != NULL && h->type == bfd_link_hash_defined && h->u.def.section != NULL)
    {
      struct bfd_section *output_section = h->u.def.section->output_section;
      if (output_section != NULL)
        {
          rw_small_data_pointer = h->u.def.value
                                + output_section->vma
                                + h->u.def.section->output_offset;
        }
    }
}

static bfd_vma
dtprel_base (struct bfd_link_info *info)
{
  struct elf_link_hash_table *hash_table;
  asection *tls_sec;
  
  if (info == NULL)
    return 0;
    
  hash_table = elf_hash_table (info);
  if (hash_table == NULL)
    return 0;
    
  tls_sec = hash_table->tls_sec;
  if (tls_sec == NULL)
    return 0;
    
  return tls_sec->vma;
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
  bfd_byte *loc;

  if (output_bfd == NULL || sreloc == NULL || sreloc->contents == NULL)
    return;

  rel.r_info = ELF32_R_INFO (indx, r_type);
  rel.r_offset = offset;
  rel.r_addend = addend;

  loc = sreloc->contents + (reloc_index * sizeof (Elf32_External_Rela));
  bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
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
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel, *relend;
  int endian;
  bool ret = true;
  asection *sreloc;
  bfd_vma *local_got_offsets;

  if (!microblaze_elf_howto_table[R_MICROBLAZE_max-1])
    microblaze_elf_howto_init ();

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  endian = bfd_little_endian (output_bfd) ? 0 : 2;
  local_got_offsets = elf_local_got_offsets (input_bfd);
  sreloc = elf_section_data (input_section)->sreloc;

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  
  for (; rel < relend; rel++)
    {
      if (!process_relocation(output_bfd, info, input_bfd, input_section,
                              contents, rel, local_syms, local_sections,
                              htab, symtab_hdr, sym_hashes, endian,
                              &sreloc, local_got_offsets, &ret))
        continue;
    }

  return ret;
}

static bool
process_relocation(bfd *output_bfd, struct bfd_link_info *info,
                   bfd *input_bfd, asection *input_section,
                   bfd_byte *contents, Elf_Internal_Rela *rel,
                   Elf_Internal_Sym *local_syms, asection **local_sections,
                   struct elf32_mb_link_hash_table *htab,
                   Elf_Internal_Shdr *symtab_hdr,
                   struct elf_link_hash_entry **sym_hashes,
                   int endian, asection **sreloc,
                   bfd_vma *local_got_offsets, bool *ret)
{
  int r_type;
  reloc_howto_type *howto;
  unsigned long r_symndx;
  bfd_vma addend;
  bfd_vma offset;
  struct elf_link_hash_entry *h;
  Elf_Internal_Sym *sym;
  asection *sec;
  const char *sym_name;
  bfd_reloc_status_type r;
  bool unresolved_reloc;
  unsigned int tls_type;

  h = NULL;
  sym = NULL;
  sec = NULL;
  unresolved_reloc = false;
  r = bfd_reloc_ok;
  
  r_type = ELF32_R_TYPE (rel->r_info);
  tls_type = 0;
  addend = rel->r_addend;
  offset = rel->r_offset;

  if (r_type < 0 || r_type >= (int) R_MICROBLAZE_max)
    {
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
                          input_bfd, (int) r_type);
      bfd_set_error (bfd_error_bad_value);
      *ret = false;
      return false;
    }

  howto = microblaze_elf_howto_table[r_type];
  r_symndx = ELF32_R_SYM (rel->r_info);

  if (bfd_link_relocatable (info))
    return handle_relocatable_link(input_bfd, contents, rel, local_syms,
                                   local_sections, symtab_hdr, howto,
                                   r_symndx, &addend, offset);

  if (!resolve_symbol(output_bfd, info, input_bfd, rel, r_symndx,
                      symtab_hdr, sym_hashes, local_syms, local_sections,
                      &h, &sym, &sec, &sym_name, &addend, &unresolved_reloc))
    return false;

  if (offset > bfd_get_section_limit (input_bfd, input_section))
    {
      r = bfd_reloc_outofrange;
      handle_reloc_error(info, h, sym, sec, sym_name, symtab_hdr,
                        input_bfd, input_section, offset, r, NULL);
      return true;
    }

  r = apply_relocation(output_bfd, info, input_bfd, input_section,
                      contents, rel, h, sec, sym_name, howto,
                      r_type, offset, addend, endian, htab,
                      local_got_offsets, r_symndx, sreloc,
                      &unresolved_reloc, &tls_type);

  if (r != bfd_reloc_ok)
    handle_reloc_error(info, h, sym, sec, sym_name, symtab_hdr,
                      input_bfd, input_section, offset, r, NULL);

  return true;
}

static bool
handle_relocatable_link(bfd *input_bfd, bfd_byte *contents,
                        Elf_Internal_Rela *rel, Elf_Internal_Sym *local_syms,
                        asection **local_sections, Elf_Internal_Shdr *symtab_hdr,
                        reloc_howto_type *howto, unsigned long r_symndx,
                        bfd_vma *addend, bfd_vma offset)
{
  asection *sec;
  Elf_Internal_Sym *sym;

  if (r_symndx >= symtab_hdr->sh_info)
    return false;

  sym = local_syms + r_symndx;
  if (ELF_ST_TYPE (sym->st_info) != STT_SECTION)
    return false;

  sec = local_sections[r_symndx];
  *addend += sec->output_offset + sym->st_value;

#ifdef USE_REL
  if (howto->partial_inplace)
    _bfd_relocate_contents (howto, input_bfd, *addend, contents + offset);
#endif

  return false;
}

static bool
resolve_symbol(bfd *output_bfd, struct bfd_link_info *info, bfd *input_bfd,
               Elf_Internal_Rela *rel, unsigned long r_symndx,
               Elf_Internal_Shdr *symtab_hdr,
               struct elf_link_hash_entry **sym_hashes,
               Elf_Internal_Sym *local_syms, asection **local_sections,
               struct elf_link_hash_entry **h, Elf_Internal_Sym **sym,
               asection **sec, const char **sym_name, bfd_vma *addend,
               bool *unresolved_reloc)
{
  bfd_vma relocation;

  if (r_symndx < symtab_hdr->sh_info)
    {
      *sym = local_syms + r_symndx;
      *sec = local_sections[r_symndx];
      if (*sec == 0)
        return false;
      *sym_name = "<local symbol>";
      relocation = _bfd_elf_rela_local_sym (output_bfd, *sym, sec, rel);
      *addend = rel->r_addend;
    }
  else
    {
      bool warned = false;
      bool ignored = false;

      RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, NULL, rel,
                               r_symndx, symtab_hdr, sym_hashes,
                               *h, *sec, relocation,
                               *unresolved_reloc, warned, ignored);
      *sym_name = (*h)->root.root.string;
    }

  return true;
}

static bfd_reloc_status_type
apply_relocation(bfd *output_bfd, struct bfd_link_info *info,
                 bfd *input_bfd, asection *input_section,
                 bfd_byte *contents, Elf_Internal_Rela *rel,
                 struct elf_link_hash_entry *h, asection *sec,
                 const char *sym_name, reloc_howto_type *howto,
                 int r_type, bfd_vma offset, bfd_vma addend,
                 int endian, struct elf32_mb_link_hash_table *htab,
                 bfd_vma *local_got_offsets, unsigned long r_symndx,
                 asection **sreloc, bool *unresolved_reloc,
                 unsigned int *tls_type)
{
  bfd_vma relocation = 0;
  bool resolved_to_zero;

  resolved_to_zero = (h != NULL && UNDEFWEAK_NO_DYNAMIC_RELOC (info, h));

  switch (r_type)
    {
    case R_MICROBLAZE_SRO32:
      return handle_sro32_relocation(input_bfd, input_section, contents,
                                     offset, sec, sym_name, howto, addend, info);

    case R_MICROBLAZE_SRW32:
      return handle_srw32_relocation(input_bfd, input_section, contents,
                                     offset, sec, sym_name, howto, addend, info);

    case R_MICROBLAZE_32_SYM_OP_SYM:
      return bfd_reloc_ok;

    case R_MICROBLAZE_GOTPC_64:
      return handle_gotpc_64_relocation(input_bfd, input_section, contents,
                                        offset, addend, endian, htab);

    case R_MICROBLAZE_TEXTPCREL_64:
      return handle_textpcrel_64_relocation(input_bfd, input_section, contents,
                                            offset, addend, endian);

    case R_MICROBLAZE_PLT_64:
      return handle_plt_64_relocation(input_bfd, input_section, contents,
                                      offset, endian, htab, h);

    case R_MICROBLAZE_TLSGD:
      *tls_type = (TLS_TLS | TLS_GD);
      return handle_got_relocation(output_bfd, info, input_bfd, contents,
                                   offset, endian, htab, h, local_got_offsets,
                                   r_symndx, addend, *tls_type, unresolved_reloc,
                                   resolved_to_zero);

    case R_MICROBLAZE_TLSLD:
      *tls_type = (TLS_TLS | TLS_LD);
      return handle_got_relocation(output_bfd, info, input_bfd, contents,
                                   offset, endian, htab, h, local_got_offsets,
                                   r_symndx, addend, *tls_type, unresolved_reloc,
                                   resolved_to_zero);

    case R_MICROBLAZE_GOT_64:
      return handle_got_relocation(output_bfd, info, input_bfd, contents,
                                   offset, endian, htab, h, local_got_offsets,
                                   r_symndx, addend, 0, unresolved_reloc,
                                   resolved_to_zero);

    case R_MICROBLAZE_GOTOFF_64:
      return handle_gotoff_64_relocation(input_bfd, contents, offset,
                                         addend, endian, htab);

    case R_MICROBLAZE_GOTOFF_32:
      return handle_gotoff_32_relocation(input_bfd, contents, offset,
                                         addend, htab);

    case R_MICROBLAZE_TLSDTPREL64:
      return handle_tlsdtprel64_relocation(input_bfd, contents, offset,
                                           addend, endian, info);

    case R_MICROBLAZE_TEXTREL_64:
    case R_MICROBLAZE_TEXTREL_32_LO:
    case R_MICROBLAZE_64_PCREL:
    case R_MICROBLAZE_64:
    case R_MICROBLAZE_32:
      return handle_generic_relocation(output_bfd, info, input_bfd,
                                       input_section, contents, rel,
                                       h, sec, howto, r_type, offset,
                                       addend, endian, r_symndx,
                                       sreloc, resolved_to_zero);

    default:
      return _bfd_final_link_relocate (howto, input_bfd, input_section,
                                       contents, offset, relocation, addend);
    }
}

static bfd_reloc_status_type
handle_sro32_relocation(bfd *input_bfd, asection *input_section,
                        bfd_byte *contents, bfd_vma offset, asection *sec,
                        const char *sym_name, reloc_howto_type *howto,
                        bfd_vma addend, struct bfd_link_info *info)
{
  const char *name;
  bfd_vma relocation;

  if (!sec)
    return bfd_reloc_ok;

  name = bfd_section_name (sec);
  if (strcmp (name, ".sdata2") != 0 && strcmp (name, ".sbss2") != 0)
    {
      _bfd_error_handler
        (_("%pB: the target (%s) of an %s relocation"
           " is in the wrong section (%pA)"),
         input_bfd, sym_name, howto->name, sec);
      return bfd_reloc_continue;
    }

  if (ro_small_data_pointer == 0)
    microblaze_elf_final_sdp (info);
  if (ro_small_data_pointer == 0)
    return bfd_reloc_undefined;

  relocation = _bfd_elf_rela_local_sym (input_bfd, NULL, &sec, NULL);
  relocation -= ro_small_data_pointer;
  return _bfd_final_link_relocate (howto, input_bfd, input_section,
                                   contents, offset, relocation, addend);
}

static bfd_reloc_status_type
handle_srw32_relocation(bfd *input_bfd, asection *input_section,
                        bfd_byte *contents, bfd_vma offset, asection *sec,
                        const char *sym_name, reloc_howto_type *howto,
                        bfd_vma addend, struct bfd_link_info *info)
{
  const char *name;
  bfd_vma relocation;

  if (!sec)
    return bfd_reloc_ok;

  name = bfd_section_name (sec);
  if (strcmp (name, ".sdata") != 0 && strcmp (name, ".sbss") != 0)
    {
      _bfd_error_handler
        (_("%pB: the target (%s) of an %s relocation"
           " is in the wrong section (%pA)"),
         input_bfd, sym_name, howto->name, sec);
      return bfd_reloc_continue;
    }

  if (rw_small_data_pointer == 0)
    microblaze_elf_final_sdp (info);
  if (rw_small_data_pointer == 0)
    return bfd_reloc_undefined;

  relocation = _bfd_elf_rela_local_sym (input_bfd, NULL, &sec, NULL);
  relocation -= rw_small_data_pointer;
  return _bfd_final_link_relocate (howto, input_bfd, input_section,
                                   contents, offset, relocation, addend);
}

static bfd_reloc_status_type
handle_gotpc_64_relocation(bfd *input_bfd, asection *input_section,
                           bfd_byte *contents, bfd_vma offset,
                           bfd_vma addend, int endian,
                           struct elf32_mb_link_hash_table *htab)
{
  bfd_vma relocation;

  relocation = (htab->elf.sgotplt->output_section->vma
                + htab->elf.sgotplt->output_offset);
  relocation -= (input_section->output_section->vma
                 + input_section->output_offset
                 + offset + INST_WORD_SIZE);
  relocation += addend;
  bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
              contents + offset + endian);
  bfd_put_16 (input_bfd, relocation & 0xffff,
              contents + offset + endian + INST_WORD_SIZE);
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
handle_textpcrel_64_relocation(bfd *input_bfd, asection *input_section,
                                bfd_byte *contents, bfd_vma offset,
                                bfd_vma addend, int endian)
{
  bfd_vma relocation;

  relocation = input_section->output_section->vma;
  relocation -= (input_section->output_section->vma
                 + input_section->output_offset
                 + offset + INST_WORD_SIZE);
  relocation += addend;
  bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
              contents + offset + endian);
  bfd_put_16 (input_bfd, relocation & 0xffff,
              contents + offset + endian + INST_WORD_SIZE);
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
handle_plt_64_relocation(bfd *input_bfd, asection *input_section,
                         bfd_byte *contents, bfd_vma offset, int endian,
                         struct elf32_mb_link_hash_table *htab,
                         struct elf_link_hash_entry *h)
{
  bfd_vma relocation;
  bfd_vma immediate;

  if (htab->elf.splt != NULL && h != NULL && h->plt.offset != (bfd_vma) -1)
    {
      relocation = (htab->elf.splt->output_section->vma
                    + htab->elf.splt->output_offset
                    + h->plt.offset);
    }
  else
    {
      relocation = 0;
    }

  immediate = relocation - (input_section->output_section->vma
                           + input_section->output_offset
                           + offset + INST_WORD_SIZE);
  bfd_put_16 (input_bfd, (immediate >> 16) & 0xffff,
              contents + offset + endian);
  bfd_put_16 (input_bfd, immediate & 0xffff,
              contents + offset + endian + INST_WORD_SIZE);
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
handle_got_relocation(bfd *output_bfd, struct bfd_link_info *info,
                      bfd *input_bfd, bfd_byte *contents, bfd_vma offset,
                      int endian, struct elf32_mb_link_hash_table *htab,
                      struct elf_link_hash_entry *h, bfd_vma *local_got_offsets,
                      unsigned long r_symndx, bfd_vma addend,
                      unsigned int tls_type, bool *unresolved_reloc,
                      bool resolved_to_zero)
{
  bfd_vma *offp;
  bfd_vma off, off2;
  unsigned long indx;
  bfd_vma static_value;
  bool need_relocs = false;
  bfd_vma relocation;

  if (htab->elf.sgot == NULL)
    abort ();

  indx = 0;
  offp = get_got_offset_pointer(htab, h, local_got_offsets, r_symndx, tls_type);
  if (!offp)
    abort ();

  off = (*offp) & ~1;
  off2 = (IS_TLS_LD(tls_type) || IS_TLS_GD(tls_type)) ? off + 4 : off;

  if (h != NULL)
    {
      bool dyn = elf_hash_table (info)->dynamic_sections_created;
      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h)
          && (!bfd_link_pic (info) || !SYMBOL_REFERENCES_LOCAL (info, h)))
        indx = h->dynindx;
    }

  need_relocs = should_generate_relocs(info, h, indx, resolved_to_zero);
  static_value = relocation + addend;

  if (!((*offp) & 1))
    {
      process_got_entry(output_bfd, info, htab, off, off2, tls_type,
                       need_relocs, indx, static_value, offp);
    }

  relocation = htab->elf.sgot->output_section->vma
               + htab->elf.sgot->output_offset
               + off
               - htab->elf.sgotplt->output_section->vma
               - htab->elf.sgotplt->output_offset;

  bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
              contents + offset + endian);
  bfd_put_16 (input_bfd, relocation & 0xffff,
              contents + offset + endian + INST_WORD_SIZE);

  *unresolved_reloc = false;
  return bfd_reloc_ok;
}

static bfd_vma *
get_got_offset_pointer(struct elf32_mb_link_hash_table *htab,
                       struct elf_link_hash_entry *h,
                       bfd_vma *local_got_offsets,
                       unsigned long r_symndx, unsigned int tls_type)
{
  if (IS_TLS_LD (tls_type))
    return &htab->tlsld_got.offset;
  
  if (h != NULL)
    {
      if (htab->elf.sgotplt != NULL && h->got.offset != (bfd_vma) -1)
        return &h->got.offset;
      abort ();
    }
  
  if (local_got_offsets == NULL)
    abort ();
  
  return &local_got_offsets[r_symndx];
}

static bool
should_generate_relocs(struct bfd_link_info *info,
                       struct elf_link_hash_entry *h,
                       unsigned long indx, bool resolved_to_zero)
{
  if (!(bfd_link_pic (info) || indx != 0))
    return false;

  if (h == NULL)
    return true;

  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT || resolved_to_zero)
    return false;

  if (h->root.type == bfd_link_hash_undefweak)
    return false;

  return true;
}

static void
process_got_entry(bfd *output_bfd, struct bfd_link_info *info,
                  struct elf32_mb_link_hash_table *htab,
                  bfd_vma off, bfd_vma off2, unsigned int tls_type,
                  bool need_relocs, unsigned long indx,
                  bfd_vma static_value, bfd_vma *offp)
{
  bfd_vma got_offset;

  got_offset = (htab->elf.sgot->output_section->vma
                + htab->elf.sgot->output_offset + off);

  if (IS_TLS_LD(tls_type))
    {
      if (!bfd_link_pic (info))
        bfd_put_32 (output_bfd, 1, htab->elf.sgot->contents + off);
      else
        microblaze_elf_output_dynamic_relocation
          (output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++,
           0, R_MICROBLAZE_TLSDTPMOD32, got_offset, 0);
      *offp |= 1;
      bfd_put_32 (output_bfd, 0, htab->elf.sgot->contents + off2);
    }
  else if (IS_TLS_GD(tls_type))
    {
      if (!need_relocs)
        bfd_put_32 (output_bfd, 1, htab->elf.sgot->contents + off);
      else
        microblaze_elf_output_dynamic_relocation
          (output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++,
           indx, R_MICROBLAZE_TLSDTPMOD32, got_offset, indx ? 0 : static_value);

      got_offset = (htab->elf.sgot->output_section->vma
                    + htab->elf.sgot->output_offset + off2);
      *offp |= 1;
      static_value -= dtprel_base(info);
      if (need_relocs)
        microblaze_elf_output_dynamic_relocation
          (output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++,
           indx, R_MICROBLAZE_TLSDTPREL32, got_offset, indx ? 0 : static_value);
      else
        bfd_put_32 (output_bfd, static_value, htab->elf.sgot->contents + off2);
    }
  else
    {
      bfd_put_32 (output_bfd, static_value, htab->elf.sgot->contents + off2);
      if (bfd_link_pic (info) && indx == 0)
        {
          *offp |= 1;
          microblaze_elf_output_dynamic_relocation
            (output_bfd, htab->elf.srelgot, htab->elf.srelgot->reloc_count++,
             indx, R_MICROBLAZE_REL, got_offset, static_value);
        }
    }

  if (htab->elf.srelgot == NULL)
    abort ();
}

static bfd_reloc_status_type
handle_gotoff_64_relocation(bfd *input_bfd, bfd_byte *contents,
                            bfd_vma offset, bfd_vma addend, int endian,
                            struct elf32_mb_link_hash_table *htab)
{
  bfd_vma immediate;
  unsigned short lo, high;

  immediate = addend - (htab->elf.sgotplt->output_section->vma
                       + htab->elf.sgotplt->output_offset);
  lo = immediate & 0x0000ffff;
  high = (immediate >> 16) & 0x0000ffff;
  bfd_put_16 (input_bfd, high, contents + offset + endian);
  bfd_put_16 (input_bfd, lo, contents + offset + INST_WORD_SIZE + endian);
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
handle_gotoff_32_relocation(bfd *input_bfd, bfd_byte *contents,
                            bfd_vma offset, bfd_vma addend,
                            struct elf32_mb_link_hash_table *htab)
{
  bfd_vma relocation;

  relocation = addend - (htab->elf.sgotplt->output_section->vma
                        + htab->elf.sgotplt->output_offset);
  bfd_put_32 (input_bfd, relocation, contents + offset);
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
handle_tlsdtprel64_relocation(bfd *input_bfd, bfd_byte *contents,
                              bfd_vma offset, bfd_vma addend, int endian,
                              struct bfd_link_info *info)
{
  bfd_vma relocation;

  relocation = addend - dtprel_base(info);
  bfd_put_16 (input_bfd, (relocation >> 16) & 0xffff,
              contents + offset + endian);
  bfd_put_16 (input_bfd, relocation & 0xffff,
              contents + offset + endian + INST_WORD_SIZE);
  return bfd_reloc_ok;
}

static bfd_reloc_status_type
handle_generic_relocation(bfd *output_bfd, struct bfd_link_info *info,
                         bfd *input_bfd, asection *input_section,
                         bfd_byte *contents, Elf_Internal_Rela *rel,
                         struct elf_link_hash_entry *h, asection *sec,
                         reloc_howto_type *howto, int r_type,
                         bfd_vma offset, bfd_vma addend, int endian,
                         unsigned long r_symndx, asection **sreloc,
                         bool resolved_to_zero)
{
  bfd_vma relocation = 0;

  if (r_symndx == STN_UNDEF || (input_section->flags & SEC_ALLOC) == 0)
    {
      apply_direct_relocation(input_bfd, input_section, contents,
                              r_type, offset, addend, endian);
      return bfd_reloc_ok;
    }

  if (should_generate_dynamic_reloc(info, h, howto, resolved_to_zero))
    {
      generate_dynamic_relocation(output_bfd, info, input_bfd, input_section,
                                  rel, h, r_type, addend, sreloc);
      return bfd_reloc_ok;
    }

  apply_direct_relocation(input_bfd, input_section, contents,
                         r_type

/* Calculate fixup value for reference.  */

static size_t
calc_fixup (bfd_vma start, bfd_vma size, asection *sec)
{
  struct _microblaze_elf_section_data *sdata;
  bfd_vma end;
  size_t fixup = 0;
  size_t i;

  if (sec == NULL)
    return 0;

  sdata = microblaze_elf_section_data (sec);
  if (sdata == NULL)
    return 0;

  end = start + size;

  for (i = 0; i < sdata->relax_count; i++)
    {
      if (end <= sdata->relax[i].addr)
        break;
      
      if (end != start && start > sdata->relax[i].addr)
        continue;
      
      fixup += sdata->relax[i].size;
    }
  
  return fixup;
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   a 32-bit instruction.  */
static void
microblaze_bfd_write_imm_value_32 (bfd *abfd, bfd_byte *bfd_addr, bfd_vma val)
{
    const unsigned long IMM_MASK = 0x0000ffffUL;
    unsigned long instr;
    
    if (abfd == NULL || bfd_addr == NULL) {
        return;
    }
    
    instr = bfd_get_32 (abfd, bfd_addr);
    instr &= ~IMM_MASK;
    instr |= (val & IMM_MASK);
    bfd_put_32 (abfd, instr, bfd_addr);
}

/* Read-modify-write into the bfd, an immediate value into appropriate fields of
   two consecutive 32-bit instructions.  */
static void
microblaze_bfd_write_imm_value_64 (bfd *abfd, bfd_byte *bfd_addr, bfd_vma val)
{
    const unsigned long mask = 0x0000ffff;
    unsigned long instr_hi = bfd_get_32 (abfd, bfd_addr);
    unsigned long instr_lo = bfd_get_32 (abfd, bfd_addr + INST_WORD_SIZE);
    
    instr_hi = (instr_hi & ~mask) | ((val >> 16) & mask);
    instr_lo = (instr_lo & ~mask) | (val & mask);
    
    bfd_put_32 (abfd, instr_hi, bfd_addr);
    bfd_put_32 (abfd, instr_lo, bfd_addr + INST_WORD_SIZE);
}

static bool
microblaze_elf_relax_section (bfd *abfd,
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
  size_t i, sym_index;
  asection *o;
  struct elf_link_hash_entry *sym_hash;
  Elf_Internal_Sym *isymbuf, *isymend;
  Elf_Internal_Sym *isym;
  size_t symcount;
  size_t offset;
  bfd_vma src, dest;
  struct _microblaze_elf_section_data *sdata;

  *again = false;

  if (bfd_link_relocatable (link_info)
      || (sec->flags & SEC_RELOC) == 0
      || (sec->flags & SEC_CODE) == 0
      || sec->reloc_count == 0)
    return true;

  sdata = microblaze_elf_section_data (sec);
  if (sdata == NULL)
    return true;

  BFD_ASSERT ((sec->size > 0) || (sec->rawsize > 0));

  if (sec->size == 0)
    sec->size = sec->rawsize;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
  symcount = symtab_hdr->sh_size / sizeof (Elf32_External_Sym);
  if (isymbuf == NULL)
    {
      isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr, symcount,
                                      0, NULL, NULL, NULL);
      if (isymbuf == NULL)
        return false;
    }

  internal_relocs = _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL, link_info->keep_memory);
  if (internal_relocs == NULL)
    return false;
  if (!link_info->keep_memory)
    free_relocs = internal_relocs;

  sdata->relax_count = 0;
  sdata->relax = (struct relax_table *) bfd_malloc ((sec->reloc_count + 1)
                                                    * sizeof (*sdata->relax));
  if (sdata->relax == NULL)
    goto error_return;

  irelend = internal_relocs + sec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      if (!process_relaxable_relocation (abfd, sec, irel, symtab_hdr, 
                                        isymbuf, &contents, &free_contents, sdata))
        goto error_return;
    }

  if (sdata->relax_count > 0)
    {
      if (!adjust_relocations_and_symbols (abfd, sec, internal_relocs, irelend,
                                          isymbuf, symtab_hdr, sdata, contents))
        goto error_return;

      if (!update_other_sections (abfd, sec, isymbuf, symtab_hdr->sh_info))
        goto error_return;

      if (!compact_section_contents (sec, sdata, contents))
        goto error_return;

      elf_section_data (sec)->relocs = internal_relocs;
      free_relocs = NULL;
      elf_section_data (sec)->this_hdr.contents = contents;
      free_contents = NULL;
      symtab_hdr->contents = (bfd_byte *) isymbuf;
      *again = true;
    }
  else
    {
      free (sdata->relax);
      sdata->relax = NULL;
    }

  if (free_relocs != NULL)
    free (free_relocs);
  if (free_contents != NULL && !link_info->keep_memory)
    free (free_contents);
  else if (free_contents != NULL)
    elf_section_data (sec)->this_hdr.contents = contents;

  return true;

error_return:
  if (free_relocs != NULL)
    free (free_relocs);
  if (free_contents != NULL)
    free (free_contents);
  if (sdata->relax != NULL)
    {
      free (sdata->relax);
      sdata->relax = NULL;
    }
  sdata->relax_count = 0;
  return false;
}

static bool
process_relaxable_relocation (bfd *abfd, asection *sec, Elf_Internal_Rela *irel,
                             Elf_Internal_Shdr *symtab_hdr, Elf_Internal_Sym *isymbuf,
                             bfd_byte **contents, bfd_byte **free_contents,
                             struct _microblaze_elf_section_data *sdata)
{
  bfd_vma symval;
  int reloc_type = ELF32_R_TYPE (irel->r_info);

  if (reloc_type != R_MICROBLAZE_64_PCREL
      && reloc_type != R_MICROBLAZE_64
      && reloc_type != R_MICROBLAZE_TEXTREL_64)
    return true;

  if (*contents == NULL)
    {
      if (!load_section_contents (abfd, sec, contents, free_contents))
        return false;
    }

  if (!get_symbol_value (abfd, irel, symtab_hdr, isymbuf, &symval))
    return true;

  symval = adjust_symbol_value (irel, sec, symval, reloc_type);

  if ((symval & 0xffff8000) == 0 || (symval & 0xffff8000) == 0xffff8000)
    {
      sdata->relax[sdata->relax_count].addr = irel->r_offset;
      sdata->relax[sdata->relax_count].size = INST_WORD_SIZE;
      sdata->relax_count++;
      rewrite_relocation_type (irel, reloc_type);
    }

  return true;
}

static bool
load_section_contents (bfd *abfd, asection *sec, bfd_byte **contents, 
                      bfd_byte **free_contents)
{
  if (elf_section_data (sec)->this_hdr.contents != NULL)
    {
      *contents = elf_section_data (sec)->this_hdr.contents;
    }
  else
    {
      *contents = (bfd_byte *) bfd_malloc (sec->size);
      if (*contents == NULL)
        return false;
      *free_contents = *contents;
      if (!bfd_get_section_contents (abfd, sec, *contents, 0, sec->size))
        return false;
      elf_section_data (sec)->this_hdr.contents = *contents;
    }
  return true;
}

static bool
get_symbol_value (bfd *abfd, Elf_Internal_Rela *irel, 
                 Elf_Internal_Shdr *symtab_hdr, Elf_Internal_Sym *isymbuf,
                 bfd_vma *symval)
{
  if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
    {
      asection *sym_sec;
      Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irel->r_info);
      
      if (isym->st_shndx == SHN_UNDEF)
        sym_sec = bfd_und_section_ptr;
      else if (isym->st_shndx == SHN_ABS)
        sym_sec = bfd_abs_section_ptr;
      else if (isym->st_shndx == SHN_COMMON)
        sym_sec = bfd_com_section_ptr;
      else
        sym_sec = bfd_section_from_elf_index (abfd, isym->st_shndx);
      
      *symval = _bfd_elf_rela_local_sym (abfd, isym, &sym_sec, irel);
    }
  else
    {
      unsigned long indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
      struct elf_link_hash_entry *h = elf_sym_hashes (abfd)[indx];
      
      if (h == NULL)
        return false;
      
      if (h->root.type != bfd_link_hash_defined
          && h->root.type != bfd_link_hash_defweak)
        return false;
      
      *symval = h->root.u.def.value
                + h->root.u.def.section->output_section->vma
                + h->root.u.def.section->output_offset;
    }
  return true;
}

static bfd_vma
adjust_symbol_value (Elf_Internal_Rela *irel, asection *sec, 
                    bfd_vma symval, int reloc_type)
{
  if (reloc_type == R_MICROBLAZE_64_PCREL)
    {
      return symval + irel->r_addend
             - (irel->r_offset + sec->output_section->vma + sec->output_offset);
    }
  else if (reloc_type == R_MICROBLAZE_TEXTREL_64)
    {
      return symval + irel->r_addend - sec->output_section->vma;
    }
  return symval + irel->r_addend;
}

static void
rewrite_relocation_type (Elf_Internal_Rela *irel, int reloc_type)
{
  unsigned long sym = ELF32_R_SYM (irel->r_info);
  
  switch (reloc_type)
    {
    case R_MICROBLAZE_64_PCREL:
      irel->r_info = ELF32_R_INFO (sym, R_MICROBLAZE_32_PCREL_LO);
      break;
    case R_MICROBLAZE_64:
      irel->r_info = ELF32_R_INFO (sym, R_MICROBLAZE_32_LO);
      break;
    case R_MICROBLAZE_TEXTREL_64:
      irel->r_info = ELF32_R_INFO (sym, R_MICROBLAZE_TEXTREL_32_LO);
      break;
    default:
      BFD_ASSERT (false);
    }
}

static bool
adjust_relocations_and_symbols (bfd *abfd, asection *sec,
                               Elf_Internal_Rela *internal_relocs,
                               Elf_Internal_Rela *irelend,
                               Elf_Internal_Sym *isymbuf,
                               Elf_Internal_Shdr *symtab_hdr,
                               struct _microblaze_elf_section_data *sdata,
                               bfd_byte *contents)
{
  unsigned int shndx = _bfd_elf_section_from_bfd_section (abfd, sec);
  Elf_Internal_Sym *isymend;
  size_t sym_index, symcount;
  
  sdata->relax[sdata->relax_count].addr = sec->size;
  
  if (!adjust_internal_relocations (internal_relocs, irelend, isymbuf, 
                                   symtab_hdr, shndx, sec, contents))
    return false;
  
  isymend = isymbuf + symtab_hdr->sh_info;
  for (Elf_Internal_Sym *isym = isymbuf; isym < isymend; isym++)
    {
      if (isym->st_shndx == shndx)
        {
          isym->st_value -= calc_fixup (isym->st_value, 0, sec);
          if (isym->st_size)
            isym->st_size -= calc_fixup (isym->st_value, isym->st_size, sec);
        }
    }
  
  symcount = (symtab_hdr->sh_size / sizeof (Elf32_External_Sym)) - symtab_hdr->sh_info;
  for (sym_index = 0; sym_index < symcount; sym_index++)
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
  
  return true;
}

static bool
adjust_internal_relocations (Elf_Internal_Rela *internal_relocs,
                            Elf_Internal_Rela *irelend,
                            Elf_Internal_Sym *isymbuf,
                            Elf_Internal_Shdr *symtab_hdr,
                            unsigned int shndx,
                            asection *sec,
                            bfd_byte *contents)
{
  for (Elf_Internal_Rela *irel = internal_relocs; irel < irelend; irel++)
    {
      bfd_vma nraddr = irel->r_offset - calc_fixup (irel->r_offset, 0, sec);
      int reloc_type = ELF32_R_TYPE (irel->r_info);
      
      switch (reloc_type)
        {
        case R_MICROBLAZE_TEXTREL_64:
        case R_MICROBLAZE_TEXTREL_32_LO:
        case R_MICROBLAZE_64:
        case R_MICROBLAZE_32_LO:
          if (ELF32_R_SYM (irel->r_info) < symtab_hdr->sh_info)
            {
              Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irel->r_info);
              if (isym->st_shndx == shndx && ELF32_ST_TYPE (isym->st_info) == STT_SECTION)
                irel->r_addend -= calc_fixup (irel->r_addend, 0, sec);
            }
          break;
          
        case R_MICROBLAZE_NONE:
        case R_MICROBLAZE_32_NONE:
          handle_none_relocation (abfd, irel, sec, contents, false);
          break;
          
        case R_MICROBLAZE_64_NONE:
          handle_none_relocation (abfd, irel, sec, contents, true);
          break;
        }
      
      irel->r_offset = nraddr;
    }
  return true;
}

static void
handle_none_relocation (bfd *abfd, Elf_Internal_Rela *irel, 
                       asection *sec, bfd_byte *contents, bool is_64bit)
{
  size_t sfix, efix;
  bfd_vma target_address;
  
  if (is_64bit)
    {
      target_address = irel->r_addend + irel->r_offset + INST_WORD_SIZE;
      sfix = calc_fixup (irel->r_offset + INST_WORD_SIZE, 0, sec);
      efix = calc_fixup (target_address, 0, sec);
      irel->r_addend -= (efix - sfix);
      microblaze_bfd_write_imm_value_32 (abfd, contents + irel->r_offset + INST_WORD_SIZE,
                                        irel->r_addend);
    }
  else
    {
      target_address = irel->r_addend + irel->r_offset;
      sfix = calc_fixup (irel->r_offset, 0, sec);
      efix = calc_fixup (target_address, 0, sec);
      irel->r_addend -= (efix - sfix);
      microblaze_bfd_write_imm_value_32 (abfd, contents + irel->r_offset, irel->r_addend);
    }
}

static bool
update_other_sections (bfd *abfd, asection *sec, Elf_Internal_Sym *isymbuf,
                      unsigned int local_sym_count)
{
  unsigned int shndx = _bfd_elf_section_from_bfd_section (abfd, sec);
  
  for (asection *o = abfd->sections; o != NULL; o = o->next)
    {
      if (o == sec || (o->flags & SEC_RELOC) == 0 || o->reloc_count == 0)
        continue;
      
      if (!process_other_section_relocs (abfd, o, sec, isymbuf, shndx))
        return false;
    }
  return true;
}

static bool
process_other_section_relocs (bfd *abfd, asection *o, asection *target_sec,
                             Elf_Internal_Sym *isymbuf, unsigned int target_shndx)
{
  Elf_Internal_Rela *irelocs = _bfd_elf_link_read_relocs (abfd, o, NULL, NULL, true);
  if (irelocs == NULL)
    return false;
  
  bfd_byte *ocontents = NULL;
  Elf_Internal_Rela *irelscanend = irelocs + o->reloc_count;
  
  for (Elf_Internal_Rela *irelscan = irelocs; irelscan < irelscanend; irelscan++)
    {
      int reloc_type = ELF32_R_TYPE (irelscan->r_info);
      
      if (reloc_type == R_MICROBLAZE_32 || reloc_type == R_MICROBLAZE_32_NONE)
        {
          if (!handle_32_relocation (abfd, o, irelscan, isymbuf, target_shndx, 
                                   target_sec, &ocontents))
            return false;
        }
      else if (reloc_type == R_MICROBLAZE_32_SYM_OP_SYM)
        {
          if (!handle_sym_op_sym_relocation (abfd, o, irelscan, isymbuf, 
                                           target_sec, &ocontents))
            return false;
        }
      else if (reloc_type == R_MICROBLAZE_32_PCREL_LO
               || reloc_type == R_MICROBLAZE_32_LO
               || reloc_type == R_MICROBLAZE_TEXTREL_32_LO)
        {
          if (!handle_lo_relocation (abfd, o, irelscan, isymbuf, target_shndx,
                                   target_sec, &ocontents))
            return false;
        }
      else if (reloc_type == R_MICROBLAZE_64 || reloc_type == R_MICROBLAZE_TEXTREL_64)
        {
          if (!handle_64_relocation (abfd, o, irelscan, isymbuf, target_shndx,
                                   target_sec, &ocontents))
            return false;
        }
      else if (reloc_type == R_MICROBLAZE_64_PCREL)
        {
          if (!handle_64_pcrel_relocation (abfd, o, irelscan, isymbuf, target_shndx,
                                         target_sec, &ocontents))
            return false;
        }
    }
  
  return true;
}

static bool
handle_32_relocation (bfd *abfd, asection *o, Elf_Internal_Rela *irelscan,
                     Elf_Internal_Sym *isymbuf, unsigned int target_shndx,
                     asection *target_sec, bfd_byte **ocontents)
{
  Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irelscan->r_info);
  
  if (isym->st_shndx == target_shndx && ELF32_ST_TYPE (isym->st_info) == STT_SECTION)
    {
      if (*ocontents == NULL && !load_other_section_contents (abfd, o, ocontents))
        return false;
      irelscan->r_addend -= calc_fixup (irelscan->r_addend, 0, target_sec);
    }
  return true;
}

static bool
handle_sym_op_sym_relocation (bfd *abfd, asection *o, Elf_Internal_Rela *irelscan,
                             Elf_Internal_Sym *isymbuf, asection *target_sec,
                             bfd_byte **ocontents)
{
  Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irelscan->r_info);
  
  if (*ocontents == NULL && !load_other_section_contents (abfd, o, ocontents))
    return false;
  
  irelscan->r_addend -= calc_fixup (irelscan->r_addend + isym->st_value, 0, target_sec);
  return true;
}

static bool
handle_lo_relocation (bfd *abfd, asection *o, Elf_Internal_Rela *irelscan,
                     Elf_Internal_Sym *isymbuf, unsigned int target_shndx,
                     asection *target_sec, bfd_byte **ocontents)
{
  Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irelscan->r_info);
  
  if (isym->st_shndx == target_shndx && ELF32_ST_TYPE (isym->st_info) == STT_SECTION)
    {
      if (*ocontents == NULL && !load_other_section_contents (abfd, o, ocontents))
        return false;
      
      unsigned long instr = bfd_get_32 (abfd, *ocontents + irelscan->r_offset);
      bfd_vma immediate = instr & 0x0000ffff;
      size_t offset = calc_fixup (immediate, 0, target_sec);
      irelscan->r_addend -= offset;
      microblaze_bfd_write_imm_value_32 (abfd, *ocontents + irelscan->r_offset,
                                        irelscan->r_addend);
    }
  return true;
}

static bool
handle_64_relocation (bfd *abfd, asection *o, Elf_Internal_Rela *irelscan,
                     Elf_Internal_Sym *isymbuf, unsigned int target_shndx,
                     asection *target_sec, bfd_byte **ocontents)
{
  Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irelscan->r_info);
  
  if (isym->st_shndx == target_shndx && ELF32_ST_TYPE (isym->st_info) == STT_SECTION)
    {
      if (*ocontents == NULL && !load_other_section_contents (abfd, o, ocontents))
        return false;
      
      size_t offset = calc_fixup (irelscan->r_addend, 0, target_sec);
      irelscan->r_addend -= offset;
    }
  return true;
}

static bool
handle_64_pcrel_relocation (bfd *abfd, asection *o, Elf_Internal_Rela *irelscan,
                           Elf_Internal_Sym *isymbuf, unsigned int target_shndx,
                           asection *target_sec, bfd_byte **ocontents)
{
  Elf_Internal_Sym *isym = isymbuf + ELF32_R_SYM (irelscan->r_info);
  
  if (isym->st_shndx == target_shndx && ELF32_ST_TYPE (isym->st_info) == STT_SECTION)
    {
      if (*ocontents == NULL && !load_other_section_contents (abfd, o, ocontents))
        return false;
      
      unsigned long instr_hi = bfd_get_32 (abfd, *ocontents + irelscan->r_offset);
      unsigned long instr_lo = bfd_get_32 (abfd, *ocontents + irelscan->r_offset + INST_WORD_SIZE);
      bfd_vma immediate = ((instr_hi & 0x0000ffff) << 16) | (instr_lo & 0x0000ffff);
      size_t offset = calc_fixup (immediate, 0, target_sec);
      immediate -= offset;
      irelscan->r_addend -= offset;
      microblaze_bfd_write_imm_value_64 (abfd, *ocontents + irelscan->r_offset, immediate);
    }
  return true;
}

static bool
load_other_section_contents (bfd *abfd, asection *o, bfd_byte **ocontents)
{
  if (elf_section_data (o)->this_hdr.contents != NULL)
    {
      *ocontents = elf_section_data (o)->this_hdr.contents;
    }
  else
    {
      if (o->rawsize == 0)
        o->rawsize = o->size;
      *ocontents = (bfd_byte *) bfd_malloc (o->rawsize);
      if (*ocontents == NULL)
        return false;
      if (!bfd_get_section_contents (abfd, o, *ocontents, 0, o->rawsize))
        return false;
      elf_section_data (o)->this_hdr.contents = *ocontents;
    }
  return true;
}

static bool
compact_section_contents (asection *sec, struct _microblaze_elf_section_data *sdata,
                         bfd_byte *contents)
{
  bfd_vma dest = sdata->relax[0].addr;
  
  for (size_t i = 0; i < sdata->relax_count; i++)
    {
      bfd_vma src = sdata->relax[i].addr + sdata->relax[i].size;
      size_t len = sdata->relax[i + 1].addr - sdata->relax[i].addr - sdata->relax[i].size;
      
      memmove (contents + dest, contents + src, len);
      sec->size -= sdata->relax[i].size;
      dest += len;
    }
  
  return true;
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
      if (r_type == R_MICROBLAZE_GNU_VTINHERIT || 
          r_type == R_MICROBLAZE_GNU_VTENTRY)
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
  bfd_signed_vma *local_got_refcounts;
  unsigned char *local_got_tls_masks;
  bfd_size_type sym_count;
  bfd_size_type total_size;

  if (abfd == NULL || symtab_hdr == NULL)
    return false;

  sym_count = symtab_hdr->sh_info;
  if (r_symndx >= sym_count)
    return false;

  local_got_refcounts = elf_local_got_refcounts (abfd);

  if (local_got_refcounts == NULL)
    {
      if (sym_count == 0)
        return false;

      total_size = sym_count * sizeof (*local_got_refcounts);
      total_size += sym_count * sizeof (*local_got_tls_masks);

      local_got_refcounts = bfd_zalloc (abfd, total_size);
      if (local_got_refcounts == NULL)
        return false;

      elf_local_got_refcounts (abfd) = local_got_refcounts;
    }

  local_got_tls_masks = (unsigned char *) (local_got_refcounts + sym_count);
  local_got_tls_masks[r_symndx] |= tls_type;
  local_got_refcounts[r_symndx] += 1;

  return true;
}
/* Look through the relocs for a section during the first phase.  */

static bool
microblaze_elf_check_relocs (bfd * abfd,
			     struct bfd_link_info * info,
			     asection * sec,
			     const Elf_Internal_Rela * relocs)
{
  Elf_Internal_Shdr *		symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  const Elf_Internal_Rela *	rel;
  const Elf_Internal_Rela *	rel_end;
  struct elf32_mb_link_hash_table *htab;
  asection *sreloc = NULL;

  if (bfd_link_relocatable (info))
    return true;

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;

  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      struct elf_link_hash_entry * h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	{
	  h = sym_hashes [r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}

      if (!process_relocation_type(abfd, info, sec, rel, h, r_type, r_symndx, 
                                   symtab_hdr, htab, &sreloc))
        return false;
    }

  return true;
}

static bool
process_relocation_type(bfd *abfd, struct bfd_link_info *info, asection *sec,
                        const Elf_Internal_Rela *rel, struct elf_link_hash_entry *h,
                        unsigned int r_type, unsigned long r_symndx,
                        Elf_Internal_Shdr *symtab_hdr,
                        struct elf32_mb_link_hash_table *htab,
                        asection **sreloc)
{
  switch (r_type)
    {
    case R_MICROBLAZE_GNU_VTINHERIT:
      return bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset);

    case R_MICROBLAZE_GNU_VTENTRY:
      return bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend);

    case R_MICROBLAZE_PLT_64:
      return handle_plt_relocation(h);

    case R_MICROBLAZE_TLSGD:
      return handle_tls_relocation(abfd, info, sec, h, r_symndx, symtab_hdr, 
                                   htab, TLS_TLS | TLS_GD);

    case R_MICROBLAZE_TLSLD:
      return handle_tls_relocation(abfd, info, sec, h, r_symndx, symtab_hdr,
                                   htab, TLS_TLS | TLS_LD);

    case R_MICROBLAZE_GOT_64:
      return handle_got_relocation(abfd, info, h, r_symndx, symtab_hdr, htab, 0);

    case R_MICROBLAZE_GOTOFF_64:
    case R_MICROBLAZE_GOTOFF_32:
      return ensure_got_section(abfd, info, htab);

    case R_MICROBLAZE_64:
    case R_MICROBLAZE_64_PCREL:
    case R_MICROBLAZE_32:
      return handle_data_relocation(abfd, info, sec, h, r_type, r_symndx,
                                    symtab_hdr, htab, sreloc);

    default:
      break;
    }
  return true;
}

static bool
handle_plt_relocation(struct elf_link_hash_entry *h)
{
  if (h != NULL)
    {
      h->needs_plt = 1;
      h->plt.refcount += 1;
    }
  return true;
}

static bool
handle_tls_relocation(bfd *abfd, struct bfd_link_info *info, asection *sec,
                      struct elf_link_hash_entry *h, unsigned long r_symndx,
                      Elf_Internal_Shdr *symtab_hdr,
                      struct elf32_mb_link_hash_table *htab,
                      unsigned char tls_type)
{
  sec->has_tls_reloc = 1;
  return handle_got_relocation(abfd, info, h, r_symndx, symtab_hdr, htab, tls_type);
}

static bool
handle_got_relocation(bfd *abfd, struct bfd_link_info *info,
                      struct elf_link_hash_entry *h, unsigned long r_symndx,
                      Elf_Internal_Shdr *symtab_hdr,
                      struct elf32_mb_link_hash_table *htab,
                      unsigned char tls_type)
{
  if (!ensure_got_section(abfd, info, htab))
    return false;

  if (h != NULL)
    {
      h->got.refcount += 1;
      if (tls_type != 0)
        elf32_mb_hash_entry (h)->tls_mask |= tls_type;
    }
  else
    {
      if (!update_local_sym_info(abfd, symtab_hdr, r_symndx, tls_type))
        return false;
    }
  return true;
}

static bool
ensure_got_section(bfd *abfd, struct bfd_link_info *info,
                  struct elf32_mb_link_hash_table *htab)
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
needs_dynamic_reloc(struct bfd_link_info *info, asection *sec,
                    struct elf_link_hash_entry *h, unsigned int r_type)
{
  if ((sec->flags & SEC_ALLOC) == 0)
    return false;

  if (bfd_link_pic (info))
    {
      if (r_type == R_MICROBLAZE_64_PCREL && h == NULL)
        return false;
      if (h != NULL && info->symbolic && h->def_regular &&
          h->root.type != bfd_link_hash_defweak)
        return false;
      return true;
    }

  return h != NULL && (h->root.type == bfd_link_hash_defweak || !h->def_regular);
}

static bool
handle_data_relocation(bfd *abfd, struct bfd_link_info *info, asection *sec,
                      struct elf_link_hash_entry *h, unsigned int r_type,
                      unsigned long r_symndx, Elf_Internal_Shdr *symtab_hdr,
                      struct elf32_mb_link_hash_table *htab,
                      asection **sreloc)
{
  if (h != NULL && !bfd_link_pic (info))
    {
      h->non_got_ref = 1;
      h->plt.refcount += 1;
      if (r_type != R_MICROBLAZE_64_PCREL)
        h->pointer_equality_needed = 1;
    }

  if (!needs_dynamic_reloc(info, sec, h, r_type))
    return true;

  if (*sreloc == NULL)
    {
      if (!create_dynamic_reloc_section(abfd, sec, htab, sreloc))
        return false;
    }

  return add_dynamic_reloc_entry(abfd, sec, h, r_symndx, r_type,
                                 symtab_hdr, htab);
}

static bool
create_dynamic_reloc_section(bfd *abfd, asection *sec,
                             struct elf32_mb_link_hash_table *htab,
                             asection **sreloc)
{
  bfd *dynobj;

  if (htab->elf.dynobj == NULL)
    htab->elf.dynobj = abfd;
  dynobj = htab->elf.dynobj;

  *sreloc = _bfd_elf_make_dynamic_reloc_section (sec, dynobj, 2, abfd, 1);
  return *sreloc != NULL;
}

static bool
add_dynamic_reloc_entry(bfd *abfd, asection *sec, struct elf_link_hash_entry *h,
                        unsigned long r_symndx, unsigned int r_type,
                        Elf_Internal_Shdr *symtab_hdr,
                        struct elf32_mb_link_hash_table *htab)
{
  struct elf_dyn_relocs *p;
  struct elf_dyn_relocs **head;

  if (h != NULL)
    {
      head = &h->dyn_relocs;
    }
  else
    {
      head = get_local_dynrel_head(abfd, r_symndx, symtab_hdr, htab);
      if (head == NULL)
        return false;
    }

  p = *head;
  if (p == NULL || p->sec != sec)
    {
      p = allocate_dynrel_entry(htab, head, sec);
      if (p == NULL)
        return false;
    }

  p->count += 1;
  if (r_type == R_MICROBLAZE_64_PCREL)
    p->pc_count += 1;

  return true;
}

static struct elf_dyn_relocs **
get_local_dynrel_head(bfd *abfd, unsigned long r_symndx,
                     Elf_Internal_Shdr *symtab_hdr,
                     struct elf32_mb_link_hash_table *htab)
{
  asection *s;
  Elf_Internal_Sym *isym;
  void *vpp;

  isym = bfd_sym_from_r_symndx (&htab->elf.sym_cache, abfd, r_symndx);
  if (isym == NULL)
    return NULL;

  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
  if (s == NULL)
    return NULL;

  vpp = &elf_section_data (s)->local_dynrel;
  return (struct elf_dyn_relocs **) vpp;
}

static struct elf_dyn_relocs *
allocate_dynrel_entry(struct elf32_mb_link_hash_table *htab,
                     struct elf_dyn_relocs **head, asection *sec)
{
  struct elf_dyn_relocs *p;
  size_t amt = sizeof *p;

  p = (struct elf_dyn_relocs *) bfd_alloc (htab->elf.dynobj, amt);
  if (p == NULL)
    return NULL;

  p->next = *head;
  *head = p;
  p->sec = sec;
  p->count = 0;
  p->pc_count = 0;

  return p;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
microblaze_elf_copy_indirect_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *dir,
				     struct elf_link_hash_entry *ind)
{
  struct elf32_mb_link_hash_entry *edir;
  struct elf32_mb_link_hash_entry *eind;

  if (dir == NULL || ind == NULL || info == NULL) {
    return;
  }

  edir = (struct elf32_mb_link_hash_entry *) dir;
  eind = (struct elf32_mb_link_hash_entry *) ind;

  if (edir != NULL && eind != NULL) {
    edir->tls_mask |= eind->tls_mask;
  }

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

static bool
microblaze_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
				      struct elf_link_hash_entry *h)
{
  struct elf32_mb_link_hash_table *htab;
  asection *s, *srel;
  unsigned int power_of_two;

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  if (h->type == STT_FUNC || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || SYMBOL_CALLS_LOCAL (info, h)
	  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
	      && h->root.type == bfd_link_hash_undefweak))
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
      if (def == NULL || def->root.type != bfd_link_hash_defined)
        return false;
      h->root.u.def.section = def->root.u.def.section;
      h->root.u.def.value = def->root.u.def.value;
      return true;
    }

  if (bfd_link_pic (info))
    return true;

  if (!h->non_got_ref)
    return true;

  if (info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return true;
    }

  if (!_bfd_elf_readonly_dynrelocs (h))
    {
      h->non_got_ref = 0;
      return true;
    }

  if ((h->root.u.def.section->flags & SEC_READONLY) != 0)
    {
      s = htab->elf.sdynrelro;
      srel = htab->elf.sreldynrelro;
    }
  else
    {
      s = htab->elf.sdynbss;
      srel = htab->elf.srelbss;
    }

  if (s == NULL || srel == NULL)
    return false;

  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      srel->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  power_of_two = bfd_log2 (h->size);
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
allocate_dynrelocs (struct elf_link_hash_entry *h, void * dat)
{
  struct bfd_link_info *info;
  struct elf32_mb_link_hash_table *htab;
  struct elf32_mb_link_hash_entry *eh;
  struct elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return true;

  info = (struct bfd_link_info *) dat;
  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  if (!allocate_plt_entries(h, info, htab))
    return false;

  if (!allocate_got_entries(h, info, htab))
    return false;

  if (!process_dynamic_relocs(h, info, htab))
    return false;

  allocate_reloc_space(h);

  return true;
}

static bool
allocate_plt_entries(struct elf_link_hash_entry *h, 
                     struct bfd_link_info *info,
                     struct elf32_mb_link_hash_table *htab)
{
  if (!htab->elf.dynamic_sections_created || h->plt.refcount <= 0)
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
      return true;
    }

  if (!ensure_dynamic_symbol(h, info))
    return false;

  if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, bfd_link_pic (info), h))
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
      return true;
    }

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

  return true;
}

static bool
allocate_got_entries(struct elf_link_hash_entry *h,
                     struct bfd_link_info *info,
                     struct elf32_mb_link_hash_table *htab)
{
  struct elf32_mb_link_hash_entry *eh = (struct elf32_mb_link_hash_entry *) h;
  
  if (h->got.refcount <= 0)
    {
      h->got.offset = (bfd_vma) -1;
      return true;
    }

  if (!ensure_dynamic_symbol(h, info))
    return false;

  unsigned int need = calculate_got_size(eh, htab);
  
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

static unsigned int
calculate_got_size(struct elf32_mb_link_hash_entry *eh,
                   struct elf32_mb_link_hash_table *htab)
{
  unsigned int need = 0;
  
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
  
  return need;
}

static bool
ensure_dynamic_symbol(struct elf_link_hash_entry *h,
                      struct bfd_link_info *info)
{
  if (h->dynindx == -1 && !h->forced_local)
    {
      if (!bfd_elf_link_record_dynamic_symbol (info, h))
        return false;
    }
  return true;
}

static bool
process_dynamic_relocs(struct elf_link_hash_entry *h,
                       struct bfd_link_info *info,
                       struct elf32_mb_link_hash_table *htab)
{
  if (h->dyn_relocs == NULL)
    return true;

  if (bfd_link_pic (info))
    {
      process_shared_relocs(h, info);
    }
  else
    {
      if (!process_non_shared_relocs(h, info, htab))
        return false;
    }

  return true;
}

static void
process_shared_relocs(struct elf_link_hash_entry *h,
                      struct bfd_link_info *info)
{
  if (h->def_regular && (h->forced_local || info->symbolic))
    {
      struct elf_dyn_relocs **pp;
      struct elf_dyn_relocs *p;

      for (pp = &h->dyn_relocs; (p = *pp) != NULL; )
        {
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
process_non_shared_relocs(struct elf_link_hash_entry *h,
                          struct bfd_link_info *info,
                          struct elf32_mb_link_hash_table *htab)
{
  if (should_keep_relocs(h, htab))
    {
      if (!ensure_dynamic_symbol(h, info))
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
should_keep_relocs(struct elf_link_hash_entry *h,
                   struct elf32_mb_link_hash_table *htab)
{
  return !h->non_got_ref &&
         ((h->def_dynamic && !h->def_regular) ||
          (htab->elf.dynamic_sections_created &&
           (h->root.type == bfd_link_hash_undefweak ||
            h->root.type == bfd_link_hash_undefined)));
}

static void
allocate_reloc_space(struct elf_link_hash_entry *h)
{
  struct elf_dyn_relocs *p;
  
  for (p = h->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }
}

/* Set the sizes of the dynamic sections.  */

static bool
microblaze_elf_late_size_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				   struct bfd_link_info *info)
{
  struct elf32_mb_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd *ibfd;

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->elf.dynobj;
  if (dynobj == NULL)
    return true;

  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      if (!process_input_bfd_sections (ibfd, info))
	return false;
      
      if (!process_local_got_entries (ibfd, htab, info))
	return false;
    }

  elf_link_hash_traverse (elf_hash_table (info), allocate_dynrelocs, info);

  if (!allocate_tlsld_got (htab, info))
    return false;

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      if (htab->elf.splt->size > 0)
	htab->elf.splt->size += 4;
    }

  if (!allocate_dynamic_sections (dynobj, htab))
    return false;

  info->flags |= DF_BIND_NOW;
  return _bfd_elf_add_dynamic_tags (output_bfd, info, true);
}

static bool
process_input_bfd_sections (bfd *ibfd, struct bfd_link_info *info)
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
	  if (!process_dynreloc (p, info))
	    return false;
	}
    }
  return true;
}

static bool
process_dynreloc (struct elf_dyn_relocs *p, struct bfd_link_info *info)
{
  asection *srel;

  if (bfd_is_abs_section (p->sec) || 
      bfd_is_abs_section (p->sec->output_section))
    return true;

  if (p->count == 0)
    return true;

  srel = elf_section_data (p->sec)->sreloc;
  if (srel == NULL)
    return false;

  srel->size += p->count * sizeof (Elf32_External_Rela);
  
  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
    info->flags |= DF_TEXTREL;

  return true;
}

static bool
process_local_got_entries (bfd *ibfd, struct elf32_mb_link_hash_table *htab,
			   struct bfd_link_info *info)
{
  bfd_signed_vma *local_got;
  bfd_signed_vma *end_local_got;
  bfd_size_type locsymcount;
  Elf_Internal_Shdr *symtab_hdr;
  unsigned char *lgot_masks;
  asection *s;
  asection *srel;

  local_got = elf_local_got_refcounts (ibfd);
  if (!local_got)
    return true;

  symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
  locsymcount = symtab_hdr->sh_info;
  end_local_got = local_got + locsymcount;
  lgot_masks = (unsigned char *) end_local_got;
  s = htab->elf.sgot;
  srel = htab->elf.srelgot;

  if (s == NULL || srel == NULL)
    return false;

  for (; local_got < end_local_got; ++local_got, ++lgot_masks)
    {
      if (*local_got <= 0)
	{
	  *local_got = (bfd_vma) -1;
	  continue;
	}

      unsigned int need = calculate_got_need (*lgot_masks, htab);
      
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
  return true;
}

static unsigned int
calculate_got_need (unsigned char lgot_mask, struct elf32_mb_link_hash_table *htab)
{
  if ((lgot_mask & TLS_TLS) == 0)
    return 4;

  unsigned int need = 0;
  
  if ((lgot_mask & TLS_GD) != 0)
    need += 8;
  
  if ((lgot_mask & TLS_LD) != 0)
    htab->tlsld_got.refcount += 1;

  return need;
}

static bool
allocate_tlsld_got (struct elf32_mb_link_hash_table *htab,
		   struct bfd_link_info *info)
{
  if (htab->tlsld_got.refcount <= 0)
    {
      htab->tlsld_got.offset = (bfd_vma) -1;
      return true;
    }

  if (htab->elf.sgot == NULL || htab->elf.srelgot == NULL)
    return false;

  htab->tlsld_got.offset = htab->elf.sgot->size;
  htab->elf.sgot->size += 8;
  
  if (bfd_link_pic (info))
    htab->elf.srelgot->size += sizeof (Elf32_External_Rela);

  return true;
}

static bool
allocate_dynamic_sections (bfd *dynobj, struct elf32_mb_link_hash_table *htab)
{
  asection *s;

  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (!should_allocate_section (s, htab))
	continue;

      if (should_strip_section (s))
	{
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if (startswith (bfd_section_name (s), ".rela") && s->size != 0)
	s->reloc_count = 0;

      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL && s->size != 0)
	return false;
      s->alloced = 1;
    }
  return true;
}

static bool
should_allocate_section (asection *s, struct elf32_mb_link_hash_table *htab)
{
  const char *name = bfd_section_name (s);
  
  if (startswith (name, ".rela"))
    return true;
    
  return (s == htab->elf.splt ||
	  s == htab->elf.sgot ||
	  s == htab->elf.sgotplt ||
	  s == htab->elf.sdynbss ||
	  s == htab->elf.sdynrelro);
}

static bool
should_strip_section (asection *s)
{
  const char *name = bfd_section_name (s);
  return startswith (name, ".rela") && s->size == 0;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bool
microblaze_elf_finish_dynamic_symbol (bfd *output_bfd,
				      struct bfd_link_info *info,
				      struct elf_link_hash_entry *h,
				      Elf_Internal_Sym *sym)
{
  struct elf32_mb_link_hash_table *htab;
  struct elf32_mb_link_hash_entry *eh;

  if (!output_bfd || !info || !h || !sym)
    return false;

  htab = elf32_mb_hash_table (info);
  if (!htab)
    return false;

  eh = elf32_mb_hash_entry(h);

  if (h->plt.offset != (bfd_vma) -1)
    {
      if (!microblaze_finish_plt_entry(output_bfd, info, h, sym, htab))
        return false;
    }

  if ((h->got.offset != (bfd_vma) -1)
      && !(h->got.offset & 1)
      && !IS_TLS_LD(eh->tls_mask) 
      && !IS_TLS_GD(eh->tls_mask))
    {
      if (!microblaze_finish_got_entry(output_bfd, info, h, htab))
        return false;
    }

  if (h->needs_copy)
    {
      if (!microblaze_finish_copy_reloc(output_bfd, h, htab))
        return false;
    }

  if (h == htab->elf.hdynamic
      || h == htab->elf.hgot
      || h == htab->elf.hplt)
    sym->st_shndx = SHN_ABS;

  return true;
}

static bool
microblaze_finish_plt_entry(bfd *output_bfd,
                            struct bfd_link_info *info,
                            struct elf_link_hash_entry *h,
                            Elf_Internal_Sym *sym,
                            struct elf32_mb_link_hash_table *htab)
{
  asection *splt;
  asection *srela;
  asection *sgotplt;
  Elf_Internal_Rela rela;
  bfd_byte *loc;
  bfd_vma plt_index;
  bfd_vma got_offset;
  bfd_vma got_addr;

  if (h->dynindx == -1)
    return false;

  splt = htab->elf.splt;
  srela = htab->elf.srelplt;
  sgotplt = htab->elf.sgotplt;
  
  if (!splt || !srela || !sgotplt)
    return false;

  plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
  got_offset = (plt_index + 3) * 4;
  got_addr = got_offset;

  if (!bfd_link_pic (info))
    got_addr += sgotplt->output_section->vma + sgotplt->output_offset;

  bfd_put_32 (output_bfd, PLT_ENTRY_WORD_0 + ((got_addr >> 16) & 0xffff),
              splt->contents + h->plt.offset);
  
  if (bfd_link_pic (info))
    bfd_put_32 (output_bfd, PLT_ENTRY_WORD_1 + (got_addr & 0xffff),
                splt->contents + h->plt.offset + 4);
  else
    bfd_put_32 (output_bfd, PLT_ENTRY_WORD_1_NOPIC + (got_addr & 0xffff),
                splt->contents + h->plt.offset + 4);
  
  bfd_put_32 (output_bfd, (bfd_vma) PLT_ENTRY_WORD_2,
              splt->contents + h->plt.offset + 8);
  bfd_put_32 (output_bfd, (bfd_vma) PLT_ENTRY_WORD_3,
              splt->contents + h->plt.offset + 12);

  rela.r_offset = (sgotplt->output_section->vma
                   + sgotplt->output_offset
                   + got_offset);
  rela.r_info = ELF32_R_INFO (h->dynindx, R_MICROBLAZE_JUMP_SLOT);
  rela.r_addend = 0;
  loc = srela->contents;
  loc += plt_index * sizeof (Elf32_External_Rela);
  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

  if (!h->def_regular)
    {
      sym->st_shndx = SHN_UNDEF;
      sym->st_value = 0;
    }

  return true;
}

static bool
microblaze_finish_got_entry(bfd *output_bfd,
                            struct bfd_link_info *info,
                            struct elf_link_hash_entry *h,
                            struct elf32_mb_link_hash_table *htab)
{
  asection *sgot;
  asection *srela;
  bfd_vma offset;

  sgot = htab->elf.sgot;
  srela = htab->elf.srelgot;
  
  if (!sgot || !srela)
    return false;

  offset = (sgot->output_section->vma + sgot->output_offset
            + (h->got.offset &~ (bfd_vma) 1));

  if (bfd_link_pic (info)
      && ((info->symbolic && h->def_regular)
          || h->dynindx == -1))
    {
      asection *sec = h->root.u.def.section;
      bfd_vma value;

      value = h->root.u.def.value;
      if (sec->output_section != NULL)
        value += sec->output_section->vma + sec->output_offset;

      microblaze_elf_output_dynamic_relocation (output_bfd,
                                                srela, srela->reloc_count++,
                                                0,
                                                R_MICROBLAZE_REL, offset,
                                                value);
    }
  else
    {
      microblaze_elf_output_dynamic_relocation (output_bfd,
                                                srela, srela->reloc_count++,
                                                h->dynindx,
                                                R_MICROBLAZE_GLOB_DAT,
                                                offset, 0);
    }

  bfd_put_32 (output_bfd, (bfd_vma) 0,
              sgot->contents + (h->got.offset &~ (bfd_vma) 1));

  return true;
}

static bool
microblaze_finish_copy_reloc(bfd *output_bfd,
                             struct elf_link_hash_entry *h,
                             struct elf32_mb_link_hash_table *htab)
{
  asection *s;
  Elf_Internal_Rela rela;
  bfd_byte *loc;

  if (h->dynindx == -1)
    return false;

  rela.r_offset = (h->root.u.def.value
                   + h->root.u.def.section->output_section->vma
                   + h->root.u.def.section->output_offset);
  rela.r_info = ELF32_R_INFO (h->dynindx, R_MICROBLAZE_COPY);
  rela.r_addend = 0;
  
  if (h->root.u.def.section == htab->elf.sdynrelro)
    s = htab->elf.sreldynrelro;
  else
    s = htab->elf.srelbss;
  
  if (!s)
    return false;
    
  loc = s->contents + s->reloc_count++ * sizeof (Elf32_External_Rela);
  bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

  return true;
}


/* Finish up the dynamic sections.  */

static bool
microblaze_elf_finish_dynamic_sections (bfd *output_bfd,
					struct bfd_link_info *info)
{
  struct elf32_mb_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;
  asection *sgot;

  htab = elf32_mb_hash_table (info);
  if (htab == NULL)
    return false;

  dynobj = htab->elf.dynobj;
  if (dynobj == NULL)
    return false;

  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      if (!microblaze_update_dynamic_entries(output_bfd, dynobj, sdyn, htab))
        return false;
      
      if (!microblaze_init_plt_section(output_bfd, htab, sdyn))
        return false;
    }

  sgot = htab->elf.sgotplt;
  if (sgot != NULL && sgot->size > 0)
    {
      bfd_vma got_value = 0;
      if (sdyn != NULL)
        got_value = sdyn->output_section->vma + sdyn->output_offset;
      
      bfd_put_32 (output_bfd, got_value, sgot->contents);
      elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;
    }

  if (htab->elf.sgot != NULL && htab->elf.sgot->size > 0)
    elf_section_data (htab->elf.sgot->output_section)->this_hdr.sh_entsize = 4;

  return true;
}

static bool
microblaze_update_dynamic_entries(bfd *output_bfd, bfd *dynobj, 
                                  asection *sdyn, 
                                  struct elf32_mb_link_hash_table *htab)
{
  Elf32_External_Dyn *dyncon;
  Elf32_External_Dyn *dynconend;

  if (sdyn == NULL || sdyn->contents == NULL)
    return true;

  dyncon = (Elf32_External_Dyn *) sdyn->contents;
  dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
  
  for (; dyncon < dynconend; dyncon++)
    {
      Elf_Internal_Dyn dyn;
      bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);
      
      if (!microblaze_process_dyn_entry(output_bfd, htab, &dyn, dyncon))
        return false;
    }
  
  return true;
}

static bool
microblaze_process_dyn_entry(bfd *output_bfd, 
                             struct elf32_mb_link_hash_table *htab,
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
      return true;
    }

  if (s == NULL)
    {
      dyn->d_un.d_val = 0;
    }
  else if (use_size)
    {
      dyn->d_un.d_val = s->size;
    }
  else
    {
      dyn->d_un.d_ptr = s->output_section->vma + s->output_offset;
    }
  
  bfd_elf32_swap_dyn_out (output_bfd, dyn, dyncon);
  return true;
}

static bool
microblaze_init_plt_section(bfd *output_bfd, 
                            struct elf32_mb_link_hash_table *htab,
                            asection *sdyn)
{
  asection *splt;

  splt = htab->elf.splt;
  if (splt == NULL || sdyn == NULL)
    return false;

  if (splt->size > 0)
    {
      memset (splt->contents, 0, PLT_ENTRY_SIZE);
      bfd_put_32 (output_bfd, (bfd_vma) 0x80000000,
                  splt->contents + splt->size - 4);

      if (splt->output_section != bfd_abs_section_ptr)
        elf_section_data (splt->output_section)->this_hdr.sh_entsize = 4;
    }
  
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
  if (sym == NULL || secp == NULL || valp == NULL || abfd == NULL || info == NULL)
    return false;

  if (sym->st_shndx != SHN_COMMON)
    return true;

  if (bfd_link_relocatable (info))
    return true;

  if (sym->st_size > elf_gp_size (abfd))
    return true;

  *secp = bfd_make_section_old_way (abfd, ".sbss");
  if (*secp == NULL)
    return false;

  if (!bfd_set_section_flags (*secp, SEC_IS_COMMON | SEC_SMALL_DATA))
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
