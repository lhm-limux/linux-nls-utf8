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

#ifdef __KERNEL__

#include <linux/string.h>
#include <linux/slab.h>

#else

#include <string.h>
#include <locale.h>

#endif

#include "gunicode.h"
#include "gunichartables.h"

#define ATTR_TABLE(Page) (((Page) <= G_UNICODE_LAST_PAGE_PART1) \
                          ? attr_table_part1[Page] \
                          : attr_table_part2[(Page) - 0xe00])

#define ATTTABLE(Page, Char) \
  ((ATTR_TABLE(Page) == G_UNICODE_MAX_TABLE_INDEX) ? 0 : (attr_data[ATTR_TABLE(Page)][Char]))

#define TTYPE_PART1(Page, Char) \
  ((type_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part1[Page]][Char]))

#define TTYPE_PART2(Page, Char) \
  ((type_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part2[Page]][Char]))

#define TYPE(Char) \
  (((Char) <= G_UNICODE_LAST_CHAR_PART1) \
   ? TTYPE_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR) \
      ? TTYPE_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
      : G_UNICODE_UNASSIGNED))


#define IS(Type, Class)	(((guint)1 << (Type)) & (Class))
#define OR(Type, Rest)	(((guint)1 << (Type)) | (Rest))

#define ISALPHA(Type)	IS ((Type),				\
			    OR (G_UNICODE_LOWERCASE_LETTER,	\
			    OR (G_UNICODE_UPPERCASE_LETTER,	\
			    OR (G_UNICODE_TITLECASE_LETTER,	\
			    OR (G_UNICODE_MODIFIER_LETTER,	\
			    OR (G_UNICODE_OTHER_LETTER,		0))))))

#define ISALDIGIT(Type)	IS ((Type),				\
			    OR (G_UNICODE_DECIMAL_NUMBER,	\
			    OR (G_UNICODE_LETTER_NUMBER,	\
			    OR (G_UNICODE_OTHER_NUMBER,		\
			    OR (G_UNICODE_LOWERCASE_LETTER,	\
			    OR (G_UNICODE_UPPERCASE_LETTER,	\
			    OR (G_UNICODE_TITLECASE_LETTER,	\
			    OR (G_UNICODE_MODIFIER_LETTER,	\
			    OR (G_UNICODE_OTHER_LETTER,		0)))))))))

#define ISMARK(Type)	IS ((Type),				\
			    OR (G_UNICODE_NON_SPACING_MARK,	\
			    OR (G_UNICODE_SPACING_MARK,	\
			    OR (G_UNICODE_ENCLOSING_MARK,	0))))

#define ISZEROWIDTHTYPE(Type)	IS ((Type),			\
			    OR (G_UNICODE_NON_SPACING_MARK,	\
			    OR (G_UNICODE_ENCLOSING_MARK,	\
			    OR (G_UNICODE_FORMAT,		0))))


/**
 * h_unichar_toupper:
 * @c: a Unicode character
 * 
 * Converts a character to uppercase.
 * 
 * Return value: the result of converting @c to uppercase.
 *               If @c is not an lowercase or titlecase character,
 *               or has no upper case equivalent @c is returned unchanged.
 **/
gunichar
h_unichar_toupper (gunichar c)
{
  int t = TYPE (c);
  if (t == G_UNICODE_LOWERCASE_LETTER)
    {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
	{
	  const gchar *p = special_case_table + val - 0x1000000;
          val = h_utf8_get_char (p);
	}
      /* Some lowercase letters, e.g., U+000AA, FEMININE ORDINAL INDICATOR,
       * do not have an uppercase equivalent, in which case val will be
       * zero. 
       */
      return val ? val : c;
    }
  else if (t == G_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
	{
	  if (title_table[i][0] == c)
	    return title_table[i][1] ? title_table[i][1] : c;
	}
    }
  return c;
}

/**
 * h_unichar_tolower:
 * @c: a Unicode character.
 * 
 * Converts a character to lower case.
 * 
 * Return value: the result of converting @c to lower case.
 *               If @c is not an upperlower or titlecase character,
 *               or has no lowercase equivalent @c is returned unchanged.
 **/
