#ifndef  __JSON_H__
#define  __JSON_H__

typedef enum
{
  JSON_TYPE_STR,
  JSON_TYPE_INT,
  JSON_TYPE_BOOL,
  JSON_TYPE_NEST_STR
}json_item_type_t;

#define  JSON_ITEM_NAME_MAX_SIZE       16
#define  JSON_ITEM_VALUE_MAX_SIZE      40

typedef struct
{
 json_item_type_t type;
 uint8_t name[JSON_ITEM_NAME_MAX_SIZE];
 uint8_t value[JSON_ITEM_VALUE_MAX_SIZE];
}json_item_t;
typedef struct 
{
  uint8_t item_cnt;
}json_header_t; 
typedef struct 
{
 json_header_t header;
 json_item_t pid;
 json_item_t version;
}json_pull_open_instruction_t;


typedef struct 
{
 json_header_t header;
 json_item_t pid;
 json_item_t version;
 json_item_t open;
 json_item_t open_uuid;
 json_item_t error;
}json_report_open_status_t;

typedef struct 
{
 json_header_t header;
 json_item_t pid;
 json_item_t version;
 json_item_t user_pin;
 json_item_t open_uuid;
 json_item_t type;
 json_item_t auto_lock;
}json_report_close_status_t;

typedef struct 
{
 json_header_t header;
 json_item_t pid;
 json_item_t type;
 json_item_t log;
 json_item_t version;
 json_item_t ip;
 json_item_t m_power;
 json_item_t e_power;
 json_item_t lock;
 json_item_t net;
 json_item_t rssi;
 json_item_t push_id;
 json_item_t boot;
 json_item_t temperature;
}json_report_device_status_t;

app_bool_t json_get_item_value_by_name_from_json_str(uint8_t *ptr_json_str,uint8_t *ptr_name,uint8_t *ptr_value);

app_bool_t json_find_cpy_json_str_to(uint8_t *ptr_str_buff,uint8_t *ptr_json_str);

app_bool_t json_set_item_name_value(json_item_t *ptr_item,uint8_t *ptr_name,uint8_t *ptr_value);

app_bool_t json_body_to_str(void *ptr_json_body,uint8_t *ptr_json_str);













#endif