/*
 *  File: binpatch.c
 *
 *  Description: Modern Linux versions generate an extra header in the boot image called GNU_STACK.
 *  This stack is used to instruct modern processors to protect certain memory areas
 *  from code being executed, where only data is meant to be loaded in.
 *
 *  The IBM Netvista 8363 is not aware of this capability and the BIOS loader crashes
 *  when it engulfs an extra header from the kernel image file.
 *
 *  This tiny program patches de kernel image to strip this extra header off the image.
 *
 *  For a detailed explanation please visit the following sites:
 *
 *    http://www.gentoo.org/proj/en/hardened/gnu-stack.xml
 *    http://en.wikipedia.org/wiki/NX_bit
 *
 * puspus - a GNU/Debian based creation of a distribution for the IBM Netvista model 8363
 * Copyright (C) 2008, Albert Casals
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *  USA
 *
 *
 */

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])
{
  unsigned char *patchme=NULL;
  unsigned long j=0, k=0;
  char chFilename[1024];
  signed int exit_code=-1;
  FILE *fp=NULL;

  if (argc < 2) {
    printf ("What kernel image do you want me to patch?\n");
    exit_code = -1;
  }
  else {
    strcpy (chFilename, argv[1]);

    fp = fopen (chFilename, "rb+");
    if (!fp) {
      printf ("Could not find kernel image: %s\n", chFilename);
      exit_code = -2;
    }
    else {

      /* TODO: 4MB is enough for now, but we should ask its file size and allocate a proper buffer */
      patchme = (unsigned char *) calloc (4, (1024 * 1024));
      j = fread (patchme, sizeof (unsigned char), 4 * (1024 * 1024), fp);

      /* TODO: include elf.h to manage the kernel image file in a more pleasant way */

      if (patchme[0x2c] == 0x01) {
	printf ("This kernel image is already patched\n");
	fclose (fp);
	exit_code = 0;
      }
      else {
	patchme[0x2c] = 0x01;
	rewind (fp);
	k = fwrite (patchme, sizeof (unsigned char), j, fp);
	fclose (fp);

	if (k == j) {
	  printf ("Kernel image succesfully patched (%ld-%ld bytes read/write)\n", j, k);
	  exit_code = 0;
	}
	else {
	  printf ("There was a problem writing the new kernel image! (bytes read=%ld, written=%ld)\n", j, k);
	  exit_code = -3;
	}
      } 
    }
  }

  exit (exit_code);
}
