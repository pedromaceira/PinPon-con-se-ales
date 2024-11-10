#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>

void handler(int numsenal);

int aleatorio(int a, int b);

struct Registro {
    int saque;
    int posJug;
    int posBola;
};

struct Jugador {
    char nombre[10];
    pid_t pid;
    int senal;
    int puntos;
    struct Registro reg;
};

pid_t padre; // se crea una variable global para el padre para poder después mandarle señales desde el manejador

int fichero;




int main() {

    padre = getpid();
    printf("\nComienza el programa... \nEl proceso padre (PID: %d) gestionará el marcador y los turnos\n", padre);
    
    
    // se abre el archivo donde se van a compartir los datos
    if ((fichero = open("fichero.bin", O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("\nError en la apertura del archivo\n");
        exit(0);
    }

    // se configura el manejo de la señal SIGCONT
    if (signal(SIGCONT, handler) == SIG_ERR) {
        printf("\nError al realizar el signal(), no se pudo establecer el manejador de SIGUSR2\n");
        exit(0);
    }    
    
    // se inicializan los jugadores JUGADOR y MÁQUINA
    struct Jugador j[2] = {
    
        {"JUGADOR", 0, SIGUSR1, 0, {1, 0, 0}},
        {"MAQUINA", 0, SIGUSR2, 0, {1, 0, 0}}
        
    };
    
    // semilla para calcular los números aleatorios
    srand(time(NULL));
    
    // lo siguiente es crear los hijos e interactuar con ellos
    
    // se crean los dos hijos
    
    // primero vamos con el hijo JUGADOR
    j[0].pid = fork();
    
    if (j[0].pid < 0) {
            
        printf("\nError en la creación del hijo JUGADOR\n");
        exit(0);
                
    }
    
    // si el hijo JUGADOR se crea correctamente
    if (j[0].pid == 0) {
    
        printf("\nEl proceso padre ha creado correctamente al proceso JUGADOR\n");
            
        if (signal(SIGUSR1, handler) == SIG_ERR) {
            printf("\nError al realizar el signal(), no se pudo establecer el manejador de SIGUSR2\n");
            exit(0);
        }
        
        // el JUGADOR entra en un bucle infinito esperando que el padre le mande señales
        while(1) {
            pause();      
        }
                
    }
    
    // ahora vamos con el hijo MAQUINA
    j[1].pid = fork();
    
    if (j[1].pid < 0) {
            
        printf("\nError en la creación del hijo MAQUINA\n");
        exit(0);
                
    }    
    
    // si el hijo MAQUINA se crea correctamente
    if (j[1].pid == 0) {
    
        printf("\nEl proceso padre ha creado correctamente al proceso MAQUINA\n");
            
        if (signal(SIGUSR2, handler) == SIG_ERR) {
            printf("\nError al realizar el signal(), no se pudo establecer el manejador de SIGUSR2\n");
            exit(0);
        }
        
        // la MAQUINA entra en un bucle infinito esperando que el padre le mande señales
        while(1) {
            pause();      
        }
                
    }
    
    
    
    
    // A PARTIR DE AQUÍ ES CÓDIGO DEL PROCESO PADRE, QUE DEBE GESTIONAR LOS TURNOS Y EL MARCADOR
    
    // la variable saque se inicializa a 0 y sirve para tener en cuenta si es la primera jugada del punto o la bola ya está en juego
    int turno, saque = 1;
    
    // mientras ninguno llegue al máximo de puntos la partida no terminará
    while (j[0].puntos < 10 && j[1].puntos < 10) {
    
        // si esto se cumple, quiere decir que es la primera jugada del punto
        if (saque == 1) {
        
            // se imprime un mensaje para llevar la información de cómo va la partida
            printf("\nMARCADOR: Jugador %d - %d Máquina\n", j[0].puntos, j[1].puntos);
            
            // la variable turno lleva la cuenta de que jueguen los dos ¿¿¿¿¿¿¿¿¿¿¿¿
            for (turno = 0; turno <= 1; turno++) {

                // se escriben los datos en el fichero compartido
                if (pwrite(fichero, &j[turno].reg, sizeof(struct Registro), 0) == -1) {
    		    printf("\nError al escribir en el archivo\n");
    		    exit(0);
		}

// Sincronizar los datos para asegurar que se guarden inmediatamente
		if (fdatasync(fichero) == -1) {
    		    printf("\nError al sincronizar datos con el disco\n");
    		    exit(0);
		}
                
                kill(j[turno].pid, j[turno].senal);
                
                pause();
                
                // se leen los datos del fichero compartido
                if (pread(fichero, &j[turno].reg, sizeof(struct Registro), 0) == -1) {
                    printf("\nError al leer el archivo\n");
                    exit(0);
		}
		
                j[turno].reg.saque = 0;
            
            }
            
            // el padre elige de manera aleatoria si saca la máquina o el jugador (0/1)
            turno = aleatorio(0, 2);
            
            // se indica que ya no es el primer punto de la partida
            saque = 0;
            
            // se informa de quién saca
            printf("\nComienza el punto, saca %s\n", j[turno].nombre);

        }
        
        // ESTO ES SI LO QUE PASA EN CADA JUGADA DEL PUNTO, DESPUÉS DE DECIDIR QUIÉN SACA       
        
        // se esriben los datos en el fichero compartido
        if (pwrite(fichero, &j[turno].reg, sizeof(struct Registro), 0) == -1) {
    	    printf("\nError al escribir en el archivo\n");
    	    exit(0);
	}

	// se sincronizan los datos para asegurar que se guarden inmediatamente
	if (fdatasync(fichero) == -1) {
    	    printf("\nError al sincronizar datos con el disco\n");
    	    exit(0);
	}

	// el padre le envía la señal de que le toca jugar al que tiene el turno
        kill(j[turno].pid, j[turno].senal);
        
        // el padre espera hasta que recibe la señal de que el jugador terminó su turno
        pause();
        
        // se escriben los datos en el fichero
        if (pread(fichero, &j[turno].reg, sizeof(struct Registro), 0) == -1) {
             printf("\nError al leer el archivo\n");
             exit(0);
	}
	
        
        // AQUÍ SE GESTIONA CÓMO SE HACEN LOS PUNTOS
        
        // este if sirve para comprobar si se anota punto, si la bola va a más de 3 casillas de la posición del jugador que la recibe 
        if (abs(j[turno].reg.posBola - j[!turno].reg.posJug) > 3) {
        
            //Se anotan puntos para diferencias entre posiciones > 3
            j[turno].puntos += 1;
            printf("\n%s anota un punto\n", j[turno].nombre);

            //El siguiente Registro sera el primero del punto a jugar
            saque = 1;
            j[0].reg.saque = 1;
            j[1].reg.saque = 1;
            
        } else {
           
            printf("\n%s no anota punto, por lo que el turno cambia\n", j[turno].nombre);
            turno = !turno;   
            
        }
         
    }
    
    // cuando se sale del bucle quiere decir que uno de los dos ha llegado a 10 puntos, por lo que la partida termina
    
    printf("\nMARCADOR: Máquina %d - %d Jugador\n", j[0].puntos, j[1].puntos);
    
    if (j[0].puntos == 10) {
        printf("\nLa partida ha terminado, el ganador es %s\n\n", j[0].nombre);
    } else {
        printf("\nLa partida ha terminado, el ganador es %s\n\n", j[1].nombre);
    }

    // para acabar correctamente se mata a los procesos JUGADOR y MAQUINA
    kill(j[0].pid, SIGKILL);
    kill(j[1].pid, SIGKILL);
    
    // se comprueba que los hijos terminan
    waitpid(j[0].pid, NULL, 0);
    waitpid(j[1].pid, NULL, 0);
    
    // se cierra el archivo y termina el programa
    close(fichero);

    exit(1);
         
}


// ESTA ES LA FUNCIÓN MANEJADORA DE LAS SEÑALES
void handler(int numsenal) {

    struct Registro reg;
    
    // se ejecuta un switch para ir gestionando las señales recibidas
    switch (numsenal) {
     
        // si se recibe SIGUSR1 se asocia con el proceso del jugador
        case SIGUSR1:
        
            int pos;
            
            if (pread(fichero, &reg, sizeof(struct Registro), 0) == -1) {
                printf("\nError al leer el archivo\n");
                exit(0);
	    }
    	    
    	    // si es en la jugada del saque, se le pide al usuario que introduzca la posición inicial del jugador
    	    if (reg.saque) {
    	    
    	        do {
    	        
    	            printf("\nIntroduce la posición inicial del 0 al 9 para JUGADOR: \n");
    	            scanf(" %d", &pos);
    	            
    	        } while ((pos<0) || (pos>9));
    	        
    	        reg.posJug = pos;
    	        printf("\nLa posición inicial del jugador es: %d\n", reg.posJug);
    	        
    	        // se desactiva la variable de saque, ya que las siguientes no son para la primera jugada
    	        reg.saque = 0;
            
            } else {
            
                // si el movimiento no es de saque
                
                int base = ((reg.posJug - 2) < 0) ? 0 : (reg.posJug - 2);
                int tope = ((reg.posJug + 2) > 9) ? 9 : (reg.posJug + 2);
                
                do {
                
                    printf("\nIntroduce la posición donde JUGADOR va a mandar la bola: \n");
                    scanf(" %d", &pos);
                    
                } while ((pos < 0) || (pos > 9));
                
                reg.posBola = pos;
                
                do {
                
                    printf("\nIntroduce la nueva posición de JUGADOR a dos unidades o menos de la actual (entre %d y %d): \n", base, tope);
                    scanf(" %d", &pos);
                    
                } while ((pos < base) || (pos > tope));
                
                reg.posJug = pos;
                
                printf("\nEntendido, la posición de JUGADOR pasa a ser %hu y tira la bola a la posición %hu\n", reg.posJug, reg.posBola);
                
            }
            
            // se escriben los datos en el fichero compartido
            if (pwrite(fichero, &reg, sizeof(struct Registro), 0) == -1) {
    	        printf("\nError al escribir en el archivo\n");
    	        exit(0);
	    }

	    // se sincronizan los datos para asegurar que se guarden inmediatamente
	    if (fdatasync(fichero) == -1) {
    	        printf("\nError al sincronizar datos con el disco\n");
    	        exit(0);
	    }
            
            // se le envía la señal al padre para que continúe
            kill(padre, SIGCONT);
            break;
        
        
        // si se recibe SIGUSR2 se asocia con el proceso de la máquina
        case SIGUSR2:
        
            // SE LEE DEL FICHERO
            if (pread(fichero, &reg, sizeof(struct Registro), 0) == -1) {
                printf("\nError al leer el archivo\n");
                exit(0);
	    }
            
            
            // si es en la jugada del saque, para seleccionar de manera aleatoria la posición de la máquina
            if (reg.saque) {
                
                reg.posJug = aleatorio(0, 10);
                printf("\nLa posición inicial seleccionada al azar de la máquina es: %hu\n", reg.posJug);
                
                // se desactiva la variable de saque, ya que las siguientes no son la primera jugada
                reg.saque = 0;
                
            } else {
            
                // si no es el saque, sólo se puede mover dos posiciones de la actual
                
                // se calcula a qué posición va a ir la bola
                reg.posBola = aleatorio(0, 10);
                
                // se calcula la nueva posición de la máquina
                reg.posJug = aleatorio((((reg.posJug - 2) < 0) ? 0 : (reg.posJug - 2)), (((reg.posJug + 2) > 9) ? 9 : (reg.posJug + 2)) + 1);
                
                printf("\nLa máquina tira la bola a la posición: %hu\ny se mueve a la posición %hu\n", reg.posBola, reg.posJug);                   
            
            }
            
            // SE ESCRIBE EN EL FICHERO
            if (pwrite(fichero, &reg, sizeof(struct Registro), 0) == -1) {
    	        printf("\nError al escribir en el archivo\n");
    	        exit(0);
	    }

// Sincronizar los datos para asegurar que se guarden inmediatamente
	    if (fdatasync(fichero) == -1) {
    	        printf("\nError al sincronizar datos con el disco\n");
    	        exit(0);
	    }
            
            // cuando se termina de interactuar, se le manda la señal al padre para que continúe
            kill(padre, SIGCONT);
            break;
        
        
        // señal del proceso padre
        case SIGCONT:
            break;
        
    }
    
}
            
            
// FUNCIÓN QUE CALCULE UN NÚMERO ALEATORIO DENTRO DE UN RANGO DETERMINADO
int aleatorio(int a, int b) {

    if (a > b) {
        
        int aux;
        aux = a;
        a = b;
        b = aux;
        
    }
    
    double escala = (double) rand() / (double) ((double) RAND_MAX + 1);
    return a + (int) ((b - a) * escala);

} 
