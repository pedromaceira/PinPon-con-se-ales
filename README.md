El objetivo de este código es ver de una manera ilustrativa cómo comunicar procesos mediante señales.

Consiste en una partida de pin pon, en la que el proceso padre iniciará dos procesos: el jugador, que será el usuario, y la máquina, que jugará aleatoriamente. El padre se encargará de llevar el marcador y gestionar los turnos, mientras que los dos hijos simularán una partida siguiendo unas reglas determinadas:
- La posición de la máquina en la mesa se elige aleatoriamente en cada turno en el intervalo [0-9], mientras que la del jugador la introduce el usuario.
- Un jugador puede devolver una bola si se recibe a una distancia de 3 o menos respecto a su posición actual. En caso contrario, no puede tocarla y encaja un punto.
- Al tocar la bola para devolverla, el jugador decide a que posición del intervalo [0,9] la envía. Tras devolver la bola, también podrá moverse si lo desea hasta 2 posiciones desde la suya actual. La máquina lo decide aleatoriamente, mientras que el jugador se lo pregunta al usuario.

Para compartir información entre procesos se utiliza un archivo compartido de tipo binario y la partida la gana el primero en llegar a 10 puntos.
