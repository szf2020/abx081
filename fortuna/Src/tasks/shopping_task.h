#ifndef  __SHOPPING_TASK_H__
#define  __SHOPPING_TASK_H__
#include "app_common.h"

/*购物流程任务*/
void shopping_task(void const * argument);
/*主机通信任务任务handle*/
extern osThreadId shopping_task_hdl;

#define  SHOPPING_TASK_INTERVAL                           10/*购物任务运行间隔*/
#define  SHOPPING_TASK_RETRY_TIMEOUT                      10
#define  SHOPPING_TASK_DOWNLOAD_TIMEOUT                   2000/*POST数据到模块等待时间*/


#define  SHOPPING_TASK_AUTO_LOCK_LOCK_SUCCESS_SIGNAL      (1<<0)
#define  SHOPPING_TASK_MAN_LOCK_LOCK_SUCCESS_SIGNAL       (1<<1)
#define  SHOPPING_TASK_LOCK_LOCK_FAIL_SIGNAL              (1<<2)
#define  SHOPPING_TASK_UNLOCK_LOCK_SUCCESS_SIGNAL         (1<<3)
#define  SHOPPING_TASK_UNLOCK_LOCK_FAIL_SIGNAL            (1<<4)
#define  SHOPPING_TASK_DOOR_OPEN_SIGNAL                   (1<<5)
#define  SHOPPING_TASK_ALL_SIGNALS                        ((1<<6)-1)

/*同步事件*/
#define  SHOPPING_TASK_SYNC_EVT                           (1<<1)

#define  PULL_OPEN_URL                                    "\"URL\",\"http://rack-brain-app-pre.jd.com/brain/pullOpenInstruction\""
#define  REPORT_OPEN_URL                                  "\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportLockOpenStatus\""
#define  REPORT_CLOSE_URL                                 "\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportSaleSkuInfo\""
#endif