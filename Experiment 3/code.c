#include <REG51F.H>

#define FreqClock 12000000
#define FreqTimer 100
#define FatorCorrecao 10 // tempo gasto na atualizacao do timer0 durante a interrupcao
#define ValorInicial (65536 - (FreqClock / (12 * FreqTimer)) + FatorCorrecao)
#define TH0_Inicial (ValorInicial >> 8) // o compilador pega apenas os 8 bits menos significativos da variavel de 16 bits
#define TL0_Inicial (ValorInicial & 0x00FF)

sbit P2_0 = P2 ^ 0;
sbit P2_1 = P2 ^ 1;

// declaracao de variaveis globais
unsigned int start_counterP20 = 0;
unsigned int start_counterP21 = 0;

unsigned int counterP20 = 0;
unsigned int counterP21 = 0;

// assinatura de funcoes
void P20();
void P21();
void inicializa_timer();

enum states_machine
{
    bit_transfer,
    counting,
    waiting
};

void main()
{
    inicializa_timer();
    EA = 1; // as interrupcoes habilitadas serao atendidas

    while (1)
    {
        P20();
        P21();
    }
}

void inicializa_timer()
{
    TR0 = 0; // desliga o timer

    TMOD = (TMOD & 0XF0) | 0x01; // configura o modo de operacao do timer0

    TL0 = TL0_Inicial;
    TH0 = TH0_Inicial;

    TR0 = 1; // religa o timer
    ET0 = 1; // habilita interrupcoes do timer0
}

void ISR_timer0() interrupt 1 using 2
{
    TR0 = 0;
    TL0 += TL0_Inicial;                     // acrescenta a tl0 o valor contado a mais até que o ISR fosse executado
    TH0 += TH0_Inicial + (unsigned char)CY; // agrega a TH0 o carry que pode ter sido gerado por TL0
    TR0 = 1;

    if (start_counterP20)
        counterP20 += 1;
    if (start_counterP21)
        counterP21 += 1;
}

void P20()
{
    static enum states_machine state = waiting;

    switch (state)
    {
    case bit_transfer:
        if (P2_0)
        { // aguarda transicao positiva
            P1 = (P1 & 0xF0) | (P0 & 0x0F);
            state = counting;
            start_counterP20 = 1;
        }
        break;

    case counting:
        if (counterP20 == 100)
        {
            P1 = P1 & 0xF0;

            start_counterP20 = 0; // para a contagem do counterP20
            counterP20 = 0;       // limpa o contador e prepara para a proxima contagem

            state = waiting;
        }
        break;

    case waiting:
        if (!P2_0)
            state = bit_transfer; // aguarda transicao negativa
        break;
    }
}

void P21()
{
    static enum states_machine state = waiting;

    switch (state)
    {
    case bit_transfer:
        if (P2_1)
        { // aguarda transicao positiva
            P1 = (P1 & 0x0F) | (P0 & 0xF0);
            state = counting;
            start_counterP21 = 1;
        }
        break;

    case counting:
        if (counterP21 == 100)
        {
            P1 = P1 & 0x0F;

            start_counterP21 = 0; // para a contagem do counterP21
            counterP21 = 0;       // limpa o contador e prepara para a proxima contagem

            state = waiting;
        }
        break;

    case waiting:
        if (!P2_1)
            state = bit_transfer; // aguarda transicao negativa
        break;
    }
}