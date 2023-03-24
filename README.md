## Chat TCP/IP no modelo cliente-servidor

Um pequeno programa para ilustrar como ocorre a comunicação entre dois sistemas no modelo cliente servidor.</br>

## Compilação

1 - Os dois computadores, um para o cliente e o outro para o servidor, devem executar linux.

2 - Para cada computador, instale os seguintes pacotes: gcc, g++ e make

3 - Mantenha no primeiro computador uma cópia do arquivo Makefile, junto com o client.c. No outro computador, tenha também uma cópia do Makefile junto com o arquivo server.c. Em cada um dos computadores, execute a compilação apenas executando o comando "make" na pasta contendo o projeto

## Uso

Execute o binário do client no computador em que você o compilou, faça o mesmo para o server. Executar o client ou server primeiro não faz diferença. Para o client, a execução deve ser dada por ./client <endereco_ip>
