#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "at_cmd_set.h"
#include "service.h"
#include "string.h"
#include "json.h"
#include "ABDK_ABX081_ZK.h"
#define APP_LOG_MODULE_NAME   "[service]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"
#include "stdlib.h"

/*服务互斥体*/
osMutexId service_mutex_id;
static at_cmd_response_t at_cmd_response;
static uint8_t service_net_err_cnt,service_device_err_cnt;

static void service_uint16_to_str(uint16_t num,uint8_t *ptr_str);
/*处理GPRS通信错误 如通信失败*/
static app_bool_t service_handle_net_err(app_bool_t result);
/*处理GPRS设备错误 如死机*/
static app_bool_t service_handle_device_err(app_bool_t result);

static app_bool_t service_init();
static app_bool_t service_http_init();


/*获取运营商字符串*/
static app_bool_t service_get_operator_str(uint8_t *ptr_operator_str);
/*获取IMEI字符串*/
static app_bool_t service_get_imei_str(uint8_t *ptr_imei_str);

static uint8_t operator_str[5];
static uint8_t imei_str[20];

void service_mutex_init()
{
   /*创建互斥体*/
  if(service_mutex_id==NULL)
  {
   osMutexDef(service_mutex);
   service_mutex_id=osMutexCreate(osMutex(service_mutex)); 
   APP_ASSERT(service_mutex_id);
  }
}

/*获取互斥体*/
void take_service_mutex()
{
 if(osMutexWait(service_mutex_id,osWaitForever)!=osOK)
 {
  APP_ERROR_HANDLER(0);
 }
}

/*释放互斥体*/
void release_service_mutex()
{
 
 if(osMutexRelease(service_mutex_id)!=osOK)
 {
  APP_ERROR_HANDLER(0);
 }
}

#if (AT_MODULE == SIM900A_MODULE )  