gunichar
h_unichar_tolower (gunichar c)
{
  int t = TYPE (c);
  if (t == G_UNICODE_UPPERCASE_LETTER)
    {
      gunichar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
	{
	  const gchar *p = special_case_table + val - 0x1000000;
	  return h_utf8_get_char (p);
	}
      else
	{
	  /* Not all uppercase letters are guaranteed to have a lowercase
	   * equivalent.  If this is the case, val will be zero. */
	  return val ? val : c;
	}
    }
  else if (t == G_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
	{
	  if (title_table[i][0] == c)
	    return title_table[i][2];
	}
    }
  return c;
}

/*
 * Case mapping functions
 */

typedef enum {
  LOCALE_NORMAL,
  LOCALE_TURKIC,
  LOCALE_LITHUANIAN
} LocaleType;

static LocaleType
get_locale_type (void)
{
#ifdef HAVE_LOCALE
  const char *locale = setlocale (LC_CTYPE, NULL);

  if (locale == NULL)
    return LOCALE_NORMAL;

  switch (locale[0])
    {
   case 'a':
      if (locale[1] == 'z')
	return LOCALE_TURKIC;
      break;
    case 'l':
      if (locale[1] == 't')
	return LOCALE_LITHUANIAN;
      break;
    case 't':
      if (locale[1] == 'r')
	return LOCALE_TURKIC;
      break;
    }
#endif
  return LOCALE_NORMAL;
}

static gint
output_marks (const char **p_inout,
	      char        *out_buffer,
	      gboolean     remove_dot)
{
  const char *p = *p_inout;
  gint len = 0;
  
  while (*p)
    {
      gunichar c = h_utf8_get_char (p);
      
      if (ISMARK (TYPE (c)))
	{
	  if (!remove_dot || c != 0x307 /* COMBINING DOT ABOVE */)
	    len += h_unichar_to_utf8 (c, out_buffer ? out_buffer + len : NULL);
	  p = h_utf8_next_char (p);
	}
      else
	break;
    }

  *p_inout = p;
  return len;
}

static gint
output_special_case (gchar *out_buffer,
		     int    offset,
		     int    type,
		     int    which)
{
  const gchar *p = special_case_table + offset;
  gint len;

  if (type != G_UNICODE_TITLECASE_LETTER)
    p = h_utf8_next_char (p);

  if (which == 1)
    p += strlen (p) + 1;

  len = strlen (p);
  if (out_buffer)
    memcpy (out_buffer, p, len);

  return len;
}

