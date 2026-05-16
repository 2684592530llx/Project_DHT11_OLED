#include "mqtt/bsp_mqtt.h"
#include "esp8266/bsp_esp8266.h"
#include "usart/bsp_usart.h"
#include "dht11/bsp_dht11.h"      
#include "light/bsp_adc.h"         
#include "gpio/bsp_gpio.h"
#include "light/bsp_light.h"

/* 通用函数：发送 AT 指令并等待 "OK" 响应 */
static uint8_t SendATAndWaitOK(const char *at_cmd, uint32_t timeout_ms)
{
    printf(">>> AT cmd: %s\r\n", at_cmd);

    if(!ESP8266_Cmd(at_cmd))
    {
        printf(">>> ESP8266_Cmd send failed\r\n");
        return 0;
    }

    printf(">>> AT sent, waiting response...\r\n");

    uint32_t tick_start = HAL_GetTick();
    
    while(1)
    {
        if(HAL_GetTick() - tick_start > timeout_ms)
        {
            printf(">>> Timeout!\r\n");
            return 0;
        }
        
        if(ESP_RX_Complete)
        {
            ESP8266_RxBuff[UART_RX_BUFF_LEN - 1] = '\0';
            printf(">>> ESP response: %s\r\n", (char *)ESP8266_RxBuff);
            
            if(strstr((char *)ESP8266_RxBuff, "OK") != NULL)
            {
                printf(">>> OK found\r\n");
                return 1;
            }
            
            if(strstr((char *)ESP8266_RxBuff, "ERROR") != NULL)
            {
                printf(">>> ERROR found\r\n");
                return 0;
            }
            
            ESP_RX_Complete = 0;
        }
    }
}

/* 1. AT+MQTTUSERCFG 配置用户参数 */
uint8_t MQTT_SetUserConfig(void)
{
    const char *mqtt_user = "AT+MQTTUSERCFG=0,1,\"DHT11\",\"ia7oX3UrRq\",\"version=2018-10-31&res=products%2Fia7oX3UrRq%2Fdevices%2FDHT11&et=1809748855&method=sha1&sign=k2KkpjREaoeoSFtBVNyJXUSVX6A%3D\",0,0,\"\"\r\n";
    return SendATAndWaitOK(mqtt_user, 3000);
}

/* 2. AT+MQTTCONN 连接 MQTT 服务器 */
uint8_t MQTT_Connect(void)
{
    const char *mqtt_con = "AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n";
    return SendATAndWaitOK(mqtt_con, 5000);
}

/* 3. AT+MQTTSUB 订阅回复主题 */
uint8_t MQTT_ReplySubscribe(void)
{
    const char *mqtt_reply = "AT+MQTTSUB=0,\"$sys/ia7oX3UrRq/DHT11/thing/property/post/reply\",0\r\n";
    return SendATAndWaitOK(mqtt_reply, 3000);
}

/* 4. AT+MQTTSUB 订阅设置主题 */
uint8_t MQTT_SetSubscribe(void)
{
    const char *mqtt_reply = "AT+MQTTSUB=0,\"$sys/ia7oX3UrRq/DHT11/thing/property/set\",0\r\n";
    return SendATAndWaitOK(mqtt_reply, 3000);
}

/* 上传传感器数据 */
uint8_t MQTT_Upload_ConstData(float temp , float humi , int light_percent)
{
    static char payload[256];
    sprintf(payload,
        "{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{"
        "\\\"Light\\\":{\\\"value\\\":%d}\\,"
        "\\\"Temp\\\":{\\\"value\\\":%.1f}\\,"
        "\\\"Humi\\\":{\\\"value\\\":%.1f}}}",
        light_percent, temp, humi);
	
    static char cmd[600];
    sprintf(cmd, "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n", MQTT_PUB_TOPIC, payload);

    return SendATAndWaitOK(cmd, 5000);
}

/* 上传设备状态 */
uint8_t MQTT_Upload_StatusData(int beep_on , int ledr_on)
{
    static char payload[256];
    sprintf(payload,
        "{\\\"id\\\":\\\"456\\\"\\,\\\"params\\\":{"
        "\\\"Beep\\\":{\\\"value\\\":%s}\\,"
        "\\\"LEDR\\\":{\\\"value\\\":%s}}}",
        beep_on ? "true" : "false",
        ledr_on ? "true" : "false");
	
    static char cmd[600];
    sprintf(cmd, "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n", MQTT_PUB_TOPIC, payload);

    return SendATAndWaitOK(cmd, 5000);
}
