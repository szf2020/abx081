#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_ABX081_ZK.h"
#include "lock_ctrl_task.h"
#include "lock_switch_task.h"
#include "door_status_task.h"
#include "glass_pwr_task.h"
#include "fan_ctrl_task.h"
#define APP_LOG_MODULE_NAME   "[door_status]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId door_status_task_hdl;

static uint8_t door_status=DOOR_STATUS_TASK_DOOR_STATUS_INIT;

uint8_t door_status_task_get_door_status()
{
  return door_status;
}

void door_status_task(void const * argument)
{
 bsp_status_t door_up_status,door_dwn_status;
 APP_LOG_INFO("@门状态任务开始.\r\n");
 
 while(1)
 {
  door_up_status=bsp_get_door_up_status();
  door_dwn_status=bsp_get_door_dwn_status();
  
  if(door_up_status==door_dwn_status && door_up_status==DOOR_STATUS_OPEN)
  {
    /*门从关闭状态变化成开启状态*/
    if(door_status!=DOOR_STATUS_TASK_DOOR_STATUS_OPEN)
    {
    door_status=DOOR_STATUS_TASK_DOOR_STATUS_OPEN;
    /*向锁/风扇/加热玻璃/锁按键任务发送门开启信号*/
    osSignalSet(lock_ctrl_task_hdl,LOCK_CTRL_TASK_DOOR_STATUS_OPEN_SIGNAL);
    osSignalSet(fan_ctrl_task_hdl,FAN_CTRL_TASK_DOOR_STATUS_OPEN_SIGNAL);
    osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_DOOR_STATUS_OPEN_SIGNAL);
    osSignalSet(lock_switch_task_hdl,LOCK_SWITCH_TASK_DOOR_STATUS_OPEN_SIGNAL);
    }
  }
  if(door_up_status==door_dwn_status && door_up_status==DOOR_STATUS_CLOSE)
  {
     /*门从开启状态变化成关闭状态*/
    if(door_status!=DOOR_STATUS_TASK_DOOR_STATUS_CLOSE)
    {
      door_status=DOOR_STATUS_TASK_DOOR_STATUS_CLOSE;
      /*向锁/风扇/加热玻璃发送门开启信号*/
      osSignalSet(lock_ctrl_task_hdl,LOCK_CTRL_TASK_DOOR_STATUS_CLOSE_SIGNAL);
      osSignalSet(fan_ctrl_task_hdl,FAN_CTRL_TASK_DOOR_STATUS_CLOSE_SIGNAL);
      osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_DOOR_STATUS_CLOSE_SIGNAL);
    } 
  } 
  osDelay(DOOR_STATUS_TASK_INTERVAL);
 }  
}