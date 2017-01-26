#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define TAMANHO_LINHA 255
#define NUM_ARGS 128
#define ERRO_FORK -1

int intTamanhoLinha;
char strLinha[TAMANHO_LINHA+1];


char * eCommands[TAMANHO_LINHA];
int numECmds;
char * pipeCommands[TAMANHO_LINHA];
int numPipeCmds;

char * strInFile = NULL;
char * strOutFile = NULL;


void execRedirCmd(char * strCmd, int imprime_pid);
void execPipeCmd(char * strCmd, int imprime_pid);

void trim(char * strString)
{
	int Tamanho;
	char * pString = strString;
	
	if (strString == NULL) return; /* String Nula */

	Tamanho = strlen(strString) - 1;

	if (Tamanho == -1) return; /* String Vazia! */


	while (strString[Tamanho] == ' ' || strString[Tamanho] == '\t' || strString[Tamanho] == '\n')
	{
		strString[Tamanho] = 0;
		Tamanho--;
		
	}
	while (*pString == ' ' || *pString == '\t')
	{
		*pString++;
	}
	strcpy(strString, pString);
	
}

int prompt()
{
	strInFile = NULL;
	strOutFile = NULL;

	printf("[lsh]$ ");
	if (!fgets(strLinha, TAMANHO_LINHA, stdin)) return 0;
		trim(strLinha);

	
	/* Caso seja o comando de saida, para o programa imediatamente */
	if (strcmp(strLinha, "exit") == 0)
	{
		/* sai do shell*/
		printf("Tchau!\n");
		exit(0);
	}
	intTamanhoLinha = strlen(strLinha);
	return 1;
}

void ImprimeMsgErroFork()
{
	printf("Erro! Não foi possível criar um novo processo!\n");
}



void ParseECommand(char * strString)
{
	int i;
	numECmds = 1;
	
	/* Zera o vetor de commandos */
	for (i = 0; i < TAMANHO_LINHA; i++) eCommands[i] = 0;
	
	eCommands[0] = strString;
	
	for (i = 0; strString[i] != '\0'; i++)
	{
		if (strString[i] == '&')
		{
			eCommands[numECmds++] = &(strString[i+1]);
			strString[i] = '\0';
		}
	}
	for (i = 0; i < numPipeCmds; i++)
	{
		trim(eCommands[i]);
		printf("& %d: %s\n", i, eCommands[i]);
	}
}


