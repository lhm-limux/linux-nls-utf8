/* gunicode.h - Unicode manipulation functions
 *
 *  Copyright (C) 1999, 2000 Tom Tromey
 *  Copyright 2000, 2005 Red Hat, Inc.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.
 */

#ifndef __H_UNICODE_H__
#define __H_UNICODE_H__

#ifdef __KERNEL__

#include <linux/slab.h>
#define g_malloc(size) kmalloc(size, GFP_KERNEL)
#define g_free kfree

#else

#include <stdlib.h>
#define g_malloc malloc
#define g_free free

#endif

#include "gtypes.h"

/**
 * gunichar:
 *
 * A type which can hold any UTF-32 or UCS-4 character code,
 * also known as a Unicode code point.
 *
 * If you want to produce the UTF-8 representation of a #gunichar,
 * use g_ucs4_to_utf8(). See also g_utf8_to_ucs4() for the reverse
 * process.
 *
 * To print/scan values of this type as integer, use
 * %G_GINT32_MODIFIER and/or %G_GUINT32_FORMAT.
 *
 * The notation to express a Unicode code point in running text is
 * as a hexadecimal number with four to six digits and uppercase
 * letters, prefixed by the string "U+". Leading zeros are omitted,
 * unless the code point would have fewer than four hexadecimal digits.
 * For example, "U+0041 LATIN CAPITAL LETTER A". To print a code point
 * in the U+-notation, use the format string "U+\%04"G_GINT32_FORMAT"X".
 * To scan, use the format string "U+\%06"G_GINT32_FORMAT"X".
 *
 * |[
 * gunichar c;
 * sscanf ("U+0041", "U+%06"G_GINT32_FORMAT"X", &amp;c)
 * g_print ("Read U+%04"G_GINT32_FORMAT"X", c);
 * ]|
 */
typedef guint32 gunichar;

/**
 * GUnicodeType:
 * @G_UNICODE_CONTROL: General category "Other, Control" (Cc)
 * @G_UNICODE_FORMAT: General category "Other, Format" (Cf)
 * @G_UNICODE_UNASSIGNED: General category "Other, Not Assigned" (Cn)
 * @G_UNICODE_PRIVATE_USE: General category "Other, Private Use" (Co)
 * @G_UNICODE_SURROGATE: General category "Other, Surrogate" (Cs)
 * @G_UNICODE_LOWERCASE_LETTER: General category "Letter, Lowercase" (Ll)
 * @G_UNICODE_MODIFIER_LETTER: General category "Letter, Modifier" (Lm)
 * @G_UNICODE_OTHER_LETTER: General category "Letter, Other" (Lo)
 * @G_UNICODE_TITLECASE_LETTER: General category "Letter, Titlecase" (Lt)
 * @G_UNICODE_UPPERCASE_LETTER: General category "Letter, Uppercase" (Lu)
 * @G_UNICODE_SPACING_MARK: General category "Mark, Spacing" (Mc)
 * @G_UNICODE_ENCLOSING_MARK: General category "Mark, Enclosing" (Me)
 * @G_UNICODE_NON_SPACING_MARK: General category "Mark, Nonspacing" (Mn)
 * @G_UNICODE_DECIMAL_NUMBER: General category "Number, Decimal Digit" (Nd)
 * @G_UNICODE_LETTER_NUMBER: General category "Number, Letter" (Nl)
 * @G_UNICODE_OTHER_NUMBER: General category "Number, Other" (No)
 * @G_UNICODE_CONNECT_PUNCTUATION: General category "Punctuation, Connector" (Pc)
 * @G_UNICODE_DASH_PUNCTUATION: General category "Punctuation, Dash" (Pd)
 * @G_UNICODE_CLOSE_PUNCTUATION: General category "Punctuation, Close" (Pe)
 * @G_UNICODE_FINAL_PUNCTUATION: General category "Punctuation, Final quote" (Pf)
 * @G_UNICODE_INITIAL_PUNCTUATION: General category "Punctuation, Initial quote" (Pi)
 * @G_UNICODE_OTHER_PUNCTUATION: General category "Punctuation, Other" (Po)
 * @G_UNICODE_OPEN_PUNCTUATION: General category "Punctuation, Open" (Ps)
 * @G_UNICODE_CURRENCY_SYMBOL: General category "Symbol, Currency" (Sc)
 * @G_UNICODE_MODIFIER_SYMBOL: General category "Symbol, Modifier" (Sk)
 * @G_UNICODE_MATH_SYMBOL: General category "Symbol, Math" (Sm)
 * @G_UNICODE_OTHER_SYMBOL: General category "Symbol, Other" (So)
 * @G_UNICODE_LINE_SEPARATOR: General category "Separator, Line" (Zl)
 * @G_UNICODE_PARAGRAPH_SEPARATOR: General category "Separator, Paragraph" (Zp)
 * @G_UNICODE_SPACE_SEPARATOR: General category "Separator, Space" (Zs)
 *
 * These are the possible character classifications from the
 * Unicode specification.
 * See <ulink url="http://www.unicode.org/Public/UNIDATA/UnicodeData.html">http://www.unicode.org/Public/UNIDATA/UnicodeData.html</ulink>.
 */
