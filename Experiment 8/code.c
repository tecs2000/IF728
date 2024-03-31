#include <REG517A.H>

#define ONESEC 31500;

unsigned int counter = 0;

void main()
{

    while (1)
    {
        /* 31500 eh o numero de instrucoes necessarias para obtermos, aproximadamente, 1s */
        /* Dessa forma, as luzes passam 1 segundo ligadas, e 1 segundo desligadas. */
        P4 = 0xFF;
        while (counter < 31500)
            ++counter;
        counter = 0;

        P4 = 0x0;
        while (counter < 31500)
            ++counter;
        counter = 0;
    }
}