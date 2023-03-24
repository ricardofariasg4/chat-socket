#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "rickwebtools.h"

int main (void)
{
	int 
		sock = 0, 
		true = 1, 
		cliente_sock = 0,
		solicitacao = 1,
		recebe_arquivo = 0;
	
	char
		menu_a = -1,
		menu_b = -1,
		buffer[TAM_MENSAGEM_TXT];

	struct tm *data_hora;
	struct sockaddr_in endereco_servidor, endereco_cliente;    
	unsigned int sin_size;

	MENSAGEM msgcli, msgserver;
	INTERFACE interface;
	ARQUIVO arq;
	FILE *arquivo_server = NULL;

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
	
	// Verifica se o socket foi criado corretamente, caso contrario, imprime o erro
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("Socket");
		exit(1);
	}

	// Definicao e configuracao de parametros de enderecamento (opcao extra)
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(int)) == -1) 
	{
		perror("Setsockopt");
		exit(1);
	}

	endereco_servidor.sin_family = AF_INET;         
	endereco_servidor.sin_port = htons(PORTA);     
	endereco_servidor.sin_addr.s_addr = INADDR_ANY; 
	memset(&endereco_servidor.sin_zero, 0, sizeof(endereco_servidor.sin_zero));

	// Associa o socket com o endereco local
	if (bind(sock, (struct sockaddr *) &endereco_servidor, sizeof(struct sockaddr)) == -1) 
	{
		perror("Unable to bind");
		exit(1);
	}

	// Aguarda solicitacoes de conexoes por IP
	if (listen(sock, 5) == -1) 
	{
		perror("Listen");
		exit(1);
	}

	setbuf(stdout, NULL);
	LIMPA_TELA
	printf("\nAguardando conexao com CLI...");
	sin_size = sizeof(struct sockaddr_in);
	cliente_sock = accept(sock, (struct sockaddr *) &endereco_cliente, &sin_size);

	printf("\nConectado no ip %s", inet_ntoa(endereco_cliente.sin_addr));
	sleep(3);

	setbuf(stdout, NULL);
	LIMPA_TELA
	printf("\nInsira seu nome: ");
	fgets(msgserver.usuario, TAM_USUARIO_NOME-1, stdin);
	msgserver.usuario[strcspn(msgserver.usuario, "\n")] = '\0';	

	// Abre o servidor para iniciar conexoes com algum IP
	while (menu_a < 0)
	{
		setbuf(stdout, NULL);
		LIMPA_TELA
		printf
		(
			"\nTodt Network v1.0 SERVER"
			"\n1 - Chat de mensagens"
			"\n2 - Receber arquivos"
			"\n0 - Sair do Todt Network"
			"\n<comando>: "
		);

		scanf("%c", &menu_a);
		limpaBuffer();

		switch (menu_a)
		{
			case CHAT:
				// Inicialmente verifica se tem solicitacao do cliente
				setbuf(stdout, NULL);
				LIMPA_TELA	
				printf("\nAguardando interacao do CLI...");
				
				if (recv(cliente_sock, &interface.codigo_cli, sizeof(char), 0) > 0) 
				{
					// Se entrei na opcao de chat, verifico apenas chat, ignoro outras solicitacoes
					if (interface.codigo_cli == CHAT)
					{
						while (solicitacao)
						{
							setbuf(stdout, NULL);
							LIMPA_TELA
							printf("\nDeseja aceitar solicitacao de chat do CLI? [s/n] ");
							scanf("%c", &interface.codigo_server);
							limpaBuffer();

							if (interface.codigo_server == 's' || interface.codigo_server == 'S')
							{
								interface.codigo_server = CHAT;
								send(cliente_sock, &interface.codigo_server, sizeof(char), 0);
								solicitacao = 0;
								menu_b = -1;
							}

							else if (interface.codigo_server == 'n' || interface.codigo_server == 'N')
							{
								interface.codigo_server = SAIR_DO_CHAT;
								send(cliente_sock, &interface.codigo_server, sizeof(char), 0);
								solicitacao = 0;
								menu_b = 1;
							}
							
							else 
							{
								setbuf(stdout, NULL);
								LIMPA_TELA
								printf("\nOpcao invalida, digite 's' ou 'n'");
								sleep(3);
							}
						}
						
						solicitacao = 1; // Apenas para fins de reset
					}
					else {
						// Menu b recebe 1 pois se a solicitacao do servidor nao eh para chat, fodase o chat
						menu_b = 1;
					}
				}

				// Dialogo entre cliente e servidor
				setbuf(stdout, NULL);
				LIMPA_TELA
				while (menu_b < 0)
				{
					// Recebe resposta do cliente
					if (recv(cliente_sock, msgcli.mensagem, TAM_MENSAGEM_TXT, 0) > 0)
					{
						printf("\n%s", msgcli.mensagem);

						if (localizar_substring(msgcli.mensagem, ": q") == 0)
						{
							menu_b = 0;
							break;
						}	
					}
					
					memset(msgcli.mensagem, 0, sizeof(msgcli.mensagem));
					
					data_hora = obter_data_hora();
					
					sprintf
					(
						msgserver.mensagem, 
						"[%d-%d-%d as %dh%dm%ds]<%s> : ", 
						data_hora->tm_mday, data_hora->tm_mon+1, data_hora->tm_year+1900,
						data_hora->tm_hour, data_hora->tm_min, data_hora->tm_sec, 
						msgserver.usuario
					);

					printf
					(
						"\nEnvie 'q' para encerrar"
						"\n%s", msgserver.mensagem
					);

					fgets(buffer, TAM_MENSAGEM_TXT-strlen(msgserver.mensagem)-1, stdin);
					strcat(msgserver.mensagem, buffer);

					// Envia mensagem ao cliente
					send(cliente_sock, msgserver.mensagem, strlen(msgserver.mensagem), 0);

					if (localizar_substring(msgserver.mensagem, ": q") == 0)
						menu_b = 0;

					// Zera a estrutura de mensagem
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
				// Inicialmente verifica se tem solicitacao de envio de arquivo
				setbuf(stdout, NULL);
				LIMPA_TELA	
				printf("\nAguardando interacao do CLI...");
				
				if (recv(cliente_sock, &interface.codigo_cli, sizeof(char), 0) > 0) 
				{
					// Se entrei na opcao de envio de arquivo, verifico apenas envio de arquivo, ignoro outras solicitacoes
					if (interface.codigo_cli == ENVIAR_ARQUIVO)
					{
						while (solicitacao)
						{
							setbuf(stdout, NULL);
							LIMPA_TELA
							printf("\nDeseja receber arquivo do CLI? [s/n] ");
							scanf("%c", &interface.codigo_server);
							limpaBuffer();

							if (interface.codigo_server == 's' || interface.codigo_server == 'S')
							{
								interface.codigo_server = ENVIAR_ARQUIVO;
								send(cliente_sock, &interface.codigo_server, sizeof(char), 0);
								solicitacao = 0;
								recebe_arquivo = 1;
							}

							else if (interface.codigo_server == 'n' || interface.codigo_server == 'N')
							{
								interface.codigo_server = -1; // Tanto faz o valor enviado, cliente vai apenas ignorar
								send(cliente_sock, &interface.codigo_server, sizeof(char), 0);
								solicitacao = 0;
								recebe_arquivo = 0;
							}
							
							else 
							{
								setbuf(stdout, NULL);
								LIMPA_TELA
								printf("\nOpcao invalida, digite 's' ou 'n'");
								sleep(3);
							}
						}
						
						solicitacao = 1; // Apenas para fins de reset
					}
					else {
						// Menu b recebe 1 pois se a solicitacao do servidor nao eh para chat, fodase o chat
						recebe_arquivo = 0;
					}
				}

				if (recebe_arquivo)
				{
					// Inicialziar variáveis
					arquivo_server = fopen("download_from_cli", "w");

					if (!arquivo_server)
					{
						perror("arquivo");
						exit(1);
					}

					while (recv(cliente_sock, &arq, sizeof(ARQUIVO), 0) > 0)
					{
						if (arq.bytes_lidos > 0)
						{
							// printf("\n%d bytes recebidos", arq.bytes_lidos);
							fwrite(arq.buffer_arquivo, sizeof(char), arq.bytes_lidos, arquivo_server);
						}

						else {
							printf("Transferencia de arquivo finalizada.");
							PAUSE
							break;
						}
						
					}

					recebe_arquivo = 0;
					fclose(arquivo_server);
					arquivo_server = NULL;
				}

				menu_a = -1;
			break;

			case SAIR:
				menu_a = 4;
			break;
		}	
	}    

	close(cliente_sock);
	close(sock);

	return 0;
} 