void ParsePipeCommand(char * strString)
{
	int i;
	numPipeCmds = 1;
	
	/* Zera o vetor de commandos */
	for (i = 0; i < TAMANHO_LINHA; i++) pipeCommands[i] = 0;
	
	pipeCommands[0] = strString;
	
	for (i = 0; strString[i] != '\0'; i++)
	{
		if (strString[i] == '|')
		{
			pipeCommands[numPipeCmds++] = &(strString[i+1]);
			strString[i] = '\0';
		}
	}
	for (i = 0; i < numPipeCmds; i++)
	{
		trim(pipeCommands[i]);
		/* DEBUG :
		printf("Pipe %d: %s\n", i, pipeCommands[i]);
		*/
	}
}
int ParseRedirectCommand(char * strString)
{
	int i;
	int SinalMenor = 0;
	int SinalMaior = 0;

	/* DEBUG: printf("String de entrada: \"%s\"\n", strString); */
	
	/* Aqui procura os caracteres < e >, se houver mais de um de cada retorna com erro */
	for (i = 0; strString[i] != '\0'; i++)
	{
		if (strString[i] == '<' && !SinalMenor) SinalMenor = i;
		else if (strString[i] == '<' && SinalMenor) return 0; 
		/* Duas redirecoes de entrada encontradas: ERRO ! */
		
		if (strString[i] == '>' && !SinalMaior) SinalMaior = i;
		else if (strString[i] == '>' && SinalMaior) return 0; 
		/* Duas redirecoes de saida encontradas: ERRO ! */

	}
	/* DEBUG: printf("SinalMaior = %d, SinalMenor = %d\n", SinalMaior, SinalMenor); */
		
	if (!SinalMaior && !SinalMenor) 
		return 1; 
		/* Nao foi encontrada redirecao alguma, nao ha parsing a ser feito */

	if (SinalMaior != 0)
	{
		strString[SinalMaior] = '\0'; 
		/* termina a string do comando e separa o outfile em outra string */

		strOutFile = &(strString[SinalMaior + 1]);
	}

	if (SinalMenor != 0)
	{
		strString[SinalMenor] = '\0'; 
		/* termina a string do comando e separa o infile em outra string */

		strInFile = &(strString[SinalMenor + 1]);
	}
	
	trim(strString);
	trim(strOutFile);
	trim(strInFile);	
	
	return 1; /* Sai com sucesso */
	
	
	
}
int ParseCommand(char * strCommand, char ** pComando,  char ** vetArgs)
{

	/* procura comando principal */
	int num_args = 0;
	char * pLinha = strCommand;
	for (num_args = 0; num_args < NUM_ARGS; num_args++) vetArgs[num_args] = (char *)0;
	num_args = 1;
	if (!*pLinha) return 0; /* linha vazia */
	vetArgs[0] = *pComando = pLinha; 
	/* O proprio comando eh passado para o programa como o primeiro argumento*/


	/* Comeca com o vetor vazio */

	while (*pLinha)
	{
		/* Anda ate o final da palavra */
		while (*pLinha != ' ' && *pLinha != '\0') pLinha++;
		
		/* se a string nao terminou, procura a proxima palavra */
		if (*pLinha)
		{
			*pLinha = 0; /* termina a string do comando ou do argumento anterior */
			pLinha++;
			
			/* anda os espacos vazios e procura ate aparecer um caracter visivel */
			while ((*pLinha == ' ') || (*pLinha == '\t')) pLinha++;
			
			/* Se o caracter encontrado nao for fim de string, 
				coloca o endereco dela como proximo argumento 
			*/
			if (*pLinha)
				vetArgs[num_args++] = pLinha;
				
			else break;

		} else break;
	}
}


void printErroParseRedir()
{
	printf("Erro: duas ou mais redireções de entrada ou saída encontradas\n");
}

int verifica(char * strCmd)
{
	/*
	int i;
	int tam = strlen(strCmd);
	int cmd_ok = 1;
	int em_redir = 0;
	int em_pipe = 0;
	for (i = 0; i < tam; i++)
	{
		if (em_pipe)
		{
			if (strCmd[i] == '|' || strCmd[i] == '>' || strCmd[i] == '<');
		}
		if (!em_redir)
		{
			if (strCmd[i] == '>' || strCmd[i] == '<') 
				em_redir = 1;

		} else {
			if (strCmd[i] == '>' || strCmd[i] == '<') 
				return 0;
			else if (strCmd[i] == '|')
				return 0;
		}

	}
	*/
}

void execPipeAux(int numCmd, int imprime_pid)
{
	/* Função recursiva para cascatear os pipes entre os processos. */
	int fd[2];
	int pid;
	/* Cria o pipe para levar a saida do primeiro processo para a entrada do segundo */
	pipe(fd);

	/* Cria um processo filho */
	pid = fork();
	if (pid == ERRO_FORK)
	{
		ImprimeMsgErroFork();
		exit(1);
	}
	if (pid == 0)
	{
		/* Processo Filho: */
		
		close(fd[0]); /* Fecha a ponta de leitura do pipe, pois nao é usada neste processo */
		dup2(fd[1], STDOUT_FILENO); /* duplica o descritor de escrita para o descritor da saida padrão */
		close(fd[1]); /* fecha o descritor original de escrita */
		
		if (numCmd==1)
		{
			/* Caso base da recursão, se for o primeiro processo nao se chama recursivamente */
			execRedirCmd(pipeCommands[0], 0);
		} else {
			/* se ainda tiverem mais processos se chama recursivamente */
			execPipeAux(numCmd - 1, 0);
		}
		/* nunca deve chegar neste ponto, mas se chegar, sai sinalizando erro */
		exit(1);
	}
	/* Processo Pai: */
	
	/* Cria outro processo filho */
	pid = fork();
	if (pid == ERRO_FORK)
	{
		ImprimeMsgErroFork();
		exit(1);
	}
	if (pid == 0)
	{
		/* Processo Filho*/
		close(fd[1]); /* fecha a ponta de escrita do pipe, pois nao é usada aqui */
		dup2(fd[0], STDIN_FILENO); /* duplica o descritor de leitura para o descritor da entrada padrao */
		close(fd[0]); /* fecha o descritor original de leitura */

		execRedirCmd(pipeCommands[numCmd], 0); /* executa o comando */

		/* nunca deve chegar neste ponto, mas se chegar sai, sinalizando erro */
		exit(1);
	}
	
	if (imprime_pid) printf("%d\n", pid);
	
	close(fd[0]); /* No processo pai nao é usado o pipe, entao fecha os dois descritores*/
	close(fd[1]);

	wait(NULL); /* espera os dois filhos terminarem */
	wait(NULL);

	return;
}

