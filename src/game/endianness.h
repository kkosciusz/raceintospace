#ifndef __ENDIANNESS_H__
#define __ENDIANNESS_H__

#include <cstdint>

void Swap32bit(uint32_t& value);
void Swap32bit(int32_t& value);
void Swap16bit(uint16_t& value);
void Swap16bit(int16_t& value);
void SwapFloat(float& value);
void SwapGameDat();

#endif // __ENDIANNESS_H__
