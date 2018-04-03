#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_ABX081_ZK.h"
#include "temperature_task.h"
#include "compressor_task.h"
#define APP_LOG_MODULE_NAME   "[compressor]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

osThreadId compressor_task_hdl;


osTimerId compressor_work_time_timer_id;
osTimerId compressor_rest_time_timer_id;

app_bool_t is_compressor_enable_pwr_turn_on=APP_TRUE;

static void compressor_work_time_timer_expired(void const * argument);
static void compressor_rest_time_timer_expired(void const * argument);

static void compressor_work_time_timer_init();
static void compressor_work_time_timer_start();
static void compressor_work_time_timer_stop();

static void compressor_rest_time_timer_init();
static void compressor_rest_time_timer_start();
//static void compressor_rest_time_timer_stop();

static void compressor_task_pwr_turn_on();
static void compressor_task_pwr_turn_off();

static void compressor_task_pwr_turn_on();
static void compressor_task_pwr_turn_off();


static void compressor_work_time_timer_init()
{
 osTimerDef(compressor_work_time_timer,compressor_work_time_timer_expired);
 compressor_work_time_timer_id=osTimerCreate(osTimer(compressor_work_time_timer),osTimerOnce,0);
 APP_ASSERT(compressor_work_time_timer_id);
}

static void compressor_work_time_timer_start()
{
 APP_LOG_DEBUG("压缩机工作定时器开始.\r\n");
 osTimerStart(compressor_work_time_timer_id,COMPRESSOR_TASK_WORK_TIMEOUT);
}
static void compressor_work_time_timer_stop()
{
 APP_LOG_DEBUG("压缩机工作定时器停止.\r\n");
 osTimerStop(compressor_work_time_timer_id);
}
static void compressor_work_time_timer_expired(void const * argument)
{
 APP_LOG_DEBUG("压缩机到达最大工作时长.需要关闭休息.\r\n");
 compressor_task_pwr_turn_off();
 /*禁止打开压缩机*/
 is_compressor_enable_pwr_turn_on=APP_FALSE;
 compressor_rest_time_timer_start();
}

static void compressor_rest_time_timer_init()
{
 osTimerDef(compressor_rest_time_timer,compressor_rest_time_timer_expired);
 compressor_rest_time_timer_id=osTimerCreate(osTimer(compressor_rest_time_timer),osTimerOnce,0);
 APP_ASSERT(compressor_rest_time_timer_id);
}

static void compressor_rest_time_timer_start()
{
 APP_LOG_DEBUG("压缩机休息定时器开始.\r\n");
 osTimerStart(compressor_rest_time_timer_id,COMPRESSOR_TASK_REST_TIMEOUT);
}
/*
static void compressor_rest_time_timer_stop()
{
 APP_LOG_DEBUG("压缩机休息定时器停止.\r\n");
 osTimerStop(compressor_rest_time_timer_id);
}
*/
static void compressor_rest_time_timer_expired(void const * argument)
{
 APP_LOG_DEBUG("压缩机到达最小休息时长.可以被开启.\r\n");
 is_compressor_enable_pwr_turn_on=APP_TRUE;/*使能打开压缩机*/
 /*假设温度发生了变化*/
 osSignalSet(compressor_task_hdl,COMPRESSOR_TASK_TEMPERATURE_CHANGED_SIGNAL);
}


static void compressor_task_pwr_turn_on()
{
 compressor_work_time_timer_start();
 APP_LOG_DEBUG("打开压缩机！\r\n");
 BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_ON); 
}

static void compressor_task_pwr_turn_off()
{
 compressor_work_time_timer_stop();
 APP_LOG_DEBUG("关闭压缩机！\r\n");
 BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_OFF); 
}

static void compressor_task_process_temperature_change()
{
 int16_t t;
 bsp_status_t compressor_pwr_status;
 APP_LOG_DEBUG("处理压缩机工作状态...\r\n"); 
 compressor_pwr_status=bsp_get_compressor_pwr_status();
 t=get_average_temperature();
 APP_LOG_DEBUG("当前温度值：%d ℃.\r\n",t);
 /*温度错误或者小于最低温度值*/
 if(t==TEMPERATURE_TASK_ERR_T_VALUE || t<=COMPRESSOR_TASK_TEMPERATURE_MIN)
 {
  if(compressor_pwr_status==COMPRESSOR_PWR_STATUS_ON)
  compressor_task_pwr_turn_off();
  else
  {
   APP_LOG_DEBUG("压缩机已经是关闭状态.不需再次关闭.\r\n");  
  }
 }
 else if(t>=COMPRESSOR_TASK_TEMPERATURE_MAX)
 {
  if(compressor_pwr_status==COMPRESSOR_PWR_STATUS_OFF)
  {
   if(is_compressor_enable_pwr_turn_on==APP_TRUE)
   compressor_task_pwr_turn_on();
   else
   {
   APP_LOG_WARNING("压缩机在休息时间.无法打开压缩机！\r\n"); 
   }
  }
  else
  {
  APP_LOG_DEBUG("压缩机已经是打开状态.不需再次打开\r\n");  
  }
 }   
}

static void compressor_task_debug_pwr_turn_on()
{
 APP_LOG_DEBUG("调试打开压缩机！\r\n");
 BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_ON); 
}
static void compressor_task_debug_pwr_turn_off()
{
 APP_LOG_DEBUG("调试关闭压缩机！\r\n");
 BSP_COMPRESSOR_TURN_ON_OFF(COMPRESSOR_PWR_CTL_OFF); 
}




void compressor_task(void const * argument)
{
 osEvent signal;
 APP_LOG_INFO("@压缩机任务开始.\r\n"); 
 
 compressor_work_time_timer_init();
 compressor_rest_time_timer_init();
 
 /*开机时先关闭压缩机*/
 compressor_task_pwr_turn_off();
 osDelay(1000);/*等待1s*/
 
 while(1)
 {
 signal=osSignalWait(COMPRESSOR_TASK_ALL_SIGNALS,COMPRESSOR_TASK_INTERVAL);
 
 /*处理信号*/
 if(signal.status==osEventSignal)
 {
  APP_LOG_DEBUG("压缩机任务收到信号.\r\n");
 if(signal.value.signals & COMPRESSOR_TASK_PWR_TURN_ON_SIGNAL)
 {
  APP_LOG_DEBUG("打开压缩机！\r\n");
  compressor_task_pwr_turn_on();
 }
 if(signal.value.signals & COMPRESSOR_TASK_PWR_TURN_OFF_SIGNAL)
 {
  APP_LOG_DEBUG("关闭压缩机！\r\n");
  compressor_task_pwr_turn_off();
 }
 if(signal.value.signals & COMPRESSOR_TASK_TEMPERATURE_CHANGED_SIGNAL)
 {
  APP_LOG_DEBUG("压缩机 收到温度变化.\r\n");
  compressor_task_process_temperature_change();
 }
 
 if(signal.value.signals & COMPRESSOR_TASK_DEBUG_PWR_TURN_ON_SIGNAL)
 {
  APP_LOG_DEBUG("调试打开压缩机！\r\n");
  compressor_task_debug_pwr_turn_on();
 }
 if(signal.value.signals & COMPRESSOR_TASK_DEBUG_PWR_TURN_OFF_SIGNAL)
 {
  APP_LOG_DEBUG("调试关闭压缩机！\r\n");
  compressor_task_debug_pwr_turn_off();
 }
 }
 }
}
