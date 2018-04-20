#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "string.h"
#include "at_cmd_set.h"
#include "at_cmd_port_serial.h"
#include "at_cmd_port_timer.h"
#define APP_LOG_MODULE_NAME   "[AT_CMD_SET]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


static uint8_t send_adu_buff[MAX_ADU_SIZE];
static uint8_t volatile recv_adu_buff[MAX_ADU_SIZE];

static uint16_t volatile send_adu_size;
static uint16_t volatile recv_adu_size;

EventGroupHandle_t at_cmd_evt_group_hdl;
osMutexId at_cmd_mutex_id;

#define  AT_CMD_SEND_COMPLETED_EVT                 (1<<0)
#define  AT_CMD_RECV_COMPLETED_EVT                 (1<<1)
#define  AT_CMD_SEND_TIMEOUT_EVT                   (1<<2)
#define  AT_CMD_RECV_TIMEOUT_EVT                   (1<<3)
#define  AT_CMD_ALL_EVT                            ((1<<4)-1)


#define  AT_CMD_HEAD                                "AT"
#define  AT_CMD_OP_EQ                               "="
#define  AT_CMD_PARAM_QUERY                         "?"
#define  AT_CMD_DELIMITER                           ";"
#define  AT_CMD_END                                 "\r"

static at_cmd_status_t at_cmd_wait_response(at_cmd_response_t *ptr_response);


void at_cmd_byte_receive(void)
{
 uint8_t recv_byte;
 at_cmd_port_serial_get_byte(&recv_byte);
 if(recv_adu_size>=MAX_ADU_SIZE)
 {
 APP_LOG_ERROR("AT串口缓存溢出.本次数据无效.\r\n");
 recv_adu_size=0;
 }
 recv_adu_buff[recv_adu_size++]=recv_byte;
 /*重新开始t35定时器*/
 at_cmd_port_timer_35_start();
}

void at_cmd_byte_send(void)
{
static uint8_t send_idx=0;
if(send_adu_size!=0)
{
 at_cmd_port_serial_send_byte(*(uint8_t*)&send_adu_buff[send_idx++]); 
 send_adu_size--;
}
else
{
 send_idx=0;
 /*禁止发送，禁止接收*/
 at_cmd_port_serial_enable(AT_CMD_FALSE,AT_CMD_FALSE); 
 xEventGroupSetBitsFromISR(at_cmd_evt_group_hdl,AT_CMD_SEND_COMPLETED_EVT,NULL);
}
}

void at_cmd_timer_35_expired()
{
 xEventGroupSetBits(at_cmd_evt_group_hdl,AT_CMD_RECV_COMPLETED_EVT);  
}

void at_cmd_init()
{
 /*创建互斥体*/
 if(at_cmd_mutex_id==NULL)
 {
 osMutexDef(at_cmd_mutex);
 at_cmd_mutex_id=osMutexCreate(osMutex(at_cmd_mutex)); 
 APP_ASSERT(at_cmd_mutex_id);
 }

 /*创建事件组*/
 if(at_cmd_evt_group_hdl==NULL)
 {
 at_cmd_evt_group_hdl=xEventGroupCreate();
 APP_ASSERT(at_cmd_evt_group_hdl);
 }
  
 /*串口和定时器初始化*/
 at_cmd_port_serial_init();
 at_cmd_port_timer_init(); 
}

/*获取互斥体*/
static void at_cmd_take_mutex()
{ 
 if(osMutexWait(at_cmd_mutex_id,osWaitForever)!=osOK)
 {
  APP_ERROR_HANDLER(0);
 }
}

/*释放互斥体*/
static void at_cmd_release_mutex()
{
 if(osMutexRelease(at_cmd_mutex_id)!=osOK)
 {
  APP_ERROR_HANDLER(0);
 }
}


static at_cmd_status_t at_cmd_send_cmd()
{
 EventBits_t events;
 at_cmd_status_t status=AT_CMD_STATUS_SUCCESS;
 send_adu_buff[send_adu_size]='\0';
 
APP_LOG_DEBUG("发送的数据:---->\r\n%s\r\n",send_adu_buff);
 /*启动串口发送数据*/
 at_cmd_port_serial_enable(AT_CMD_FALSE,AT_CMD_TRUE);
 /*等待发送完成*/
 events= xEventGroupWaitBits( at_cmd_evt_group_hdl,AT_CMD_ALL_EVT,pdTRUE,pdFALSE,AT_CMD_SEND_TIMEOUT);

 if(!(events &  AT_CMD_SEND_COMPLETED_EVT))
 {
  /*禁止串口发送数据*/
  at_cmd_port_serial_enable(AT_CMD_FALSE,AT_CMD_FALSE);
  status= AT_CMD_STATUS_SEND_TIMEOUT;
  APP_LOG_ERROR("发送数据失败.status:%d\r\n",status);
 } 
 return status;
}

