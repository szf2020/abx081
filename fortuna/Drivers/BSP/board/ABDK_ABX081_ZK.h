#ifndef  __ABDK_ABX081_ZK_H__
#define  __ABDK_ABX081_ZK_H__
#include "gpio.h"
#include "usart.h"
#include "adc.h"


#define  SYS_LED                        (1<<0)
#define  DOOR_RED_LED                   (1<<1)
#define  DOOR_GREEN_LED                 (1<<2)
#define  DOOR_ORANGE_LED                (1<<3)


typedef enum 
{
/*LED*/
 LED_CTL_ON =GPIO_PIN_RESET,
 LED_CTL_OFF=GPIO_PIN_SET,
 LED_STATUS_ON=GPIO_PIN_RESET,                     
 LED_STATUS_OFF=GPIO_PIN_SET,
 /*485 接收和发送使能控制*/
 RS485_RX_CTL_ENABLE=GPIO_PIN_RESET,
 RS485_TX_CTL_ENABLE=GPIO_PIN_SET,
 /*交流电*/
 AC_CTL_ON=GPIO_PIN_SET,
 AC_CTL_OFF=GPIO_PIN_RESET,
 AC_STATUS_ON=GPIO_PIN_SET,
 AC_STATUS_OFF=GPIO_PIN_RESET,
 /*按键*/
 SW_STATUS_PRESS=GPIO_PIN_RESET,
 SW_STATUS_RELEASE=GPIO_PIN_SET,
 /*UPS*/
 UPS_PWR_STATUS_ON=GPIO_PIN_SET,/*UPS连接了主电源*/
 UPS_PWR_STATUS_OFF=GPIO_PIN_RESET,/*UPS断开了主电源*/
 /*锁*/
 LOCK_CTL_UNLOCK=GPIO_PIN_SET,
 LOCK_CTL_LOCK=GPIO_PIN_RESET,
 LOCK_STATUS_UNLOCK=GPIO_PIN_SET,
 LOCK_STATUS_LOCK=GPIO_PIN_RESET,
 /*吸住门的电磁锁*/
 DOOR_CTL_LOCK=GPIO_PIN_SET,
 DOOR_CTL_UNLOCK=GPIO_PIN_RESET,
 DOOR_STATUS_UNLOCK=GPIO_PIN_SET,
 DOOR_STATUS_LOCK=GPIO_PIN_RESET,
 /*门的状态*/
 DOOR_STATUS_OPEN=GPIO_PIN_SET,
 DOOR_STATUS_CLOSE=GPIO_PIN_RESET,
 /*灯条*/
 LIGHT_CTL_ON=GPIO_PIN_SET,
 LIGHT_CTL_OFF=GPIO_PIN_RESET,
 LIGHT_STATUS_ON=GPIO_PIN_SET,
 LIGHT_STATUS_OFF=GPIO_PIN_RESET,
 /*DC风扇*/
 FAN_CTL_ON=GPIO_PIN_SET,
 FAN_CTL_OFF=GPIO_PIN_RESET,
 FAN_STATUS_ON=GPIO_PIN_SET,
 FAN_STATUS_OFF=GPIO_PIN_RESET,
 /*玻璃电源*/
 GLASS_PWR_CTL_ON=GPIO_PIN_RESET,
 GLASS_PWR_CTL_OFF=GPIO_PIN_SET,
 GLASS_PWR_STATUS_ON=GPIO_PIN_RESET,
 GLASS_PWR_STATUS_OFF=GPIO_PIN_SET,
 /*压缩机*/
 COMPRESSOR_PWR_CTL_ON=GPIO_PIN_SET,
 COMPRESSOR_PWR_CTL_OFF=GPIO_PIN_RESET,
 COMPRESSOR_PWR_STATUS_ON=GPIO_PIN_SET,
 COMPRESSOR_PWR_STATUS_OFF=GPIO_PIN_RESET,
 /*GPRS模块*/
 GPRS_MODULE_PWR_CTL_ON=GPIO_PIN_SET,
 GPRS_MODULE_PWR_CTL_OFF=GPIO_PIN_RESET,
}bsp_status_t;

/*获取锁舌传感器状态*/
bsp_status_t bsp_get_lock_status();
/*获取开锁按键状态*/
bsp_status_t bsp_get_lock_sw_status();
/*获取UPS1状态*/
bsp_status_t bsp_get_ups1_status();
/*获取UPS2状态*/
bsp_status_t bsp_get_ups2_status();
/*获取门上部传感器状态*/
bsp_status_t bsp_get_door_up_status();
/*获取门下部传感器状态*/
bsp_status_t bsp_get_door_dwn_status();
/*获取压缩机电源状态*/
bsp_status_t bsp_get_compressor_pwr_status();
/*获取玻璃加热电源状态*/
bsp_status_t bsp_get_glass_pwr_status();
/*获取灯带状态*/
bsp_status_t bsp_get_light_status();

/*led灯控制*/
void BSP_LED_TURN_ON_OFF(uint8_t led,bsp_status_t status);
/*RS485接收控制*/
void BSP_RS485_RX_ENABLE();
/*RS485发送控制*/
void BSP_RS485_TX_ENABLE();
/*交流电控制*/
void BSP_AC_TURN_ON_OFF(uint8_t ac,bsp_status_t status);
/*锁控制*/
void BSP_LOCK_CTL(bsp_status_t status);
/*锁控制*/
void BSP_DOOR_CTL(bsp_status_t status);
/*玻璃加热电源控制*/
void BSP_GLASS_PWR_TURN_ON_OFF(bsp_status_t status);
/*灯带控制*/
void BSP_LIGHT_TURN_ON_OFF(bsp_status_t status);
/*压缩机控制*/
void BSP_COMPRESSOR_TURN_ON_OFF(bsp_status_t status);
/*直流风扇输出控制*/
void BSP_FAN_TURN_ON_OFF(bsp_status_t status);
/*GPRS模块控制--IO控制*/
void BSP_GPRS_MODULE_PWR_CTL(bsp_status_t status);














#endif