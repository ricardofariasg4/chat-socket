#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#include "rickwebtools.h"

int main(int argc, char *argv[])
{
	long base, qtd_pacotes;
	
	int sock = 0, envia_arquivo;

	char 
		menu_a = -1,
		menu_b = -1,
		buffer[TAM_MENSAGEM_TXT],
		caminho[TAM_CAMINHO_ARQUIVO];

	struct tm *data_hora;
	struct sockaddr_in endereco_servidor;
	struct hostent *host;

	MENSAGEM msgcli, msgserver;
	INTERFACE interface;
	ARQUIVO arq;

	// Inicialização de variáveis e estruturas
	memset(msgcli.mensagem, 0, sizeof(msgcli.mensagem));
	memset(msgcli.usuario, 0, sizeof(msgcli.usuario));
	memset(msgserver.mensagem, 0, sizeof(msgserver.mensagem));
	memset(msgserver.usuario, 0, sizeof(msgserver.usuario));
	interface.codigo_cli = 0;
	interface.codigo_server = 0;
	arq.arquivo = NULL;
	memset(arq.buffer_arquivo, 0, MAX_BYTES);
	arq.bytes_lidos = 0;
	arq.fim_de_arquivo = 0;

	if (tratamento_entrada(argc, argv, &host) == 0)
		exit(0);

	// Verifica se o socket foi criado corretamente, caso contrario, imprime o erro
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("Socket");
		exit(1);
	}

	// Configuracao para estabelecer conexao (endereco)
	endereco_servidor.sin_family = PF_INET;     
	endereco_servidor.sin_port = htons(PORTA);   
	endereco_servidor.sin_addr = *((struct in_addr *) host->h_addr_list[PRIMEIRA_INTERFACE]);
	memset(&endereco_servidor.sin_zero, 0, sizeof(endereco_servidor.sin_zero));

	// Tenta realizar uma conexao ao servidor
	while (connect(sock, (struct sockaddr *) &endereco_servidor, sizeof(struct sockaddr)) == -1)
	{
		LIMPA_TELA
		printf("\nSem resposta do servidor...");
		sleep(2);

		for (int i=4; i>0; i--)
		{
			LIMPA_TELA
			printf("\nFalha de conexao, nova tentativa em %ds...", i);
			sleep(1);
		}
	}

	// Sistema de troca de mensagens e envio de arquivos abaixo
	setbuf(stdout, NULL);
	LIMPA_TELA
	printf("\nInsira seu nome: ");
	fgets(msgcli.usuario, TAM_USUARIO_NOME-1, stdin);
	msgcli.usuario[strcspn(msgcli.usuario, "\n")] = '\0';	
	
	while (menu_a < 0)
	{
		setbuf(stdout, NULL);
		LIMPA_TELA
		printf
		(
			"\nTodt Network v1.0 CLI"
			"\n1 - Chat de mensagens"
			"\n2 - Enviar arquivos"
			"\n0 - Sair do Todt Network"
			"\n<comando>: "
		);

		scanf("%c", &menu_a);
		limpaBuffer();

		switch (menu_a)
		{
			case CHAT:
				// Envia mensagem ao servidor avisando sobre o bate papo
				interface.codigo_cli = CHAT;
				interface.codigo_server = 0;
				send(sock, &interface.codigo_cli, sizeof(char), 0);

				// Verifica se houve aceitacao ao chat
				setbuf(stdout, NULL);
				LIMPA_TELA
				printf("\nAguardando aceitacao do servidor...");
				if (recv(sock, &interface.codigo_server, sizeof(char), 0) > 0)
				{
					if (interface.codigo_server == SAIR_DO_CHAT)
						menu_b = 1;
					else
						menu_b = -1;
				}
				
				// Dialogo entre cliente e servidor
				setbuf(stdout, NULL);
				LIMPA_TELA
				while (menu_b < 0)
				{
					data_hora = obter_data_hora();
					
					sprintf
					(
						msgcli.mensagem, 
						"[%d-%d-%d as %dh%dm%ds]<%s> : ", 
						data_hora->tm_mday, data_hora->tm_mon+1, data_hora->tm_year+1900,
						data_hora->tm_hour, data_hora->tm_min, data_hora->tm_sec, 
						msgcli.usuario
					);

					printf
					(
						"\nEnvie 'q' para encerrar"
						"\n%s", msgcli.mensagem
					);

					fgets(buffer, TAM_MENSAGEM_TXT-strlen(msgcli.mensagem)-1, stdin);
					strcat(msgcli.mensagem, buffer);

					// Envia mensagem ao servidor
					send(sock, msgcli.mensagem, strlen(msgcli.mensagem), 0);

					if (localizar_substring(msgcli.mensagem, ": q") == 0)
					{
						menu_b = 0;
					}
					
					else
					{
						// Recebe resposta do servidor
						if (recv(sock, msgserver.mensagem, TAM_MENSAGEM_TXT, 0) > 0)
						{
							printf("\n%s", msgserver.mensagem);

							if (localizar_substring(msgserver.mensagem, ": q") == 0)
								menu_b = 0;
						}	
					}
					
					// Limpa os campos de mensagem para nao acumular lixo
					memset(msgcli.mensagem, 0, sizeof(msgcli.mensagem));
					memset(msgserver.mensagem, 0, sizeof(msgserver.mensagem));
				}

				// Fecha o socket
				memset(msgcli.mensagem, 0, sizeof(msgcli.mensagem));
				memset(msgserver.mensagem, 0, sizeof(msgserver.mensagem));
				interface.codigo_cli = 0;
				interface.codigo_server = 0;
				menu_a = -1;
				menu_b = 0;
			break;

			case ENVIAR_ARQUIVO:
				envia_arquivo = 0;
				
				// Envia mensagem ao servidor avisando sobre o envio de arquivo
				interface.codigo_cli = ENVIAR_ARQUIVO;
				interface.codigo_server = 0;
				send(sock, &interface.codigo_cli, sizeof(char), 0);

				// Verifica se houve aceitacao para recepcao de arquivo
				setbuf(stdout, NULL);
				LIMPA_TELA
				printf("\nAguardando aceitacao do servidor...");
				if (recv(sock, &interface.codigo_server, sizeof(char), 0) > 0)
				{
					if (interface.codigo_server != ENVIAR_ARQUIVO)
						envia_arquivo = 0;
					else
						envia_arquivo = 1;
				}

				if (envia_arquivo)
				{
					// Abrir o arquivo
					printf("\nLocal do arquivo: ");
					scanf("%s", caminho);
					limpaBuffer();
					
					arq.arquivo = fopen(caminho, "rb");

					if (!arq.arquivo)
					{
						perror("fopen origem");
						exit(1);
					}

					// Calcular o tamanho do arquivo e enviar pela janela
					fseek(arq.arquivo, 0, SEEK_END);
					arq.tamanho_arquivo = ftell(arq.arquivo);

					// printf("\nTamanho do arquivo: %d bytes", arq.tamanho_arquivo);
					// PAUSE

					qtd_pacotes = arq.tamanho_arquivo / MAX_BYTES;

					if ((arq.tamanho_arquivo % MAX_BYTES) > 0)
						qtd_pacotes = qtd_pacotes+1;

					// printf("\nQuantidade de pacotes: %ld pacotes", qtd_pacotes);
					// PAUSE

					// Voltando o ponteiro ao inicio
					fseek(arq.arquivo, 0, SEEK_SET);

					base = 0;
					while (base < qtd_pacotes)
					{
						// Envia a janela inteira e confere depois
						for (int i=0; i<TAM_JANELA; i++)
						{
							arq.bytes_lidos = fread(arq.buffer_arquivo, sizeof(char), MAX_BYTES, arq.arquivo);

							arq.checksum = gerar_checksum(&arq);
							
							if (arq.bytes_lidos > 0)
							{
								// printf("\n%ld - Janela %d - %d bytes lidos", base, i, arq.bytes_lidos);
								send(sock, &arq, sizeof(ARQUIVO), 0);
							}
							else {
								send(sock, &arq, sizeof(ARQUIVO), 0);
								base = qtd_pacotes;
								break;
							}
						}

						PAUSE
						base += TAM_JANELA;
					}

					// Envia mensagem falando que o envio do arquivo acabou
					// arq.fim_de_arquivo = 1;
					// send(sock, &arq, sizeof(ARQUIVO), 0);
					fclose(arq.arquivo);
					arq.arquivo = NULL; // Sera mesmo necessario?
					envia_arquivo = 0;
					// arq.fim_de_arquivo = 0;
				}

				menu_a = -1;
			break;

			case SAIR:
				menu_a = 4;
			break;
		}
	}

	close(sock);

	return 0;
}