static gsize
real_toupper (const gchar *str,
	      gssize       max_len,
	      gchar       *out_buffer,
	      LocaleType   locale_type)
{
  const gchar *p = str;
  const char *last = NULL;
  gsize len = 0;
  gboolean last_was_i = FALSE;

  while ((max_len < 0 || p < str + max_len) && *p)
    {
      gunichar c = h_utf8_get_char (p);
      int t = TYPE (c);
      gunichar val;

      last = p;
      p = h_utf8_next_char (p);

      if (locale_type == LOCALE_LITHUANIAN)
	{
	  if (c == 'i')
	    last_was_i = TRUE;
	  else 
	    {
	      if (last_was_i)
		{
		  /* Nasty, need to remove any dot above. Though
		   * I think only E WITH DOT ABOVE occurs in practice
		   * which could simplify this considerably.
		   */
		  gsize decomp_len, i;
		  gunichar decomp[G_UNICHAR_MAX_DECOMPOSITION_LENGTH];

		  decomp_len = h_unichar_fully_decompose (c, FALSE, decomp, G_N_ELEMENTS (decomp));
		  for (i=0; i < decomp_len; i++)
		    {
		      if (decomp[i] != 0x307 /* COMBINING DOT ABOVE */)
			len += h_unichar_to_utf8 (h_unichar_toupper (decomp[i]), out_buffer ? out_buffer + len : NULL);
		    }
		  
		  len += output_marks (&p, out_buffer ? out_buffer + len : NULL, TRUE);

		  continue;
		}

	      if (!ISMARK (t))
		last_was_i = FALSE;
	    }
	}

      if (locale_type == LOCALE_TURKIC && c == 'i')
	{
	  /* i => LATIN CAPITAL LETTER I WITH DOT ABOVE */
	  len += h_unichar_to_utf8 (0x130, out_buffer ? out_buffer + len : NULL); 
	}
      else if (c == 0x0345)	/* COMBINING GREEK YPOGEGRAMMENI */
	{
	  /* Nasty, need to move it after other combining marks .. this would go away if
	   * we normalized first.
	   */
	  len += output_marks (&p, out_buffer ? out_buffer + len : NULL, FALSE);

	  /* And output as GREEK CAPITAL LETTER IOTA */
	  len += h_unichar_to_utf8 (0x399, out_buffer ? out_buffer + len : NULL); 	  
	}
      else if (IS (t,
		   OR (G_UNICODE_LOWERCASE_LETTER,
		   OR (G_UNICODE_TITLECASE_LETTER,
		  0))))
	{
	  val = ATTTABLE (c >> 8, c & 0xff);

	  if (val >= 0x1000000)
	    {
	      len += output_special_case (out_buffer ? out_buffer + len : NULL, val - 0x1000000, t,
					  t == G_UNICODE_LOWERCASE_LETTER ? 0 : 1);
	    }
	  else
	    {
	      if (t == G_UNICODE_TITLECASE_LETTER)
		{
		  unsigned int i;
		  for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
		    {
		      if (title_table[i][0] == c)
			{
			  val = title_table[i][1];
			  break;
			}
		    }
		}

	      /* Some lowercase letters, e.g., U+000AA, FEMININE ORDINAL INDICATOR,
	       * do not have an uppercase equivalent, in which case val will be
	       * zero. */
	      len += h_unichar_to_utf8 (val ? val : c, out_buffer ? out_buffer + len : NULL);
	    }
	}
      else
	{
	  gsize char_len = h_utf8_skip[*(guchar *)last];

	  if (out_buffer)
	    memcpy (out_buffer + len, last, char_len);

	  len += char_len;
	}

    }

  return len;
}

/**
 * h_utf8_strup:
 * @str: a UTF-8 encoded string
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 * 
 * Converts all Unicode characters in the string that have a case
 * to uppercase. The exact manner that this is done depends
 * on the current locale, and may result in the number of
 * characters in the string increasing. (For instance, the
 * German ess-zet will be changed to SS.)
 * 
 * Return value: a newly allocated string, with all characters
 *    converted to uppercase.  
 **/
gchar *
h_utf8_strup (const gchar *str,
	      gssize       len)
{
  gsize result_len;
  LocaleType locale_type;
  gchar *result;

  g_return_val_if_fail (str != NULL, NULL);

  locale_type = get_locale_type ();
  
  /*
   * We use a two pass approach to keep memory management simple
   */
  result_len = real_toupper (str, len, NULL, locale_type);
  result = g_malloc (result_len + 1);
  real_toupper (str, len, result, locale_type);
  result[result_len] = '\0';

  return result;
}

/* traverses the string checking for characters with combining class == 230
 * until a base character is found */
static gboolean
has_more_above (const gchar *str)
{
  const gchar *p = str;
  gint combining_class;

  while (*p)
    {
      combining_class = h_unichar_combining_class (h_utf8_get_char (p));
      if (combining_class == 230)
        return TRUE;
      else if (combining_class == 0)
        break;

      p = h_utf8_next_char (p);
    }

  return FALSE;
}

