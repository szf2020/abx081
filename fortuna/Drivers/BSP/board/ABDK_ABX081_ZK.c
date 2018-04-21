#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "main.h"
#include "tim.h"
#include "ABDK_ABX081_ZK.h"
#define APP_LOG_MODULE_NAME   "[BSP]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


/*获取锁舌传感器状态*/
bsp_status_t bsp_get_lock_status()
{
 return (bsp_status_t)HAL_GPIO_ReadPin(LOCK_STATUS_POS_GPIO_Port,LOCK_STATUS_POS_Pin);
}

/*获取开锁按键状态*/
bsp_status_t bsp_get_lock_sw_status()
{
 return (bsp_status_t)HAL_GPIO_ReadPin(LOCK_SW_STATUS_POS_GPIO_Port,LOCK_SW_STATUS_POS_Pin);
}

/*获取UPS 1状态*/
bsp_status_t bsp_get_ups1_status()
{
 return (bsp_status_t)HAL_GPIO_ReadPin(UPS_1_STATUS_POS_GPIO_Port,UPS_1_STATUS_POS_Pin);
}
/*获取UPS 状态*/
bsp_status_t bsp_get_ups2_status()
{
 return (bsp_status_t)HAL_GPIO_ReadPin(UPS_2_STATUS_POS_GPIO_Port,UPS_2_STATUS_POS_Pin);
}

/*获取门上部传感器状态*/
bsp_status_t bsp_get_door_up_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(DOOR_UP_STATUS_POS_GPIO_Port,DOOR_UP_STATUS_POS_Pin);
}
/*获取门下部传感器状态*/
bsp_status_t bsp_get_door_dwn_status()
{
return (bsp_status_t)HAL_GPIO_ReadPin(DOOR_DWN_STATUS_POS_GPIO_Port,DOOR_DWN_STATUS_POS_Pin);
}

/*获取压缩机电源状态*/
static bsp_status_t bsp_compressor_pwr_status=COMPRESSOR_PWR_STATUS_OFF;
bsp_status_t bsp_get_compressor_pwr_status()
{
 return bsp_compressor_pwr_status;  
}

/*获取玻璃加热电源状态*/
static bsp_status_t bsp_glass_pwr_status=GLASS_PWR_STATUS_OFF;
bsp_status_t bsp_get_glass_pwr_status()
{
 return bsp_glass_pwr_status;
}


/*获取灯带状态--IO直接控制*/
static bsp_status_t bsp_light_status=LIGHT_STATUS_ON;

bsp_status_t bsp_get_light_status()
{
 return bsp_light_status;  
}

/*led灯操作*/
void BSP_LED_TURN_ON_OFF(uint8_t led,bsp_status_t status)
{
 /*系统运行状态LED灯*/
 if(led & SYS_LED)
 {
 HAL_GPIO_WritePin(SYS_LED_CTL_POS_GPIO_Port,SYS_LED_CTL_POS_Pin,(GPIO_PinState)status); 
 }
/*门锁状态LED红灯*/
 if(led & DOOR_RED_LED)
 {
 HAL_GPIO_WritePin(DOOR_RED_LED_CTL_POS_GPIO_Port,DOOR_RED_LED_CTL_POS_Pin,(GPIO_PinState)status); 
 }
/*门锁状态LED绿灯*/
 if(led & DOOR_GREEN_LED)
 {
 HAL_GPIO_WritePin(DOOR_GREEN_LED_CTL_POS_GPIO_Port,DOOR_GREEN_LED_CTL_POS_Pin,(GPIO_PinState)status); 
 }
/*门锁状态LED橙色灯*/
 if(led & DOOR_ORANGE_LED)
 {
 HAL_GPIO_WritePin(DOOR_ORANGE_LED_CTL_POS_GPIO_Port,DOOR_ORANGE_LED_CTL_POS_Pin,(GPIO_PinState)status); 
 }
}
/*RS485接收和发送控制*/
void BSP_RS485_RX_ENABLE()
{
 HAL_GPIO_WritePin(RS485_RT_CTL_POS_GPIO_Port,RS485_RT_CTL_POS_Pin,(GPIO_PinState)RS485_RX_CTL_ENABLE); 
}
void BSP_RS485_TX_ENABLE()
{
 HAL_GPIO_WritePin(RS485_RT_CTL_POS_GPIO_Port,RS485_RT_CTL_POS_Pin,(GPIO_PinState)RS485_TX_CTL_ENABLE);  
}

/*锁操作*/
void BSP_LOCK_CTL(bsp_status_t status)
{
 HAL_GPIO_WritePin(LOCK_CTL_POS_GPIO_Port,LOCK_CTL_POS_Pin,(GPIO_PinState)status); 
}

/*电磁锁操作*/
void BSP_DOOR_CTL(bsp_status_t status)
{
 HAL_GPIO_WritePin(DOOR_CTL_POS_GPIO_Port,DOOR_CTL_POS_Pin,(GPIO_PinState)status); 
}

/*玻璃加热电源操作*/
void BSP_GLASS_PWR_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(GLASS_PWR_CTL_POS_GPIO_Port,GLASS_PWR_CTL_POS_Pin,(GPIO_PinState)status);
 bsp_glass_pwr_status=status;
}

/*灯带控制--IO控制*/
void BSP_LIGHT_TURN_ON_OFF(bsp_status_t status)
{
 if(status==LIGHT_CTL_ON)
 {
 HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
 //HAL_GPIO_WritePin(LIGHT_CTL_POS_GPIO_Port,LIGHT_CTL_POS_Pin,(GPIO_PinState)status);
 }
 else
 {
 HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1); 
 }  
 bsp_light_status=status;
}
/*压缩机控制*/
void BSP_COMPRESSOR_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(COMPRESSOR_CTL_POS_GPIO_Port,COMPRESSOR_CTL_POS_Pin,(GPIO_PinState)status);  
 bsp_compressor_pwr_status=status;
}

/*风扇输出控制--IO控制*/
void BSP_FAN_TURN_ON_OFF(bsp_status_t status)
{
 HAL_GPIO_WritePin(FAN_CTL_POS_GPIO_Port,FAN_CTL_POS_Pin,(GPIO_PinState)status);   
}

/*GPRS模块控制--IO控制*/
void BSP_GPRS_MODULE_PWR_CTL(bsp_status_t status)
{
 HAL_GPIO_WritePin(GPRS_MODULE_PWR_CTL_POS_GPIO_Port,GPRS_MODULE_PWR_CTL_POS_Pin,(GPIO_PinState)status);  
}

