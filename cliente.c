#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PUERTO 8080
#define IP_SERVIDOR "127.0.0.1"
#define MAX_LONGITUD_PREGUNTA 256
#define MAX_LONGITUD_RESPUESTA 128
#define NUM_RESPUESTAS 4

typedef struct {
    char pregunta[MAX_LONGITUD_PREGUNTA];
    char respuestas[NUM_RESPUESTAS][MAX_LONGITUD_RESPUESTA];
} Pregunta;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    Pregunta pregunta_actual;
    int puntos = 0;

    // Crear socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Error al abrir el socket");

    // Inicializar la estructura de dirección del servidor
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PUERTO);

    // Convertir la dirección IP de texto a forma binaria
    if (inet_pton(AF_INET, IP_SERVIDOR, &serv_addr.sin_addr) <= 0)
        error("Dirección inválida/ Dirección no soportada");

    // Conectar al servidor
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Conexión fallida");

    // Recibir las preguntas del servidor y mostrarlas al usuario
    while (1) {
        if (read(sockfd, &pregunta_actual, sizeof(pregunta_actual)) < 0)
            error("Error al leer del socket");

        if (strlen(pregunta_actual.pregunta) == 0)
            break;  // Termina cuando no hay más preguntas

        printf("Pregunta: %s\n", pregunta_actual.pregunta);
        printf("Opciones de respuesta:\n");
        for (int j = 0; j < NUM_RESPUESTAS; ++j) {
            printf("%d. %s\n", j + 1, pregunta_actual.respuestas[j]);
        }

        int respuesta_usuario;
        printf("Ingrese su respuesta (1-4): ");
        scanf("%d", &respuesta_usuario);

        // Limpiar el búfer de entrada
        while (getchar() != '\n');

        if (write(sockfd, &respuesta_usuario, sizeof(respuesta_usuario)) < 0)
            error("Error al escribir en el socket");

        // Recibir puntos del servidor y actualizar puntuación
        if (read(sockfd, &puntos, sizeof(puntos)) < 0)
            error("Error al leer del socket");

        printf("Puntos obtenidos: %d\n", puntos);
    }

    // Indicador de fin de preguntas
    int fin_preguntas = 0;
    write(sockfd, &fin_preguntas, sizeof(fin_preguntas));

    close(sockfd);
    return 0;
}

