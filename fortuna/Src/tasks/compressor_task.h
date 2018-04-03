#ifndef  __COMPRESSOR_TASK_H__
#define  __COMPRESSOR_TASK_H__

/*压缩机任务*/
void compressor_task(void const * argument);
extern osThreadId compressor_task_hdl;


#define  COMPRESSOR_TASK_INTERVAL                    osWaitForever/*压缩机任务间隔*/

/*定义压缩机的温度工作区间 10倍温度值*/
#define  COMPRESSOR_TASK_TEMPERATURE_MIN              (45)
#define  COMPRESSOR_TASK_TEMPERATURE_MAX              (75)

/*压缩机连续工作的最大时间长 最长连续时间1小时30分钟*/
#define  COMPRESSOR_TASK_WORK_TIMEOUT                 (90*60*1000UL)
/*压缩机停止工作最小的时间长 最低需要暂停10分钟*/
#define  COMPRESSOR_TASK_REST_TIMEOUT                 (10*60*1000UL)

/*压缩机任务的信号*/
#define  COMPRESSOR_TASK_PWR_TURN_ON_SIGNAL           (1<<0)
#define  COMPRESSOR_TASK_PWR_TURN_OFF_SIGNAL          (1<<1)
#define  COMPRESSOR_TASK_DEBUG_PWR_TURN_ON_SIGNAL     (1<<2)
#define  COMPRESSOR_TASK_DEBUG_PWR_TURN_OFF_SIGNAL    (1<<3)
#define  COMPRESSOR_TASK_TEMPERATURE_CHANGED_SIGNAL   (1<<4)
#define  COMPRESSOR_TASK_ALL_SIGNALS                  ((1<<5)-1)

#endif