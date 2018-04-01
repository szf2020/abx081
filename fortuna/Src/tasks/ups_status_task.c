#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_ABX081_ZK.h"
#include "ups_status_task.h"
#include "light_ctrl_task.h"
#include "lock_ctrl_task.h"
#include "fan_ctrl_task.h"
#include "glass_pwr_task.h"
#include "shopping_task.h"
#define APP_LOG_MODULE_NAME   "[ups_status]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


osThreadId ups_status_task_hdl;

static uint8_t ups_status=UPS_STATUS_TASK_STATUS_INIT;

uint8_t ups_status_task_get_ups_status()
{
return ups_status;
}

static void update_ups_status()
{
/*UPS有两条信号指示IO 必须全部一致才可以判断UPS状态*/
bsp_status_t status1,status2;

static uint8_t pwr_off_cnt=0;
static uint8_t pwr_on_cnt=0;

status1=bsp_get_ups1_status();
status2=bsp_get_ups2_status();

/*如果UPS连接了主电源*/
if(status1==UPS_PWR_STATUS_ON && status2==UPS_PWR_STATUS_ON)
{
 pwr_off_cnt=0;
 pwr_on_cnt++;
 if(pwr_on_cnt>=UPS_STATUS_TASK_HOLD_CNT_MAX)/*过滤异常电平跳变*/
 {
  pwr_on_cnt=0;
 /*UPS从电池供电变为市电时*/
 if(ups_status!=UPS_STATUS_TASK_STATUS_PWR_ON)
 {
  APP_LOG_DEBUG("UPS状态切换为-->市电.\r\n");
  ups_status=UPS_STATUS_TASK_STATUS_PWR_ON;
  osSignalSet(light_ctrl_task_hdl,LIGHT_CTRL_TASK_UPS_PWR_ON_SIGNAL);
  osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_UPS_PWR_ON_SIGNAL);
  osSignalSet(fan_ctrl_task_hdl,FAN_CTRL_TASK_UPS_PWR_ON_SIGNAL);
  osSignalSet(shopping_task_hdl,SHOPPING_TASK_UPS_PWR_ON_SIGNAL);
 }
 }
}
else
{
 pwr_on_cnt=0;
 pwr_off_cnt++;
 if(pwr_off_cnt>=UPS_STATUS_TASK_HOLD_CNT_MAX)/*过滤异常电平跳变*/
 {
 pwr_off_cnt=0;
 if(ups_status!=UPS_STATUS_TASK_STATUS_PWR_OFF)
 {
  APP_LOG_DEBUG("UPS状态切换为-->电池.\r\n");
  ups_status=UPS_STATUS_TASK_STATUS_PWR_OFF;  
  osSignalSet(light_ctrl_task_hdl,LIGHT_CTRL_TASK_UPS_PWR_OFF_SIGNAL);
  osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_UPS_PWR_OFF_SIGNAL);
  osSignalSet(fan_ctrl_task_hdl,FAN_CTRL_TASK_UPS_PWR_OFF_SIGNAL);
  osSignalSet(shopping_task_hdl,SHOPPING_TASK_UPS_PWR_OFF_SIGNAL);
 }
 }
}
}

/*UPS状态查询任务*/
void ups_status_task(void const * argument)
{
 APP_LOG_INFO("@UPS状态任务开始.\r\n");
 while(1)
 {
 osDelay(UPS_STATUS_TASK_INTERVAL);
 update_ups_status();
 }
   
}