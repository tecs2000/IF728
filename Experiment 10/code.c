#include <reg517A.h>

#define SIZE 5
#define uc unsigned char

#define FreqClock 12000000
#define FreqTimer 100
#define FatorCorrecao 10 // tempo gasto na atualizacao do timer0 durante a interrupcao
#define ValorInicial (65536 - (FreqClock / (12 * FreqTimer)) + FatorCorrecao)
#define TH0_Inicial (ValorInicial >> 8) // o compilador pega apenas os 8 bits menos significativos da variavel de 16 bits
#define TL0_Inicial (ValorInicial & 0x00FF)

unsigned int counter = 0;

// assinatura de funcoes
void inicializa_timer();
void setup_serial();

void getADC();
void readADC();
void sendString();
void sendChar(char);

// T_BUFFER
char T_Buffer[SIZE];
uc T_IN = 0;
uc T_OUT = 0;
uc T_Ocupado = 0;

// T_BUFFER FUNCTIONS
bit T_Buffer_Vazio();

unsigned int result;
long int convertedResult;

void main()
{

    inicializa_timer();
    setup_serial();
    EAL = 1; // as interrupcoes habilitadas serao atendidas
    ADCON1 = 0;
    MX0 = 0;
    MX1 = 0;
    MX2 = 0;
    ADEX = 0;
    ADM = 1; // configura adm = 1, para conversao continua
    ADDATL = 0;

    while (1)
    {
        readADC();
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

void setup_serial()
{
    BD = 1;           // habilita baud rate
    PCON |= 0x80;     // seta smod para 1
    ES0 = 1;          // habilita as interrupcoes da serial
    SM0 = 0, SM1 = 1; // configura como modo 1
    SM20 = 0;         // permite que o stop bit funcione
    REN0 = 1;         // habilita a recepcao
}

void ISR_timer0() interrupt 1 using 2
{
    TR0 = 0;
    TL0 += TL0_Inicial;                     // acrescenta a tl0 o valor contado a mais até que o ISR fosse executado
    TH0 += TH0_Inicial + (unsigned char)CY; // agrega a TH0 o carry que pode ter sido gerado por TL0
    TR0 = 1;

    counter += 1;
}

void ISR_Serial() interrupt 4
{
    if (TI0)
    {
        TI0 = 0;
        if (!T_Buffer_Vazio())
        {                            // checa se entrou na rotina pela primeira vez
            S0BUF = T_Buffer[T_OUT]; // envia o dado
            T_OUT = (T_OUT + 1) % SIZE;
        }
        else
        {
            T_Ocupado = 0;
        }
    }
}

// T_BUFFER FUNCTIONS

bit T_Buffer_Vazio()
{
    return (T_IN == T_OUT) ? 1 : 0;
}

void SendChar(char c)
{
    // Se T_Buffer nao estiver cheio, (T_IN + 1 ) % bufferSizer != T_OUT,
    // coloca um char na proxima posicao disponivel.
    // Se primeiro envio pela serial, forca a inicializacao com TI0 = 1

    if (((T_IN + 1) % SIZE) != T_OUT)
    {
        T_Buffer[T_IN] = c;
        T_IN = (T_IN + 1) % SIZE;

        if (!T_Ocupado)
        {
            T_Ocupado = 1;
            TI0 = 1;
        }
    }
}

void sendString()
{
    SendChar((convertedResult / 100) + '0');

    SendChar('.');

    SendChar(((convertedResult % 100) / 10) + '0');

    SendChar(((convertedResult % 100) % 10) + '0');

    sendChar(0x0a);
}

void getADC()
{
    while (BSY); // espera até que a conversao esteja completa
    result = (((unsigned int)ADDATH << 2) | (ADDATL >> 6));
    convertedResult = (500 * (long int)result) / 1023;
}

void readADC()
{
    if (counter == 100)
    {
        getADC();
        sendString();
        counter = 0; // limpa o contador e prepara para a proxima contagem
    }
}