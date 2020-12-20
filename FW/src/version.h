#ifndef VERSION_H
#define VERSION_H

#include "defines.h"

const char DEVICE_VERSION[2];
const char *DEVICE_ID;

enum HW_REVISIONS
{
  HW_REVISION_1 = 1,
  HW_REVISION_LAST
};

uint8_t VERSION_GetHW(void);

#endif
