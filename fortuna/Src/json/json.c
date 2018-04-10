#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "string.h"
#include "app_common.h"
#include "json.h"

app_bool_t json_get_item_value_by_name_from_json_str(uint8_t *ptr_json_str,uint8_t *ptr_name,uint8_t *ptr_value)
{
  uint8_t *ptr_addr;
  if(ptr_json_str==NULL)
  return APP_FALSE; 
  ptr_addr=(uint8_t *)strstr((const char *)ptr_json_str,(const char *)ptr_name);
  if(ptr_addr==NULL)
  return APP_FALSE;
  ptr_addr=(uint8_t *)strstr((const char *)ptr_addr,":");/*查找“：”*/
  if(ptr_addr==NULL)
  return APP_FALSE;
  ptr_addr++;
  while(*ptr_addr!=',' && *ptr_addr!='}' && *ptr_addr !=0)
  {
  *ptr_value++=*ptr_addr++; 
  }
  *ptr_value=0;/*设置成字符串*/
  return APP_TRUE;
}

app_bool_t json_find_cpy_json_str_to(uint8_t *ptr_str_buff,uint8_t *ptr_json_str)
{
  uint8_t *ptr_addr;
  ptr_addr=(uint8_t *)strstr((const char *)ptr_str_buff,"{");
  if(ptr_addr)
  {
   strcpy((char *)ptr_json_str,(const char *)ptr_addr);
  return APP_TRUE;
  }
  return APP_FALSE;
}



app_bool_t json_set_item_name_value(json_item_t *ptr_item,uint8_t *ptr_name,uint8_t *ptr_value)
{
 if(ptr_item==NULL )
 {
  return APP_FALSE; 
 }
 if(ptr_name)
 strcpy((char *)ptr_item->name,(const char *)ptr_name);
 if(ptr_value)
 strcpy((char *)ptr_item->value,(const char *)ptr_value);
 
 return APP_TRUE; 
}


app_bool_t json_body_to_str(void *ptr_json_body,uint8_t *ptr_json_str)
{
 uint8_t len=0;
 uint8_t i,cnt;
 json_item_t *ptr_item;
 if(ptr_json_body==NULL)
 return APP_FALSE;
 cnt=((json_header_t*)ptr_json_body)->item_cnt;
 
 ptr_item=(json_item_t*)((uint8_t *)ptr_json_body+sizeof(json_header_t));
 /*第一个是{*/
 *ptr_json_str++='{';
 
  for(i=0;i<cnt;i++)
  {
  len=strlen((const char *)ptr_item->name);
  memcpy(ptr_json_str,ptr_item->name,len);
  ptr_json_str+=len;
  *ptr_json_str++=':';
  len=strlen((const char *)ptr_item->value);
  memcpy(ptr_json_str,ptr_item->value,len);
  ptr_json_str+=len;
  *ptr_json_str++=',';
  ptr_item++;
  }
 /*最后一个}*/
 *(ptr_json_str-1)='}';
 *ptr_json_str=0;
 return APP_TRUE; 
}

/*针对长字符串嵌套对象*/
app_bool_t json_body_to_str_ex(void *ptr_json_body,uint8_t *ptr_json_str)
{
 uint8_t len=0;
 uint8_t i,cnt,j,cnt_nest;
 json_item_t *ptr_item;
 if(ptr_json_body==NULL)
 return APP_FALSE;
 cnt=((json_header_t*)ptr_json_body)->item_cnt;
 
 ptr_item=(json_item_t*)((uint8_t *)ptr_json_body+sizeof(json_header_t));
 /*第一个是{*/
 *ptr_json_str++='{';
 
  for(i=0;i<cnt;i++)
  {
  if(ptr_item->type!=JSON_TYPE_NEST_STR)
  {
  len=strlen((const char *)ptr_item->name);
  memcpy(ptr_json_str,ptr_item->name,len);
  ptr_json_str+=len;
  *ptr_json_str++=':';
  len=strlen((const char *)ptr_item->value);
  memcpy(ptr_json_str,ptr_item->value,len);
  ptr_json_str+=len;
  *ptr_json_str++=',';
  ptr_item++;
  }
  else
  {
  len=strlen((const char *)ptr_item->name);
  memcpy(ptr_json_str,ptr_item->name,len);
  ptr_json_str+=len;
  *ptr_json_str++=':';  
  
  *ptr_json_str++='\"';
  *ptr_json_str++='{';
  
  cnt_nest=ptr_item->value[0];/*保存有嵌套的数量*/  
  ptr_item++;/*嵌套处理开始*/
  for(j=0;j<cnt_nest;j++)
  {
  len=strlen((const char *)ptr_item->name);
  memcpy(ptr_json_str,ptr_item->name,len);
  ptr_json_str+=len;
  *ptr_json_str++=':';
  len=strlen((const char *)ptr_item->value);
  memcpy(ptr_json_str,ptr_item->value,len);
  ptr_json_str+=len;
  *ptr_json_str++=',';
  ptr_item++; 
  }
 /*结束嵌套处理*/
 *(ptr_json_str-1)='}';
 *ptr_json_str++='\"';
 *ptr_json_str++=',';
  }
  }
 /*最后一个}*/
 *(ptr_json_str-1)='}';
 *ptr_json_str=0;
 return APP_TRUE; 
}
