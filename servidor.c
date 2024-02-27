#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PUERTO 8080
#define MAX_CLIENTES 5
#define MAX_LONGITUD_PREGUNTA 256
#define MAX_LONGITUD_RESPUESTA 128
#define NUM_RESPUESTAS 4
#define NUM_PREGUNTAS 10

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    char pregunta[MAX_LONGITUD_PREGUNTA];
    char respuestas[NUM_RESPUESTAS][MAX_LONGITUD_RESPUESTA];
    int respuesta_correcta;
} Pregunta;

void ingresar_pregunta(Pregunta *pregunta) {
    printf("Ingrese la pregunta: ");
    fgets(pregunta->pregunta, MAX_LONGITUD_PREGUNTA, stdin);
    pregunta->pregunta[strcspn(pregunta->pregunta, "\n")] = '\0';  // Elimina el salto de línea al final
    
    for (int j = 0; j < NUM_RESPUESTAS - 1; ++j) {
        printf("Ingrese la respuesta #%d incorrecta: ", j + 1);
        fgets(pregunta->respuestas[j], MAX_LONGITUD_RESPUESTA, stdin);
        pregunta->respuestas[j][strcspn(pregunta->respuestas[j], "\n")] = '\0';  // Elimina el salto de línea al final
    }

    printf("Ingrese la respuesta correcta: ");
    fgets(pregunta->respuestas[NUM_RESPUESTAS - 1], MAX_LONGITUD_RESPUESTA, stdin);
    pregunta->respuestas[NUM_RESPUESTAS - 1][strcspn(pregunta->respuestas[NUM_RESPUESTAS - 1], "\n")] = '\0';  // Elimina el salto de línea al final

    pregunta->respuesta_correcta = NUM_RESPUESTAS - 1;
}

int main() {
    int sockfd, nuevo_sockfd;
    socklen_t long_cli;
    struct sockaddr_in serv_addr, cli_addr;
    Pregunta preguntas[NUM_PREGUNTAS];
    int puntos = 0;

    // Crear socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error al abrir el socket");

    // Opción para permitir reutilizar la dirección y el puerto
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        error("Error al configurar setsockopt");

    // Inicializar la estructura de dirección del servidor
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PUERTO);

    // Enlazar el socket a la dirección
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Error al enlazar");

    // Comenzar a escuchar las conexiones entrantes
    listen(sockfd, MAX_CLIENTES);

    long_cli = sizeof(cli_addr);

    int num_preguntas = 0;
    char respuesta;
    do {
        ingresar_pregunta(&preguntas[num_preguntas++]);

        printf("¿Desea ingresar otra pregunta? (s/n): ");
        scanf(" %c", &respuesta);
        getchar(); // Limpiar el buffer de entrada

    } while (respuesta == 's');

    printf("Esperando el ingreso del cliente...\n"); // Indicador de espera

    while (1) {
        // Aceptar la conexión del cliente
        nuevo_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &long_cli);
        if (nuevo_sockfd < 0)
            error("Error al aceptar");

        // Envía las preguntas al cliente
        for (int i = 0; i < num_preguntas; ++i) {
            if (write(nuevo_sockfd, &preguntas[i], sizeof(Pregunta)) < 0)
                error("Error al escribir en el socket");
            printf("Enviando pregunta %d\n", i + 1); // Debugging
            sleep(1); // Espera para dar tiempo al cliente para recibir
        }

        // Recibir respuestas del cliente y evaluar
        int respuesta_usuario;
        do {
            if (read(nuevo_sockfd, &respuesta_usuario, sizeof(respuesta_usuario)) < 0)
                error("Error al leer del socket");

            // Evaluar respuesta y asignar puntos
            if (respuesta_usuario >= 1 && respuesta_usuario <= NUM_RESPUESTAS) {
                int pregunta_actual = num_preguntas - 1;
                if (respuesta_usuario == preguntas[pregunta_actual].respuesta_correcta + 1) {
                    puntos += 100;
                    printf("Respuesta correcta! Puntos obtenidos: %d\n", puntos);
                } else {
                    puntos += 0;
                    printf("Respuesta incorrecta. Puntos obtenidos: %d\n", puntos);
                }
            }
        } while (respuesta_usuario >= 1 && respuesta_usuario <= NUM_RESPUESTAS);

        // Envía los puntos al cliente
        if (write(nuevo_sockfd, &puntos, sizeof(puntos)) < 0)
            error("Error al escribir en el socket");

        // Espera a que el cliente confirme
        char confirmacion;
        printf("Esperando confirmación del cliente... Presione 'c' para continuar: ");
        scanf(" %c", &confirmacion);

        while (confirmacion != 'c') {
            printf("Esperando confirmación del cliente... Presione 'c' para continuar: ");
            scanf(" %c", &confirmacion);
        }

        close(nuevo_sockfd);
        puntos = 0; // Reinicia los puntos para el próximo cliente
    }

    close(sockfd);
    return 0;
}

