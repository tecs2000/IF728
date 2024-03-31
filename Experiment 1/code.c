#include <REG51F.H>

sbit P2_0 = P2 ^ 0;
sbit P2_1 = P2 ^ 1;

void main()
{
    while (1)
    {

        if (P2_0)
            P1 = (P1 & 0xF0) | (P0 & 0x0F);
        else
            P1 = P1 & 0xF0;

        if (P2_1)
            P1 = (P1 & 0x0F) | (P0 & 0xF0);
        else
            P1 = P1 & 0X0F;
    }
}