#include "getRandom.h"

// found on: https://forum.arduino.cc/index.php?topic=52502.0
unsigned long getRandom(unsigned long max)
{
    static unsigned long m_w = 1;
    static unsigned long m_z = 2;

    m_z = 36969L * (m_z & 65535L) + (m_z >> 16);
    m_w = 18000L * (m_w & 65535L) + (m_w >> 16);

    // return (m_z << 16) + m_w;  /* 32-bit result */
    unsigned long rnd = (m_z << 16) + m_w;
    double rnd2 = ((double) rnd / (double) (4294967295)) * (double) max;
    return ((unsigned long) rnd2);
}