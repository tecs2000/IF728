#include <REG51F.H>

#define ONESEC 31500;

sbit P2_0 = P2 ^ 0;
sbit P2_1 = P2 ^ 1;

void P20();
void P21();

enum states_machine
{
    bit_transfer,
    counting,
    waiting
};

void main()
{
    while (1)
    {
        P20();
        P21();
    }
}

void P20()
{
    static enum states_machine state = waiting;
    static unsigned int counter = ONESEC;

    switch (state)
    {
    case bit_transfer:
        if (P2_0)
        {
            P1 = (P1 & 0xF0) | (P0 & 0x0F);
            state = counting;
        }
        break;

    case counting:
        if (counter == 0)
        {
            P1 = P1 & 0xF0;
            counter = ONESEC;
            state = waiting;
        }
        else
            counter -= 1;
        break;

    case waiting:
        if (!P2_0)
            state = bit_transfer;
        break;
    }
}

void P21()
{
    static enum states_machine state = waiting;
    static unsigned int counter = ONESEC;

    switch (state)
    {
    case bit_transfer:
        if (P2_1)
        {
            P1 = (P1 & 0x0F) | (P0 & 0xF0);
            state = counting;
        }
        break;

    case counting:
        if (counter == 0)
        {
            P1 = P1 & 0x0F;
            counter = ONESEC;
            state = waiting;
        }
        else
            counter -= 1;
        break;

    case waiting:
        if (!P2_1)
            state = bit_transfer;
        break;
    }
}