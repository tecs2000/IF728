#include <REG517A.H>

#define uc unsigned char
#define ui unsigned int
#define SIZE 2

const ui lookup[] = {65535, 58248, 50967, 43686, 36405, 29124, 21843, 14562, 7281, 0};

// TIMER
void set_timer();

// SERIAL
void ISR_Serial();
void Setup_Serial(); // configura serial no modo 1

// R_BUFFER
char R_Buffer[SIZE]; // buffer de recepcao
uc R_IN = 0;
uc R_OUT = 0;

// R_BUFFER FUNCTIONS
bit R_Buffer_Vazio();
char ReceiveChar();
void ReceiveString(char *);

// T_BUFFER
char T_Buffer[SIZE];
uc T_IN = 0;
uc T_OUT = 0;
uc T_Ocupado = 0;

// T_BUFFER FUNCTIONS
bit T_Buffer_Vazio();
void SendString(char *);
void SendChar(char);

uc RecebeuString = 0;
uc s[SIZE];
ui intensity;

ui counterTimer = 0;

void main()
{
    EAL = 1;

    /* configuracao dos pinos da porta p4 */
    CMSEL = 0XFF; // as portas vao comparar com o compare timer
    CMEN = 0XFF;  // diz quais portas vao comparar com o compare timer, nesse caso, todas
    CTCON = 0X00;

    Setup_Serial();
    set_timer();

    while (1)
    {
        if (RecebeuString)
        {
            ReceiveString(s);
            SendString(s);
        }
    }
}

void set_timer()
{

    CTRELL = 0x00; // valor de recarga do timer
    CTRELH = 0X00;
}

void Setup_Serial()
{
    BD = 1;           // habilita baud rate
    PCON |= 0x80;     // seta smod para 1
    ES0 = 1;          // habilita as interrupcoes da serial
    SM0 = 0, SM1 = 1; // configura como modo 1
    SM20 = 0;         // permite que o stop bit funcione
    REN0 = 1;         // habilita a recepcao
}

void ISR_Serial() interrupt 4
{
    if (RI0)
    {
        RI0 = 0;

        if (((R_IN + 1) % SIZE) != R_OUT)
        {                             // se o buffer n?o estiver cheio
            R_Buffer[R_IN] = S0BUF;   // armazena o dado na posicao disponivel de R_Buffer
            RecebeuString++;          // indica a main que uma nova string completa foi recebida
            R_IN = (R_IN + 1) % SIZE; // incrementa R_IN para a prox pos vazia
        }
    }
    if (TI0)
    {
        TI0 = 0;
        if (!T_Buffer_Vazio())
        {                            // checa se entrou na rotina pela primeira vez
            S0BUF = T_Buffer[T_OUT]; // envia o dado incrementado pela main ? serial
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

void SendString(char *s)
{
    SendChar(*(s)); // Envia a string s para o buffer de saida, T_Buffer, atraves de SendChar
}

// R_BUFFER FUNCTIONS

bit R_Buffer_Vazio()
{
    return (R_IN == R_OUT) ? 1 : 0;
}

char ReceiveChar()
{
    char recebido = R_Buffer[R_OUT]; // consome o prox char disponivel de R_Buffer;

    R_OUT = (R_OUT + 1) % SIZE; // com o mod, retorna pro indice 0 caso ja tenha chegado ao final do array.

    return recebido;
}

void setP4Intensity()
{
    intensity = lookup[(*s) - '0'];

    // configura os pins de p4 com a nova intensidade
    CM0 = intensity;
    CM1 = intensity;
    CM2 = intensity;
    CM3 = intensity;
    CM4 = intensity;
    CM5 = intensity;
    CM6 = intensity;
    CM7 = intensity;

    // reseta o timer 2
    set_timer();
}

void ReceiveString(char *s)
{
    if (!R_Buffer_Vazio())
    {
        *s = ReceiveChar();
        setP4Intensity();
        RecebeuString--;
    }
}