static app_bool_t service_init()
{
 service_mutex_init();
 at_cmd_init();
 
 app_bool_t result=APP_TRUE;
 at_cmd_status_t status;
  /*进入AT测试模式*/
 APP_LOG_DEBUG("进入AT命令测试模式...\r\n");
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_cmd_string("AT",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("AT命令测试失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("AT命令测试失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 /*获取运营商代码*/
 if(service_get_operator_str(operator_str)==APP_FALSE)
 {
  result=APP_FALSE;
  goto err_handle;   
 }
/*获取IMEI*/
 if(service_get_imei_str(imei_str)==APP_FALSE)
 {
  result=APP_FALSE;
  goto err_handle;   
 }

 APP_LOG_DEBUG("AT命令测试成功.GPRS模块复位成功.status:%d.\r\n",status); 
 
 err_handle:
 return result;
}

static app_bool_t service_http_init()
{
 app_bool_t result=APP_TRUE;
 at_cmd_status_t status;
 
 /*第一步 设置数据承载模式AT+SAPBR=3,1,"CONTYPE","GPRS" */
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+SAPBR","3,1,\"CONTYPE\",\"GPRS\"",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+SAPBR承载参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+SAPBR承载参数设置失败. status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;  
 }
 
 /*第二步 设置数据运营商AT+SAPBR=3,1,"APN","xxxx" */
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 
 if(strcmp((const char *)operator_str,CHINA_MOBILE_2G_CODE_STR)==0)
 status=at_ex_cmd_set("+SAPBR","3,1,\"APN\",\"CMNET\"",&at_cmd_response);
 else if(strcmp((const char *)operator_str,CHINA_UNICOM_2G_CODE_STR)==0)
 status=at_ex_cmd_set("+SAPBR","3,1,\"APN\",\"UNINET\"",&at_cmd_response);
 
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+SAPBR运营商参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+SAPBR运营商参数设置失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 
 /*第三步 打开数据承载模式AT+SAPBR=1,1*/ 
 at_cmd_response.is_response_delay=AT_CMD_TRUE;
 at_cmd_response.response_delay_timeout=AT_CMD_CONFIG_SPECIAL_RESPONSE_TIMEOUT;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_SPECIAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+SAPBR","1,1",&at_cmd_response);
 
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+SAPBRP打开承载模式参数输入失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+SAPBR打开承载模式参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;
 }
 APP_LOG_DEBUG("+SAPBR打开承载模式参数设置成功. status:%d.\r\n",status); 
 
 /*第四步 http初始化AT+HTTPINIT*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_SPECIAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+HTTPINIT",&at_cmd_response);

 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPINIT参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPINIT参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;
 }
 APP_LOG_DEBUG("+HTTPINIT参数设置成功. status:%d\r\n",status);
 /*第五步设置内容类型AT+HTTPPARA="CONTENT","application/json"*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+HTTPPARA","\"CONTENT\",\"application/json\"",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPPARA内容类型参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPPARA内容类型参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
err_handle:
 return result;
}


app_bool_t service_http_post(http_request_t *ptr_request,http_response_t *ptr_response,uint16_t response_timeout)
{
 at_cmd_status_t status;
 app_bool_t result=APP_TRUE;
 /*只有一个http POST通道所以要避免竞争*/
 take_service_mutex();
  
 /*第一步设置url AT+HTTPPARA="URL","http://xxxx"*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+HTTPPARA",ptr_request->ptr_url,&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPPARA参数输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPPARA参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("POST url设置成功. status:%d\r\n",status);
 
 /*第二步设置POST数据大小和超时时间AT+HTTPDATA=size,timeout*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+HTTPDATA",ptr_request->size_time,&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPPARA POST参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"DOWNLOAD");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPPARA POST参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("+HTTPDATA POST设置成功. status:%d\r\n",status);
 
 /*第三步设置缓存POST数据“......”*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_cmd_string(ptr_request->param,&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("POST数据输入失败 status:%d.\r\n",status); 
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("POST数据缓存失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("POST数据缓存成功. status:%d\r\n",status); 
 
 /*第四步启动http POST*/
 at_cmd_response.is_response_delay=AT_CMD_TRUE;
 at_cmd_response.response_delay_timeout=response_timeout;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_SPECIAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+HTTPACTION","1",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+HTTPACTION POST数据输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+HTTPACTION POST数据失败.\r\n");
  status=at_cmd_find_expect_from_response(&at_cmd_response,"+SAPBR 1: DEACT");
  if(status==AT_CMD_STATUS_SUCCESS)
  {
    APP_LOG_ERROR("数据承载模式被关闭.\r\n");
    /*再次打开数据承载模式AT+SAPBR=1,1*/ 
    APP_LOG_ERROR("AT+SAPBR=1,1尝试再次打开数据承载模式...status:%d.\r\n",status); 
    at_cmd_response.is_response_delay=AT_CMD_TRUE;
    at_cmd_response.response_delay_timeout=AT_CMD_CONFIG_SPECIAL_RESPONSE_TIMEOUT;
    at_cmd_response.response_timeout=AT_CMD_CONFIG_SPECIAL_RESPONSE_TIMEOUT;
    status=at_ex_cmd_set("+SAPBR","1,1",&at_cmd_response);
    if(status!=AT_CMD_STATUS_SUCCESS)
    {
     APP_LOG_ERROR("+SAPBRP再次打开承载模式参数输入失败. status:%d.\r\n",status); 
     result=APP_FALSE;
     goto err_handle;
    }
    status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
    if(status!=AT_CMD_STATUS_SUCCESS)
    {
     APP_LOG_ERROR("+SAPBR再次打开承载失败. status:%d.\r\n",status); 
    }
    APP_LOG_DEBUG("+SAPBR再次打开承载成功. status:%d.\r\n",status); 
  }
  
  /*不管怎样 这次action是失败的*/
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("+HTTPDATA POST数据成功. status:%d\r\n",status); 
 status=at_cmd_find_expect_from_response(&at_cmd_response,"1,200,");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("POST数据到服务器失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 APP_LOG_DEBUG("POST数据到服务器成功. status:%d.\r\n",status); 
 /*第五步读取回应的数据*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+HTTPREAD",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("读POST回应数据输入失败 status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;
 }

 if(json_find_cpy_json_str_to((uint8_t *)at_cmd_response.response,ptr_response->json_str)!=APP_TRUE)
 {
  APP_LOG_ERROR("http回复没有json格式.\r\n");
  result=APP_FALSE;
  goto err_handle;
 }
  /*赋值回应*/
 ptr_response->size=at_cmd_response.size;
 APP_LOG_DEBUG("http回复中找到json格式.\r\n");
err_handle:
 service_handle_net_err(result);
 release_service_mutex();
 return result;
}

