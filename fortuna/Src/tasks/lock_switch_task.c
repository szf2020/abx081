#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "ABDK_ABX081_ZK.h"
#include "lock_switch_task.h"
#include "lock_ctrl_task.h"
#include "ups_status_task.h"
#include "light_ctrl_task.h"
#include "glass_pwr_task.h"
#include "fan_ctrl_task.h"
#include "shopping_task.h"
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
while(1)
{
osDelay(LOCK_SWITCH_TASK_INTERVAL);
lock_sw_status=bsp_get_lock_sw_status();
if(lock_sw_status==SW_STATUS_PRESS)
{
 BSP_LOCK_CTL(LOCK_CTL_UNLOCK);
 osDelay(1000);
}
}
  
}