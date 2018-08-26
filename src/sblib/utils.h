/*
 *  utils.h - Utility functions.
 *
 *  Copyright (c) 2014 Stefan Taferner <stefan.taferner@gmx.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 */
#ifndef sblib_utils_h
#define sblib_utils_h

#include "types.h"


/**
 * Copy from src to dest with reversing the byte order.
 *
 * @param dest - the destination to copy to
 * @param src - the source to copy from
 * @param len - the number of bytes to copy
 */
void reverseCopy(byte* dest, const byte* src, int len);

/**
 * Call when a fatal application error happens. This function will never
 * return and the program LED will blink rapidly to indicate the error.
 */
void fatalError();

/**
 * Get the offset of a field in a class, structure or type.
 *
 * @param type - the class, structure or type.
 * @param member - the member
 * @return The offset of the member.
 *
 * @brief E.g. for the structure
 *        struct ex
 *        {
 *            char a;
 *            char b
 *            short c;
 *        };
 *
 *        OFFSET_OF(ex,c) returns 2
 */
#define OFFSET_OF(type, field)  ((unsigned int) &(((type *) 0)->field))

/**
 * Include the C++ code snippet if DEBUG is defined, do not include the code
 * if DEBUG is not defined.
 *
 * @param code - the C++ code to include
 *
 * @brief Example:  IF_DEBUG(fatalError())
 */
#ifdef DEBUG
#  define IF_DEBUG(code) { code; }
#else
#  define IF_DEBUG(code)
#endif


/**
 * Concatenate two strings.
 * C preprocessor macros are not expanded.
 *
 * @param str1 - the first string
 * @param str2 - the second string
 *
 * @brief Example:  CPP_CONCAT(begin_,BCU_NAME) results in begin_BCU_NAME
 */
#define CPP_CONCAT(str1,str2)  str1 ## str2

/**
 * Concatenate two strings.
 * C preprocessor macros are expanded before concatenation.
 *
 * @param str1 - the first string
 * @param str2 - the second string
 *
 * @brief Example:  CPP_QUOTE_EXPAND(begin_,BCU_NAME) results in begin_BCU2  (if BCU_NAME is defined as BCU2)
 */
#define CPP_CONCAT_EXPAND(str1,str2) CPP_CONCAT(str1,str2)

/**
 * Quote a string.
 * C preprocessor macros are not expanded.
 *
 * @param str - the string to quote
 *
 * @brief Example:  CPP_QUOTE(BCU_TYPE) results in "BCU_TYPE"
 */
#define CPP_QUOTE(str) #str

/**
 * Quote a string.
 * C preprocessor macros are expanded before quoting.
 *
 * @param str - the string to quote
 *
 * @brief Example:  CPP_QUOTE_EXPAND(BCU_NAME) results in "BCU2"  (if BCU_NAME is defined as BCU2)
 */
#define CPP_QUOTE_EXPAND(str) CPP_QUOTE(str)

#endif /*sblib_utils_h*/
