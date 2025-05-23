#include "stm32h7xx_it.h"
#include "stm32h7xx_hal.h"
#include "bsp/DebugIO.hpp"
#include "FreeRTOS.h"
#include "task.h"

using namespace bsp;

static void IT_NotHandle() {
    // Debug.FlashErrorLight();
    // Debug.WriteLine("IT_NotHandle");
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.InterruptNotHandle);
}

extern "C"
{
extern void xPortSysTickHandler(void);
/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
void NMI_Handler(void)
{
    IT_NotHandle();
}

void MemManage_Handler(void)
{
    IT_NotHandle();
}

void BusFault_Handler(void)
{
    IT_NotHandle();
}

void UsageFault_Handler(void)
{
    IT_NotHandle();
}

void DebugMon_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
}

void HardFault_Handler(void) {
    Debug.Init();
    Debug.PrintStackAndRegisters();
    Debug.FlashErrorLightSequence(LedErrorSequencePreset.HardFault);
}

/******************************************************************************/
/* STM32H7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h7xx.s).                    */
/******************************************************************************/

__attribute__((weak)) void WWDG_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void PVD_AVD_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TAMP_STAMP_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void RTC_WKUP_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void FLASH_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void RCC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void EXTI0_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void EXTI1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void EXTI2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void EXTI3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void EXTI4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA1_Stream0_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA1_Stream1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA1_Stream2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA1_Stream3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA1_Stream4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA1_Stream5_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA1_Stream6_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void ADC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void FDCAN1_IT0_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void FDCAN2_IT0_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void FDCAN1_IT1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void FDCAN2_IT1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void EXTI9_5_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM1_BRK_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM1_UP_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM1_TRG_COM_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM1_CC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void I2C1_EV_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void I2C1_ER_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void I2C2_EV_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void I2C2_ER_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SPI1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SPI2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void USART1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void USART2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void USART3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void EXTI15_10_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void RTC_Alarm_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM8_BRK_TIM12_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM8_UP_TIM13_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM8_TRG_COM_TIM14_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM8_CC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA1_Stream7_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void FMC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SDMMC1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM5_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SPI3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void UART4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void UART5_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM6_DAC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM7_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2_Stream0_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2_Stream1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2_Stream2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2_Stream3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2_Stream4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void ETH_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void ETH_WKUP_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void FDCAN_CAL_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2_Stream5_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2_Stream6_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2_Stream7_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void USART6_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void I2C3_EV_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void I2C3_ER_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void OTG_HS_EP1_OUT_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void OTG_HS_EP1_IN_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void OTG_HS_WKUP_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void OTG_HS_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DCMI_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void CRYP_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HASH_RNG_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void FPU_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void UART7_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void UART8_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SPI4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SPI5_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SPI6_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SAI1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void LTDC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void LTDC_ER_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMA2D_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SAI2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void QUADSPI_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void LPTIM1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void CEC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void I2C4_EV_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void I2C4_ER_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SPDIF_RX_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void OTG_FS_EP1_OUT_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void OTG_FS_EP1_IN_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void OTG_FS_WKUP_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void OTG_FS_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMAMUX1_OVR_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HRTIM1_Master_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HRTIM1_TIMA_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HRTIM1_TIMB_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HRTIM1_TIMC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HRTIM1_TIMD_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HRTIM1_TIME_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HRTIM1_FLT_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DFSDM1_FLT0_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DFSDM1_FLT1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DFSDM1_FLT2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DFSDM1_FLT3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SAI3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SWPMI1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM15_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM16_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void TIM17_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void MDIOS_WKUP_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void MDIOS_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void JPEG_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void MDMA_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SDMMC2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void HSEM1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void ADC3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void DMAMUX2_OVR_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void BDMA_Channel0_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void BDMA_Channel1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void BDMA_Channel2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void BDMA_Channel3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void BDMA_Channel4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void BDMA_Channel5_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void BDMA_Channel6_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void BDMA_Channel7_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void COMP1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void LPTIM2_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void LPTIM3_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void LPTIM4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void LPTIM5_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void LPUART1_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void CRS_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void ECC_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void SAI4_IRQHandler(void) {
IT_NotHandle();
}

__attribute__((weak)) void WAKEUP_PIN_IRQHandler(void) {
IT_NotHandle();
}


}