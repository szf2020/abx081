
/** @file
 *
 * @defgroup app_error Common application error handler
 * @{
 * @ingroup app_common
 *
 * @brief Common application error handler.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"
#include "stm32f1xx.h"
#include "app_util.h"
#define APP_LOG_MODULE_NAME "[app_error]"
#include "app_log.h"
#include "app_error.h"

/*断言处理*/
__weak void app_assert_handler(uint16_t line,uint8_t *ptr_file_name)
{
#if APP_LOG_ENABLED == 0
    UNUSED_VARIABLE(line);
    UNUSED_VARIABLE(ptr_file_name);  
    NVIC_SystemReset();
#else
   APP_LOG_ERROR("#############系统断言错误! ##############\r\n");
   APP_LOG_ERROR("断言文件：%s.\r\n",ptr_file_name);
   APP_LOG_ERROR("断言行号：%u.\r\n",line);
   while(1);
#endif

}

/*错误处理*/
__weak void app_error_handler(uint32_t err_code,uint16_t line,uint8_t *ptr_file_name)
{

#if APP_LOG_ENABLED == 0
    UNUSED_VARIABLE(err_code);
    UNUSED_VARIABLE(line);
    UNUSED_VARIABLE(ptr_file_name);
    NVIC_SystemReset();
#else
   APP_LOG_ERROR("#############系统错误! ##############\r\n");
   APP_LOG_ERROR("错误文件：%s.\r\n",ptr_file_name);
   APP_LOG_ERROR("错误行号：%u.\r\n",line);
   APP_LOG_ERROR("错误码：%u.\r\n",err_code);
   while(1);
#endif 
}