static at_cmd_status_t at_cmd_wait_response(at_cmd_response_t *ptr_response)
{
 EventBits_t events;
 at_cmd_status_t status=AT_CMD_STATUS_SUCCESS;
restart:
 /*使能串口接收数据*/
 at_cmd_port_serial_enable(AT_CMD_TRUE,AT_CMD_FALSE);
 
  /*等待接收完成*/
 events= xEventGroupWaitBits(at_cmd_evt_group_hdl,AT_CMD_ALL_EVT,pdTRUE,pdFALSE,ptr_response->response_timeout);
 /*禁止串口发送和接收数据*/
 at_cmd_port_serial_enable(AT_CMD_FALSE,AT_CMD_FALSE);
 if(!(events &  AT_CMD_RECV_COMPLETED_EVT))
 {
  status= AT_CMD_STATUS_RECV_TIMEOUT;
  return status;
 }
 /*拷贝回应*/
 APP_ASSERT(ptr_response);
 if(ptr_response->is_response_delay==AT_CMD_TRUE)/*需要等待二次回应*/
 {
   ptr_response->is_response_delay=AT_CMD_FALSE;
   ptr_response->response_timeout= ptr_response->response_delay_timeout;
   goto restart;
 }
 recv_adu_buff[recv_adu_size]='\0';
 strcpy((char*)ptr_response->response,(const char *)recv_adu_buff);
 ptr_response->size=recv_adu_size; 
 APP_LOG_DEBUG("响应的数据:---->\r\n%s\r\n",ptr_response->response);
 return status; 
}

/*AT基础指令---AT<CMDPARAM>*/
at_cmd_status_t at_base_cmd(uint8_t *ptr_cmd,at_cmd_response_t *ptr_response)
{
 at_cmd_status_t status;
 uint8_t len;
 at_cmd_take_mutex();
 APP_ASSERT(ptr_cmd);
 /*AT基础命令*/
 send_adu_size=0;
 len=strlen((char const*)AT_CMD_HEAD);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_HEAD,len);
 send_adu_size+=len;
 len=strlen((char const*)ptr_cmd);
 memcpy(&send_adu_buff[send_adu_size],ptr_cmd,len);
 send_adu_size+=len;
 len=strlen((char const*)AT_CMD_END);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_END,len);
 send_adu_size+=len;
 status=at_cmd_send_cmd();
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   goto err_handle;
 }
 
 status= at_cmd_wait_response(ptr_response);
err_handle:
 recv_adu_size=0;
 at_cmd_release_mutex();
 return status; 
}

/*AT拓展测试指令---AT+CMD=?*/
at_cmd_status_t at_ex_cmd_test(uint8_t *ptr_cmd,at_cmd_response_t *ptr_response)
{ 
 at_cmd_status_t status;
 uint8_t len;
 at_cmd_take_mutex();
 APP_ASSERT(ptr_cmd);
 /*AT拓展测试命令*/
 send_adu_size=0;
 len=strlen((char const*)AT_CMD_HEAD);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_HEAD,len);
 send_adu_size+=len;
 len=strlen((char const*)ptr_cmd);
 memcpy(&send_adu_buff[send_adu_size],ptr_cmd,len);
 send_adu_size+=len;
 len=strlen((char const*)AT_CMD_OP_EQ AT_CMD_PARAM_QUERY AT_CMD_END);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_OP_EQ AT_CMD_PARAM_QUERY AT_CMD_END,len);
 send_adu_size+=len;
 status=at_cmd_send_cmd();
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   goto err_handle;
 }
 status= at_cmd_wait_response(ptr_response);
err_handle:
 recv_adu_size=0;
 at_cmd_release_mutex();
 return status;
}

/*AT拓展查询指令---AT+CMD?*/
at_cmd_status_t at_ex_cmd_query(uint8_t *ptr_cmd,at_cmd_response_t *ptr_response)
{
 at_cmd_status_t status;
 uint8_t len;
 at_cmd_take_mutex();
 APP_ASSERT(ptr_cmd);
 /*AT拓展查询指令*/
 send_adu_size=0;
 len=strlen((char const*)AT_CMD_HEAD);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_HEAD,len);
 send_adu_size+=len;
 len=strlen((char const*)ptr_cmd);
 memcpy(&send_adu_buff[send_adu_size],ptr_cmd,len);
 send_adu_size+=len;
 len=strlen((char const*)AT_CMD_PARAM_QUERY AT_CMD_END);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_PARAM_QUERY AT_CMD_END,len);
 send_adu_size+=len;
 status=at_cmd_send_cmd();
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   goto err_handle;
 }
 status= at_cmd_wait_response(ptr_response);
