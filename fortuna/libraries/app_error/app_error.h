/** @file
 *
 * @defgroup app_error Common application error handler
 * @{
 * @ingroup app_common
 *
 * @brief Common application error handler and macros for utilizing a common error handler.
 */

#ifndef APP_ERROR_H__
#define APP_ERROR_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif 

  
/*断言处理*/
__weak void app_assert_handler(uint16_t line,uint8_t *ptr_file_name);

/*错误处理*/
__weak void app_error_handler(uint32_t err_code,uint16_t line,uint8_t *ptr_file_fname);


#define APP_ASSERT(expr)                                                    \
if(expr)                                                                    \
{                                                                           \
}                                                                           \
else                                                                        \
{                                                                           \
 app_assert_handler((uint16_t)__LINE__, (uint8_t *)__FILE__);               \
}                                                                           \

/*错误码处理*/
#define APP_ERROR_HANDLER(ERR_CODE)                                         \
    do                                                                      \
    {                                                                       \
      app_error_handler((ERR_CODE),(uint16_t)__LINE__, (uint8_t *)__FILE__);\
    } while (0)

      
      
/*错误码检查*/
#define APP_ERROR_CHECK(ERR_CODE,EXPECT_CODE)                               \
    do                                                                      \
    {                                                                       \
        if (ERR_CODE != EXPECT_CODE)                                        \
        {                                                                   \
            APP_ERROR_HANDLER(ERR_CODE);                                    \
        }                                                                   \
    } while (0)



#ifdef __cplusplus
}
#endif

#endif // APP_ERROR_H__

/** @} */
