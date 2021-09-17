#include "message.h"

void mina_write_int(void* dest, int src)
{
    int tmp = htonl(src);
    memcpy(dest, &tmp, sizeof(int));
}

void mina_write_short(void* dest, short src)
{
    short tmp = htons(src);
    memcpy(dest, &tmp, sizeof(short));
}

void mina_write_64int(void* dest, long long src)
{
    long long tmp = htonll(src);
    memcpy(dest, &tmp, sizeof(long long));
}

