/**
 * \file            main.c
 * \brief           Application
 */
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx.h"
#include "stm32l4xx_ll_gpio.h"
#include "main.h"

#include "stddef.h"
#include "stdio.h"
#include "ow/ow.h"
#include "ow/devices/ow_device_ds18x20.h"

/* Function declarations */
static void LL_Init(void);
static void SystemClock_Config(void);
static void USART_Printf_Init(void);

size_t found;

owr_t
search_cb(ow_t* ow, const uint8_t* rom_id, size_t index, void* arg) {
    if (index == 0) {
        printf("Search started!\r\n");
    }
    if (rom_id == NULL) {
        printf("Search finished. Found total %d devices\r\n", (int)index);
    } else {
        printf("New device, total: %2d; ROM_ID: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", (int)index,
            (unsigned)rom_id[0], (unsigned)rom_id[1], (unsigned)rom_id[2], (unsigned)rom_id[3],
            (unsigned)rom_id[4], (unsigned)rom_id[5], (unsigned)rom_id[6], (unsigned)rom_id[7]
        );
    }
    
    return owOK;
}

/**
 * \brief           Application entry point
 */
int
main(void) {
    uint8_t c, i;
    uint8_t id[8][8];
    owr_t res;
    ow_t ow;
    float t, t_min, t_max;
    
    LL_Init();                                  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    SystemClock_Config();                       /* Configure the system clock */

    USART_Printf_Init();                        /* Init debug UART */
    printf("OneWire over UART example!\r\n");
    
    ow_init(&ow, 0);                            /* Init OneWire and pass custom argument */
    
    ow_search_with_callback(&ow, &found, search_cb, NULL);

    /* Infinite loop */
    while (1) {
        
    }
}

/**
 * \brief           Low-Layer initialization
 */
static void
LL_Init(void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_SetPriority(DebugMonitor_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
}

/**
 * \brief           System clock configuration
 */
void
SystemClock_Config(void) {
	/* Configure flash latency */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
    if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4) {
        while (1) {}
    }

    /* Set voltage scaling */
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

    /* Enable MSI */
    LL_RCC_MSI_Enable();
    while (LL_RCC_MSI_IsReady() != 1) {}
    LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_SetRange(LL_RCC_MSIRANGE_6);
    LL_RCC_MSI_SetCalibTrimming(0);

    /* Configure PLL */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_MSI, LL_RCC_PLLM_DIV_1, 40, LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1) {}

    /* Configure system clock to PLL */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {}

    /* Set prescalers */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* SysTick_IRQn interrupt configuration */
    LL_Init1msTick(80000000);
    LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
    LL_SetSystemCoreClock(80000000);
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    LL_SYSTICK_EnableIT();
}

/**
 * \brief           Init USART2 for printf output
 */
static void
USART_Printf_Init(void) {
    LL_USART_InitTypeDef USART_InitStruct;
    LL_GPIO_InitTypeDef GPIO_InitStruct;

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
    
    /*
     * USART2 GPIO Configuration  
     *
     * PA2  ------> USART2_TX
     * PD6  ------> USART2_RX
     */
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
    LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    USART_InitStruct.BaudRate = 921600;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART2, &USART_InitStruct);

    LL_USART_ConfigAsyncMode(USART2);           /* Configure USART in async mode */
    LL_USART_Enable(USART2);                    /* Enable USART */
}

/**
 * \brief           Printf character handler
 * \param[in]       ch: Character to send
 * \param[in]       f: File pointer
 * \return          Written character
 */
#ifdef __GNUC__
int __io_putchar(int ch) {
#else
int fputc(int ch, FILE* fil) {
#endif
    LL_USART_TransmitData8(USART2, (uint8_t)ch);/* Transmit data */
    while (!LL_USART_IsActiveFlag_TXE(USART2)); /* Wait until done */
    return ch;
}
