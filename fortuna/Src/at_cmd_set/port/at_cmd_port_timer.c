#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_common.h"
#include "at_cmd_set.h"
#include "at_cmd_port_timer.h"
#define APP_LOG_MODULE_NAME   "[at_cmd_timer]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_ERROR    
#include "app_log.h"
#include "app_error.h"


static void at_cmd_port_timer_expired(void const * argument);

osTimerId at_cmd_timer_id;

void at_cmd_port_timer_init()
{
 if(at_cmd_timer_id==NULL)
 {
 osTimerDef(at_cmd_timer,at_cmd_port_timer_expired);
 at_cmd_timer_id=osTimerCreate(osTimer(at_cmd_timer),osTimerOnce,0);
 APP_ASSERT(at_cmd_timer_id);
 }
}

void at_cmd_port_timer_35_start()
{
 APP_LOG_DEBUG("timer35 start.\r\n");
 osTimerStart(at_cmd_timer_id,MB_POLL_PORT_TIMER_35_TIMEOUT);
}

void at_cmd_port_timer_stop()
{
 osTimerStop(at_cmd_timer_id);
}

static void at_cmd_port_timer_expired(void const * argument)
{
 at_cmd_timer_35_expired();
}
