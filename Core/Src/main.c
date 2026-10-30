#include "main.h"

/* Variables de control de tiempo */
uint32_t tiempoInicio = 0;

/* Secuencias del carro */
typedef enum {
    AVANZAR = 0,
    IZQUIERDA,
    RETROCEDER,
    DERECHA
} Secuencia;

/* Prototipos */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void controlarMotores(uint8_t a0, uint8_t a1, uint8_t a2,
                      uint8_t b0, uint8_t b1, uint8_t b2);
void ejecutarSecuencia(Secuencia secuencia);
uint32_t obtenerTiempo(Secuencia secuencia);

/* Control de los seis pines (3 para la izquierda, 3 para la derecha) */
void controlarMotores(uint8_t a0, uint8_t a1, uint8_t a2,
                      uint8_t b0, uint8_t b1, uint8_t b2)
{
    /* Motor A (izquierdo) -> PA0, PA1, PA2 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, a0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, a1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, a2 ? GPIO_PIN_SET : GPIO_PIN_RESET);

    /* Motor B (derecho) -> PB0, PB1, PB2 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, b0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, b1 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, b2 ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* Movimientos del carro.
 * CORRECCION: el tercer pin de cada motor (A2 y B2) ahora va en 1.
 * Si es el pin de habilitacion (enable), antes los motores nunca se activaban.
 * Si el carro sigue sin moverse, prueba con A2/B2 en 0 y 1 hasta ver cual mueve. */
void ejecutarSecuencia(Secuencia secuencia)
{
    switch(secuencia)
    {
        case AVANZAR:
            controlarMotores(1, 0, 1,  1, 0, 1);
            break;

        case IZQUIERDA:
            controlarMotores(0, 1, 1,  1, 0, 1);
            break;

        case RETROCEDER:
            controlarMotores(0, 1, 1,  0, 1, 1);
            break;

        case DERECHA:
            controlarMotores(1, 0, 1,  0, 1, 1);
            break;

        default:
            controlarMotores(0, 0, 0,  0, 0, 0);
            break;
    }
}

/* ---- Ajustes (en milisegundos) ----
 * T_LADO : cuanto avanza cada lado del cuadrado (bajalo si se sale de la pantalla)
 * T_GIRO : cuanto dura el giro de 90 grados (ajustalo hasta que el giro quede recto) */
#define T_LADO  1500
#define T_GIRO  1000

/* Recorrido: un cuadrado. Al terminar los 4 lados vuelve al punto de inicio
 * y repite, asi el carrito no se aleja de la pantalla. */
static const Secuencia recorrido[] = {
    AVANZAR, DERECHA,
    AVANZAR, DERECHA,
    AVANZAR, DERECHA,
    AVANZAR, DERECHA
};
#define NUM_PASOS (sizeof(recorrido) / sizeof(recorrido[0]))

/* Tiempo de cada movimiento (en milisegundos) */
uint32_t obtenerTiempo(Secuencia secuencia)
{
    switch(secuencia)
    {
        case AVANZAR:    return T_LADO;
        case RETROCEDER: return T_LADO;
        case IZQUIERDA:  return T_GIRO;
        case DERECHA:    return T_GIRO;
        default:         return 1000;
    }
}

/* Programa principal */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    uint32_t paso = 0;
    Secuencia secuenciaActual = recorrido[paso];
    tiempoInicio = HAL_GetTick();
    ejecutarSecuencia(secuenciaActual);

    while(1)
    {
        if((HAL_GetTick() - tiempoInicio) >= obtenerTiempo(secuenciaActual))
        {
            paso++;
            if(paso >= NUM_PASOS)
            {
                paso = 0;
            }

            secuenciaActual = recorrido[paso];
            ejecutarSecuencia(secuenciaActual);
            tiempoInicio = HAL_GetTick();
        }
    }
}

/* Configuración del reloj (HSI 16 MHz) */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

/* Configuración GPIO */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Nivel inicial en bajo para que arranquen apagados */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);

    /* PA0, PA1, PA2 como salidas */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB0, PB1, PB2 como salidas */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* Manejo de errores */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}