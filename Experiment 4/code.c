#include <REG51F.H>

#define TH1_inicial 204; // smod = 1 -> exatos 1200 bps

void ISR_Serial();
void Setup_Serial(); // configura serial no modo 1
void Setup_Timer1(); // configura o timer 1 no modo 2

char dado = 0;
unsigned int dado_recebido = 0;
unsigned int enviar_dado = 0;

void main()
{
    EA = 1;

    Setup_Serial();
    Setup_Timer1();

    while (1)
        if (dado_recebido)
        {
            dado_recebido = 0; // indica que não há novos dados

            dado++; // incrementa o dado recebido
            enviar_dado = 1;
            TI = 1; // chama a rotina de transmissao para o envio do dado incrementado
        }
}

void Setup_Timer1()
{
    ET1 = 0;                     // desabilita as interrupcoes do timer 1
    TR1 = 1;                     // liga o timer
    TMOD = (TMOD & 0X0F) | 0x20; // GATE = 0, C/T = 0, M1 = 10, M0 = 0 - configura timer1 no modo 2
    PCON |= 0x80;                // seta smod para 1

    TH1 = TH1_inicial; // configura o valor inicial de TH1
}

void Setup_Serial()
{
    ES = 1;           // habilita as interrupcoes da serial
    SM0 = 0, SM1 = 1; // configura como modo 1
    SM2 = 0;          // permite que o stop bit funcione
    REN = 1;          // habilita a recepcao
}

void ISR_Serial() interrupt 4
{
    if (RI)
    {
        RI = 0;
        dado_recebido = 1; // indica à main que há um novo dado a ser incrementado
        dado = SBUF;       // armazena o dado na var global dado
    }
    if (TI)
    {
        if (enviar_dado)
        {                // checa se entrou na rotina pela primeira vez
            SBUF = dado; // envia o dado incrementado pela main à serial
            enviar_dado = 0;
        }
        TI = 0; // reseta a interrupcao
    }
}