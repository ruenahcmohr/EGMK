#include "nopDelay.h"

void Delay(uint32_t d) {
  for (; d; d--)  asm volatile ("nop"::); 
}
