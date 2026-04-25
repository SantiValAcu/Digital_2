/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ili9341.h"	// Libreria para LCD
#include "Bitmaps.h"	// Libreria de los sprites
#include "fatfs_sd.h"	// Libreria para SD
#include "stdio.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Estados posibles del jugador (reposo, movimiento, aire, vuelo y caída)
typedef enum {
    JUGADOR_IDLE = 0,
    JUGADOR_CAMINANDO,
    JUGADOR_AIRE,
    JUGADOR_VUELO,
    JUGADOR_CAIDA
} JugadorEstado;
// Dirección en la que mira el jugador (izquierda o derecha)
typedef enum {
    DIR_IZQUIERDA = 0,
    DIR_DERECHA
} JugadorDireccion;
// Estructura principal del jugador, contiene posición, movimiento, estado y animaciones
typedef struct {
    int x;                  // Posición actual en X
    int y;                  // Posición actual en Y
    int x_anterior;         // Posición anterior en X (para borrar/redibujar)
    int y_anterior;         // Posición anterior en Y (para borrar/redibujar)
    int w_anterior;         // Ancho anterior del sprite
    int h_anterior;         // Alto anterior del sprite
    int vx;                 // Velocidad en eje X
    int vy;                 // Velocidad en eje Y
    uint8_t vidas;          // Número de vidas del jugador
    uint8_t vivo;           // Estado de vida (1 = vivo, 0 = muerto)
    uint8_t enSuelo;        // Indica si el jugador está sobre una plataforma
    uint8_t invencible;     // Indica si el jugador es invencible temporalmente
    uint32_t tiempoInvencible; // Tiempo desde que inició la invencibilidad
    JugadorEstado estado;   // Estado actual del jugador
    JugadorDireccion direccion; // Dirección actual del jugador
    JugadorEstado estado_anterior; // Estado anterior (para detectar cambios)
    JugadorDireccion direccion_anterior; // Dirección anterior
    uint8_t frameAnim;      // Frame actual de animación (caminar/vuelo)
    uint8_t frameCaida;     // Frame actual de animación de caída
    uint32_t ultimoTickAnim;   // Control de tiempo para animaciones
    uint32_t ultimoTickCaida;  // Control de tiempo para animación de caída
} Jugador;
// Estructura para definir rectángulos de colisión (plataformas, zonas de muerte, etc.)
typedef struct {
    int x;  // Posición en X
    int y;  // Posición en Y
    int w;  // Ancho del rectángulo
    int h;  // Alto del rectángulo
} RectColision;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
FATFS fs;
FIL fil;
FRESULT fres;
#define BTN_UP   	  (1 << 0) // 00000001
#define BTN_DOWN      (1 << 1) // 00000010
#define BTN_RIGHT     (1 << 2) // 00000100
#define BTN_LEFT      (1 << 3) // 00001000
#define BTN_CUADRADO  (1 << 4) // 00010000
#define BTN_X         (1 << 5) // 00100000
#define BTN_CIRCULO   (1 << 6) // 01000000
#define BTN_TRIANGULO (1 << 7) // 10000000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
/* USER CODE BEGIN PV */
//Logica de estados
typedef enum {
    ESTADO_MENU,         	// Pantalla inicial, fondo de inicio y selector de inicio
    ESTADO_TRANSICION,   	// Animación de cambio de mapa (Mapa 1, 2 o 3)
    ESTADO_DIBUJO,        	// Dibujo de mapa
    ESTADO_JUEGO,     		// Logica del juego en sí
	ESTADO_GANADOR,			// Mostrar fondo con ganador
    ESTADO_CREDITOS       	// Pantalla final
} GameState;
GameState estadoActual = ESTADO_MENU; // Empezamos en estado 0
int mapa_actual = 1; // Indica el mapa actual (1, 2 o 3)
int credito_actual = 1; // Controla qué pantalla de créditos se muestra
uint32_t tiempoInicioMenu = 0; // Guarda el tiempo de inicio del menú
uint32_t ultimoTickAnim = 0;   // Control de tiempo para animaciones generales
int frameAnim = 0;             // Frame actual de animación global
bool fondoDibujado = false;    // Indica si el fondo ya fue dibujado para evitar redibujos innecesarios

// Variables que almacenan los datos recibidos desde los controles (ESP32 vía UART)
uint8_t mando1_byte = 0; // Datos del ESP32 en UART4
uint8_t mando2_byte = 0; // Datos del ESP32 en UART5

// Estructuras de los dos jugadores
Jugador jugador1;
Jugador jugador2;

// Contador de rondas ganadas por cada jugador
uint8_t rondasJ1 = 0;
uint8_t rondasJ2 = 0;

// Variables para detectar cambios en los botones (comparación estado anterior)
uint8_t mando1_anterior = 0;
uint8_t mando2_anterior = 0;

// Posición del selector en el menú (por ejemplo, opciones 1 o 2)
uint8_t Posicion = 1; // 1 o 2

// Variable que almacena quién ganó (1 = jugador 1, 2 = jugador 2)
uint8_t Ganador = 0; // 1 o 2

// gráficos (bitmaps utilizados en la interfaz)
extern uint16_t R1[];
extern uint16_t R2[];
extern uint16_t R3[];
extern uint16_t ZERO[];
extern uint16_t ONE[];
extern uint16_t UNO[];
extern uint16_t DOS[];

// ---------- MAPA 1 ----------
// Definición de plataformas del mapa 1 (zonas donde el jugador puede pararse)
RectColision mapa1_plataformas[] = {
    {85, 105, 149, 10},
    {0, 201, 73, 38},
    {246, 201, 73, 38}
};
const int mapa1_num_plataformas = 3; // Cantidad de plataformas

// Zona de muerte del mapa 1 (agua)
RectColision mapa1_muerte[] = {
    {73, 219, 173, 20}
};
const int mapa1_num_muerte = 1; // Cantidad de zonas de muerte

// ---------- MAPA 2 ----------
// Definición de plataformas del mapa 2
RectColision mapa2_plataformas[] = {
    {0, 31, 63, 10},
    {192, 105, 127, 10},
    {0, 201, 73, 38},
    {246, 201, 73, 38}
};
const int mapa2_num_plataformas = 4; // Cantidad de plataformas

// Zona de muerte del mapa 2
RectColision mapa2_muerte[] = {
    {73, 219, 173, 20}
};
const int mapa2_num_muerte = 1; // Cantidad de zonas de muerte

// ---------- MAPA 3 ----------
// Definición de plataformas del mapa 3
RectColision mapa3_plataformas[] = {
    {0, 52, 31, 10},
    {288, 52, 31, 10},
    {128, 127, 148, 10},
    {0, 201, 73, 38},
    {150, 201, 41, 38},
    {246, 201, 73, 38}
};
const int mapa3_num_plataformas = 6;  // Cantidad de plataformas

// Zonas de muerte del mapa 3
RectColision mapa3_muerte[] = {
    {73, 219, 77, 20},
    {191, 219, 55, 20}
};
const int mapa3_num_muerte = 2; // Cantidad de zonas de muerte

