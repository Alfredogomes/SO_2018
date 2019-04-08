#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>

static volatile int semErros = 1;
int numOut =0; 

void preservaFicheiro() {
    semErros = 0;
}

struct Node  {
    char* result;
    struct Node* next;
    struct Node* prev;
};

struct Node* head; // aponta para a cabeça da lista

// dar o resultado anterior
char* lastResult;

//cria um noco nó e devolve o seu apontador 
struct Node* GetNewNode(char*r) {
    struct Node* newNode
        = (struct Node*)malloc(sizeof(struct Node));
    newNode->result = r;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

int getLength(){
	int i =0;
	struct Node* temp = head;
	while(temp != NULL) {
        
        temp = temp->next;
        i++;
    }
    return i;
}

void printList() {
    struct Node* temp = head;
    //printf("imprimir lista\n");
    while(temp != NULL) {
        printf(">>>\n ");
        printf("%s\n ",temp->result);
        printf("<<<\n ");
        temp = temp->next;
    }
    printf("\n");
}

//Insere um nó no fim da lista
void InsertNode(char*r) {
    //printf("Funcao: %s\n",c);
    //printf("Pipe contents: %s\n", r);
    struct Node* temp = head; //nó temporário para navegar na list
    struct Node* newNode = GetNewNode(r);
    if(head == NULL) {
        head = newNode;
        return;
    }
    while(temp->next != NULL) temp = temp->next; // vai até último nó
    temp->next = newNode; // apontador para o nó seguinte do último nó passa a ser o novo nó
    newNode->prev = temp; // apontador para o nó anterior ao novo nó passa a ser o último nó da lista
}

//devolve o resultado de um dos nós anteriores, indicado por pos.
//para pos = 1 é o comando atual, para pos = 2 é o comando anterior, etc
char* getPrevResult(int pos, struct Node* anterior){
    char* res;
    int i = pos - 1;
    struct Node* temp = anterior;
    while(temp -> next != NULL && i > 0) temp = temp->next;
    if(i == 0){
        return temp->result;
    }
    else {
        return NULL;
    }
}

// protótipo compativel com a chamada ao sistema read
// lê de um identificador de ficheiro (fildes),
// o que lê coloca num sitio na memória (apontador para caracteres)
// lê nbytes (no máximo)
ssize_t readln(int fildes, void *buf, size_t nbyte){
    ssize_t nb = 0;
    char* p = buf; // aponta para o primeiro char do buffer
    while (1){
        // o número de caracteres lidos igualar o máximo de caracteres
        // que se pode ler, interrompe o ciclo
        if(nb==nbyte) break;
        // chamada ao sistema read que lê o caracter em questão
        // retorna maior que zero se a leitura foi efetuada com sucesso e 
        // zero se insucesso
        ssize_t n = read(fildes, p ,1);
        // se o resultado do read for 0, significa que não tem 
        // mais para ler
        if(n<=0)break;
        //incrementa o número de caracteres lidos
        nb+=1;
        // se o caracter for \n é porque chegou ao fim da linha
        if(*p == '\n')break;
        // vai a proxima posição do buffer p (ou seja, proximo caracter a ler)
        p+=1;
    }

    return nb;
}


// 3 Interpretador de comandos baseado no exercicio 7 
//do guiao 3. Recebe o comando que é suposto executar
void executeCommand(char* command, int fildes){
    int n, i, pid;
    char buffer[4096];
    // apontador para apontador onde vamos guardar os 
    // comandos e os argumentos a executar
    char* executar[512];
    
    //buffer s para onde será enviado o comando a executar
    char* s;
    // criar um pipe (um para escrita e outro para leitura)
    int link[2];
    pid_t pid2;
    char str[4096];
    pipe(link);

    for(i=0; i!=sizeof(buffer); buffer[i++]='\0');

    // se existir mesmo um comando para executar
    if((n=strlen(command+1)) >0 ){
        
        // parte "command" com o delimitador " \n\0" e coloca em s 
        s = strtok(command, " \n\0");

        for(i=0; s!=NULL; i++){
            
            // aloca espaço em memória num ponteiro 
            // de tipo char com espaço n (comprimento do comando)
            // vezes o tamanho de um caracter
            executar[i] = (char*) malloc(n*sizeof(char));
            // copia o comando/argumento que está em s
            // para "executar"
            strcpy(executar[i], s);
            // para avançar para o próximo argumento da função
            s=strtok(NULL, " \n\0");
        }
        // coloca o ultimo elemento a NULL
        executar[i]=NULL;

        // Como é necessário fazer um execvp para executar 
        // os programas é necessário um fork e executar o mesmo
        // no processo filho pois a seguir ao exec vem um exit 
        // que mata o processo
        pid = fork();
        if(pid==0){
            // é feito um dup2 de maneira a duplicar 
            // o descritor de ficheiro que está a efetuar a 
            // escrita no pipe para 1. ou seja, 
            // o que se escreveria no standard out (1) vai 
            // se escrever em link[1]
            dup2(link[1], 1);
            // fecha se o descritor de leitura no filho pois este 
            // apenas vai escrever
            close(link[0]);
            
            // faz o execvp do que está em executar[o], com os 
            // argumentos passados 
            execvp(executar[0], executar);
             
            // // para sair do processo filho caso o exec não funcione 
            _exit(-1);
    
        }else {
            int status;
            wait(&status);

            const int what = WEXITSTATUS(status);
            //printf("Isto é isto %d\n",what);

            if(WEXITSTATUS(status)) {
                //printf("child exited with = %d\n",WEXITSTATUS(status));
                preservaFicheiro();
                n=2;
            }
            
            else  {
                close(link[1]);
                while(1){
           
            
                    size_t n = sprintf(buffer, " ");
                    ssize_t nb = read(link[0], buffer+n, sizeof(buffer)-n);
                    //printf("\nbuffer %s\n", buffer+n);
                    write(fildes, buffer+n, nb);
                    if(nb==0) break;
                    int i;
                    for(i = 0; i<nb; i++){
                        if(buffer[i]=='\n'){
                            buffer[i] = ' ';
                        }    
                    }
                    lastResult = malloc(strlen(buffer)+1);
                    for(i=0; i!=sizeof(lastResult); lastResult[i++]='\0');
                    strcpy(lastResult, buffer); 
                    
                    

                    // criar ficheiros temporarios onde vamos armazenar o 
                    // resultado dos comandos
                    char* pos;
                    sprintf(pos, "teste/%d", numOut);
                    char* outputFile = strcat(pos,".txt");
                    int fd = open(outputFile, O_CREAT | O_WRONLY | O_RDONLY , 0666);
                    for(i = 0; i<nb; i++){
                        if(lastResult[i]==' '){
                            lastResult[i] = '\n';
                        }
                       
                    }
                    if (lastResult[0] == '\n') memmove(lastResult, lastResult+1, strlen(lastResult));    
                   
                    lastResult[i] = '\0';

                    ssize_t w = write(fd, lastResult, strlen(lastResult));
                    close(fd);
                    // estrutura
                    InsertNode(lastResult);

                    for(i=0; i!=sizeof(lastResult); lastResult[i++]='\0');
                    for(i=0; i!=sizeof(buffer); buffer[i++]='\0');
                }
        
            wait(NULL);
            }
        }
    }
    numOut +=1;
}

// Interpretador de comandos baseado no exercicio 7 
//do guiao 3. Recebe o comando que é suposto executar
void executePrev(char* command, int fildes){
            char pos[20];
            sprintf(pos, "teste/%d", numOut-1);

            //printf("OLA %s\n" , pos);
            strcat(pos,".txt");
            //printf("OLA %s\n" , pos);
            strcat(command, pos);
            //printf("%s\n" , command);
            executeCommand(command, fildes);
}

// 1 -recebe o descritor de ficheiro do notebook que se quer processar
char* readAndWriteLine(int fe){
    for(int j = 2; j < 13; j++) if (j != 9) signal(j,preservaFicheiro);

    char buf[1024] ;
    int i = 0;

    // 2 cria-se um ficheiro temporário para fazer toda a escrita neste e não haver conflitos (explicado na secção de reprocessamento)
    char* file = "file.txt";
    // 2 chamada ao sistema da função open que abre o ficheiro em causa
    // 2 com as devidas permissões e flags
    int fd = open(file, O_CREAT | O_WRONLY | O_RDONLY , 0666);
    for(i=0; i!=sizeof(buf); buf[i++]='\0');
    // 4 para ignorar se estiver a ser reprocessado
    int signal = 0;
     while(semErros){
            size_t n = sprintf(buf, " ");
            // 1- implementação de uma função com base na função do exercício 
            // 5 do primeiro guião que recebe p decritor do ficheiro a ler,
            // o apontador para onde está a linha a ser lida
            // e o comprimento (size of) do buf-n 
            // este retorna um ssize_t com o número de caracteres lidos
            // (incluindo \n)
            ssize_t nb = readln(fe, buf+n, sizeof(buf)-n);
            
            if(buf[1] == '>' && buf[2] == '>' && buf[3] == '>'){
                signal = 1;   
                }
            
            if(signal==0){
            write(fd, buf+n, nb);
            }

            // se o resultado retornado for = 0
            // significa que o ficheiro foi lido até o fim
            // (não tem mais nenhuma linha para ler depois desta)
            // e interrompe o ciclo
            if(nb==0) break;
            // 2 inteiro para determinar se o buffer termina ou não com \n
            int has_n = 0;
            // 2 percorre o buffer todo e se este tiver um \n coloca o inteiro has_n a 1
            for(i =0 ; i<sizeof(buf); i++){
                if(buf[i]=='\n'){
                    has_n = 1;
                    break;
            }}
            // 2 se essa linha não tiver \n no fim, coloca o \n (util para não haver problemas de
            // 2 escrita, ex: pwd>>>)
            if(has_n==0) write(fd, "\n", 1);
            if(buf[1] == '$' && buf[2] == '|' && signal==0){

                char *ps = buf;
                ps = ps+3;
                int i;
                int has_n = 0;
                for(i =0 ; i<sizeof(ps); i++){
                    if(ps[i]=='\n'){
                        has_n = 1;
                        break;
                }}
                //executa cada comando começado por $
                write(fd, ">>>\n", 4);
                executePrev(ps, fd);
                write(fd, "<<<\n", 4);
            }
            //2 Lê apenas as linhas que começam com $

            if(buf[1] == '$' && signal==0 && strlen(buf+n)-2>0){
                
                //2 coloca o buffer em ps
                char *ps = buf;
                //2 avança duas posições, de maneira a ignorar o $
                ps = ps+2;
                int i;
                
                // 2 para escrever no ficheiro, sabemos a escrita dos outputs 
                // 2 tem de ser delimitada por >>> <<< 
                if(ps[1]!=' '){
                write(fd, ">>>\n", 4);
                // 2 executa o comando em causa
                executeCommand(ps, fd);
                write(fd, "<<<\n", 4);
                
                
                }
                
            }
            if(buf[1]== '<' && buf[2]== '<' && buf[3]== '<'){
                signal =0;
            }
             
            //Para limpar o buffer
            for(i=0; i!=sizeof(buf); buf[i++]='\0');
            }
    
    return file;
    }


int main (int argc, char* argv[]) {
    //1- o ficheiro é aberto 
    //usando o operador ternário ?:
    // se o número de argumentos (argc) for igual a dois (exec notebook)
    // ele abre o ficheiro que está em argv[1], atribuindo o 
    // resultado do open a fe (será 3 uma vez que 0 é standard in
    // 1 standard output e 2 standard error)
    // se não atribui 0 e não executa
    int fe = argc == 2 ? open(argv[1], O_RDONLY) : 0 ;
    char* input = argv[1];
    // 1-atribui a new o resultado de readLine
    // (o ficheiro para o qual está a apontar)
    char* new = readAndWriteLine(fe);
    if(semErros){
        // Re processamento de notebook
        remove(argv[1]);
        rename(new, input);
    }
    else {
        remove(new);
        printf("O notebook possui comandos errados\n");
    }
    return 0;
}