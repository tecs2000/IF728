#include <REG517A.H>

#define uc unsigned char
#define SIZE 16

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

void main()
{
    EAL = 1;

    Setup_Serial();

    while (1)
    {
        if (RecebeuString)
        {
            ReceiveString(s);
            SendString(s);
        }
    }
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
        {                           // se o buffer não estiver cheio
            R_Buffer[R_IN] = S0BUF; // armazena o dado na posicao disponivel de R_Buffer
            if (R_Buffer[R_IN] == '$')
                RecebeuString++;      // indica a main que uma nova string completa foi recebida
            R_IN = (R_IN + 1) % SIZE; // incrementa R_IN para a prox pos vazia
        }
    }
    if (TI0)
    {
        TI0 = 0;
        if (!T_Buffer_Vazio())
        {                            // checa se entrou na rotina pela primeira vez
            S0BUF = T_Buffer[T_OUT]; // envia o dado incrementado pela main à serial
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
    // Envia a string s para o buffer de saida, T_Buffer, atraves de SendChar
    char i;
    for (i = 0; *(s + i) != '$'; i++)
    {
        SendChar(*(s + i));
    }
    SendChar('$');
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

void ReceiveString(char *s)
{
    if (!R_Buffer_Vazio())
    {
        uc index = 0;
        uc counting = 1;

        while (counting)
        {
            *(s + index) = ReceiveChar();

            if (*(s + index) == '$')
            {
                counting = 0;
                RecebeuString--;
            }
            else
                index++;
        }
    }
}