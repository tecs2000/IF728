#include <REG517A.H>

sbit P5_0 = P5 ^ 0;
sbit P5_1 = P5 ^ 1;
sbit P5_2 = P5 ^ 2;
sbit P5_3 = P5 ^ 3;
sbit P5_4 = P5 ^ 4;
sbit P5_5 = P5 ^ 5;
sbit P5_6 = P5 ^ 6;

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
void sendChar(char);
void getKey();
void stateMachine();

// T_BUFFER
char T_Buffer[SIZE];
uc T_IN = 0;
uc T_OUT = 0;
uc T_Ocupado = 0;

// T_BUFFER FUNCTIONS
bit T_Buffer_Vazio();

unsigned int result;
long int convertedResult;

char centena;
char dezena;
char unidade;

char key = '$';

void main()
{

    inicializa_timer(); // Inicializa o timer
    setup_serial();     // Configura a interface serial
    EAL = 1;            // as interrupcoes habilitadas serao atendidas
    while (1)
    {
        stateMachine();
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

void ISR_timer0() interrupt 1
{
    /*
    Trata a interrupcao do timer 0
    */
    TR0 = 0;                                // Desliga o timer 0
    TL0 += TL0_Inicial;                     // acrescenta a tl0 o valor contado a mais até que o ISR fosse executado
    TH0 += TH0_Inicial + (unsigned char)CY; // agrega a TH0 o carry que pode ter sido gerado por TL0
    TR0 = 1;                                // Religa o timer 0
    counter += 1;                           // Incrementa a variável counter a cada 10ms
}

void stateMachine()
{
    /*
    Maquina de estados que espera pelo bouncing e
    pelo debouncing terminar
    */
    static char s = 0;

    switch (s)
    {
    case 0:
        getKey();
        if (key != '$') // se uma tecla esta pressionada troca de estado
        {
            counter = 0;
            s = 1;
            sendChar(key);
        }
        break;
    case 1:
        // Espera 40 milisegundos para o bouncing terminar
        if (counter >= 4)
            s = 2;
        break;
    case 2:
        if ((P5 & 0x70) == 0x70) // Se nenhuma tecla esta pressionada, troca de estado
        {
            counter = 0;
            s = 3;
        }
        break;
    case 3:
        // Espera 40 milisegundos para o debouncing terminar
        if (counter >= 4)
            s = 0;
        break;
    }
}

void getKey()
{
    /*
    Salva a tecla que foi pressionada no teclado, na variavel key
    */
    static char state = 0;

    if (state == 0) // primeira linha
    {
        P5_0 = 0;
        P5_1 = 1;
        P5_2 = 1;
        P5_3 = 1;

        if (!P5_4) // coluna 1
            key = '1';
        else if (!P5_5) // coluna 2
            key = '2';
        else if (!P5_6) // coluna 3
            key = '3';
        else
            key = '$';

        state = (state + 1) % 4; // troca para o estado 1
    }
    else if (state == 1)
    { // segunda linha
        P5_0 = 1;
        P5_1 = 0;
        P5_2 = 1;
        P5_3 = 1;

        if (!P5_4) // coluna 1
            key = '4';
        else if (!P5_5) // coluna 2
            key = '5';
        else if (!P5_6) // coluna 3
            key = '6';
        else
            key = '$';

        state = (state + 1) % 4; // troca para o estado 2
    }
    else if (state == 2)
    { // terceira linha
        P5_0 = 1;
        P5_1 = 1;
        P5_2 = 0;
        P5_3 = 1;

        if (!P5_4) // coluna 1
            key = '7';
        else if (!P5_5) // coluna 2
            key = '8';
        else if (!P5_6) // coluna 3
            key = '9';
        else
            key = '$';

        state = (state + 1) % 4; // troca para o estado 3
    }
    else
    { // quarta linha
        P5_0 = 1;
        P5_1 = 1;
        P5_2 = 1;
        P5_3 = 0;

        if (!P5_4) // coluna 1
            key = '*';
        else if (!P5_5) // coluna 2
            key = '0';
        else if (!P5_6) // coluna 3
            key = '#';
        else
            key = '$';

        state = (state + 1) % 4; // volta para o estado 0
    }
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
