#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_ABX081_ZK.h"
#include "eeprom.h"
#include "string.h"
#include "json.h"
#include "service.h"
#include "report_task.h"
#include "lock_ctrl_task.h"

#define APP_LOG_MODULE_NAME   "[eeprom]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"


#define  EEPROM_INFO_HEAD_PAGE_ADDR_START    0
#define  EEPROM_INFO_HEAD_PAGE_CNT           1

#define  EEPROM_OPEN_UUID_PAGE_ADDR_START    1
#define  EEPROM_OPEN_UUID_PAGE_CNT           3

#define  EEPROM_USER_PIN_PAGE_ADDR_START     4
#define  EEPROM_USER_PIN_PAGE_CNT            3

#define  EEPROM_TYPE_PAGE_ADDR_START         7
#define  EEPROM_TYPE_PAGE_CNT                1


#define  EEPROM_DEVICE_ADDR                  0xA0  

#define  EEPROM_PAGE_SIZE                    16
#define  EEPROM_PAGE_CNT                     16                  


extern I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef *ptr_hi2c=&hi2c1;

info_head_t info_head;


void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
APP_LOG_DEBUG("iic写一次完毕.\r\n");  
}
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  APP_LOG_DEBUG("iic读一次完毕.\r\n");
}
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  APP_LOG_DEBUG("iic错误.\r\n");
}
HAL_I2C_StateTypeDef HAL_I2C_GetState(I2C_HandleTypeDef *hi2c);


app_bool_t at24xx_multi_bytes_write(uint8_t device_addr,uint8_t addr,uint8_t *ptr_data,uint16_t cnt)
{
  static HAL_StatusTypeDef status;
  HAL_I2C_StateTypeDef i2c_status;
  uint8_t remain_byte_cnt,head_byte_cnt,page_cnt;
  uint8_t page_remain_byte_cnt;
  uint16_t timeout;
  
  page_remain_byte_cnt=EEPROM_PAGE_SIZE-(addr%EEPROM_PAGE_SIZE);
  
  if(cnt > page_remain_byte_cnt)
  {
  head_byte_cnt=EEPROM_PAGE_SIZE-addr%EEPROM_PAGE_SIZE;
  page_cnt=(cnt-head_byte_cnt)/EEPROM_PAGE_SIZE;
  remain_byte_cnt=(cnt-head_byte_cnt)%EEPROM_PAGE_SIZE;
  }
  else
  {
   head_byte_cnt=cnt;
   page_cnt=0;
   remain_byte_cnt=0;
  }
  
  status=HAL_I2C_Mem_Write_IT(ptr_hi2c,device_addr,addr,I2C_MEMADD_SIZE_8BIT,ptr_data,head_byte_cnt);
  if(status!=HAL_OK)
  {
  APP_LOG_ERROR("at24xx多字节写错误.\r\n");
  return APP_FALSE;
  }
  timeout=0;
  while(1)
  {
  i2c_status=HAL_I2C_GetState(ptr_hi2c);
  if(i2c_status==HAL_I2C_STATE_READY)
  {
  addr+=head_byte_cnt;
  ptr_data+=head_byte_cnt;
  break;
  }
  osDelay(EEPROM_WAIT_INTERVAL);
  timeout+=EEPROM_WAIT_INTERVAL;
  if(timeout>=EEPROM_WAIT_RESPONSE_TIMEOUT)
  {
  APP_LOG_ERROR("at24xx多字节写超时错误.\r\n");
  return APP_FALSE; 
  }
  }
  
  while(page_cnt--)
  {
  status=HAL_I2C_Mem_Write_IT(ptr_hi2c,device_addr,addr,I2C_MEMADD_SIZE_8BIT,ptr_data,EEPROM_PAGE_SIZE);
  if(status!=HAL_OK)
  {
  APP_LOG_ERROR("at24xx多字节写错误.\r\n");
  return APP_FALSE;
  }
  timeout=0;
  while(1)
  {
  i2c_status=HAL_I2C_GetState(ptr_hi2c);
  if(i2c_status==HAL_I2C_STATE_READY)
  {
  addr+=EEPROM_PAGE_SIZE;
  ptr_data+=EEPROM_PAGE_SIZE;
  break;
  }
  osDelay(EEPROM_WAIT_INTERVAL);
  timeout+=EEPROM_WAIT_INTERVAL;
  if(timeout>=EEPROM_WAIT_RESPONSE_TIMEOUT)
  {
  APP_LOG_ERROR("at24xx多字节写超时错误.\r\n");
  return APP_FALSE; 
  }
  }    
  }
  if(remain_byte_cnt)
  {
  status=HAL_I2C_Mem_Write_IT(ptr_hi2c,device_addr,addr,I2C_MEMADD_SIZE_8BIT,ptr_data,remain_byte_cnt);
  timeout=0;
  while(1)
  {
  i2c_status=HAL_I2C_GetState(ptr_hi2c);
  if(i2c_status==HAL_I2C_STATE_READY)
  {
  addr+=remain_byte_cnt;
  ptr_data+=remain_byte_cnt;
  break;
  }
  osDelay(EEPROM_WAIT_INTERVAL);
  timeout+=EEPROM_WAIT_INTERVAL;
  if(timeout>=EEPROM_WAIT_RESPONSE_TIMEOUT)
  {
  APP_LOG_ERROR("at24xx多字节写超时错误.\r\n");
  return APP_FALSE; 
  }
  }
  }
 
  APP_LOG_INFO("at24xx多字节写成功.\r\n");
  return APP_TRUE;
}


