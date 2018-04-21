#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_ABX081_ZK.h"
#include "lock_switch_task.h"
#include "lock_ctrl_task.h"
#include "ups_status_task.h"
#include "light_ctrl_task.h"
#include "glass_pwr_task.h"
#include "fan_ctrl_task.h"
#define APP_LOG_MODULE_NAME   "[lock_switch]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId lock_switch_task_hdl;

uint8_t lock_switch_status=LOCK_SWITCH_TASK_LOCK_SWITCH_STATUS_INIT;
/*锁按键任务*/
void lock_switch_task(void const * argument)
{
bsp_status_t lock_sw_status;
osEvent sig;
while(1)
{
osDelay(LOCK_SWITCH_TASK_INTERVAL);
lock_sw_status=bsp_get_lock_sw_status();
if(lock_sw_status==SW_STATUS_PRESS)
{
 APP_LOG_DEBUG("开门按键开门.\r\n");
 BSP_LOCK_CTL(LOCK_CTL_UNLOCK);
 /*等待门被打开的信号 如果没有被打开就自己关上*/
 sig=osSignalWait(LOCK_SWITCH_TASK_DOOR_STATUS_OPEN_SIGNAL,LOCK_SWITCH_TASK_WAIT_TIMEOUT);
 if(sig.status==osEventSignal && sig.value.signals & LOCK_SWITCH_TASK_DOOR_STATUS_OPEN_SIGNAL)
 {
 APP_LOG_DEBUG("开门按键按下后门被打开.\r\n"); 
 continue;
 }
 APP_LOG_DEBUG("开门按键按下后超时门自动上锁.\r\n"); 
 BSP_LOCK_CTL(LOCK_CTL_LOCK);
}
}
  
}