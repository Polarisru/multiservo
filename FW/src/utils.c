/**
 * \file  utils.c
 * \brief Small common utilities
 * \date  14.10.2015
 * \addtogroup SourceCode
 * \{
 * \defgroup Utils Utilities
 * \brief Small common utilities
 * \{
 */

#include <ctype.h>
#include <string.h>
#include "utils.h"

/** \brief Make string value for the binary number
 *
 * \param [in] value Integer value to convert to binary string
 * \param [in] len Length of binary number
 * \return String with binary value
 *
 */
const char *UTILS_Int2Bin(uint32_t value, uint8_t len)
{
  static char b[32];
  uint32_t z;
  uint8_t i;

  b[0] = '\0';
  z = 1;

  for (i = 0; i < len; i++)
  {
    strcat(b, ((value & z) == z) ? "1" : "0");
    z <<= 1;
  }

  return b;
}

/** \brief Return median value for 3 values
 *
 * \param [in] a, b, c Word values to calculate
 * \return Median value, word
 *
 */
uint16_t UTILS_Median3Filter(uint16_t a, uint16_t b, uint16_t c)
{
  uint8_t Temp = 0;

  if (a>b)
    Temp += 4;
  if (a>c)
    Temp += 2;
  if (b>c)
    Temp += 1;

  switch (Temp)
  {
    case 0:
    case 7:
      return b;
    case 3:
    case 4:
      return a;
    default:
      return c;
  }
}

/** \brief Convert string to upper case
 *
 * \param [in] Input string
 * \return Output string
 *
 */
char *UTILS_StrUpr(char *s)
{
  char *str = s;

  while (*str)
  {
    *str = toupper(*str);
    str++;
  }
  return s;
}

/**
 * }
 * }
 */
