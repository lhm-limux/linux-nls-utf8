
#include "gtypes.h"
#include "gunicode.h"
#include "gunidecomp.h"
#include "gunicomp.h"

#define CC_PART1(Page, Char) \
  ((combining_class_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (combining_class_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (cclass_data[combining_class_table_part1[Page]][Char]))

#define CC_PART2(Page, Char) \
  ((combining_class_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
   ? (combining_class_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
   : (cclass_data[combining_class_table_part2[Page]][Char]))

#define COMBINING_CLASS(Char) \
  (((Char) <= G_UNICODE_LAST_CHAR_PART1) \
   ? CC_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR) \
      ? CC_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
      : 0))

glong
h_utf8_strlen (const gchar *p,
               gssize       max);

/**
 * h_unichar_combining_class:
 * @uc: a Unicode character
 * 
 * Determines the canonical combining class of a Unicode character.
 * 
 * Return value: the combining class of the character
 *
 * Since: 2.14
 **/
gint
h_unichar_combining_class (gunichar uc)
{
  return COMBINING_CLASS (uc);
}

/* constants for hangul syllable [de]composition */
#define SBase 0xAC00 
#define LBase 0x1100 
#define VBase 0x1161 
#define TBase 0x11A7
#define LCount 19 
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)
#define SCount (LCount * NCount)

/* http://www.unicode.org/unicode/reports/tr15/#Hangul
 * r should be null or have sufficient space. Calling with r == NULL will
 * only calculate the result_len; however, a buffer with space for three
 * characters will always be big enough. */
static void
decompose_hangul (gunichar s,
                  gunichar *r,
                  gsize *result_len)
{
  gint SIndex = s - SBase;
  gint TIndex = SIndex % TCount;

  if (r)
    {
      r[0] = LBase + SIndex / NCount;
      r[1] = VBase + (SIndex % NCount) / TCount;
    }

  if (TIndex)
    {
      if (r)
	r[2] = TBase + TIndex;
      *result_len = 3;
    }
  else
    *result_len = 2;
}

/* returns a pointer to a null-terminated UTF-8 string */
static const gchar *
find_decomposition (gunichar ch,
		    gboolean compat)
{
  int start = 0;
  int end = G_N_ELEMENTS (decomp_table);
  
  if (ch >= decomp_table[start].ch &&
      ch <= decomp_table[end - 1].ch)
    {
      while (TRUE)
	{
	  int half = (start + end) / 2;
	  if (ch == decomp_table[half].ch)
	    {
	      int offset;

	      if (compat)
		{
		  offset = decomp_table[half].compat_offset;
		  if (offset == G_UNICODE_NOT_PRESENT_OFFSET)
		    offset = decomp_table[half].canon_offset;
		}
	      else
		{
		  offset = decomp_table[half].canon_offset;
		  if (offset == G_UNICODE_NOT_PRESENT_OFFSET)
		    return NULL;
		}
	      
	      return &(decomp_expansion_string[offset]);
	    }
	  else if (half == start)
	    break;
	  else if (ch > decomp_table[half].ch)
	    start = half;
	  else
	    end = half;
	}
    }

  return NULL;
}

/**
 * h_unichar_fully_decompose:
 * @ch: a Unicode character.
 * @compat: whether perform canonical or compatibility decomposition
 * @result: (allow-none): location to store decomposed result, or %NULL
 * @result_len: length of @result
 *
 * Computes the canonical or compatibility decomposition of a
 * Unicode character.  For compatibility decomposition,
 * pass %TRUE for @compat; for canonical decomposition
 * pass %FALSE for @compat.
 *
 * The decomposed sequence is placed in @result.  Only up to
 * @result_len characters are written into @result.  The length
 * of the full decomposition (irrespective of @result_len) is
 * returned by the function.  For canonical decomposition,
 * currently all decompositions are of length at most 4, but
 * this may change in the future (very unlikely though).
 * At any rate, Unicode does guarantee that a buffer of length
 * 18 is always enough for both compatibility and canonical
 * decompositions, so that is the size recommended. This is provided
 * as %G_UNICHAR_MAX_DECOMPOSITION_LENGTH.
 *
 * See <ulink url="http://unicode.org/reports/tr15/">UAX#15</ulink>
 * for details.
 *
 * Return value: the length of the full decomposition.
 *
 * Since: 2.30
 **/
gsize
h_unichar_fully_decompose (gunichar  ch,
			   gboolean  compat,
			   gunichar *result,
			   gsize     result_len)
{
  const gchar *decomp;
  const gchar *p;

  /* Hangul syllable */
  if (ch >= SBase && ch < SBase + SCount)
    {
      gsize len, i;
      gunichar buffer[3];
      decompose_hangul (ch, result ? buffer : NULL, &len);
      if (result)
        for (i = 0; i < len && i < result_len; i++)
	  result[i] = buffer[i];
      return len;
    }
  else if ((decomp = find_decomposition (ch, compat)) != NULL)
    {
      /* Found it.  */
      gsize len, i;

      len = h_utf8_strlen (decomp, -1);

      for (p = decomp, i = 0; i < len && i < result_len; p = h_utf8_next_char (p), i++)
        result[i] = h_utf8_get_char (p);

      return len;
    }

  /* Does not decompose */
  if (result && result_len >= 1)
    *result = ch;
  return 1;
}
