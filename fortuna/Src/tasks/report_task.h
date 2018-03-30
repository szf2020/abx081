#ifndef  __DEVICE_TASK_H__
#define  __DEVICE_TASK_H__

#define  REPORT_TASK_RETRY_MAX                    

#define  REPORT_TASK_INTERVAL                     (1000*10)/*10秒*/             

#define  REPORT_TASK_RETRY_TIMEOUT                10/*重试的等待时间*/
#define  REPORT_TASK_DOWNLOAD_TIMEOUT             2000/*POST数据到模块等待时间*/



extern osThreadId report_task_hdl;
/*设备上报任务*/
void   report_task(void const * argument);

extern EventGroupHandle_t task_sync_evt_group_hdl;

#define  REPORT_TASK_SYNC_EVT                      (1<<0)

#define  REPORT_DEVICE_URL                          "\"URL\",\"http://rack-brain-app-pre.jd.com/brain/reportDeviceStatus\""


#endif