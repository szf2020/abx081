#ifndef  __DEBUG_TASK_H__
#define  __DEBUG_TASK_H__

extern osThreadId debug_task_hdl;
void debug_task(void const * argument);

/*调试任务命令长度定义*/
#define  DEBUG_TASK_CMD_EOL_LEN                         2/*调试时字符串结束符号长度*/

#define  DEBUG_TASK_CMD_SET_ADDR_PARAM_LEN              2
#define  DEBUG_TASK_CMD_REMOVE_TARE_WEIGHT_PARAM_LEN    1
#define  DEBUG_TASK_CMD_ZERO_RANGE_SET_PARAM_LEN        1
#define  DEBUG_TASK_CMD_CLEAR_ZERO_PARAM_LEN            1
#define  DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_PARAM_LEN     1
#define  DEBUG_TASK_CMD_UNLOCK_DEVICE_PARAM_LEN         1
#define  DEBUG_TASK_CMD_LOCK_DEVICE_PARAM_LEN           1
#define  DEBUG_TASK_CMD_SET_MAX_WEIGHT_PARAM_LEN        1
#define  DEBUG_TASK_CMD_SET_DIVISION_PARAM_LEN          1
#define  DEBUG_TASK_CMD_CALIBRATE_WEIGHT_PARAM_LEN      6
#define  DEBUG_TASK_CMD_CALIBRATE_MEASUREMENT_PARAM_LEN 6    
#define  DEBUG_TASK_CMD_CALIBRATE_CODE_PARAM_LEN        2
#define  DEBUG_TASK_CMD_PWR_ON_COMPRESSOR_PARAM_LEN     0
#define  DEBUG_TASK_CMD_PWR_OFF_COMPRESSOR_PARAM_LEN    0
#define  DEBUG_TASK_CMD_PWR_ON_LIGHT_PARAM_LEN          0
#define  DEBUG_TASK_CMD_PWR_OFF_LIGHT_PARAM_LEN         0
#define  DEBUG_TASK_CMD_PWR_ON_12V_PARAM_LEN            0
#define  DEBUG_TASK_CMD_PWR_OFF_12V_PARAM_LEN           0
#define  DEBUG_TASK_CMD_PWR_ON_GLASS_PARAM_LEN          0
#define  DEBUG_TASK_CMD_PWR_OFF_GLASS_PARAM_LEN         0
#define  DEBUG_TASK_CMD_OBTAIN_TEMPERATURE_PARAM_LEN    1
#define  DEBUG_TASK_CMD_UNLOCK_LOCK_PARAM_LEN           0
#define  DEBUG_TASK_CMD_LOCK_LOCK_PARAM_LEN             0
#define  DEBUG_TASK_CMD_PWR_ON_AC1_PARAM_LEN            0
#define  DEBUG_TASK_CMD_PWR_OFF_AC1_PARAM_LEN           0
#define  DEBUG_TASK_CMD_PWR_ON_AC2_PARAM_LEN            0
#define  DEBUG_TASK_CMD_PWR_OFF_AC2_PARAM_LEN           0
#define  DEBUG_TASK_CMD_PWR_ON_LOCK_LED_PARAM_LEN       0
#define  DEBUG_TASK_CMD_PWR_OFF_LOCK_LED_PARAM_LEN      0
#define  DEBUG_TASK_CMD_GET_UPS_STATE_PARAM_LEN         0
#define  DEBUG_TASK_CMD_GET_LOCK_STATE_PARAM_LEN        0
#define  DEBUG_TASK_CMD_GET_DOOR_STATE_PARAM_LEN        0
#define  DEBUG_TASK_CMD_CPU_LOAD_PARAM_LEN              0
#define  DEBUG_TASK_CMD_PWR_ON_FAN_PARAM_LEN            0
#define  DEBUG_TASK_CMD_PWR_OFF_FAN_PARAM_LEN           0

#define  DEBUG_TASK_CMD_EEPROM_READ_PARAM_LEN            0
#define  DEBUG_TASK_CMD_EEPROM_WRITE_PARAM_LEN           0
#define  DEBUG_TASK_CMD_EEPROM_ERASE_PARAM_LEN           0