// Control de reproducción de sonido (tiempo y estado activo)
uint32_t tiempoSonido = 0;
uint8_t sonidoActivo = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART4_Init(void);
static void MX_UART5_Init(void);
/* USER CODE BEGIN PFP */
void LCD_DrawFromSD(char* filename); 									// Carga y dibuja una imagen desde la memoria SD
void InicializarJugadores(void); 										// Inicializa las variables de los jugadores al iniciar partida
void ActualizarJugador(Jugador *j, uint8_t mando, uint8_t numJugador); 	// Actualiza movimiento, estado y física del jugador
void DibujarJugador(Jugador *j, uint8_t numJugador); 					// Dibuja el sprite del jugador según su estado
void DibujarVidasJugador(Jugador *j, uint8_t numJugador); 				// Dibuja las vidas (globos) del jugador
void MorirJugador(Jugador *j); 											// Maneja la lógica cuando el jugador muere
void DibujarHUDVidas(void); 											// Dibuja el HUD con las vidas o rondas de los jugadores
uint8_t ColisionRect(int x, int y, int w, int h, RectColision r); 		// Verifica colisión entre un rectángulo y otro
uint8_t EstaEnZonaMuerte(Jugador *j); 									// Verifica si el jugador está en zona de muerte (agua)
uint8_t EstaSobrePlataforma(Jugador *j); 								// Verifica si el jugador está sobre una plataforma
uint8_t GolpeaPlataformaDesdeAbajo(Jugador *j); 						// Detecta colisión del jugador con plataformas desde abajo
uint8_t ColisionJugadorVsVida(Jugador *atacante, Jugador *objetivo); 	// Detecta si un jugador golpea el globo del otro
void RespawnJugador(Jugador *j, uint8_t numJugador); 					// Reubica al jugador en su posición inicial tras morir
void RestaurarZonaJugador(Jugador *j); 									// Borra el área anterior del jugador en pantalla
void ObtenerSizeJugador(Jugador *j, int *w, int *h); 					// Obtiene dimensiones del sprite actual del jugador
void EnviarSonido(uint8_t sonido); 										// Envía señal de sonido al otro núcleo
void ActualizarSonido(void); 											// Controla la duración y apagado del sonido
void ActualizarInvencibilidad(Jugador *j); 								// Maneja el tiempo de invencibilidad del jugador
uint8_t ColisionEntreJugadores(Jugador *a, Jugador *b); 				// Detecta colisión entre dos jugadores
void ResolverColisionEntreJugadores(Jugador *a, Jugador *b); 			// Resuelve la colisión entre jugadores (evita solapamiento)
uint8_t GloboGolpeaPlataforma(Jugador *j); 								// Detecta colisión del globo con plataformas por arriba
uint8_t DebeRedibujarJugador(Jugador *j); 								// Determina si el jugador necesita ser redibujado
uint8_t GolpeaPlataformaPorIzquierda(Jugador *j); 						// Detecta colisión lateral izquierda con plataformas
uint8_t GolpeaPlataformaPorDerecha(Jugador *j); 						// Detecta colisión lateral derecha con plataformas
uint8_t GloboGolpeaPlataformaPorIzquierda(Jugador *j); 					// Detecta colisión lateral izquierda del globo
uint8_t GloboGolpeaPlataformaPorDerecha(Jugador *j); 					// Detecta colisión lateral derecha del globo
int ObtenerYPlataformaDebajo(Jugador *j); 								// Obtiene la coordenada Y de la plataforma debajo del jugador
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
    LCD_Init(); // Inicializa la pantalla LCD (configuración de pines y controlador)
    LCD_Clear(0x0000); // Limpia la pantalla con color negro

    fres = f_mount(&fs, "", 1); // Monta el sistema de archivos de la memoria SD

    if (fres != FR_OK) { // Verifica si hubo error al montar la SD
        LCD_Print("SD FAIL", 10, 10, 1, 0xFFFF, 0x0000); // Muestra mensaje de error en pantalla
        Error_Handler(); // Detiene la ejecución en caso de fallo
    }

    estadoActual = ESTADO_MENU; // Inicializa el juego en el estado de menú
    mapa_actual = 1; 			// Define el primer mapa como inicio
    fondoDibujado = false; 		// Indica que el fondo aún no ha sido dibujado
    InicializarJugadores(); 	// Inicializa las variables y posiciones de los jugadores
   /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		ActualizarSonido(); // Actualizamos los sonidos para el otro nucleo

		  // Recibir de los Mandos
		if (HAL_UART_Receive(&huart4, &mando1_byte, 1, 0) != HAL_OK) {
		    // no llegó nada, conserva último valor o pon 0 según tu protocolo
		}

		if (HAL_UART_Receive(&huart5, &mando2_byte, 1, 0) != HAL_OK) {
		    // no llegó nada,conserva último valor o pon 0 según tu protocolo
		}

		// DETECCIÓN DE FLANCO (aleteo)
		if ((mando1_byte & BTN_X) && !(mando1_anterior & BTN_X)) {
		    EnviarSonido(2);
		}
		if ((mando2_byte & BTN_X) && !(mando2_anterior & BTN_X)) {
		    EnviarSonido(2);
		}

		// Lógica de estados
		switch (estadoActual) {
			case ESTADO_MENU:
				EnviarSonido(1); // Activa sonido del menú principal
				// Dibujar el fondo del menú solo una vez
				if (!fondoDibujado) {
					LCD_DrawFromSD("BFPP.bin"); // Carga y muestra la imagen del menú desde la SD
					fondoDibujado = true; // Evita redibujar el fondo en cada ciclo
				}
				// Animación de la flecha (indicador de selección)
				if (HAL_GetTick() - ultimoTickAnim > 150) { // Control de tiempo para animación
					ultimoTickAnim = HAL_GetTick();
					LCD_Sprite(95, 157, 12, 18, flecha, 6, frameAnim, 0, 0); // Dibuja el frame actual de la flecha
					frameAnim = (frameAnim + 1) % 6; // Avanza al siguiente frame (ciclo de 0 a 5)
				}
				// Condición para iniciar el juego (presionar botón X en cualquiera de los mandos)
				if ((mando1_byte & BTN_X) || (mando2_byte & BTN_X)) {
					EnviarSonido(7); // Reproduce sonido de confirmación
					fondoDibujado = false; // Forzar redibujo del siguiente estado
					tiempoInicioMenu = HAL_GetTick(); // Guarda el tiempo de transición
					frameAnim = 0; // Reinicia animación
					mapa_actual = 1; // Inicia desde el mapa 1
					rondasJ1 = 0; // Reinicia rondas del jugador 1
					rondasJ2 = 0; // Reinicia rondas del jugador 2
					InicializarJugadores(); // Reinicia estado de los jugadores
					estadoActual = ESTADO_TRANSICION; // Cambia al estado de transición
				}
				break;

			case ESTADO_TRANSICION:
				if (mapa_actual==1){// Transicion para mostrar ROUND1 en 2s
					if (!fondoDibujado) {
						LCD_Clear(0x0000); // Limpia la pantalla
						LCD_Bitmap(106, 111, 108, 18, R1); // Muestra imagen de ROUND 1
						tiempoInicioMenu = HAL_GetTick(); // Guarda el tiempo de inicio de la transición
						fondoDibujado = true; // Evita redibujar el fondo
					}
					// Espera 2 segundos antes de pasar al siguiente estado
					if (HAL_GetTick() - tiempoInicioMenu > 2000) {
						fondoDibujado = false; // Reinicia bandera para siguiente estado
						estadoActual = ESTADO_DIBUJO; // Pasa al estado de dibujo del mapa
					}
				} else if (mapa_actual==2){// Transicion para mostrar ROUND2 en 2s
					if (!fondoDibujado) {
						LCD_Clear(0x0000); // Limpia la pantalla
						LCD_Bitmap(106, 111, 108, 18, R2); // Muestra imagen de ROUND 2
						tiempoInicioMenu = HAL_GetTick(); // Guarda el tiempo de inicio
						fondoDibujado = true;
					}
					// Espera 2 segundos antes de continuar
					if (HAL_GetTick() - tiempoInicioMenu > 2000) {
						fondoDibujado = false;
						estadoActual = ESTADO_DIBUJO;
					}
				} else if (mapa_actual==3){// Transicion para mostrar ROUND3 en 2s
					if (!fondoDibujado) {
						LCD_Clear(0x0000); // Limpia la pantalla
						LCD_Bitmap(106, 111, 108, 18, R3); // Muestra imagen de ROUND 3
						tiempoInicioMenu = HAL_GetTick(); // Guarda el tiempo de inicio
						fondoDibujado = true;
					}
					// Espera 2 segundos antes de continuar
					if (HAL_GetTick() - tiempoInicioMenu > 2000) {
						fondoDibujado = false;
						estadoActual = ESTADO_DIBUJO;
					}
				}
				break;

			case ESTADO_DIBUJO:
			    if (mapa_actual == 1) {
			        if (!fondoDibujado) {
			            LCD_Clear(0x0000); // Limpia la pantalla
			            LCD_DrawFromSD("Mapa1.bin"); // Carga y dibuja el mapa 1 desde la SD

			            InicializarJugadores(); // Reinicia posiciones y variables de los jugadores
			            // FORZAR PRIMER DIBUJO
			            DibujarJugador(&jugador1, 1); // Dibuja al jugador 1 al iniciar el mapa
			            DibujarJugador(&jugador2, 2); // Dibuja al jugador 2 al iniciar el mapa
			            fondoDibujado = true; // Indica que el mapa ya fue dibujado
			            estadoActual = ESTADO_JUEGO; // Cambia al estado principal del juego
			            EnviarSonido(1); // Activa sonido de inicio de ronda
			        }
			    } else if (mapa_actual == 2) {
			        if (!fondoDibujado) {
			            LCD_Clear(0x0000); // Limpia la pantalla
			            LCD_DrawFromSD("Mapa2.bin"); // Carga y dibuja el mapa 2 desde la SD

			            // Marcador según ganador de mapa 1
			            if (rondasJ1 == 1 && rondasJ2 == 0) {
			                LCD_Bitmap(28, 6, 10, 10, ONE); // Marca punto para jugador 1
			                LCD_Bitmap(243, 6, 10, 10, ZERO); // Marca cero para jugador 2
			            }
			            else if (rondasJ1 == 0 && rondasJ2 == 1) {
			                LCD_Bitmap(28, 6, 10, 10, ZERO); // Marca cero para jugador 1
			                LCD_Bitmap(243, 6, 10, 10, ONE); // Marca punto para jugador 2
			            }

			            InicializarJugadores(); // Reinicia jugadores para la nueva ronda
			            fondoDibujado = true; // Indica que el mapa ya fue dibujado
			            estadoActual = ESTADO_JUEGO; // Cambia al estado principal del juego
			            EnviarSonido(1); // Activa sonido de inicio de ronda
			        }
			    } else if (mapa_actual == 3) {
			        if (!fondoDibujado) {
			            LCD_Clear(0x0000); // Limpia la pantalla
			            LCD_DrawFromSD("Mapa3.bin"); // Carga y dibuja el mapa 3 desde la SD

			            // Marcador 1 a 1
			            LCD_Bitmap(28, 6, 10, 10, ONE); // Marca punto para jugador 1
			            LCD_Bitmap(243, 6, 10, 10, ONE); // Marca punto para jugador 2

			            InicializarJugadores(); // Reinicia jugadores para la nueva ronda
			            fondoDibujado = true; // Indica que el mapa ya fue dibujado
			            estadoActual = ESTADO_JUEGO; // Cambia al estado principal del juego
			            EnviarSonido(1); // Activa sonido de inicio de ronda
			        }
			    }
			    break;

			case ESTADO_JUEGO:
			    // Guardar y actualizar
			    ActualizarJugador(&jugador1, mando1_byte, 1); // Actualiza movimiento, física y estado del jugador 1
			    ActualizarJugador(&jugador2, mando2_byte, 2); // Actualiza movimiento, física y estado del jugador 2

			    ActualizarInvencibilidad(&jugador1); // Actualiza el tiempo de invencibilidad del jugador 1
			    ActualizarInvencibilidad(&jugador2); // Actualiza el tiempo de invencibilidad del jugador 2

			    // Resolver colisión entre jugadores
			    ResolverColisionEntreJugadores(&jugador1, &jugador2); // Evita que los jugadores se sobrepongan

			    uint8_t redibujarJ1 = DebeRedibujarJugador(&jugador1); // Verifica si el jugador 1 debe redibujarse
			    uint8_t redibujarJ2 = DebeRedibujarJugador(&jugador2); // Verifica si el jugador 2 debe redibujarse

			    // Borrar solo si hace falta redibujar
			    if (redibujarJ1) {
			        RestaurarZonaJugador(&jugador1); // Borra la posición anterior del jugador 1
			    }

			    if (redibujarJ2) {
			        RestaurarZonaJugador(&jugador2); // Borra la posición anterior del jugador 2
			    }

			    // Dibujar solo si hace falta
			    if (redibujarJ1) {
			        DibujarJugador(&jugador1, 1); // Dibuja al jugador 1 en su nueva posición/estado
			    }

			    if (redibujarJ2) {
			        DibujarJugador(&jugador2, 2); // Dibuja al jugador 2 en su nueva posición/estado
			    }

			    // --- COLISION ENTRE JUGADORES (ATAQUE POR VIDA) ---
			    // jugador1 golpea jugador2
			    if (jugador2.vivo && jugador1.vivo &&
			        !jugador2.invencible &&
			        ColisionJugadorVsVida(&jugador1, &jugador2)) { // Verifica si jugador 1 golpea el globo de jugador 2

			        if (jugador2.vidas > 0) {
			            jugador2.vidas--; // Resta una vida al jugador 2
			            EnviarSonido(3); // Reproduce sonido de golpe
			        }

			        if (jugador2.vidas == 0) {
			            jugador2.vivo = 0; // Marca al jugador 2 como muerto
			            EnviarSonido(4); // Reproduce sonido de derrota
			        } else {
			            RespawnJugador(&jugador2, 2); // Reubica al jugador 2 si aún tiene vidas
			        }
			    }

			    // jugador2 golpea jugador1
			    if (jugador2.vivo && jugador1.vivo &&
			        !jugador1.invencible &&
			        ColisionJugadorVsVida(&jugador2, &jugador1)) { // Verifica si jugador 2 golpea el globo de jugador 1

			        if (jugador1.vidas > 0) {
			            jugador1.vidas--; // Resta una vida al jugador 1
			            EnviarSonido(3); // Reproduce sonido de golpe
			        }

			        if (jugador1.vidas == 0) {
			            jugador1.vivo = 0; // Marca al jugador 1 como muerto
			            EnviarSonido(4); // Reproduce sonido de derrota
			        } else {
			            RespawnJugador(&jugador1, 1); // Reubica al jugador 1 si aún tiene vidas
			        }
			    }

			    // --- RESULTADO DE LA RONDA ---
			    if (!jugador1.vivo && jugador2.vivo) { // Si jugador 1 pierde y jugador 2 sigue vivo
			        rondasJ2++; // Suma una ronda ganada al jugador 2

			        if (mapa_actual == 1) {
			            mapa_actual = 2; // Avanza al mapa 2
			            fondoDibujado = false;
			            frameAnim = 0;
			            estadoActual = ESTADO_TRANSICION; // Muestra transición de ronda
			        }
			        else if (mapa_actual == 2) {
			            if (rondasJ2 == 2) {
			                Ganador = 2; // Declara ganador al jugador 2
			                fondoDibujado = false;
			                Posicion = 1;
			                frameAnim = 0;
			                estadoActual = ESTADO_GANADOR; // Cambia a pantalla de ganador

			            } else {
			                mapa_actual = 3; // Avanza al mapa 3 si van empatados
			                fondoDibujado = false;
			                frameAnim = 0;
			                estadoActual = ESTADO_TRANSICION;
			            }
			        }
			        else if (mapa_actual == 3) {
			            Ganador = 2; // Declara ganador al jugador 2
			            fondoDibujado = false;
			            Posicion = 1;
			            frameAnim = 0;
			            estadoActual = ESTADO_GANADOR;
			        }

			        InicializarJugadores(); // Reinicia jugadores para la siguiente ronda o pantalla
			    }
			    else if (!jugador2.vivo && jugador1.vivo) { // Si jugador 2 pierde y jugador 1 sigue vivo
			        rondasJ1++; // Suma una ronda ganada al jugador 1

			        if (mapa_actual == 1) {
			            mapa_actual = 2; // Avanza al mapa 2
			            fondoDibujado = false;
			            frameAnim = 0;
			            estadoActual = ESTADO_TRANSICION;
			        }
			        else if (mapa_actual == 2) {
			            if (rondasJ1 == 2) {
			                Ganador = 1; // Declara ganador al jugador 1
			                fondoDibujado = false;
			                Posicion = 1;
			                frameAnim = 0;
			                estadoActual = ESTADO_GANADOR;

			            } else {
			                mapa_actual = 3; // Avanza al mapa 3 si van empatados
			                fondoDibujado = false;
			                frameAnim = 0;
			                estadoActual = ESTADO_TRANSICION;
			            }
			        }
			        else if (mapa_actual == 3) {
			            Ganador = 1; // Declara ganador al jugador 1
			            fondoDibujado = false;
			            Posicion = 1;
			            frameAnim = 0;
			            estadoActual = ESTADO_GANADOR;
			        }

			        InicializarJugadores(); // Reinicia jugadores para la siguiente ronda o pantalla
			    }

			    mando1_anterior = mando1_byte; // Guarda estado anterior del mando 1
			    mando2_anterior = mando2_byte; // Guarda estado anterior del mando 2
			    break;

			case ESTADO_GANADOR:
				EnviarSonido(6); // Activa sonido de pantalla final (ganador)

				// Dibujar fondo final solo una vez
				if (!fondoDibujado) {
					LCD_Clear(0x0000); // Limpia la pantalla
					LCD_DrawFromSD("Fondo_final.bin"); // Muestra fondo final desde la SD
					fondoDibujado = true;
				}

				// Mostrar número del jugador ganador
				if (Ganador == 1) {
					LCD_Bitmap(220, 26, 40, 40, UNO); // Muestra "1" como ganador
				}
				if (Ganador == 2) {
					LCD_Bitmap(220, 26, 40, 40, DOS); // Muestra "2" como ganador
				}

				// --- OPCIÓN 1 (REINICIAR PARTIDA) ---
				if (Posicion == 1){
					// Animación de la flecha en la opción 1
				    if (HAL_GetTick() - ultimoTickAnim > 150) {
				        ultimoTickAnim = HAL_GetTick();
				        LCD_Sprite(83, 136, 12, 18, flecha, 6, frameAnim, 0, 0); // Dibuja flecha animada
				        frameAnim = (frameAnim + 1) % 6; // Cambia de frame
				    }

				    // Cambiar a opción 2 (bajar)
				    if ((mando1_byte & BTN_DOWN) || (mando2_byte & BTN_DOWN)) {
				    	FillRect(83, 136, 12, 18, 0x0000); // Borra flecha actual
				    	Posicion = 2; // Cambia selección a opción 2
				    }

				    // Confirmar opción 1 (reiniciar juego)
				    if ( ((mando1_byte & BTN_X) && !(mando1_anterior & BTN_X)) ||
				         ((mando2_byte & BTN_X) && !(mando2_anterior & BTN_X)) ) {

				    	fondoDibujado = false; // Forzar redibujo del siguiente estado
				    	tiempoInicioMenu = HAL_GetTick(); // Reinicia tiempo
				    	frameAnim = 0; // Reinicia animación
				    	mapa_actual = 1; // Reinicia desde mapa 1
				    	estadoActual = ESTADO_TRANSICION; // Vuelve al inicio del juego
				    }
				}

				// --- OPCIÓN 2 (CRÉDITOS) ---
				if (Posicion == 2){
					// Animación de la flecha en la opción 2
				    if (HAL_GetTick() - ultimoTickAnim > 150) {
				    	ultimoTickAnim = HAL_GetTick();
				    	LCD_Sprite(83, 180, 12, 18, flecha, 6, frameAnim, 0, 0); // Dibuja flecha animada
				    	frameAnim = (frameAnim + 1) % 6;
				    }

				    // Cambiar a opción 1 (subir)
				    if ((mando1_byte & BTN_UP) || (mando2_byte & BTN_UP)) {
				    	FillRect(83, 180, 12, 18, 0x0000); // Borra flecha actual
				    	Posicion = 1; // Cambia selección a opción 1
				    }

				    // Confirmar opción 2 (ir a créditos)
				    if ((mando1_byte & BTN_X) || (mando2_byte & BTN_X)) {
				        fondoDibujado = false; // Forzar redibujo del siguiente estado
				      	tiempoInicioMenu = HAL_GetTick(); // Reinicia tiempo
				       	frameAnim = 0; // Reinicia animación
				       	estadoActual = ESTADO_CREDITOS; // Cambia a pantalla de créditos
				    }
				}
			    break;

			case ESTADO_CREDITOS:
				if (credito_actual==1){
					if (!fondoDibujado) {
						LCD_Clear(0x0000); // Limpia la pantalla antes de mostrar créditos
			  			LCD_DrawFromSD("C1.bin"); // Muestra la primera pantalla de créditos
			  			tiempoInicioMenu = HAL_GetTick(); // Empieza a contar los 3 segundos
			   			fondoDibujado = true; // Evita redibujar la imagen constantemente
					}
					if (HAL_GetTick() - tiempoInicioMenu > 3000) {
						fondoDibujado = false; // Prepara el dibujo de la siguiente pantalla
						credito_actual = 2; // Cambia a la segunda pantalla de créditos
					}
				}

				if (credito_actual==2){
					if (!fondoDibujado) {
			   			LCD_DrawFromSD("C2.bin"); // Muestra la segunda pantalla de créditos
			   			tiempoInicioMenu = HAL_GetTick(); // Empieza a contar los 3 segundos
			   			fondoDibujado = true; // Evita redibujar la imagen constantemente
					}
					if (HAL_GetTick() - tiempoInicioMenu > 3000) {
						fondoDibujado = false; // Prepara el dibujo de la siguiente pantalla
						credito_actual = 3; // Cambia a la tercera pantalla de créditos
					}
				}

				if (credito_actual==3){
					if (!fondoDibujado) {
			  			LCD_DrawFromSD("C3.bin"); // Muestra la tercera pantalla de créditos
			  			tiempoInicioMenu = HAL_GetTick(); // Empieza a contar los 3 segundos
			   			fondoDibujado = true; // Evita redibujar la imagen constantemente
					}
					if (HAL_GetTick() - tiempoInicioMenu > 3000) {
						fondoDibujado = false; // Reinicia bandera para volver al menú
						mapa_actual = 1; // Reinicia el mapa actual al mapa 1
						credito_actual = 1; // Reinicia la secuencia de créditos
						estadoActual = ESTADO_MENU; // Regresa al menú principal
					}
				}
			    break;
		    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|Audio0_Pin|Audio1_Pin|LCD_D1_Pin
                          |Audio2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_SS_GPIO_Port, SD_SS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LCD_D7_Pin
                           LCD_D0_Pin LCD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Audio0_Pin Audio1_Pin Audio2_Pin */
  GPIO_InitStruct.Pin = Audio0_Pin|Audio1_Pin|Audio2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_D6_Pin LCD_D3_Pin LCD_D5_Pin
                           LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SD_SS_Pin */
  GPIO_InitStruct.Pin = SD_SS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SD_SS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void ActualizarSonido(void)
{
    if (sonidoActivo)
    {
        if (HAL_GetTick() - tiempoSonido > 50) // 50 ms (ajustable)
        {
            HAL_GPIO_WritePin(Audio0_GPIO_Port, Audio0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Audio1_GPIO_Port, Audio1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Audio2_GPIO_Port, Audio2_Pin, GPIO_PIN_RESET);

            sonidoActivo = 0;
        }
    }
}

void EnviarSonido(uint8_t sonido)
{
	// Si pasó el filtro, enviamos los bits a los pines
	sonido &= 0x07;

	HAL_GPIO_WritePin(Audio0_GPIO_Port, Audio0_Pin,
					  (sonido & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Audio1_GPIO_Port, Audio1_Pin,
					  (sonido & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Audio2_GPIO_Port, Audio2_Pin,
					  (sonido & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

	tiempoSonido = HAL_GetTick();
	sonidoActivo = 1;
}

void LCD_DrawFromSD(char* filename)
{
    FIL fil;
    UINT br;
    FRESULT res;
    uint8_t buffer[512];

    res = f_open(&fil, filename, FA_READ);
    if (res != FR_OK) {
        LCD_Print("OPEN FAIL", 10, 50, 1, 0xFFFF, 0x0000);
        return;
    }

    // Opcional: validar tamaño esperado para 320x240 RGB565
    if (f_size(&fil) != 320UL * 240UL * 2UL) {
        LCD_Print("SIZE FAIL", 10, 70, 1, 0xFFFF, 0x0000);
        f_close(&fil);
        return;
    }

    // Activar LCD
    HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_RESET);

    // Definir ventana completa
    SetWindows(0, 0, 319, 239);

    // Modo datos
    HAL_GPIO_WritePin(GPIOA, LCD_RS_Pin, GPIO_PIN_SET);

    while (1)
    {
        res = f_read(&fil, buffer, sizeof(buffer), &br);
        if (res != FR_OK) {
            LCD_Print("READ FAIL", 10, 90, 1, 0xFFFF, 0x0000);
            break;
        }

        if (br == 0) break;

        for (UINT i = 0; i + 1 < br; i += 2)
        {
            LCD_DATA(buffer[i]);
            LCD_DATA(buffer[i + 1]);
        }
    }

    HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_SET);
    f_close(&fil);
}
void InicializarJugadores(void)
{
    // Jugador 1
    jugador1.x = 40;
    jugador1.y = 173;
    jugador1.x_anterior = jugador1.x;
    jugador1.y_anterior = jugador1.y;
    jugador1.w_anterior = 18;
    jugador1.h_anterior = 26;
    jugador1.vx = 0;
    jugador1.vy = 0;
    jugador1.vidas = 2;
    jugador1.vivo = 1;
    jugador1.enSuelo = 1;
    jugador1.estado = JUGADOR_IDLE;
    jugador1.direccion = DIR_DERECHA;
    jugador1.frameAnim = 0;
    jugador1.frameCaida = 0;
    jugador1.ultimoTickAnim = 0;
    jugador1.ultimoTickCaida = 0;
    jugador1.invencible = 0;
    jugador1.tiempoInvencible = 0;
    jugador1.estado_anterior = JUGADOR_IDLE;
    jugador1.direccion_anterior = DIR_DERECHA;

    // Jugador 2
    jugador2.x = 280;
    jugador2.y = 173;
    jugador2.x_anterior = jugador2.x;
    jugador2.y_anterior = jugador2.y;
    jugador2.w_anterior = 18;
    jugador2.h_anterior = 26;
    jugador2.vx = 0;
    jugador2.vy = 0;
    jugador2.vidas = 2;
    jugador2.vivo = 1;
    jugador2.enSuelo = 1;
    jugador2.estado = JUGADOR_IDLE;
    jugador2.direccion = DIR_IZQUIERDA;
    jugador2.frameAnim = 0;
    jugador2.frameCaida = 0;
    jugador2.ultimoTickAnim = 0;
    jugador2.ultimoTickCaida = 0;
    jugador2.invencible = 0;
    jugador2.tiempoInvencible = 0;
    jugador2.estado_anterior = JUGADOR_IDLE;
    jugador2.direccion_anterior = DIR_IZQUIERDA;

    mando1_anterior = 0;
    mando2_anterior = 0;
}
void MorirJugador(Jugador *j)
{
    if (j->vidas > 0) {
        j->vidas--;
    }

    if (j->vidas == 0) {
        j->vivo = 0;
        j->estado = JUGADOR_CAIDA;
        j->frameCaida = 0;
        j->ultimoTickCaida = HAL_GetTick();
        j->ultimoTickAnim = HAL_GetTick();
    } else {
        // Respawn simple
        j->vx = 0;
        j->vy = 0;
        j->enSuelo = 1;
        j->estado = JUGADOR_IDLE;
        j->frameAnim = 0;
        j->frameCaida = 0;
    }
}
void ActualizarJugador(Jugador *j, uint8_t mando, uint8_t numJugador)
{
    uint32_t ahora = HAL_GetTick();

    if (!j->vivo) {
        j->estado = JUGADOR_CAIDA;
        return;
    }

    j->x_anterior = j->x;
    j->y_anterior = j->y;
    j->estado_anterior = j->estado;
    j->direccion_anterior = j->direccion;
    ObtenerSizeJugador(j, &j->w_anterior, &j->h_anterior);
    j->vx = 0;

    // Movimiento horizontal
    if (mando & BTN_RIGHT) {
        j->vx = 4;
        j->direccion = DIR_DERECHA;
    }
    else if (mando & BTN_LEFT) {
        j->vx = -4;
        j->direccion = DIR_IZQUIERDA;
    }

    // Vuelo
    if (mando & BTN_X) {
        j->vy = -4;
        j->enSuelo = 0;
    } else {
        if (!j->enSuelo) {
            j->vy += 1;
            if (j->vy > 5) j->vy = 5;
        } else {
            j->vy = 0;
        }
    }

    // Aplicar movimiento horizontal
    j->x += j->vx;
    // colisión lateral con cuerpo y globo
    if (j->vx > 0 && (GolpeaPlataformaPorDerecha(j) || GloboGolpeaPlataformaPorDerecha(j))) {
        j->x = j->x_anterior;
    }
    else if (j->vx < 0 && (GolpeaPlataformaPorIzquierda(j) || GloboGolpeaPlataformaPorIzquierda(j))) {
        j->x = j->x_anterior;
    }

    // Aplicar movimiento vertical
    j->y += j->vy;

    if (j->y < 0) {
        j->y = 0;
        if (j->vy < 0) j->vy = 0;
    }
    // tope superior considerando también el globo
    if (j->y < 10) {
        j->y = 10;
        if (j->vy < 0) j->vy = 0;
    }
    if (j->x < 0) j->x = 0;
    if (j->x > 296) j->x = 296;

    // choque por debajo (cabeza o globo)
    if (j->vy < 0 && (GolpeaPlataformaDesdeAbajo(j) || GloboGolpeaPlataforma(j))) {
        j->y = j->y_anterior;
        j->vy = 0;
    }

    // Revisar si cayó en zona de muerte
    if (EstaEnZonaMuerte(j)) {
    	EnviarSonido(5);
        // MUERTE INSTANTÁNEA
        j->vidas = 0;
        j->vivo = 0;

        j->estado = JUGADOR_CAIDA;
        j->frameCaida = 0;
        j->ultimoTickCaida = ahora;
        j->ultimoTickAnim = ahora;
        return;
    }

    // Revisar si está sobre plataforma
    int yPlat = ObtenerYPlataformaDebajo(j);
    if (yPlat >= 0) {
        j->enSuelo = 1;
        j->vy = 0;
        j->y = yPlat - 29;   // súbelo 2 píxeles aprox
    } else {
        j->enSuelo = 0;
    }

    // Selección de estado
    if (!j->vivo) {
        j->estado = JUGADOR_CAIDA;
    }
    else if ((mando & BTN_X) && !j->enSuelo) {
        j->estado = JUGADOR_VUELO;
    }
    else if (!j->enSuelo) {
        j->estado = JUGADOR_AIRE;
    }
    else if (j->vx != 0) {
        j->estado = JUGADOR_CAMINANDO;
    }
    else {
        j->estado = JUGADOR_IDLE;
    }

    // Reset de frames simples
    if (j->estado == JUGADOR_IDLE || j->estado == JUGADOR_AIRE) {
        j->frameAnim = 0;
    }
}
void DibujarVidasJugador(Jugador *j, uint8_t numJugador)
{
    int vidaX = j->x;
    int vidaY = j->y - 18;

    if (numJugador == 1) {
        if (j->vidas >= 2) {
            LCD_BitmapTransparent(vidaX, vidaY, 26, 19, VidasJ1, COLOR_TRANSPARENTE);
        } else if (j->vidas == 1) {
            LCD_BitmapTransparent(vidaX, vidaY, 17, 20, VidaJ1,COLOR_TRANSPARENTE);
        }
    } else {
        if (j->vidas >= 2) {
            LCD_BitmapTransparent(vidaX, vidaY, 26, 19, VidasJ2, COLOR_TRANSPARENTE);
        } else if (j->vidas == 1) {
            LCD_BitmapTransparent(vidaX, vidaY, 17, 20, VidaJ2, COLOR_TRANSPARENTE);
        }
    }
}
void DibujarJugador(Jugador *j, uint8_t numJugador)
{
    uint32_t ahora = HAL_GetTick();

    if (numJugador == 1) {

        if (j->estado == JUGADOR_IDLE) {
            if (j->direccion == DIR_DERECHA) {
            	LCD_BitmapTransparent(j->x, j->y, 18, 26, StaticJ1D, COLOR_TRANSPARENTE);
            } else {
            	LCD_BitmapTransparent(j->x, j->y, 18, 26, StaticJ1I, COLOR_TRANSPARENTE);
            }
        }
        else if (j->estado == JUGADOR_AIRE) {
            if (j->direccion == DIR_DERECHA) {
                LCD_BitmapTransparent(j->x, j->y, 24, 26, AireJ1D, COLOR_TRANSPARENTE);
            } else {
            	LCD_BitmapTransparent(j->x, j->y, 24, 26, AireJ1I, COLOR_TRANSPARENTE);
            }
        }
        else if (j->estado == JUGADOR_VUELO) {
            if (ahora - j->ultimoTickAnim > 120) {
                j->ultimoTickAnim = ahora;
                j->frameAnim = (j->frameAnim + 1) % 2;
            }

            if (j->direccion == DIR_DERECHA) {
            	LCD_SpriteTransparent(j->x, j->y, 24, 26, VueloJ1D, 2, j->frameAnim, COLOR_TRANSPARENTE);
            } else {
            	LCD_SpriteTransparent(j->x, j->y, 24, 26, VueloJ1I, 2, j->frameAnim, COLOR_TRANSPARENTE);
            }
        }
        else if (j->estado == JUGADOR_CAMINANDO) {
            if (ahora - j->ultimoTickAnim > 70) {
                j->ultimoTickAnim = ahora;
                j->frameAnim = (j->frameAnim + 1) % 6;
            }

            if (j->direccion == DIR_DERECHA) {
            	LCD_SpriteTransparent(j->x, j->y, 22, 28, CaminarJ1D, 6, j->frameAnim, COLOR_TRANSPARENTE);
            } else {
            	LCD_SpriteTransparent(j->x, j->y, 22, 28, CaminarJ1I, 6, j->frameAnim, COLOR_TRANSPARENTE);
            }
        }
        else if (j->estado == JUGADOR_CAIDA) {
            if (j->frameCaida == 0) {
            	LCD_SpriteTransparent(j->x, j->y, 30, 28, CaidaJ1, 3, j->frameCaida, COLOR_TRANSPARENTE);
                if (ahora - j->ultimoTickCaida > 1200) {
                    j->ultimoTickCaida = ahora;
                    j->frameCaida = 1;
                }
            } else {
                if (ahora - j->ultimoTickAnim > 180) {
                    j->ultimoTickAnim = ahora;
                    if (j->frameCaida == 1) j->frameCaida = 2;
                    else j->frameCaida = 1;
                }
                LCD_SpriteTransparent(j->x, j->y, 30, 28, CaidaJ1, 3, j->frameCaida, COLOR_TRANSPARENTE);
            }
        }
    }
    else {

        if (j->estado == JUGADOR_IDLE) {
            if (j->direccion == DIR_DERECHA) {
            	LCD_BitmapTransparent(j->x, j->y, 18, 26, StaticJ2D, COLOR_TRANSPARENTE);
            } else {
            	LCD_BitmapTransparent(j->x, j->y, 18, 26, StaticJ2I, COLOR_TRANSPARENTE);
            }
        }
        else if (j->estado == JUGADOR_AIRE) {
            if (j->direccion == DIR_DERECHA) {
            	LCD_BitmapTransparent(j->x, j->y, 24, 26, AireJ2D, COLOR_TRANSPARENTE);
            } else {
            	LCD_BitmapTransparent(j->x, j->y, 24, 26, AireJ2I, COLOR_TRANSPARENTE);
            }
        }
        else if (j->estado == JUGADOR_VUELO) {
            if (ahora - j->ultimoTickAnim > 120) {
                j->ultimoTickAnim = ahora;
                j->frameAnim = (j->frameAnim + 1) % 2;
            }

            if (j->direccion == DIR_DERECHA) {
            	LCD_SpriteTransparent(j->x, j->y, 24, 26, VueloJ2D, 2, j->frameAnim, COLOR_TRANSPARENTE);
            } else {
            	LCD_SpriteTransparent(j->x, j->y, 24, 26, VueloJ2I, 2, j->frameAnim, COLOR_TRANSPARENTE);
            }
        }
        else if (j->estado == JUGADOR_CAMINANDO) {
            if (ahora - j->ultimoTickAnim > 90) {
                j->ultimoTickAnim = ahora;
                j->frameAnim = (j->frameAnim + 1) % 6;
            }

            if (j->direccion == DIR_DERECHA) {
            	LCD_SpriteTransparent(j->x, j->y, 22, 28, CaminarJ2D, 6, j->frameAnim, COLOR_TRANSPARENTE);
            } else {
            	LCD_SpriteTransparent(j->x, j->y, 22, 28, CaminarJ2I, 6, j->frameAnim, COLOR_TRANSPARENTE);
            }
        }
        else if (j->estado == JUGADOR_CAIDA) {
            if (j->frameCaida == 0) {
            	LCD_SpriteTransparent(j->x, j->y, 30, 28, CaidaJ2, 3, j->frameCaida, COLOR_TRANSPARENTE);
                if (ahora - j->ultimoTickCaida > 1200) {
                    j->ultimoTickCaida = ahora;
                    j->frameCaida = 1;
                }
            } else {
                if (ahora - j->ultimoTickAnim > 180) {
                    j->ultimoTickAnim = ahora;
                    if (j->frameCaida == 1) j->frameCaida = 2;
                    else j->frameCaida = 1;
                }
                LCD_SpriteTransparent(j->x, j->y, 30, 28, CaidaJ2, 3, j->frameCaida, COLOR_TRANSPARENTE);
            }
        }
    }

    DibujarVidasJugador(j, numJugador);
}
void DibujarHUDVidas(void)
{
    // Por si luego quieres separar HUD fijo de vidas
}
uint8_t ColisionRect(int x, int y, int w, int h, RectColision r)
{
    if (x + w <= r.x) return 0;
    if (x >= r.x + r.w) return 0;
    if (y + h <= r.y) return 0;
    if (y >= r.y + r.h) return 0;
    return 1;
}
uint8_t EstaEnZonaMuerte(Jugador *j)
{
    int x = j->x;
    int y = j->y;
    int w = 22;
    int h = 28;

    if (mapa_actual == 1) {
        for (int i = 0; i < mapa1_num_muerte; i++) {
            if (ColisionRect(x, y, w, h, mapa1_muerte[i])) return 1;
        }
    }
    else if (mapa_actual == 2) {
        for (int i = 0; i < mapa2_num_muerte; i++) {
            if (ColisionRect(x, y, w, h, mapa2_muerte[i])) return 1;
        }
    }
    else if (mapa_actual == 3) {
        for (int i = 0; i < mapa3_num_muerte; i++) {
            if (ColisionRect(x, y, w, h, mapa3_muerte[i])) return 1;
        }
    }

    return 0;
}
uint8_t EstaSobrePlataforma(Jugador *j)
{
    int pieX = j->x + 4;
    int pieY = j->y + 28;
    int pieW = 14;
    int pieH = 2;

    if (mapa_actual == 1) {
        for (int i = 0; i < mapa1_num_plataformas; i++) {
            if (ColisionRect(pieX, pieY, pieW, pieH, mapa1_plataformas[i])) {
                return 1;
            }
        }
    }
    else if (mapa_actual == 2) {
        for (int i = 0; i < mapa2_num_plataformas; i++) {
            if (ColisionRect(pieX, pieY, pieW, pieH, mapa2_plataformas[i])) {
                return 1;
            }
        }
    }
    else if (mapa_actual == 3) {
        for (int i = 0; i < mapa3_num_plataformas; i++) {
            if (ColisionRect(pieX, pieY, pieW, pieH, mapa3_plataformas[i])) {
                return 1;
            }
        }
    }

    return 0;
}
void RespawnJugador(Jugador *j, uint8_t numJugador)
{
    // borrar huella vieja antes de moverlo
    FillRect(j->x, j->y - 18, 30, 48, 0x0000);

    if (numJugador == 1) {
        j->x = 40;
        j->y = 172;
        j->direccion = DIR_DERECHA;
    } else {
        j->x = 280;
        j->y = 172;
        j->direccion = DIR_IZQUIERDA;
    }

    j->w_anterior = 18;
    j->h_anterior = 26;
    j->x_anterior = j->x;
    j->y_anterior = j->y;
    j->vx = 0;
    j->vy = 0;
    j->enSuelo = 0;
    j->invencible = 1;
    j->tiempoInvencible = HAL_GetTick();
    j->estado = JUGADOR_AIRE;
    j->frameAnim = 0;
    j->frameCaida = 0;
    j->ultimoTickAnim = HAL_GetTick();
    j->estado_anterior = JUGADOR_AIRE;
    j->direccion_anterior = j->direccion;
}
void RestaurarZonaJugador(Jugador *j)
{
    int x = j->x_anterior - 2;
    int y = j->y_anterior - 20;
    int w = j->w_anterior + 6;
    int h = j->h_anterior + 22;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > 320) {
        w = 320 - x;
    }
    if (y + h > 240) {
        h = 240 - y;
    }

    if (w > 0 && h > 0) {
        FillRect(x, y, w, h, 0x0000);
    }
}
void ObtenerSizeJugador(Jugador *j, int *w, int *h)
{
    switch (j->estado) {
        case JUGADOR_IDLE:
            *w = 18; *h = 26;
            break;
        case JUGADOR_CAMINANDO:
            *w = 22; *h = 28;
            break;
        case JUGADOR_AIRE:
            *w = 24; *h = 26;
            break;
        case JUGADOR_VUELO:
            *w = 24; *h = 26;
            break;
        case JUGADOR_CAIDA:
            *w = 30; *h = 28;
            break;
        default:
            *w = 18; *h = 26;
            break;
    }
}
uint8_t ColisionJugadorVsVida(Jugador *atacante, Jugador *objetivo)
{
    int vidaW = 17;
    int vidaH = 8;     // solo la parte superior
    int vidaX = objetivo->x;
    int vidaY = objetivo->y - 18;

    int jugW, jugH;
    ObtenerSizeJugador(atacante, &jugW, &jugH);

    return ColisionRect(atacante->x, atacante->y, jugW, jugH,
                        (RectColision){vidaX, vidaY, vidaW, vidaH});
}
uint8_t GolpeaPlataformaDesdeAbajo(Jugador *j)
{
    int cabezaX = j->x + 4;
    int cabezaY = j->y;
    int cabezaW = 14;
    int cabezaH = 2;

    if (mapa_actual == 1) {
        // solo plataforma 1
        if (ColisionRect(cabezaX, cabezaY, cabezaW, cabezaH, mapa1_plataformas[0])) return 1;
    }
    else if (mapa_actual == 2) {
        if (ColisionRect(cabezaX, cabezaY, cabezaW, cabezaH, mapa2_plataformas[0])) return 1;
        if (ColisionRect(cabezaX, cabezaY, cabezaW, cabezaH, mapa2_plataformas[1])) return 1;
    }
    else if (mapa_actual == 3) {
        if (ColisionRect(cabezaX, cabezaY, cabezaW, cabezaH, mapa3_plataformas[0])) return 1;
        if (ColisionRect(cabezaX, cabezaY, cabezaW, cabezaH, mapa3_plataformas[1])) return 1;
        if (ColisionRect(cabezaX, cabezaY, cabezaW, cabezaH, mapa3_plataformas[2])) return 1;
    }

    return 0;
}
void ActualizarInvencibilidad(Jugador *j)
{
    if (j->invencible && (HAL_GetTick() - j->tiempoInvencible >= 1000)) {
        j->invencible = 0;
    }
}
uint8_t ColisionEntreJugadores(Jugador *a, Jugador *b)
{
    int aw, ah, bw, bh;
    ObtenerSizeJugador(a, &aw, &ah);
    ObtenerSizeJugador(b, &bw, &bh);

    return ColisionRect(a->x, a->y, aw, ah,
                        (RectColision){b->x, b->y, bw, bh});
}
void ResolverColisionEntreJugadores(Jugador *a, Jugador *b)
{
    int aw, ah, bw, bh;
    ObtenerSizeJugador(a, &aw, &ah);
    ObtenerSizeJugador(b, &bw, &bh);

    // Si no hay colisión, no hacer nada
    if (!ColisionRect(a->x, a->y, aw, ah, (RectColision){b->x, b->y, bw, bh})) {
        return;
    }

    // Caso 1: A venía cayendo y estaba arriba de B
    if (a->vy > 0 && (a->y_anterior + a->h_anterior) <= b->y_anterior) {
        a->y = b->y - ah;
        a->vy = 0;
        a->enSuelo = 1;
        return;
    }

    // Caso 2: B venía cayendo y estaba arriba de A
    if (b->vy > 0 && (b->y_anterior + b->h_anterior) <= a->y_anterior) {
        b->y = a->y - bh;
        b->vy = 0;
        b->enSuelo = 1;
        return;
    }

    // Caso 3: choque lateral o superposición rara
    // regresar a ambos a su posición X anterior
    a->x = a->x_anterior;
    b->x = b->x_anterior;
}
uint8_t GloboGolpeaPlataforma(Jugador *j)
{
    int globoW = (j->vidas >= 2) ? 26 : 17;

    int testW = globoW / 2;              // solo 50% del ancho
    int testH = 1;                       // parte superior del globo
    int testX = j->x + (globoW - testW) / 2;
    int testY = j->y - 18;

    if (mapa_actual == 1) {
        for (int i = 0; i < mapa1_num_plataformas; i++) {
            if (ColisionRect(testX, testY, testW, testH, mapa1_plataformas[i])) return 1;
        }
    }
    else if (mapa_actual == 2) {
        for (int i = 0; i < mapa2_num_plataformas; i++) {
            if (ColisionRect(testX, testY, testW, testH, mapa2_plataformas[i])) return 1;
        }
    }
    else if (mapa_actual == 3) {
        for (int i = 0; i < mapa3_num_plataformas; i++) {
            if (ColisionRect(testX, testY, testW, testH, mapa3_plataformas[i])) return 1;
        }
    }

    return 0;
}
uint8_t DebeRedibujarJugador(Jugador *j)
{
    uint32_t ahora = HAL_GetTick();

    if (j->x != j->x_anterior ||
        j->y != j->y_anterior ||
        j->estado != j->estado_anterior ||
        j->direccion != j->direccion_anterior) {
        return 1;
    }

    if (j->estado == JUGADOR_CAMINANDO && (ahora - j->ultimoTickAnim > 90)) {
        return 1;
    }

    if (j->estado == JUGADOR_VUELO && (ahora - j->ultimoTickAnim > 120)) {
        return 1;
    }

    if (j->estado == JUGADOR_CAIDA) {
        if (j->frameCaida == 0 && (ahora - j->ultimoTickCaida > 1200)) {
            return 1;
        }
        if (j->frameCaida != 0 && (ahora - j->ultimoTickAnim > 180)) {
            return 1;
        }
    }

    return 0;
}
uint8_t GolpeaPlataformaPorIzquierda(Jugador *j)
{
    int bodyX = j->x;
    int bodyY = j->y + 4;
    int bodyW = 2;
    int bodyH = 20;

    if (mapa_actual == 1) {
        for (int i = 0; i < mapa1_num_plataformas; i++) {
            if (ColisionRect(bodyX, bodyY, bodyW, bodyH, mapa1_plataformas[i])) return 1;
        }
    }
    else if (mapa_actual == 2) {
        for (int i = 0; i < mapa2_num_plataformas; i++) {
            if (ColisionRect(bodyX, bodyY, bodyW, bodyH, mapa2_plataformas[i])) return 1;
        }
    }
    else if (mapa_actual == 3) {
        for (int i = 0; i < mapa3_num_plataformas; i++) {
            if (ColisionRect(bodyX, bodyY, bodyW, bodyH, mapa3_plataformas[i])) return 1;
        }
    }

    return 0;
}

uint8_t GolpeaPlataformaPorDerecha(Jugador *j)
{
    int bodyW, bodyH;
    ObtenerSizeJugador(j, &bodyW, &bodyH);

    int testX = j->x + bodyW - 2;
    int testY = j->y + 4;
    int testW = 2;
    int testH = 20;

    if (mapa_actual == 1) {
        for (int i = 0; i < mapa1_num_plataformas; i++) {
            if (ColisionRect(testX, testY, testW, testH, mapa1_plataformas[i])) return 1;
        }
    }
    else if (mapa_actual == 2) {
        for (int i = 0; i < mapa2_num_plataformas; i++) {
            if (ColisionRect(testX, testY, testW, testH, mapa2_plataformas[i])) return 1;
        }
    }
    else if (mapa_actual == 3) {
        for (int i = 0; i < mapa3_num_plataformas; i++) {
            if (ColisionRect(testX, testY, testW, testH, mapa3_plataformas[i])) return 1;
        }
    }

    return 0;
}
uint8_t GloboGolpeaPlataformaPorIzquierda(Jugador *j)
{
    int vidaX = j->x;
    int vidaY = j->y - 18;
    int testW = 2;
    int testH = 12;   // parte media-alta del globo

    RectColision *plats = 0;
    int n = 0;

    if (mapa_actual == 1) {
        plats = mapa1_plataformas; n = mapa1_num_plataformas;
    } else if (mapa_actual == 2) {
        plats = mapa2_plataformas; n = mapa2_num_plataformas;
    } else {
        plats = mapa3_plataformas; n = mapa3_num_plataformas;
    }

    for (int i = 0; i < n; i++) {
        if (ColisionRect(vidaX, vidaY + 4, testW, testH, plats[i])) return 1;
    }
    return 0;
}

uint8_t GloboGolpeaPlataformaPorDerecha(Jugador *j)
{
    int vidaW = (j->vidas >= 2) ? 26 : 17;
    int vidaX = j->x + vidaW - 2;
    int vidaY = j->y - 18;
    int testW = 2;
    int testH = 12;

    RectColision *plats = 0;
    int n = 0;

    if (mapa_actual == 1) {
        plats = mapa1_plataformas; n = mapa1_num_plataformas;
    } else if (mapa_actual == 2) {
        plats = mapa2_plataformas; n = mapa2_num_plataformas;
    } else {
        plats = mapa3_plataformas; n = mapa3_num_plataformas;
    }

    for (int i = 0; i < n; i++) {
        if (ColisionRect(vidaX, vidaY + 4, testW, testH, plats[i])) return 1;
    }
    return 0;
}
int ObtenerYPlataformaDebajo(Jugador *j)
{
    int pieX = j->x + 4;
    int pieY = j->y + 28;
    int pieW = 14;
    int pieH = 2;

    if (mapa_actual == 1) {
        for (int i = 0; i < mapa1_num_plataformas; i++) {
            if (ColisionRect(pieX, pieY, pieW, pieH, mapa1_plataformas[i])) {
                return mapa1_plataformas[i].y;
            }
        }
    }
    else if (mapa_actual == 2) {
        for (int i = 0; i < mapa2_num_plataformas; i++) {
            if (ColisionRect(pieX, pieY, pieW, pieH, mapa2_plataformas[i])) {
                return mapa2_plataformas[i].y;
            }
        }
    }
    else if (mapa_actual == 3) {
        for (int i = 0; i < mapa3_num_plataformas; i++) {
            if (ColisionRect(pieX, pieY, pieW, pieH, mapa3_plataformas[i])) {
                return mapa3_plataformas[i].y;
            }
        }
    }

    return -1;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