static void service_uint16_to_str(uint16_t num,uint8_t *ptr_str)
{
 uint8_t temp;
 uint8_t i;
 uint8_t str[]={'0','1','2','3','4','5','6','7','8','9'};
 if(num>9999)
 {
  for(i=0;i<4;i++)
  {
  ptr_str[i]='9';
  }
  ptr_str[i]=0;
  return;
 }
 temp=num/1000;
 *ptr_str++=str[temp];
 num%=1000;
 temp=num/100;
 *ptr_str++=str[temp];
 num%=100;
 temp=num/10;
 *ptr_str++=str[temp];
 num%=10;
 temp=num;
 *ptr_str++=str[temp];
 *ptr_str=0;
}

void service_http_make_request_size_time_to_str(uint16_t size,uint16_t time,uint8_t *ptr_str)
{
  service_uint16_to_str(size,ptr_str);/*写入数据大小字符串3位*/
  ptr_str[4]=',';/*写入','字符串1位*/
  service_uint16_to_str(time,ptr_str+5);/*写入时间大小字符串3位*/
}


/*获取imei值*/
static app_bool_t service_get_imei_str(uint8_t *ptr_imei_str)
{
 uint8_t *ptr_start_addr=ptr_imei_str;
 uint8_t i;
 app_bool_t result=APP_TRUE;
 at_cmd_status_t status;
 
/*AT+GSN*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+GSN",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+GSN 参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+GSN 参数设置失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 for(i=0;i<at_cmd_response.size;i++)
 {
  if(at_cmd_response.response[i]>='0' && at_cmd_response.response[i]<='9')
  {
   *ptr_start_addr++=at_cmd_response.response[i];
  }
 }
 *ptr_start_addr=0;
 APP_LOG_DEBUG("找到IMEI值.已拷贝.IMEI:%s\r\n",ptr_imei_str);
 
err_handle:
 return result;
}


/*获取rssi值*/
app_bool_t service_get_rssi_str(uint8_t *ptr_rssi_str)
{
 uint8_t *ptr_start_addr=ptr_rssi_str;
 uint8_t i;
 app_bool_t result=APP_TRUE;
 at_cmd_status_t status;
 /*只有一个http POST通道所以要避免竞争*/
 take_service_mutex(); 
 
/*第一步查看信号质量AT+CSQ*/
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_exe("+CSQ",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("+CSQ 参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+CSQ 读信号质量失败. status:%d.\r\n",status); 
  result=APP_FALSE;
  goto err_handle;  
 }
 for(i=0;i<at_cmd_response.size;i++)
 {
  if(at_cmd_response.response[i]>='0' && at_cmd_response.response[i]<='9')
  {
   *ptr_start_addr++=at_cmd_response.response[i];
  }
  if(at_cmd_response.response[i]==',')
   break;
 }
  *ptr_start_addr=0;
  APP_LOG_DEBUG("找到RSSI值.已拷贝.RSSI:%s\r\n",ptr_rssi_str);
 
err_handle:
 service_handle_device_err(result);
 /*只有一个http POST通道所以要避免竞争*/
 release_service_mutex(); 
 return result;
}

/*获取IP地址字符串*/
app_bool_t service_get_ip_str(uint8_t *ptr_ip_str)
{
 app_bool_t result=APP_TRUE;
 uint8_t len,*ptr_start,*ptr_end;
 at_cmd_status_t status;
 /*只有一个http POST通道所以要避免竞争*/
 take_service_mutex();  
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_set("+SAPBR","2,1",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("获取IP参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;  
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+获取IP失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 ptr_start=(uint8_t *)strstr((const char *)at_cmd_response.response,"\"");/*IP地址开始标志*/
 if(ptr_start==NULL)
 {
  APP_LOG_ERROR("+回应中无IP字符串.\r\n");
  result=APP_FALSE;
  goto err_handle;   
 }
 
 ptr_end=(uint8_t *)strstr((const char *)(ptr_start+1),"\"");/*IP地址结束标志*/
 if(ptr_end==NULL)
 {
  APP_LOG_ERROR("+回应中无IP字符串. .\r\n");
  result=APP_FALSE;
  goto err_handle;   
 }
len=ptr_end-ptr_start+1;
memcpy(ptr_ip_str,(const void*)ptr_start,len);

ptr_ip_str[len]=0;
APP_LOG_DEBUG("获取ip成功.ip=%s.\r\n",ptr_ip_str);
 
err_handle:
 service_handle_device_err(result);
 /*只有一个http POST通道所以要避免竞争*/
 release_service_mutex(); 
 return result;
}

/*拷贝ip嵌套字符串*/
app_bool_t service_cpy_ip_str_ex(uint8_t *ptr_dest,uint8_t *ptr_src)
{
 if(ptr_dest==NULL||ptr_src==NULL)
   return APP_FALSE;
 
 *ptr_dest++='\\';
 strcpy((char *)ptr_dest,(const char *)ptr_src);
 ptr_dest+=strlen((const char*)ptr_src);
 *(ptr_dest-1)='\\';
 *ptr_dest++='\"';
 *ptr_dest=0;
 return APP_TRUE;
 
  
  
}
/*获取运营商字符串*/
static app_bool_t service_get_operator_str(uint8_t *ptr_operator_str)
{
 app_bool_t result=APP_TRUE;
 uint8_t *ptr_start,*ptr_end;
 at_cmd_status_t status;
 
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 /*数字格式*/
 status=at_ex_cmd_set("+COPS","0,2",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("设置运营商参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;  
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("设置运营商参数失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 
 at_cmd_response.is_response_delay=AT_CMD_FALSE;
 at_cmd_response.response_timeout=AT_CMD_CONFIG_NORMAL_RESPONSE_TIMEOUT;
 status=at_ex_cmd_query("+COPS",&at_cmd_response);
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
   APP_LOG_ERROR("获取运营商参数输入失败 status:%d.\r\n",status);
   result=APP_FALSE;
   goto err_handle;  
 }
 status=at_cmd_find_expect_from_response(&at_cmd_response,"OK");
 if(status!=AT_CMD_STATUS_SUCCESS)
 {
  APP_LOG_ERROR("+获取运营商参数失败. status:%d.\r\n",status);
  result=APP_FALSE;
  goto err_handle;  
 }
 ptr_start=(uint8_t *)strstr((const char *)at_cmd_response.response,"\"");/*运运营商地址开始标志*/
 if(ptr_start==NULL)
 {
  APP_LOG_ERROR("+回应中无运营商字符串.\r\n");
  result=APP_FALSE;
  goto err_handle;   
 }
 
 ptr_end=(uint8_t *)strstr((const char *)(ptr_start+1),"\"");/*运运营商结束标志*/
 if(ptr_end==NULL)
 {
  APP_LOG_ERROR("+回应中无运营商字符串.\r\n");
  result=APP_FALSE;
  goto err_handle;   
 }
 *(ptr_end+1)=0;

/*比较运营商字符*/
if(strcmp(CHINA_MOBILE_2G_STR,(const char *)ptr_start)==0)
{
 strcpy((char *)ptr_operator_str,CHINA_MOBILE_2G_CODE_STR);
}
else if(strcmp(CHINA_UNICOM_2G_STR,(const char *)ptr_start)==0)
{
 strcpy((char *)ptr_operator_str,CHINA_UNICOM_2G_CODE_STR); 
}
else
{
APP_LOG_DEBUG("获取运营商失败.\r\n");
result=APP_FALSE;
goto err_handle; 
}
APP_LOG_DEBUG("获取运营商成功.运营商=%s.\r\n",ptr_operator_str);

err_handle:
 return result;
}
/*拷贝imei*/
app_bool_t service_cpy_imei_str_to(uint8_t *ptr_imei_str)
{
 if(ptr_imei_str==NULL)
   return APP_FALSE;
 strcpy((char *)ptr_imei_str,(const char *)imei_str);
 
 return APP_TRUE;
}

/*拷贝imei 拓展用于嵌套字符串*/
app_bool_t service_cpy_imei_str_to_ex(uint8_t *ptr_imei_str)
{
 if(ptr_imei_str==NULL)
   return APP_FALSE;
 *ptr_imei_str++='\\';
 strcpy((char *)ptr_imei_str,(const char *)imei_str);
 ptr_imei_str+=strlen((const char*)imei_str);
 *(ptr_imei_str-1)='\\';
 *ptr_imei_str++='\"';
 *ptr_imei_str=0;
 return APP_TRUE;
}

/*拷贝运营商代码*/
app_bool_t service_cpy_operator_str_to(uint8_t *ptr_operator_str)
{
 if(ptr_operator_str==NULL)
   return APP_FALSE;
 strcpy((char *)ptr_operator_str,(const char *)operator_str);
 
 return APP_TRUE;
}


app_bool_t service_reset()
{
 app_bool_t result=APP_TRUE;

reset_start: 
 APP_LOG_WARNING("GPRS模块正在复位...\r\n");
 BSP_GPRS_MODULE_PWR_CTL(GPRS_MODULE_PWR_CTL_OFF);
 osDelay(SERVICE_RESET_HOLD_ON_TIME);

 BSP_GPRS_MODULE_PWR_CTL(GPRS_MODULE_PWR_CTL_ON);
 osDelay(SERVICE_RESET_WAIT_TIME);
 
service_init_start:
 result=service_init();
 if(result==APP_FALSE)
 {
  service_device_err_cnt++;
  APP_LOG_ERROR("service_device_err_cnt= %d.\r\n",service_device_err_cnt);
  if(service_device_err_cnt>=SERVICE_DEVICE_ERR_CNT_MAX)
  {
   APP_LOG_ERROR("GPRS模块复位失败.再次尝试复位.\r\n");
   service_device_err_cnt=0; 
   goto reset_start;
  }
  goto service_init_start;
 }
 service_device_err_cnt=0;
 APP_LOG_INFO("GPRS模块复位成功.\r\n");
 
 /*http 参数初始化*/
service_http_init_start: 
 result=service_http_init();
 if(result==APP_FALSE)
 {
  service_device_err_cnt++;
  if(service_device_err_cnt>=SERVICE_DEVICE_ERR_CNT_MAX)
  {
   APP_LOG_ERROR("http初始化失败.需要复位...\r\n");
   service_device_err_cnt=0; 
   goto reset_start;
  }
  goto service_http_init_start;
 }
 APP_LOG_INFO("http初始化成功.\r\n");
 service_device_err_cnt=0;
 
 return result;
}
/*处理GPRS通信错误 如通信失败*/
static app_bool_t service_handle_net_err(app_bool_t result)
{
 if(result==APP_FALSE)
 {
  service_net_err_cnt++;
  APP_LOG_ERROR("service_net_err_cnt= %d.\r\n",service_net_err_cnt);
  
  if(service_net_err_cnt>=SERVICE_NET_ERR_CNT_MAX)
  {
   service_net_err_cnt=0;
   service_reset();/*如果复位不成功 会一直尝试复位 直到成功*/ 
  }
 }
 else
 {
 service_net_err_cnt=0;  
 }
return APP_TRUE;
}
/*处理GPRS设备错误 如死机*/
static app_bool_t service_handle_device_err(app_bool_t result)
{
 if(result==APP_FALSE)
 {
  service_device_err_cnt++;
  APP_LOG_ERROR("service_device_err_cnt= %d.\r\n",service_device_err_cnt);
  
  if(service_device_err_cnt>=SERVICE_DEVICE_ERR_CNT_MAX)
  {
   service_device_err_cnt=0;
   service_reset();/*如果复位不成功 会一直尝试复位 直到成功*/ 
  }
 }
 else
 {
 service_device_err_cnt=0;  
 }
return APP_TRUE;
}
#else




#endif





