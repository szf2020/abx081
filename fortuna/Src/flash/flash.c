#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "ABDK_ABX081_ZK.h"
#include "flash.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"
#include "string.h"
#include "json.h"
#include "service.h"
#include "report_task.h"
#include "lock_ctrl_task.h"

#define APP_LOG_MODULE_NAME   "[flash]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"

void FLASH_CRITICAL_REGION_ENTER()      
{                               
  if(__get_IPSR()==0)                  
  {                                  
   taskENTER_CRITICAL();                                              
 }  
}

void FLASH_CRITICAL_REGION_EXIT()   
{                                  
  if ( __get_IPSR()==0)                 
  {                                  
   taskEXIT_CRITICAL();              
  }                                  
}


#define  ADDR_FLASH_PAGE_253                0x8007e000  
#define  ADDR_FLASH_PAGE_254                0x8007e800
#define  ADDR_FLASH_PAGE_255                0x8007f000

/*FLASH_PAGE_SIZE*/
#define FLASH_OPEN_UUID_START_ADDR          ADDR_FLASH_PAGE_253   
#define FLASH_USER_PIN_START_ADDR           ADDR_FLASH_PAGE_254  
#define FLASH_TYPE_START_ADDR               ADDR_FLASH_PAGE_255 
volatile uint16_t data16 = 0;

#define  FLASH_OPEN_UUID_STR_LEN            50
#define  FLASH_USER_PIN_STR_LEN             50
#define  FLASH_TYPE_STR_LEN                 10

static uint8_t open_uuid[FLASH_OPEN_UUID_STR_LEN];
static uint8_t user_pin[FLASH_USER_PIN_STR_LEN];
static uint8_t type[FLASH_TYPE_STR_LEN];

static FLASH_EraseInitTypeDef EraseInitStruct;


void FLASH_If_Init(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  /* Unlock the Program memory */
  HAL_FLASH_Lock();
}


/*读取未保存上报信息*/
app_bool_t flash_read_unreport_close_info(uint8_t *ptr_open_uuid,uint8_t *ptr_user_pin,uint8_t *ptr_type)
{   
 app_bool_t result=APP_TRUE;
 uint32_t addr_read;
 uint16_t cnt=0;
 addr_read= FLASH_OPEN_UUID_START_ADDR;
 /*首先判断是否有数据存在*/
 data16 = *(__IO uint16_t *)addr_read;
 if(data16==0xffff)
 {
   APP_LOG_WARNING("flash中没有未上报open_uuid信息.\r\n");
   result=APP_FALSE;
   return result;
 }
 for(cnt=0;cnt<FLASH_OPEN_UUID_STR_LEN;cnt++)
 {
  data16 = *(__IO uint16_t *)addr_read;
  /*已经拷贝完毕*/
  if(data16!=0xffff)
  {
  open_uuid[cnt*2]=LOW_8BITS_OF_16(data16);
  open_uuid[cnt*2+1]=HIGH_8BITS_OF_16(data16);
  addr_read+=2;
  }
  else
  {
  APP_LOG_DEBUG("flash拷贝完毕open uuid信息:%s.\r\n",open_uuid);
  break;
  } 
 }
 
 addr_read= FLASH_USER_PIN_START_ADDR;
 /*首先判断是否有数据存在*/
 data16 = *(__IO uint16_t *)addr_read; 
 if(data16==0xffff)
 {
  APP_LOG_WARNING("flash中没有未上报user_pin信息.\r\n");
  result=APP_FALSE;
  return result;
 }
 
 for(cnt=0;cnt<FLASH_USER_PIN_STR_LEN;cnt++)
 {
  data16 = *(__IO uint16_t *)addr_read;
  /*已经拷贝完毕*/
  if(data16!=0xffff)
  {
  user_pin[cnt*2]=LOW_8BITS_OF_16(data16);
  user_pin[cnt*2+1]=HIGH_8BITS_OF_16(data16);
  addr_read+=2;
  }
  else
  {
   APP_LOG_DEBUG("flash拷贝完毕user pin信息:%s.\r\n",user_pin);
   break;
  } 
 }
 addr_read= FLASH_TYPE_START_ADDR;
 /*首先判断是否有数据存在*/
 data16 = *(__IO uint16_t *)addr_read; 
 if(data16==0xffff)
 {
  APP_LOG_WARNING("flash中没有未上报type信息.\r\n");
  result=APP_FALSE;
  return result;
 }
 
 for(cnt=0;cnt<FLASH_TYPE_STR_LEN;cnt++)
 {
  data16 = *(__IO uint16_t *)addr_read;
  /*已经拷贝完毕*/
  if(data16!=0xffff)
  {
  type[cnt*2]=LOW_8BITS_OF_16(data16);
  type[cnt*2+1]=HIGH_8BITS_OF_16(data16);
  addr_read+=2;
  }
  else
  {
   APP_LOG_DEBUG("flash拷贝完毕type信息:%s.\r\n",type);
   break;
  }
 }
/*拷贝信息*/
strcpy((char *)ptr_open_uuid,(const char *)open_uuid);
strcpy((char *)ptr_user_pin,(const char *)user_pin);
strcpy((char *)ptr_type,(const char *)type);

APP_LOG_WARNING("open uuid和user pin和type信息读取成功.\r\n");
return result;
}