app_bool_t at24xx_multi_bytes_read(uint8_t device_addr,uint8_t addr,uint8_t *ptr_data,uint16_t cnt)
{
  HAL_StatusTypeDef status;
  HAL_I2C_StateTypeDef i2c_status;
  uint16_t timeout;

  /*字节写操作*/
  status=HAL_I2C_Mem_Read_IT(ptr_hi2c,device_addr,addr,I2C_MEMADD_SIZE_8BIT,ptr_data,cnt);
  if(status!=HAL_OK)
  {
  APP_LOG_ERROR("at24xx多字节读错误.\r\n");
  return APP_FALSE;
  }
  timeout=0;
  while(1)
  {
  i2c_status=HAL_I2C_GetState(ptr_hi2c);
  if(i2c_status==HAL_I2C_STATE_READY)
  break;
  osDelay(EEPROM_WAIT_INTERVAL);
  timeout+=EEPROM_WAIT_INTERVAL;
  if(timeout>=EEPROM_WAIT_RESPONSE_TIMEOUT)
  {
  APP_LOG_ERROR("at24xx多字节读超时错误.\r\n");
  return APP_FALSE; 
  }
  }
  APP_LOG_INFO("at24xx多字节读成功.\r\n");
  return APP_TRUE;
}



app_bool_t eeprom_read_unreport_close_info_head(info_head_t *ptr_info_head)
{
  app_bool_t result;
  result=at24xx_multi_bytes_read(EEPROM_DEVICE_ADDR,EEPROM_INFO_HEAD_PAGE_ADDR_START*EEPROM_PAGE_SIZE,(uint8_t*)ptr_info_head,sizeof(info_head_t));
  if(result!=APP_TRUE)
  {
   APP_LOG_ERROR("读信息头错误\r\n");
   return APP_FALSE;
  }
  APP_LOG_DEBUG("读信息头成功\r\n");
  return APP_TRUE; 
}

app_bool_t eeprom_write_unreport_close_info_head(info_head_t *ptr_info_head)
{
  app_bool_t result;
  result=at24xx_multi_bytes_write(EEPROM_DEVICE_ADDR,EEPROM_INFO_HEAD_PAGE_ADDR_START*EEPROM_PAGE_SIZE,(uint8_t*)ptr_info_head,sizeof(info_head_t));
  if(result!=APP_TRUE)
  {
   APP_LOG_ERROR("写信息头错误\r\n");
   return APP_FALSE;
  }
  APP_LOG_DEBUG("写信息头成功\r\n");
  return APP_TRUE; 
}

app_bool_t eeprom_is_unreport_close_info_head_valid(info_head_t *ptr_info_head)
{
 if(ptr_info_head==NULL)
 return APP_FALSE;
 
 if(ptr_info_head->valid ==APP_FALSE)
 {
  APP_LOG_DEBUG("在未上报信息无效.\r\n");  
  return APP_FALSE;
 }
  APP_LOG_DEBUG("在未上报信息有效.\r\n");  
  return APP_TRUE;
}


