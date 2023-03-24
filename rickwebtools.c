#include "rickwebtools.h"

void limpaBuffer (void)
{
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
}

struct tm *obter_data_hora ()
{
    time_t currentTime;
    struct tm *localTime;

    time( &currentTime );
    localTime = localtime( &currentTime );

    return localTime;
}

int tratamento_entrada(int argc, char *argv[], struct hostent **host)
{
    char ip[TAM_IP];
   
   	//Lendo IP da entrada
	if (argc > 1)
	{
		strcpy(ip, argv[1]);
		*host = gethostbyname(ip);

        return 1;
	}

	else
	{
		printf
		(
			"Nao foi possivel conectar, IP nao informado\n"
			"Tente: ./client < ip >\n"
		);
	}

    return 0;
}

int localizar_substring (char *string, char *substring)
{
    int i, j, flag;

    for (i = 0; i <= strlen(string) - strlen(substring); i++) {
        flag = 1;
        for (j = 0; j < strlen(substring); j++) {
            if (string[i+j] != substring[j]) {
                flag = 0;
                break;
            }
        }
        
		if (flag == 1)
        	return 0;
    }
	
	return 1;
}

char gerar_checksum (ARQUIVO *arquivo) 
{
    int i, checksum=0;
    
    for (i=0; i<sizeof(arquivo->buffer_arquivo); i++)
        checksum += arquivo->buffer_arquivo[i];
    
    return checksum % 256;
}