/*擦除未上报信息*/
app_bool_t flash_erase_unreport_close_info()
{
 uint32_t page_err;
 //HAL_FLASH_Unlock();
 FLASH_CRITICAL_REGION_ENTER();


 EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
 EraseInitStruct.PageAddress = FLASH_OPEN_UUID_START_ADDR;
 EraseInitStruct.Banks=FLASH_BANK_1;
 EraseInitStruct.NbPages     = 1;//(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;
 
 if(HAL_FLASHEx_Erase(&EraseInitStruct, &page_err) != HAL_OK)
 {
  APP_LOG_ERROR("OPEN_UUID页擦除错误.page_err:%d\r\n",page_err);
  APP_ERROR_HANDLER(0);
 }
 EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
 EraseInitStruct.PageAddress = FLASH_USER_PIN_START_ADDR;
 EraseInitStruct.Banks=FLASH_BANK_1;
 EraseInitStruct.NbPages     = 1;//(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;
 
 if(HAL_FLASHEx_Erase(&EraseInitStruct, &page_err) != HAL_OK)
 {
  APP_LOG_ERROR("USER_PIN页擦除错误.page_err:%d\r\n",page_err);
  APP_ERROR_HANDLER(0);
 } 
 EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
 EraseInitStruct.PageAddress = FLASH_TYPE_START_ADDR;
 EraseInitStruct.Banks=FLASH_BANK_1;
 EraseInitStruct.NbPages     = 1;//(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;
 
 if(HAL_FLASHEx_Erase(&EraseInitStruct, &page_err) != HAL_OK)
 {
  APP_LOG_ERROR("TYPE页擦除错误.page_err:%d\r\n",page_err);
  APP_ERROR_HANDLER(0);
 } 
 
 //HAL_FLASH_Lock();
 APP_LOG_WARNING("open uuid和user pin和type信息擦除成功.\r\n");
 FLASH_CRITICAL_REGION_EXIT();

 return APP_TRUE;
}


/*保存未上报信息*/
app_bool_t flash_write_unreport_close_info(uint8_t *ptr_open_uuid,uint8_t *ptr_user_pin,uint8_t *ptr_type)
{

uint32_t addr_write;
uint8_t open_uuid_len,user_pin_len,type_len,write_cnt;
app_bool_t result=APP_TRUE;

FLASH_CRITICAL_REGION_ENTER();
//HAL_FLASH_Unlock();
/*写OPEN_UUID_STR*/
open_uuid_len=strlen((const char *)ptr_open_uuid)+1;
addr_write=FLASH_OPEN_UUID_START_ADDR;
write_cnt=open_uuid_len/2;
if(open_uuid_len%2==1)
{
 write_cnt++;
}
while(write_cnt)
{
data16=uint16_decode(ptr_open_uuid);
if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr_write,data16) != HAL_OK)
{
result=APP_FALSE;
APP_LOG_ERROR("OPEN_UUID页写错误.\r\n");
APP_ERROR_HANDLER(0); 
return result;
}
write_cnt--;
ptr_open_uuid+=2;
addr_write+=2;
}

/*写USER_PIN_STR*/
user_pin_len=strlen((const char *)ptr_user_pin)+1;
addr_write=FLASH_USER_PIN_START_ADDR;
write_cnt=user_pin_len/2;
if(user_pin_len%2==1)
{
 write_cnt++;
}
while(write_cnt)
{
data16=uint16_decode(ptr_user_pin);
if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr_write,data16) != HAL_OK)
{
result=APP_FALSE;
APP_LOG_ERROR("USER_PIN页写错误.\r\n");
APP_ERROR_HANDLER(0); 
return result;
}
write_cnt--;
ptr_user_pin+=2;
addr_write+=2;
}
/*写TYPE_STR*/
type_len=strlen((const char *)ptr_type)+1;
addr_write=FLASH_TYPE_START_ADDR;
write_cnt=type_len/2;
if(type_len%2==1)
{
 write_cnt++;
}
while(write_cnt)
{
data16=uint16_decode(ptr_type);
if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr_write,data16) != HAL_OK)
{
result=APP_FALSE;
APP_LOG_ERROR("TYPE_STR页写错误.\r\n");
APP_ERROR_HANDLER(0); 
return result;
}
write_cnt--;
ptr_type+=2;
addr_write+=2;
}

//HAL_FLASH_Lock();
FLASH_CRITICAL_REGION_EXIT();

APP_LOG_WARNING("open uuid/user pin/type 信息保存成功.\r\n");

return result;
}