/*读取保存的上报信息*/
app_bool_t eeprom_read_unreport_close_info(info_head_t *ptr_info_head,uint8_t *ptr_open_uuid,uint8_t *ptr_user_pin,uint8_t *ptr_type)
{ 
  app_bool_t result;
  result=at24xx_multi_bytes_read(EEPROM_DEVICE_ADDR,EEPROM_OPEN_UUID_PAGE_ADDR_START*EEPROM_PAGE_SIZE,ptr_open_uuid,ptr_info_head->open_uuid_str_cnt);
  if(result!=APP_TRUE)
  {
   APP_LOG_ERROR("读 open uuid错误.\r\n");
   return APP_FALSE;
  }
  APP_LOG_DEBUG("读的open uuid:%s.\r\n",ptr_open_uuid);
  
  result=at24xx_multi_bytes_read(EEPROM_DEVICE_ADDR,EEPROM_USER_PIN_PAGE_ADDR_START*EEPROM_PAGE_SIZE,ptr_user_pin,ptr_info_head->user_pin_str_cnt);
  if(result!=APP_TRUE)
  {
   APP_LOG_ERROR("读user pin 错误.\r\n");
   return APP_FALSE;
  }
  APP_LOG_DEBUG("读的user pin:%s.\r\n",ptr_user_pin);
  
  result=at24xx_multi_bytes_read(EEPROM_DEVICE_ADDR,EEPROM_TYPE_PAGE_ADDR_START*EEPROM_PAGE_SIZE,ptr_type,ptr_info_head->type_str_cnt);
  if(result!=APP_TRUE)
  {
   APP_LOG_ERROR("读 type 错误.\r\n");
   return APP_FALSE;
  }
  APP_LOG_DEBUG("读的 type:%s.\r\n",ptr_type);
  APP_LOG_DEBUG("读取未保存上报信息成功.\r\n");
  return APP_TRUE;
}
/*保存上报信息*/
app_bool_t eeprom_write_unreport_close_info(info_head_t *ptr_info_head,uint8_t *ptr_open_uuid,uint8_t *ptr_user_pin,uint8_t *ptr_type)  
{
  app_bool_t result;
  
  if(ptr_info_head==NULL ||ptr_open_uuid==NULL ||ptr_user_pin==NULL ||ptr_type==NULL)
  return APP_FALSE;
  
  result=at24xx_multi_bytes_write(EEPROM_DEVICE_ADDR,EEPROM_OPEN_UUID_PAGE_ADDR_START*EEPROM_PAGE_SIZE,ptr_open_uuid,ptr_info_head->open_uuid_str_cnt);
  if(result!=APP_TRUE)
  {
   APP_LOG_ERROR("写 open uuid错误.\r\n");
   return APP_FALSE;
  }
  APP_LOG_DEBUG("写的open uuid:%s.\r\n",ptr_open_uuid);
  
  result=at24xx_multi_bytes_write(EEPROM_DEVICE_ADDR,EEPROM_USER_PIN_PAGE_ADDR_START*EEPROM_PAGE_SIZE,ptr_user_pin,ptr_info_head->user_pin_str_cnt);
  if(result!=APP_TRUE)
  {
   APP_LOG_ERROR("写user pin 错误.\r\n");
   return APP_FALSE;
  }
  APP_LOG_DEBUG("写的user pin:%s.\r\n",ptr_user_pin);
  
  result=at24xx_multi_bytes_write(EEPROM_DEVICE_ADDR,EEPROM_TYPE_PAGE_ADDR_START*EEPROM_PAGE_SIZE,ptr_type,ptr_info_head->type_str_cnt);
  if(result!=APP_TRUE)
  {
   APP_LOG_ERROR("写 type 错误.\r\n");
   return APP_FALSE;
  }
  APP_LOG_DEBUG("写的 type:%s.\r\n",ptr_type);
  APP_LOG_DEBUG("保存上报信息成功.\r\n");
  return APP_TRUE; 
  
  
  
  
  
}