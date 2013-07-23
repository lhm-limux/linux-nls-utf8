/* guniprop.c - Unicode character properties.
 *
 * Copyright (C) 1999 Tom Tromey
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>

#include "gunicode.h"

int main(int argc, char **argv)
{
  gchar *in = "aBC";
  gchar *out;
  if (argc == 2)
    in = argv[1];

  out = h_utf8_casefold(in, -1);
  printf( "Fold - In: %s\nOut: %s\n", in, out );
  g_free( out );

  out = h_utf8_strup(in, -1);
  printf( "Up - In: %s\nOut: %s\n", in, out );
  g_free( out );

  out = h_utf8_strdown(in, -1);
  printf( "Down - In: %s\nOut: %s\n", in, out );
  g_free( out );
  return 0;
}