static gsize
real_tolower (const gchar *str,
	      gssize       max_len,
	      gchar       *out_buffer,
	      LocaleType   locale_type)
{
  const gchar *p = str;
  const char *last = NULL;
  gsize len = 0;

  while ((max_len < 0 || p < str + max_len) && *p)
    {
      gunichar c = h_utf8_get_char (p);
      int t = TYPE (c);
      gunichar val;

      last = p;
      p = h_utf8_next_char (p);

      if (locale_type == LOCALE_TURKIC && c == 'I')
	{
          if (h_utf8_get_char (p) == 0x0307)
            {
              /* I + COMBINING DOT ABOVE => i (U+0069) */
              len += h_unichar_to_utf8 (0x0069, out_buffer ? out_buffer + len : NULL); 
              p = h_utf8_next_char (p);
            }
          else
            {
              /* I => LATIN SMALL LETTER DOTLESS I */
              len += h_unichar_to_utf8 (0x131, out_buffer ? out_buffer + len : NULL); 
            }
        }
      /* Introduce an explicit dot above when lowercasing capital I's and J's
       * whenever there are more accents above. [SpecialCasing.txt] */
      else if (locale_type == LOCALE_LITHUANIAN && 
               (c == 0x00cc || c == 0x00cd || c == 0x0128))
        {
          len += h_unichar_to_utf8 (0x0069, out_buffer ? out_buffer + len : NULL); 
          len += h_unichar_to_utf8 (0x0307, out_buffer ? out_buffer + len : NULL); 

          switch (c)
            {
            case 0x00cc: 
              len += h_unichar_to_utf8 (0x0300, out_buffer ? out_buffer + len : NULL); 
              break;
            case 0x00cd: 
              len += h_unichar_to_utf8 (0x0301, out_buffer ? out_buffer + len : NULL); 
              break;
            case 0x0128: 
              len += h_unichar_to_utf8 (0x0303, out_buffer ? out_buffer + len : NULL); 
              break;
            }
        }
      else if (locale_type == LOCALE_LITHUANIAN && 
               (c == 'I' || c == 'J' || c == 0x012e) && 
               has_more_above (p))
        {
          len += h_unichar_to_utf8 (h_unichar_tolower (c), out_buffer ? out_buffer + len : NULL); 
          len += h_unichar_to_utf8 (0x0307, out_buffer ? out_buffer + len : NULL); 
        }
      else if (c == 0x03A3)	/* GREEK CAPITAL LETTER SIGMA */
	{
	  if ((max_len < 0 || p < str + max_len) && *p)
	    {
	      gunichar next_c = h_utf8_get_char (p);
	      int next_type = TYPE(next_c);

	      /* SIGMA mapps differently depending on whether it is
	       * final or not. The following simplified test would
	       * fail in the case of combining marks following the
	       * sigma, but I don't think that occurs in real text.
	       * The test here matches that in ICU.
	       */
	      if (ISALPHA (next_type)) /* Lu,Ll,Lt,Lm,Lo */
		val = 0x3c3;	/* GREEK SMALL SIGMA */
	      else
		val = 0x3c2;	/* GREEK SMALL FINAL SIGMA */
	    }
	  else
	    val = 0x3c2;	/* GREEK SMALL FINAL SIGMA */

	  len += h_unichar_to_utf8 (val, out_buffer ? out_buffer + len : NULL);
	}
      else if (IS (t,
		   OR (G_UNICODE_UPPERCASE_LETTER,
		   OR (G_UNICODE_TITLECASE_LETTER,
		  0))))
	{
	  val = ATTTABLE (c >> 8, c & 0xff);

	  if (val >= 0x1000000)
	    {
	      len += output_special_case (out_buffer ? out_buffer + len : NULL, val - 0x1000000, t, 0);
	    }
	  else
	    {
	      if (t == G_UNICODE_TITLECASE_LETTER)
		{
		  unsigned int i;
		  for (i = 0; i < G_N_ELEMENTS (title_table); ++i)
		    {
		      if (title_table[i][0] == c)
			{
			  val = title_table[i][2];
			  break;
			}
		    }
		}

	      /* Not all uppercase letters are guaranteed to have a lowercase
	       * equivalent.  If this is the case, val will be zero. */
	      len += h_unichar_to_utf8 (val ? val : c, out_buffer ? out_buffer + len : NULL);
	    }
	}
      else
	{
	  gsize char_len = h_utf8_skip[*(guchar *)last];

	  if (out_buffer)
	    memcpy (out_buffer + len, last, char_len);

	  len += char_len;
	}

    }

  return len;
}

