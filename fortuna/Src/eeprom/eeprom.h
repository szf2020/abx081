#ifndef  __EEPROM_H__
#define  __EEPROM_H__

#define  EEPROM_WAIT_INTERVAL                   1
#define  EEPROM_WAIT_RESPONSE_TIMEOUT           10


typedef struct 
{
 app_bool_t valid;
 uint8_t open_uuid_str_cnt;
 uint8_t user_pin_str_cnt;
 uint8_t type_str_cnt;
}info_head_t;



app_bool_t eeprom_read_unreport_close_info_head(info_head_t *ptr_info_head);
app_bool_t eeprom_write_unreport_close_info_head(info_head_t *ptr_info_head);
app_bool_t eeprom_is_unreport_close_info_head_valid(info_head_t *ptr_info_head);


/*读取保存的上报信息*/
app_bool_t eeprom_read_unreport_close_info(info_head_t *ptr_info_head,uint8_t *ptr_open_uuid,uint8_t *ptr_user_pin,uint8_t *ptr_type);
/*保存上报信息*/
app_bool_t eeprom_write_unreport_close_info(info_head_t *ptr_info_head,uint8_t *ptr_open_uuid,uint8_t *ptr_user_pin,uint8_t *ptr_type);


#endif