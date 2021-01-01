#include "version.h"

const char DEVICE_VERSION[2] = {0, 2};
const char *DEVICE_ID = "MLS";

uint8_t VERSION_GetHW(void)
{
  return HW_REVISION_1;
}
