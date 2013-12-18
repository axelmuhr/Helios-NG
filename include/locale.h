/* locale.h: ANSI draft (X3J11 Oct 86) library header, section 4.3 */
/* Copyright (C) A. Mycroft and A.C. Norman */
/* version 0.01 - SccsId: %W% %G% */
/* $Id: locale.h,v 1.2 1992/08/12 16:39:41 nickc Exp $ */

#ifndef __locale_h
#define __locale_h

#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_NUMERIC  4
#define LC_TIME     8
#define LC_ALL     15

extern char *	   setlocale(int category, const char *locale);

struct lconv
  {
  char *decimal_point;
       /* The decimal point character used to format non-monetary quantities */
  char *thousands_sep;
       /* The character used to separate groups of digits to the left of the */
       /* decimal point character in formatted non-monetary quantities.      */
  char *grouping;
       /* A string whose elements indicate the size of each group of digits  */
       /* in formatted non-monetary quantities. See below for more details.  */
  char *int_curr_symbol;
       /* The international currency symbol applicable to the current locale.*/
       /* The first three characters contain the alphabetic international    */
       /* currency symbol in accordance with those specified in ISO 4217     */
       /* Codes for the representation of Currency and Funds. The fourth     */
       /* character (immediately preceding the null character) is the        */
       /* character used to separate the international currency symbol from  */
       /* the monetary quantity.                                             */
  char *currency_symbol;
       /* The local currency symbol applicable to the current locale.        */
  char *mon_decimal_point;
       /* The decimal-point used to format monetary quantities.              */
  char *mon_thousands_sep;
       /* The separator for groups of digits to the left of the decimal-point*/
       /* in formatted monetary quantities.                                  */
  char *mon_grouping;
       /* A string whose elements indicate the size of each group of digits  */
       /* in formatted monetary quantities. See below for more details.      */
  char *positive_sign;
       /* The string used to indicate a nonnegative-valued formatted         */
       /* monetary quantity.                                                 */
  char *negative_sign;
       /* The string used to indicate a negative-valued formatted monetary   */
       /* quantity.                                                          */
  char int_frac_digits;
       /* The number of fractional digits (those to the right of the         */
       /* decimal-point) to be displayed in an internationally formatted     */
       /* monetary quantities.                                               */
  char frac_digits;
       /* The number of fractional digits (those to the right of the         */
       /* decimal-point) to be displayed in a formatted monetary quantity.   */
  char p_cs_precedes;
       /* Set to 1 or 0 if the currency_symbol respectively precedes or      */
       /* succeeds the value for a nonnegative formatted monetary quantity.  */
  char p_sep_by_space;
       /* Set to 1 or 0 if the currency_symbol respectively is or is not     */
       /* separated by a space from the value for a nonnegative formatted    */
       /* monetary quantity.                                                 */
  char n_cs_precedes;
       /* Set to 1 or 0 if the currency_symbol respectively precedes or      */
       /* succeeds the value for a negative formatted monetary quantity.     */
  char n_sep_by_space;
       /* Set to 1 or 0 if the currency_symbol respectively is or is not     */
       /* separated by a space from the value for a negative formatted       */
       /* monetary quantity.                                                 */
  char p_sign_posn;
       /* Set to a value indicating the position of the positive_sign for a  */
       /* nonnegative formatted monetary quantity. See below for more details*/
  char n_sign_posn;
       /* Set to a value indicating the position of the negative_sign for a  */
       /* negative formatted monetary quantity. See below for more details.  */

  /*
   * The elements of grouping amd mon_grouping are interpreted according to
   * the following:
   * CHAR_MAX   No further grouping is to be performed.
   * 0          The previous element is to be repeatedly used for the
   *            remainder of the digits.
   * other      The value is the number of digits that compromise the current
   *            group. The next element is examined to determine the size of
   *            the next group of digits to the left of the current group.
   *
   * The value of p_sign_posn and n_sign_posn is interpreted according to
   * the following:
   * 0          Parentheses surround the quantity and currency_symbol.
   * 1          The sign string preceeds the quantity and currency_symbol.
   * 2          The sign string succeeds the quantity and currency_symbol.
   * 3          The sign string immediately preceeds the currency_symbol.
   * 4          The sign string immediately succeeds the currency_symbol.
   */
};

extern struct lconv *	localeconv( void );
  /*
   * Sets the components of an object with type struct lconv with values
   * appropriate for the formatting of numeric quantities (monetary and
   * otherwise) according to the rules of the current locale.
   * The members of the structure with type char * are strings, any of which
   * (except decimal_point) can point to "", to indicate that the value is not
   * available in the current locale or is of zero length. The members with
   * type char are nonnegative numbers, any of which can be CHAR_MAX to
   * indicate that the value is not available in the current locale.
   * The members included are described above.
   *
   * Return value:
   * A pointer to the filled in object. The structure pointed to by the return
   * value shall not be modified by the program, but may be overwritten by a
   * subsequent call to the localeconv function. In addition, calls to the
   * setlocale function with categories LC_ALL, LC_MONETARY, or LC_NUMERIC may
   * overwrite the contents of the structure.
   */

#endif

/* end of locale.h */
