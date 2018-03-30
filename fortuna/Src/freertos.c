/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */     
#include "app_common.h"
#include "shopping_task.h"
#include "report_task.h"
#include "watch_dog_task.h"
#include "lock_ctrl_task.h"
#include "lock_switch_task.h"
#include "lock_status_task.h"
#include "door_status_task.h"
#include "display_task.h"
#include "temperature_cache_task.h"
#include "debug_task.h"
#include "light_ctrl_task.h"
#include "fan_ctrl_task.h"
#include "sys_led_task.h"
#include "glass_pwr_task.h"
#include "temperature_task.h"
#include "compressor_task.h"
#include "ups_status_task.h"
/*
#include "temperature_memory_task.h"
*/
/*
#include "debug_task.h"
*/
#define APP_LOG_MODULE_NAME   "[freertos]"
#define APP_LOG_MODULE_LEVEL   APP_LOG_LEVEL_DEBUG    
#include "app_log.h"
#include "app_error.h"
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;

/* USER CODE BEGIN Variables */
static void create_user_tasks();
/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationIdleHook(void);
void vApplicationTickHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 2 */
__weak void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/* USER CODE END 2 */

/* USER CODE BEGIN 3 */
__weak void vApplicationTickHook( void )
{
   /* This function will be called by each tick interrupt if
   configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
   added here, but the tick hook is called from an interrupt context, so
   code must not attempt to block, and only the interrupt safe FreeRTOS API
   functions can be used (those that end in FromISR()). */
}
/* USER CODE END 3 */

/* USER CODE BEGIN 4 */
__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
  APP_LOG_ERROR("%s任务栈溢�?.\r\n",pcTaskName);
  APP_ERROR_HANDLER(0);
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
__weak void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
  APP_LOG_ERROR("系统内存分配失败.\r\n");
  APP_ERROR_HANDLER(0);
}
/* USER CODE END 5 */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  /*创建用户任务*/
  APP_LOG_DEBUG("创建用户任务...\r\n");
  
  create_user_tasks();

  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */
static void create_user_tasks()
{
  task_sync_evt_group_hdl=xEventGroupCreate();
  APP_ASSERT(task_sync_evt_group_hdl);
  
  osThreadDef(shopping_task, shopping_task, osPriorityNormal, 0, 256);
  shopping_task_hdl = osThreadCreate(osThread(shopping_task), NULL); 
  APP_ASSERT(shopping_task_hdl);
  
  osThreadDef(report_task, report_task, osPriorityNormal, 0, 256);
  report_task_hdl = osThreadCreate(osThread(report_task), NULL); 
  APP_ASSERT(report_task_hdl);
  
  osThreadDef(watch_dog_task, watch_dog_task, osPriorityNormal, 0, 128);
  watch_dog_task_hdl = osThreadCreate(osThread(watch_dog_task), NULL); 
  APP_ASSERT(watch_dog_task_hdl);
  
  osThreadDef(compressor_task, compressor_task, osPriorityNormal, 0, 128);
  compressor_task_hdl = osThreadCreate(osThread(compressor_task), NULL); 
  APP_ASSERT(compressor_task_hdl);
  
  osThreadDef(sys_led_task, sys_led_task, osPriorityNormal, 0, 128);
  sys_led_task_hdl = osThreadCreate(osThread(sys_led_task), NULL); 
  APP_ASSERT(sys_led_task_hdl);
  
  osThreadDef(lock_ctrl_task, lock_ctrl_task, osPriorityNormal, 0, 256);
  lock_ctrl_task_hdl = osThreadCreate(osThread(lock_ctrl_task), NULL); 
  APP_ASSERT(lock_ctrl_task_hdl);
  
  osThreadDef(lock_status_task, lock_status_task, osPriorityNormal, 0, 128);
  lock_status_task_hdl = osThreadCreate(osThread(lock_status_task), NULL); 
  APP_ASSERT(lock_status_task_hdl);
  
  osThreadDef(lock_switch_task, lock_switch_task, osPriorityNormal, 0, 128);
  lock_switch_task_hdl = osThreadCreate(osThread(lock_switch_task), NULL); 
  APP_ASSERT(lock_switch_task_hdl);
  
  osThreadDef(door_status_task, door_status_task, osPriorityNormal, 0, 128);
  door_status_task_hdl = osThreadCreate(osThread(door_status_task), NULL); 
  APP_ASSERT(door_status_task_hdl);
  
  osThreadDef(light_ctrl_task, light_ctrl_task, osPriorityNormal, 0, 128);
  light_ctrl_task_hdl = osThreadCreate(osThread(light_ctrl_task), NULL); 
  APP_ASSERT(light_ctrl_task_hdl);
  
  osThreadDef(fan_ctrl_task, fan_ctrl_task, osPriorityNormal, 0, 128);
  fan_ctrl_task_hdl = osThreadCreate(osThread(fan_ctrl_task), NULL); 
  APP_ASSERT(fan_ctrl_task_hdl);
  
  osThreadDef(glass_pwr_task, glass_pwr_task, osPriorityNormal, 0, 128);
  glass_pwr_task_hdl = osThreadCreate(osThread(glass_pwr_task), NULL); 
  APP_ASSERT(glass_pwr_task_hdl);
  
  osThreadDef(ups_status_task, ups_status_task, osPriorityNormal, 0, 128);
  ups_status_task_hdl = osThreadCreate(osThread(ups_status_task), NULL); 
  APP_ASSERT(ups_status_task_hdl);
  
  osThreadDef(temperature_task, temperature_task, osPriorityNormal, 0, 128);
  temperature_task_hdl = osThreadCreate(osThread(temperature_task), NULL); 
  APP_ASSERT(temperature_task_hdl);
  
  osThreadDef(temperature_cache_task, temperature_cache_task, osPriorityNormal, 0, 128);
  temperature_cache_task_hdl = osThreadCreate(osThread(temperature_cache_task), NULL); 
  APP_ASSERT(temperature_cache_task_hdl);
  
  osThreadDef(display_task, display_task, osPriorityNormal, 0, 128);
  display_task_hdl = osThreadCreate(osThread(display_task), NULL); 
  APP_ASSERT(display_task_hdl);
  
  osThreadDef(debug_task, debug_task, osPriorityAboveNormal, 0, 256);
  debug_task_hdl = osThreadCreate(osThread(debug_task), NULL); 
  APP_ASSERT(debug_task_hdl);
  
  APP_LOG_INFO("@�?有的任务创建成功.\r\n"); 
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
