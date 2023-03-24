#ifndef __RICKWEBTOOLS__
#define __RICKWEBTOOLS__

#include <time.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Tamanhos
#define TAM_IP 21
#define TAM_USUARIO_NOME 50
#define TAM_DATA_HORA 27
#define TAM_MENSAGEM_TXT 4096
#define TAM_JANELA 16
#define TAM_MAX_DADO 63
#define TAM_CAMINHO_ARQUIVO 100

// Definições Gerais
#define PORTA 5000
#define PRIMEIRA_INTERFACE 0
#define LIMPA_TELA system("clear");
#define PAUSE getchar();
#define PULA_LINHA printf("\n");
#define MAX_BYTES 1000
#define TRANSMISSAO_ENCERRADA 1

// Interface de comunicação
#define INSERIR_TEXTO 105 // i na tabela ASCII
#define ENVIAR_MENSAGEM 101 // e na tabela ASCII
#define SAIR_DO_CHAT 113 // q na tabela ASCII
#define CHAT 49 // 1 na tabela ASCII
#define ENVIAR_ARQUIVO 50 // 2 na tabela ASCII
#define SAIR 48 // 0 na tabela ASCII
#define CONSULTA 99 // c na tabela ASCII

typedef struct _INTERFACE {
    char codigo_cli;
    char codigo_server;
} INTERFACE;

typedef struct _MENSAGEM {
    char usuario[TAM_USUARIO_NOME];
    char mensagem[TAM_MENSAGEM_TXT];    
} MENSAGEM;

// 1021 BYTES
typedef struct _ARQUIVO  {   
    int fim_de_arquivo;
    int tamanho_arquivo;
    int bytes_lidos;
    FILE *arquivo;
    char buffer_arquivo[MAX_BYTES];
    char checksum;
} ARQUIVO;

void limpaBuffer (void);
struct tm *obter_data_hora ();  
int tratamento_entrada(int argc, char *argv[], struct hostent **host);
int localizar_substring (char *string, char *substring);
char gerar_checksum (ARQUIVO *arquivo);

#endif