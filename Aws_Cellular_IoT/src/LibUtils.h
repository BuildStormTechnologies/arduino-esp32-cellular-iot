/**
 * \copyright Copyright (c) 2021, Buildstorm Pvt Ltd
 *
 * \file Http.h
 *  \brief HTTP library header file.
 */

#ifndef __LIB_UTILS_H__
#define __LIB_UTILS_H__

class LibUtils
{

private:
public:
    uint32_t getNumU32(const char *pStr);
    int32_t getNumI32(const char *pStr);
    float getFloat(const char *pStr, uint8_t decPoints);
    bool isValidString(packet_st *ps_packet);
    void hexDump(uint8_t *buffPtr, uint16_t dumpSize, uint8_t charsPerLine, bool ascii);
};

#endif //__LIB_UTILS_H__