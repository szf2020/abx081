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


#define  EEPROM_OPEN_UUID_STR_LEN            50
#define  EEPROM_USER_PIN_STR_LEN             50
#define  EEPROM_TYPE_STR_LEN                 10

static uint8_t open_uuid[EEPROM_OPEN_UUID_STR_LEN];
static uint8_t user_pin[EEPROM_USER_PIN_STR_LEN];
static uint8_t type[EEPROM_TYPE_STR_LEN];



/*读取未保存上报信息*/
app_bool_t flash_read_unreport_close_info(uint8_t *ptr_open_uuid,uint8_t *ptr_user_pin,uint8_t *ptr_type)
{
  
  
  
  
  
  
  
  
  
  
  
  