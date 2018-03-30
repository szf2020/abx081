#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "string.h"
#include "app_common.h"
#include "lock_ctrl_task.h"
#include "door_status_task.h"
#include "compressor_task.h"
#include "temperature_task.h"
#include "light_ctrl_task.h"
#include "fan_ctrl_task.h"
#include "glass_pwr_task.h"
#include "ups_status_task.h"
#include "flash.h"
#include "debug_task.h"
#include "cpu_utils.h"
#include "ABDK_ABX081_ZK.h"
#define APP_LOG_MODULE_NAME   "[debug]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

#define  DEBUG_CMD_MAX_LEN            30

osThreadId debug_task_hdl;
uint8_t cmd[DEBUG_CMD_MAX_LEN];


static uint16_t data_cnt;
static uint8_t  offset,cmd_len,recv_len;


/*RTT调试任务*/
void debug_task(void const * argument)
{
 app_bool_t debug_enable=APP_TRUE;

 APP_LOG_INFO("@调试任务开始.\r\n");

 while(1)
 {
  osDelay(DEBUG_TASK_WAIT_TIMEOUT); 
  data_cnt=SEGGER_RTT_HasData(0);
  /*buff0没有数据或者有超出最大数量值的数据*/
  if(data_cnt==0)
    continue;  
  data_cnt=SEGGER_RTT_Read(0,cmd,DEBUG_CMD_MAX_LEN);
  if(debug_enable!=APP_TRUE)
    continue;
  APP_LOG_DEBUG("读的字节数：%d\r\n",data_cnt);  
  if(data_cnt>DEBUG_CMD_MAX_LEN-1)
    data_cnt=DEBUG_CMD_MAX_LEN-1;
  cmd[data_cnt]=0;/*填充为完整字符串*/
  recv_len=strlen((char const*)cmd)-DEBUG_TASK_CMD_EOL_LEN;
  
 /*打开压缩机*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_COMPRESSOR);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_COMPRESSOR,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_COMPRESSOR_PARAM_LEN)
 {
   APP_LOG_ERROR("压缩机命令长度非法.\r\n");
 continue;
 } 
 /*向压缩机任务发送开机信号*/
 APP_LOG_DEBUG("向压缩机任务发送开机信号.\r\n");
 osSignalSet(compressor_task_hdl,COMPRESSOR_TASK_DEBUG_PWR_TURN_ON_SIGNAL);
 continue;
 }
 /*关闭压缩机*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_COMPRESSOR);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_COMPRESSOR,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_COMPRESSOR_PARAM_LEN)
 {
 APP_LOG_ERROR("压缩机命令长度非法.\r\n");
 continue;
 } 
 /*向压缩机任务发送关机信号*/
 APP_LOG_DEBUG("向压缩机任务发送关机信号.\r\n");
 osSignalSet(compressor_task_hdl,COMPRESSOR_TASK_DEBUG_PWR_TURN_OFF_SIGNAL);
 continue;
 }
 /*打开所有灯带*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_LIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_LIGHT,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_LIGHT_PARAM_LEN)
 {
 APP_LOG_ERROR("灯带命令长度非法.\r\n");
 continue;
 } 
 /*向灯带任务发送打开信号*/
 APP_LOG_DEBUG("向灯带任务发送打开信号.\r\n");
 osSignalSet(light_ctrl_task_hdl,LIGHT_CTRL_TASK_DEBUG_LIGHT_TURN_ON_SIGNAL);
 continue;
 }
 /*关闭所有灯带*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_LIGHT);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_LIGHT,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_LIGHT_PARAM_LEN)
 {
 APP_LOG_ERROR("灯带命令长度非法.\r\n");
 continue;
 } 
 /*向灯带任务发送关闭信号*/
 APP_LOG_DEBUG("向灯带任务发送关闭信号.\r\n");
 osSignalSet(light_ctrl_task_hdl,LIGHT_CTRL_TASK_DEBUG_LIGHT_TURN_OFF_SIGNAL);
 continue;
 } 
 
 /*打开玻璃电源*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_GLASS);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_GLASS,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_GLASS_PARAM_LEN)
 {
 APP_LOG_ERROR("命令长度非法.\r\n");
 continue;
 } 
 /*玻璃电源任务发送打开信号*/
 APP_LOG_DEBUG("玻璃电源任务发送打开信号.\r\n");
 osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_DEBUG_TURN_ON_SIGNAL);
 continue;
 } 
  /*关闭玻璃电源*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_GLASS);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_GLASS,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_GLASS_PARAM_LEN)
 {
 APP_LOG_ERROR("命令长度非法.\r\n");
 continue;
 } 
 /*玻璃电源任务发送关闭信号*/
 APP_LOG_DEBUG("玻璃电源任务发送关闭信号.\r\n");
 osSignalSet(glass_pwr_task_hdl,GLASS_PWR_TASK_DEBUG_TURN_OFF_SIGNAL);
 continue;
 } 
 
 /*获取UPS状态*/
 cmd_len=strlen(DEBUG_TASK_CMD_GET_UPS_STATE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_GET_UPS_STATE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_GET_UPS_STATE_PARAM_LEN)
 {
 APP_LOG_ERROR("UPS状态命令长度非法.\r\n");
 continue;
 } 
 /*获取UPS状态*/
 uint8_t status;
 status=ups_status_task_get_ups_status();
 if(status==UPS_STATUS_TASK_STATUS_PWR_ON)
 {
  APP_LOG_DEBUG("UPS状态--接通市电.\r\n");
 }
 else
 {
  APP_LOG_DEBUG("UPS状态--断开市电.\r\n");
 }
 continue;
 }
 /*获取锁状态*/
 cmd_len=strlen(DEBUG_TASK_CMD_GET_LOCK_STATE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_GET_LOCK_STATE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_GET_LOCK_STATE_PARAM_LEN)
 {
 APP_LOG_ERROR("锁状态命令长度非法.\r\n");
 continue;
 } 
 /*获取锁状态*/
 bsp_status_t status;
 status=bsp_get_lock_status();
 if(status==LOCK_STATUS_LOCK)
 {
  APP_LOG_DEBUG("锁状态--关闭.\r\n");
 }
 else
 {
 APP_LOG_DEBUG("锁状态--打开.\r\n");
 }
 continue;
 } 
  /*获取门状态*/
 cmd_len=strlen(DEBUG_TASK_CMD_GET_DOOR_STATE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_GET_DOOR_STATE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_GET_DOOR_STATE_PARAM_LEN)
 {
 APP_LOG_ERROR("门状态命令长度非法.\r\n");
 continue;
 } 
 /*获取门状态*/
 uint8_t status;
 status=door_status_task_get_door_status();
 if(status==DOOR_STATUS_TASK_DOOR_STATUS_CLOSE)
 {
  APP_LOG_DEBUG("门状态--关闭.\r\n");
 }
 if(status==DOOR_STATUS_TASK_DOOR_STATUS_OPEN)
 {
 APP_LOG_DEBUG("门状态--打开.\r\n");
 }
 continue;
 } 
 
 /*获取温度值*/
 cmd_len=strlen(DEBUG_TASK_CMD_OBTAIN_TEMPERATURE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_OBTAIN_TEMPERATURE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_OBTAIN_TEMPERATURE_PARAM_LEN)
 {
  APP_LOG_ERROR("温度命令长度非法.\r\n");
  continue;
 }
 int16_t t;
 offset=cmd_len;
 if(cmd[offset]=='0')/*0代表所有温度计平均值*/
 t=get_average_temperature();
 else if(cmd[offset]=='1')
 t=get_temperature(0);
 else if(cmd[offset]=='2')
 t=get_temperature(1);
 else
 {
  APP_LOG_ERROR("温度命令参数%2d非法.0-1-2之一.\r\n",cmd[offset]);
  continue;  
 }
 APP_LOG_DEBUG("温度值x10：%d 摄氏度.\r\n",t);
 continue;
 } 
  /*开锁*/
 cmd_len=strlen(DEBUG_TASK_CMD_UNLOCK_LOCK);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_UNLOCK_LOCK,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_UNLOCK_LOCK_PARAM_LEN)
 {
  APP_LOG_ERROR("开锁命令长度非法.\r\n");
  continue;
 }
 osSignalSet(lock_ctrl_task_hdl,LOCK_CTRL_TASK_DEBUG_UNLOCK_SIGNAL);
 continue;
 }
 /*关锁*/
 cmd_len=strlen(DEBUG_TASK_CMD_LOCK_LOCK);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_LOCK_LOCK,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_LOCK_LOCK_PARAM_LEN)
 {
  APP_LOG_ERROR("关锁命令长度非法.\r\n");
  continue;
 }
 osSignalSet(lock_ctrl_task_hdl,LOCK_CTRL_TASK_DEBUG_LOCK_SIGNAL);
 continue;
 }
 /*打开门指示灯*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_LOCK_LED);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_LOCK_LED,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_LOCK_LED_PARAM_LEN)
 {
  APP_LOG_ERROR("开门指示灯命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("打开门所有指示灯.\r\n");
 BSP_LED_TURN_ON_OFF(DOOR_RED_LED|DOOR_GREEN_LED|DOOR_ORANGE_LED,LED_CTL_ON);
 continue;
 }
 /*关闭门指示灯*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_LOCK_LED);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_LOCK_LED,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_LOCK_LED_PARAM_LEN)
 {
  APP_LOG_ERROR("关闭门指示灯命令长度非法.\r\n");
  continue;
 }
 APP_LOG_DEBUG("关闭门所有指示灯.\r\n");
 BSP_LED_TURN_ON_OFF(DOOR_RED_LED|DOOR_GREEN_LED|DOOR_ORANGE_LED,LED_CTL_OFF);
 continue;
 }
 
  /*cpu负载查询*/
 cmd_len=strlen(DEBUG_TASK_CMD_CPU_LOAD);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_CPU_LOAD,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_CPU_LOAD_PARAM_LEN)
 {
  APP_LOG_ERROR("查询cpu负载命令长度非法.\r\n");
  continue;
 }
 
 APP_LOG_DEBUG("CPU负载：%d %%.\r\n",osGetCPUUsage());
 continue;
 }
 
 /*flash测试*/
 cmd_len=strlen(DEBUG_TASK_CMD_FLASH_ERASE);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_FLASH_ERASE,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_FLASH_ERASE_PARAM_LEN)
 {
  APP_LOG_ERROR("擦除flash命令长度非法.\r\n");
  continue;
 }
 
 flash_erase_unreport_close_info();
 continue;
 }
 
 /*打开风扇*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_ON_FAN);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_ON_FAN,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_ON_FAN_PARAM_LEN)
 {
 APP_LOG_ERROR("风扇命令长度非法.\r\n");
 continue;
 } 
 /*向风扇任务发送打开信号*/
 APP_LOG_DEBUG("向风扇任务发送打开信号.\r\n");
 osSignalSet(fan_ctrl_task_hdl,FAN_CTRL_TASK_DEBUG_FAN_TURN_ON_SIGNAL);
 continue;
 }
 
 /*关闭风扇*/
 cmd_len=strlen(DEBUG_TASK_CMD_PWR_OFF_FAN);
 if(memcmp((const char*)cmd,DEBUG_TASK_CMD_PWR_OFF_FAN,cmd_len)==0)
 { 
 if(recv_len !=cmd_len+DEBUG_TASK_CMD_PWR_OFF_FAN_PARAM_LEN)
 {
 APP_LOG_ERROR("风扇命令长度非法.\r\n");
 continue;
 } 
 /*向风扇任务发送关闭信号*/
 APP_LOG_DEBUG("向风扇任务发送关闭信号.\r\n");
 osSignalSet(fan_ctrl_task_hdl,FAN_CTRL_TASK_DEBUG_FAN_TURN_OFF_SIGNAL);
 continue;
 }
 
 }
 }
  
  
  
  
  
  
  
  
  
  
  
  
  