typedef enum
{
  G_UNICODE_CONTROL,
  G_UNICODE_FORMAT,
  G_UNICODE_UNASSIGNED,
  G_UNICODE_PRIVATE_USE,
  G_UNICODE_SURROGATE,
  G_UNICODE_LOWERCASE_LETTER,
  G_UNICODE_MODIFIER_LETTER,
  G_UNICODE_OTHER_LETTER,
  G_UNICODE_TITLECASE_LETTER,
  G_UNICODE_UPPERCASE_LETTER,
  G_UNICODE_SPACING_MARK,
  G_UNICODE_ENCLOSING_MARK,
  G_UNICODE_NON_SPACING_MARK,
  G_UNICODE_DECIMAL_NUMBER,
  G_UNICODE_LETTER_NUMBER,
  G_UNICODE_OTHER_NUMBER,
  G_UNICODE_CONNECT_PUNCTUATION,
  G_UNICODE_DASH_PUNCTUATION,
  G_UNICODE_CLOSE_PUNCTUATION,
  G_UNICODE_FINAL_PUNCTUATION,
  G_UNICODE_INITIAL_PUNCTUATION,
  G_UNICODE_OTHER_PUNCTUATION,
  G_UNICODE_OPEN_PUNCTUATION,
  G_UNICODE_CURRENCY_SYMBOL,
  G_UNICODE_MODIFIER_SYMBOL,
  G_UNICODE_MATH_SYMBOL,
  G_UNICODE_OTHER_SYMBOL,
  G_UNICODE_LINE_SEPARATOR,
  G_UNICODE_PARAGRAPH_SEPARATOR,
  G_UNICODE_SPACE_SEPARATOR
} GUnicodeType;

/**
 * G_UNICHAR_MAX_DECOMPOSITION_LENGTH:
 *
 * The maximum length (in codepoints) of a compatibility or canonical
 * decomposition of a single Unicode character.
 *
 * This is as defined by Unicode 6.1.
 *
 * Since: 2.32
 */
#define G_UNICHAR_MAX_DECOMPOSITION_LENGTH 18 /* codepoints */

/* Array of skip-bytes-per-initial character.
 */
GLIB_VAR const gchar * const h_utf8_skip;

/**
 * h_utf8_next_char:
 * @p: Pointer to the start of a valid UTF-8 character
 *
 * Skips to the next character in a UTF-8 string. The string must be
 * valid; this macro is as fast as possible, and has no error-checking.
 * You would use this macro to iterate over a string character by
 * character. The macro returns the start of the next UTF-8 character.
 * Before using this macro, use h_utf8_validate() to validate strings
 * that may contain invalid UTF-8.
 */
#define h_utf8_next_char(p) (char *)((p) + h_utf8_skip[*(const guchar *)(p)])

gunichar h_utf8_get_char           (const gchar  *p) G_GNUC_PURE;
glong    h_utf8_strlen            (const gchar *p,
                                   gssize       max) G_GNUC_PURE;

gint      h_unichar_to_utf8 (gunichar    c,
                             gchar      *outbuf);

gchar *h_utf8_strup   (const gchar *str,
                       gssize       len) G_GNUC_MALLOC;
gchar *h_utf8_strdown (const gchar *str,
                       gssize       len) G_GNUC_MALLOC;
gchar *h_utf8_casefold (const gchar *str,
                        gssize       len) G_GNUC_MALLOC;

gsize
h_unichar_fully_decompose (gunichar  ch,
                           gboolean  compat,
                           gunichar *result,
                           gsize     result_len);

gint
h_unichar_combining_class (gunichar uc);

#endif /* __H_UNICODE_H__ */