/**
 * h_utf8_strdown:
 * @str: a UTF-8 encoded string
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 * 
 * Converts all Unicode characters in the string that have a case
 * to lowercase. The exact manner that this is done depends
 * on the current locale, and may result in the number of
 * characters in the string changing.
 * 
 * Return value: a newly allocated string, with all characters
 *    converted to lowercase.  
 **/
gchar *
h_utf8_strdown (const gchar *str,
		gssize       len)
{
  gsize result_len;
  LocaleType locale_type;
  gchar *result;

  g_return_val_if_fail (str != NULL, NULL);

  locale_type = get_locale_type ();
  
  /*
   * We use a two pass approach to keep memory management simple
   */
  result_len = real_tolower (str, len, NULL, locale_type);
  result = g_malloc (result_len + 1);
  real_tolower (str, len, result, locale_type);
  result[result_len] = '\0';

  return result;
}

static void
do_resize(gchar **str, gchar **pos, gssize *len, int resize)
{
  gchar *newstr;

  (*len) += resize;
  if (*len <= 0)
    return;

  newstr = g_malloc(*len);
  if (NULL != newstr) 
    {
       memcpy(newstr, *str, *pos - *str);
       (*pos) = newstr + (*pos - *str);
       g_free(*str);
    }
  else
    g_free(*str);
  *str = newstr;
}

/**
 * h_utf8_casefold:
 * @str: a UTF-8 encoded string
 * @len: length of @str, in bytes, or -1 if @str is nul-terminated.
 * 
 * Converts a string into a form that is independent of case. The
 * result will not correspond to any particular case, but can be
 * compared for equality or ordered with the results of calling
 * h_utf8_casefold() on other strings.
 * 
 * Note that calling h_utf8_casefold() followed by h_utf8_collate() is
 * only an approximation to the correct linguistic case insensitive
 * ordering, though it is a fairly good one. Getting this exactly
 * right would require a more sophisticated collation function that
 * takes case sensitivity into account. GLib does not currently
 * provide such a function.
 * 
 * Return value: a newly allocated string, that is a
 *   case independent form of @str.
 **/
gchar *
h_utf8_casefold (const gchar *str,
		 gssize       len)
{
  gssize reslen;
  gchar *result, *rp;
  const char *p;

  if (len < 0)
    len = strlen(str);

  // h_unichar_to_utf8 may need up to 6 bytes
  reslen = len + 6 + 1;
  
  result = g_malloc (reslen);
  rp = result;

  if (NULL == result)
    goto bailout;

  g_return_val_if_fail (str != NULL, NULL);
  
  p = str;
  while ((len < 0 || p < str + len) && *p)
    {
      gunichar ch = h_utf8_get_char (p);

      int start = 0;
      int end = G_N_ELEMENTS (casefold_table);

      if (ch >= casefold_table[start].ch &&
          ch <= casefold_table[end - 1].ch)
	{
	  while (TRUE)
	    {
	      int half = (start + end) / 2;
	      if (ch == casefold_table[half].ch)
		{
                  int copylen = strlen(casefold_table[half].data);

                  // resize result string
                  if (result + reslen - rp < copylen)
                    {
                      do_resize(&result, &rp, &reslen, 100);
                      if (NULL == result) 
                        goto bailout;
                    }

		  memcpy(result, casefold_table[half].data, copylen);
		  goto next;
		}
	      else if (half == start)
		break;
	      else if (ch > casefold_table[half].ch)
		start = half;
	      else
		end = half;
	    }
	}

      if (result + reslen - rp < 7)
        {
          do_resize(&result, &rp, &reslen, 100);
          if (NULL == result)
            goto bailout;
        }
         
      rp += h_unichar_to_utf8(h_unichar_tolower (ch), rp);
      
    next:
      p = h_utf8_next_char (p);
    }

  *rp = '\0';

bailout:
  return result; 
}