err_handle:
 recv_adu_size=0;
 at_cmd_release_mutex();
 return status;  
  
}
/*AT拓展设置指令---AT+CMD=PARAM*/
at_cmd_status_t at_ex_cmd_set(uint8_t *ptr_cmd,uint8_t *ptr_param,at_cmd_response_t *ptr_response)
{
 at_cmd_status_t status;
 uint8_t len;
 at_cmd_take_mutex();
 APP_ASSERT(ptr_cmd);
 /*AT拓展设置命令参数*/
 send_adu_size=0;
 len=strlen((char const*)AT_CMD_HEAD);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_HEAD,len);
 send_adu_size+=len;
 /*命令类型*/
 len=strlen((char const*)ptr_cmd);
 memcpy(&send_adu_buff[send_adu_size],ptr_cmd,len);
 send_adu_size+=len;
 /*命令操作符*/
 len=strlen((char const*)AT_CMD_OP_EQ);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_OP_EQ,len);
 send_adu_size+=len;
 /*命令参数*/
 len=strlen((char const*)ptr_param);
 memcpy(&send_adu_buff[send_adu_size],ptr_param,len);
 send_adu_size+=len;
 /*命令结尾*/
 len=strlen((char const*)AT_CMD_END);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_END,len);
 send_adu_size+=len;
 status=at_cmd_send_cmd();
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   goto err_handle;
 }
 status=at_cmd_wait_response(ptr_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   goto err_handle;
 }
err_handle: 
 recv_adu_size=0;
 at_cmd_release_mutex();
 return status;  
  
}
/*AT拓展执行指令---AT+CMD*/
at_cmd_status_t at_ex_cmd_exe(uint8_t *ptr_cmd,at_cmd_response_t *ptr_response)
{
 at_cmd_status_t status;
 uint8_t len;
 at_cmd_take_mutex();
 APP_ASSERT(ptr_cmd);
 /*AT拓展执行命令*/
 send_adu_size=0;
 len=strlen((char const*)AT_CMD_HEAD);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_HEAD,len);
 send_adu_size+=len;
 /*命令类型*/
 len=strlen((char const*)ptr_cmd);
 memcpy(&send_adu_buff[send_adu_size],ptr_cmd,len);
 send_adu_size+=len;
 /*命令结尾*/
 len=strlen((char const*)AT_CMD_END);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_END,len);
 send_adu_size+=len;
 status=at_cmd_send_cmd();
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   goto err_handle;
 }
 status= at_cmd_wait_response(ptr_response);
 
err_handle:
 recv_adu_size=0;
 at_cmd_release_mutex();
 return status;
}
/*AT自由数据发送 自定义字符串*/
at_cmd_status_t at_cmd_string(uint8_t *ptr_str,at_cmd_response_t *ptr_response)
{
 at_cmd_status_t status;
 uint8_t len;
 at_cmd_take_mutex();
 APP_ASSERT(ptr_str);
 /*AT拓展执行命令*/
 send_adu_size=0;
 len=strlen((char const*)ptr_str);
 memcpy(&send_adu_buff[send_adu_size],ptr_str,len);
 send_adu_size+=len;
 /*命令结尾*/
 len=strlen((char const*)AT_CMD_END);
 memcpy(&send_adu_buff[send_adu_size],AT_CMD_END,len);
 send_adu_size+=len;
 status=at_cmd_send_cmd();
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  goto err_handle;
 }
 status= at_cmd_wait_response(ptr_response);
err_handle:
 recv_adu_size=0;
 at_cmd_release_mutex();
 return status; 
  
}
/*AT回应中查找期望的字符串*/
at_cmd_status_t at_cmd_find_expect_from_response(at_cmd_response_t *ptr_response,uint8_t *ptr_expect)
{
 at_cmd_status_t status=AT_CMD_STATUS_SUCCESS; 
 APP_ASSERT(ptr_response); 
 APP_ASSERT(ptr_expect); 
 if(strstr((char const*)ptr_response->response,(char const*)ptr_expect)==NULL)
 {
  status= AT_CMD_STATUS_INVALID_RESPONSE;
 }
 
 return status;
}