/*调试任务命令值定义*/
#define  DEBUG_TASK_CMD_SET_ADDR                       "设置地址"    
#define  DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT              "获取净重" 
#define  DEBUG_TASK_CMD_UNLOCK_DEVICE                  "解锁设备"
#define  DEBUG_TASK_CMD_LOCK_DEVICE                    "上锁设备" 
#define  DEBUG_TASK_CMD_REMOVE_TARE_WEIGHT             "去皮"
#define  DEBUG_TASK_CMD_ZERO_RANGE_SET                 "设置清零范围"
#define  DEBUG_TASK_CMD_CLEAR_ZERO                     "清零"
#define  DEBUG_TASK_CMD_SET_MAX_WEIGHT                 "设置最大值" 
#define  DEBUG_TASK_CMD_SET_DIVISION                   "设置分度值"
#define  DEBUG_TASK_CMD_CALIBRATE_WEIGHT               "校准重量"
#define  DEBUG_TASK_CMD_CALIBRATE_MEASUREMENT          "校准测量值"
#define  DEBUG_TASK_CMD_CALIBRATE_CODE                 "校准内码值"
#define  DEBUG_TASK_CMD_PWR_ON_COMPRESSOR              "打开压缩机"
#define  DEBUG_TASK_CMD_PWR_OFF_COMPRESSOR             "关闭压缩机"
#define  DEBUG_TASK_CMD_PWR_ON_LIGHT                   "打开灯带"
#define  DEBUG_TASK_CMD_PWR_OFF_LIGHT                  "关闭灯带"
#define  DEBUG_TASK_CMD_PWR_ON_12V                     "打开12V"
#define  DEBUG_TASK_CMD_PWR_OFF_12V                    "关闭12V"
#define  DEBUG_TASK_CMD_PWR_ON_GLASS                   "加热玻璃"
#define  DEBUG_TASK_CMD_PWR_OFF_GLASS                  "冷却玻璃"
#define  DEBUG_TASK_CMD_OBTAIN_TEMPERATURE             "获取温度"
#define  DEBUG_TASK_CMD_UNLOCK_LOCK                    "开锁"
#define  DEBUG_TASK_CMD_LOCK_LOCK                      "关锁"
#define  DEBUG_TASK_CMD_PWR_ON_AC1                     "打开交流电1"
#define  DEBUG_TASK_CMD_PWR_OFF_AC1                    "关闭交流电1"
#define  DEBUG_TASK_CMD_PWR_ON_AC2                     "打开交流电2"
#define  DEBUG_TASK_CMD_PWR_OFF_AC2                    "关闭交流电2"
#define  DEBUG_TASK_CMD_PWR_ON_LOCK_LED                "打开门灯"
#define  DEBUG_TASK_CMD_PWR_OFF_LOCK_LED               "关闭门灯"
#define  DEBUG_TASK_CMD_GET_UPS_STATE                  "UPS状态"
#define  DEBUG_TASK_CMD_GET_LOCK_STATE                 "锁状态"
#define  DEBUG_TASK_CMD_GET_DOOR_STATE                 "门状态"
#define  DEBUG_TASK_CMD_CPU_LOAD                       "cpu"
#define  DEBUG_TASK_CMD_PWR_ON_FAN                     "打开风扇"
#define  DEBUG_TASK_CMD_PWR_OFF_FAN                    "关闭风扇"

#define  DEBUG_TASK_CMD_EEPROM_READ                      "er"
#define  DEBUG_TASK_CMD_EEPROM_WRITE                     "ew"
#define  DEBUG_TASK_CMD_EEPROM_ERASE                     "ee"

#define  DEBUG_TASK_WAIT_TIMEOUT                       550

/*调试任务超时值定义*/
/*
#define  DEBUG_TASK_CMD_UNLOCK_TIMEOUT                 1000
#define  DEBUG_TASK_CMD_LOCK_TIMEOUT                   1000
#define  DEBUG_TASK_CMD_SET_ADDR_TIMEOUT               1000
#define  DEBUG_TASK_CMD_OBTAIN_NET_WEIGHT_TIMEOUT      1000
#define  DEBUG_TASK_CMD_SET_MAX_WEIGHT_TIMEOUT         1000
#define  DEBUG_TASK_CMD_SET_DIVISION_TIMEOUT           1000
#define  DEBUG_TASK_CMD_CALIBRATE_WEIGHT_TIMEOUT       1000
*/

/*调试任务信号*/

/*
#define  DEBUG_TASK_SET_ADDR_OK_SIGNAL                (1<<0)
#define  DEBUG_TASK_SET_ADDR_ERR_SIGNAL               (1<<1)
#define  DEBUG_TASK_OBTAIN_NET_WEIGHT_OK_SIGNAL       (1<<2)
#define  DEBUG_TASK_OBTAIN_NET_WEIGHT_ERR_SIGNA       (1<<3)
#define  DEBUG_TASK_UNLOCK_DEVICE_OK_SIGNAL           (1<<4)
#define  DEBUG_TASK_UNLOCK_DEVICE_ERR_SIGNAL          (1<<5)
#define  DEBUG_TASK_LOCK_DEVICE_OK_SIGNAL             (1<<6)
#define  DEBUG_TASK_LOCK_DEVICE_ERR_SIGNAL            (1<<7)
#define  DEBUG_TASK_SET_MAX_WEIGHT_OK_SIGNAL          (1<<8)
#define  DEBUG_TASK_SET_MAX_WEIGHT_ERR_SIGNAL         (1<<9)
#define  DEBUG_TASK_SET_DIVISION_OK_SIGNAL            (1<<10)
#define  DEBUG_TASK_SET_DIVISION_ERR_SIGNAL           (1<<11)
#define  DEBUG_TASK_CALIBRATE_WEIGHT_OK_SIGNAL        (1<<12)
#define  DEBUG_TASK_CALIBRATE_WEIGHT_ERR_SIGNAL       (1<<13)
#define  DEBUG_TASK_ALL_SIGNALS                       ((1<<14)-1)

*/


#endif