void execECmd(char * strCmd)
{
	int i;
	int pid;
	ParseECommand(strCmd);
	for (i = 0; i < numECmds - 1; i++)
	{
		pid = fork();
		if (pid == ERRO_FORK)
		{
			ImprimeMsgErroFork();
			exit(1);
		}
		if (pid == 0)
		{
			execPipeCmd(eCommands[i], 1);
			exit(1);
		}
	}
	execPipeCmd(eCommands[i], 0);
	
}
void execPipeCmd(char * strCmd, int imprime_pid)
{
	ParsePipeCommand(strCmd);
	if (!numPipeCmds) exit(0);

	if (numPipeCmds > 1)
	{
		execPipeAux(numPipeCmds-1, imprime_pid);	
	} else if (numPipeCmds == 1) {
		execRedirCmd(pipeCommands[0], imprime_pid);
	}
}

void execRedirCmd(char * strCmd, int imprime_pid)
{
	FILE* InFile;
	FILE* OutFile;
	char* comando;
	char* vetArgumentos[NUM_ARGS];
	int pid;
	int i;	
	
	
	
	/* string vazia e ignorada */
	if (*strCmd == 0) exit(0);
	
	if (!ParseRedirectCommand(strCmd))
	{
		printErroParseRedir();
		exit(1);
	}

	if (imprime_pid)
	{
		pid = fork();
		if (pid == ERRO_FORK)
		{
			ImprimeMsgErroFork();
			exit(1);
		}
		if (pid)
		{
			printf("%d\n", pid);
			exit(0);
		}
	}
	ParseCommand(strCmd, &comando, vetArgumentos);
	
	if (strInFile != NULL)
	{
		/* printf("Redirecionando %s para stdin\n", strInFile); */
		/* redirecionamos o arquivo desejado para a entrada padrao */
		InFile = freopen(strInFile, "r", stdin);
		stdin = InFile;
		
	}
	if (strOutFile != NULL)
	{
		/* printf("Redirecionando stdout para %s\n", strOutFile); */
		/* redirecionamos o arquivo desejado para a entrada padrao */
		OutFile = freopen(strOutFile, "w", stdout);
		stdout = OutFile;
		
	}


/*	
*	IMPIME ARGUMENTOS PASSADOS *
	
	fprintf(stderr, "Iniciando Processo:\n");
	
	fprintf(stderr, "Comando: %s\n", comando);
	for (i = 0; vetArgumentos[i]; i++)
		fprintf(stderr, "Arg %d: %s\n", i, vetArgumentos[i]);
		
	*/

	if (execvp(comando, vetArgumentos) == -1)
	{
		/* O programa não rodou */
		fprintf(stderr, "Comando inválido.\n");
	}

	exit(0);
}


void processoFilho()
{
	execECmd(strLinha);
	exit(0);
}

void processoPai(int childPID)
{
	int ret; /* receberÃ¡ o status de retorno do processo criado */
	waitpid(childPID, &ret, 0);

}

int zmain()
{
	while(1)
	{
		prompt();
		ParseRedirectCommand(strLinha);
	}
}

int main()
{
	int childPID = -1;
	FILE * fd;
	char buf[255];
	printf("\nTrabalho de SC1. Shell Simples\n\n");
	
	while(1)
	{
		
		prompt();

		childPID = fork();

		if (childPID == ERRO_FORK)
		{
			ImprimeMsgErroFork();
			continue;
		}
		if (childPID) processoPai(childPID);
		else processoFilho();
	}
}



