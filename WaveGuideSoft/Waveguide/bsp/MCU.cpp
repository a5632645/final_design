#include "MCU.hpp"
#include "stm32h7xx_hal.h"
#include "SystemHook.hpp"
#include "bsp/DebugIO.hpp"

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct{};
    RCC_ClkInitTypeDef RCC_ClkInitStruct{};

    /** Supply configuration update enable
     */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
    {
    }
    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    // RCC_OscInitStruct.PLL.PLLM = 5;
    // RCC_OscInitStruct.PLL.PLLN = 160;
    RCC_OscInitStruct.PLL.PLLM = 4; // 32 / 4 = 8
    RCC_OscInitStruct.PLL.PLLN = 120; // 480MHz
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        // Error_Handler();
        bsp::Debug.FlashErrorLightSequence(bsp::LedErrorSequencePreset.CloclError);
    }
    
    /* PLL3 for USB Clock */
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    // PeriphClkInitStruct.PLL3.PLL3M = 25;
    PeriphClkInitStruct.PLL3.PLL3M = 32;
    PeriphClkInitStruct.PLL3.PLL3N = 336;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 7;
    
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
    
    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        bsp::Debug.FlashErrorLightSequence(bsp::LedErrorSequencePreset.PeriphClockError);
        // Error_Handler();
    }

    /*
    Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
            (GPIO, SPI, FMC, QSPI ...)  when  operating at  high frequencies(please refer to product datasheet)
            The I/O Compensation Cell activation  procedure requires :
            - The activation of the CSI clock
            - The activation of the SYSCFG clock
            - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
    */

    /*activate CSI clock mondatory for I/O Compensation Cell*/
    __HAL_RCC_CSI_ENABLE() ;

    /* Enable SYSCFG clock mondatory for I/O Compensation Cell */
    __HAL_RCC_SYSCFG_CLK_ENABLE() ;

    /* Enables the I/O Compensation Cell */
    HAL_EnableCompensationCell();
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(const char* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

namespace bsp {

void CMCU::Init() {
    SCB_EnableDCache();
    SCB_EnableICache();
    HAL_Init();
    SystemClock_Config();
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
}

extern "C" {

    extern uint32_t _specify_sramd1_bss_start;
    extern uint32_t _specify_sramd1_bss_end;
    
    extern uint32_t _specify_sramd2_bss_start;
    extern uint32_t _specify_sramd2_bss_end;
    
    extern uint32_t _specify_sramd3_bss_start;
    extern uint32_t _specify_sramd3_bss_end;
    
    extern uint32_t _itcmram_start;
    extern uint32_t _itcmram_end;
    
}
void CMCU::InitD1Memory() {
    auto* pstart = &_specify_sramd1_bss_start;
    auto* pend = &_specify_sramd1_bss_end;
    for (auto* p = pstart; p < pend; ++p) *p = 0;
}

void CMCU::InitD2Memory() {
    auto* pstart = &_specify_sramd2_bss_start;
    auto* pend = &_specify_sramd2_bss_end;
    for (auto* p = pstart; p < pend; ++p) *p = 0;
}

void CMCU::InitD3Memory() {
    auto* pstart = &_specify_sramd3_bss_start;
    auto* pend = &_specify_sramd3_bss_end;
    for (auto* p = pstart; p < pend; ++p) *p = 0;
}

void CMCU::InitITCRAM() {
    auto* pram_start = &_itcmram_start;
    auto* pram_end = &_itcmram_end;
    for (auto* p = pram_start; p < pram_end; ++p) {
        *p = 0;
    }
}

void CMCU::MpuInitDmaMemory() {
    HAL_MPU_Disable();

    MPU_Region_InitTypeDef dmaRegion;
    dmaRegion.Enable = MPU_REGION_ENABLE;
    dmaRegion.BaseAddress = 0x24000000; // SRAM D1
    dmaRegion.Size = MPU_REGION_SIZE_64KB;
    dmaRegion.AccessPermission = MPU_REGION_FULL_ACCESS;
    dmaRegion.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    dmaRegion.IsCacheable = MPU_ACCESS_CACHEABLE;
    dmaRegion.IsShareable = MPU_ACCESS_SHAREABLE;
    dmaRegion.Number = MPU_REGION_NUMBER0;
    dmaRegion.TypeExtField = MPU_TEX_LEVEL0;
    dmaRegion.SubRegionDisable = 0x00;
    dmaRegion.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&dmaRegion);

    dmaRegion.BaseAddress = 0x38000000; // SRAM D3
    dmaRegion.Number = MPU_REGION_NUMBER1;
    dmaRegion.Size = MPU_REGION_SIZE_4KB;
    HAL_MPU_ConfigRegion(&dmaRegion);
    
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}
    
} // namespace bsp
