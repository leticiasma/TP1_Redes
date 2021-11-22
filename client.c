#include "uteisClient.h"

//---------------------------------------------------------------------------------

int main(int argc, char **argv) {

    int socketCliente;

    configurarCliente(argc, argv, &socketCliente);

	char buf[BUFSZ];

    bool desconectar = false;
    bool mensagemCompleta = false;

    //Permite a troca de mensagens até enviar um comando kill ou até o servidor parar de executar
    while(1){

        //Limpa o buffer
        memset(buf, 0, BUFSZ); 
    
        fgets(buf, BUFSZ-1, stdin);

        //Quantidade de bytes enviada
        size_t count = send(socketCliente, buf, strlen(buf), 0);

        if (count != strlen(buf)) {
            logexit("send"); 
        }

        memset(buf, 0, BUFSZ);

        unsigned total = 0;
        mensagemCompleta = false;
        
        //Fica recebendo pacotes do servidor
        while(1) { 

            count = recv(socketCliente, buf + total, BUFSZ - total, 0); 

            if (count == 0) {
                desconectar = true;
                break;
            }

            char barra_n[2] = "\n";

            for (int i=total; i<count+total; i++){

                if (strcmp(&buf[i], barra_n)==0){
                    break;
                }

            }

            total += count;

            if(mensagemCompleta){
                break;
            }

        }

        fputs(buf, stdout);

        if (desconectar){
            break;
        }
        
    }

	close(socketCliente);

	exit(EXIT_SUCCESS);
}