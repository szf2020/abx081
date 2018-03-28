#ifndef  __LOCK_SWITCH_TASK_H__
#define  __LOCK_SWITCH_TASK_H__

#define  LOCK_SWITCH_TASK_INTERVAL                      10

#define  LOCK_SWITCH_TASK_LOCK_SWITCH_STATUS_PRESS      1
#define  LOCK_SWITCH_TASK_LOCK_SWITCH_STATUS_RELEASE    2
#define  LOCK_SWITCH_TASK_LOCK_SWITCH_STATUS_INIT       3


/*锁按键任务*/
void lock_switch_task(void const * argument);
extern osThreadId lock_switch_task_hdl;



#endif