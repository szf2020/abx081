#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "string.h"
#include "json.h"
#include "lock_ctrl_task.h"
#include "ups_status_task.h"
#include "temperature_task.h"
#include "service.h"
#include "shopping_task.h"
#include "report_task.h"


#define APP_LOG_MODULE_NAME   "[report]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


osThreadId report_task_hdl;

static http_response_t report_response;
static http_request_t  report_request;

EventGroupHandle_t task_sync_evt_group_hdl;

extern json_report_device_status_t  report_device;


/*设备上报任务*/
void report_task(void const * argument)
{
  app_bool_t result;
  uint8_t param_size;
  json_item_t item;
  
  APP_LOG_INFO("@上报设备状态任务开始.\r\n");
  /*复位*/
  service_reset();   
  APP_LOG_DEBUG("上报设备状态任务等待同步完成...\r\n");
  /*等待任务同步*/
  xEventGroupSync(task_sync_evt_group_hdl,REPORT_TASK_SYNC_EVT,SHOPPING_TASK_SYNC_EVT|REPORT_TASK_SYNC_EVT,osWaitForever); 
  APP_LOG_DEBUG("上报设备状态任务等待同步完成.\r\n");
  
 /*获取运营商字符*/
  service_cpy_operator_str_to(report_device.net.value);
 /*串号--MAC地址*/
  /* 暂时需要固定imei值 
  service_cpy_imei_str_to(report_device.pid.value);
  service_cpy_imei_str_to(report_device.push_id.value); 
  */
  
  while(1)
  {
  osDelay(REPORT_TASK_INTERVAL);
  /*上报设备状态*/

  while(1)
  {
  /*获取IP地址 这个是随着复位变化的所以要每一次读取*/
  do
  {
  result=service_get_ip_str(report_device.ip.value);
  }while(result==APP_FALSE); 
  
 /*信号质量 这个是变化的所以要每一次读取*/
  do
  {
  result=service_get_rssi_str(report_device.rssi.value);
  }while(result==APP_FALSE);
  
  /*获取UPS状态*/
  if(ups_status_task_get_ups_status()==UPS_STATUS_TASK_STATUS_PWR_ON)
  {
   json_set_item_name_value(&report_device.m_power,NULL,"1"); /*主电源状态*/ 
  }
  else
  {
   json_set_item_name_value(&report_device.m_power,NULL,"2"); /*主电源状态*/  
  }
  /*获取温度值*/
  json_set_item_name_value(&report_device.temperature,NULL,get_average_temperature_str());
  /*获取锁的异常状态*/
  if(lock_ctrl_task_get_lock_exception()==LOCK_CTRL_TASK_LOCK_EXCEPTION_NONE)
  {
  json_set_item_name_value(&report_device.lock,NULL,"1"); 
  }
  else
  {
  json_set_item_name_value(&report_device.lock,NULL,"2");  
  }
  
  /*上报开始*/
  report_request.ptr_url=REPORT_DEVICE_URL;
  if(json_body_to_str(&report_device,report_request.param)!=APP_TRUE)
  {
  APP_LOG_ERROR("report param err.\r\n");
  }
  param_size=strlen((const char *)report_request.param);
  service_http_make_request_size_time_to_str(param_size,REPORT_TASK_DOWNLOAD_TIMEOUT,report_request.size_time);
  
  result=service_http_post(&report_request,&report_response,HTTP_RESPONSE_TIMEOUT);
  if(result==APP_TRUE)
  {
  json_set_item_name_value(&item,"code",NULL);
  json_get_item_value_by_name_from_json_str(report_response.json_str,item.name,item.value); 
  /*服务器回应code:"0"*/
  if(strcmp((const char *)item.value,"\"0\"")==0)
  break;
  }
  APP_LOG_ERROR("上报设备状态失败.\r\n");
  osDelay(REPORT_TASK_RETRY_TIMEOUT);
  }
  APP_LOG_INFO("上报设备状态成功.\r\n");  
  } 
}
