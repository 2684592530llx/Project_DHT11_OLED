#include "mqtt/bsp_mqtt.h"
#include "esp8266/bsp_esp8266.h"
#include "usart/bsp_usart.h"
#include "dht11/bsp_dht11.h"
#include "light/bsp_adc.h"
#include "gpio/bsp_gpio.h"
#include "light/bsp_light.h"
#include <stdio.h>
#include <string.h>

/* 命令下行：手动控制标志位（复位后默认为 0 = 自动模式） */
volatile uint8_t mqtt_manual_beep = 0;
volatile uint8_t mqtt_beep_state  = 0;
volatile uint8_t mqtt_manual_ledr = 0;
volatile uint8_t mqtt_ledr_state  = 0;

/* 命令避损缓冲区：在等待 AT 应答时收到的 +MQTTSUBRECV 暂存于此 */
static char  mqtt_saved_cmd[UART_RX_BUFF_LEN];
static volatile uint8_t mqtt_cmd_pending = 0;

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

            /* 如果是命令下行消息，先保存起来，不丢弃 */
            if(strstr((char *)ESP8266_RxBuff, "+MQTTSUBRECV") != NULL)
            {
                strncpy(mqtt_saved_cmd, (char *)ESP8266_RxBuff, UART_RX_BUFF_LEN - 1);
                mqtt_saved_cmd[UART_RX_BUFF_LEN - 1] = '\0';
                mqtt_cmd_pending = 1;
                printf(">>> Command saved for later\r\n");
                ESP_RX_Complete = 0;
                continue;
            }

            if(strstr((char *)ESP8266_RxBuff, "OK") != NULL)
            {
                printf(">>> OK found\r\n");
                ESP_RX_Complete = 0;
                return 1;
            }

            if(strstr((char *)ESP8266_RxBuff, "ERROR") != NULL)
            {
                printf(">>> ERROR found\r\n");
                ESP_RX_Complete = 0;
                return 0;
            }

            ESP_RX_Complete = 0;
        }
    }
}

/* 1. AT+MQTTUSERCFG 配置用户参数（EMQX 无需认证，用户名密码留空） */
uint8_t MQTT_SetUserConfig(void)
{
    const char *mqtt_user = "AT+MQTTUSERCFG=0,1,\"STM32_DHT11\",\"\",\"\",0,0,\"\"\r\n";
    printf(">>> 配置 MQTT 用户参数 (EMQX)...\r\n");
    uint8_t ret = SendATAndWaitOK(mqtt_user, 3000);
    if(ret)
        printf(">>> MQTT 用户配置成功\r\n");
    return ret;
}

/* 2. AT+MQTTCONN 连接 EMQX Broker（1883 非加密端口） */
uint8_t MQTT_Connect(void)
{
    const char *mqtt_con = "AT+MQTTCONN=0,\"broker.emqx.io\",1883,0\r\n";
    printf(">>> 连接 EMQX Broker...\r\n");
    uint8_t ret = SendATAndWaitOK(mqtt_con, 8000);
    if(ret)
        printf(">>> EMQX 连接成功\r\n");
    return ret;
}

/* 3. AT+MQTTSUB 订阅命令下行主题 STM32/command */
uint8_t MQTT_CommandSubscribe(void)
{
    const char *mqtt_sub = "AT+MQTTSUB=0,\"STM32/command\",0\r\n";
    printf(">>> 订阅命令主题: STM32/command\r\n");
    return SendATAndWaitOK(mqtt_sub, 3000);
}

/* 使用 AT+MQTTPUBRAW 发送原始 JSON payload（无需转义双引号） */
static uint8_t SendMQTTPUBRAW(const char *topic, const char *payload)
{
    int len = strlen(payload);
    static char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", topic, len);

    printf(">>> AT cmd: %s", cmd);

    if(!ESP8266_Cmd(cmd))
    {
        printf(">>> ESP8266_Cmd send failed\r\n");
        return 0;
    }

    uint32_t tick = HAL_GetTick();
    while(HAL_GetTick() - tick < 5000)
    {
        if(ESP_RX_Complete)
        {
            ESP8266_RxBuff[UART_RX_BUFF_LEN - 1] = '\0';
            printf(">>> ESP response: %s\r\n", (char *)ESP8266_RxBuff);

            if(strchr((char *)ESP8266_RxBuff, '>') != NULL)
            {
                printf(">>> Got '>', sending payload (%d bytes)...\r\n", len);
                ESP_RX_Complete = 0;

                HAL_UART_Transmit(&huart3, (uint8_t *)payload, len, 5000);

                tick = HAL_GetTick();
                while(HAL_GetTick() - tick < 5000)
                {
                    if(ESP_RX_Complete)
                    {
                        ESP8266_RxBuff[UART_RX_BUFF_LEN - 1] = '\0';
                        printf(">>> ESP response: %s\r\n", (char *)ESP8266_RxBuff);

                        if(strstr((char *)ESP8266_RxBuff, "+MQTTSUBRECV") != NULL)
                        {
                            strncpy(mqtt_saved_cmd, (char *)ESP8266_RxBuff, UART_RX_BUFF_LEN - 1);
                            mqtt_saved_cmd[UART_RX_BUFF_LEN - 1] = '\0';
                            mqtt_cmd_pending = 1;
                            printf(">>> Command saved for later\r\n");
                            ESP_RX_Complete = 0;
                            continue;
                        }

                        if(strstr((char *)ESP8266_RxBuff, "OK") != NULL)
                        {
                            printf(">>> OK found\r\n");
                            ESP_RX_Complete = 0;
                            return 1;
                        }
                        if(strstr((char *)ESP8266_RxBuff, "ERROR") != NULL)
                        {
                            printf(">>> ERROR found\r\n");
                            ESP_RX_Complete = 0;
                            return 0;
                        }
                        ESP_RX_Complete = 0;
                    }
                }
                printf(">>> Timeout waiting for OK\r\n");
                return 0;
            }

            ESP_RX_Complete = 0;
        }
    }
    printf(">>> Timeout waiting for '>'\r\n");
    return 0;
}

/* 上传传感器数据到 STM32/data（JSON 格式） */
uint8_t MQTT_Upload_ConstData(float temp, float humi, int light_percent)
{
    static char payload[256];
    sprintf(payload,
        "{\"type\":\"sensor\",\"temp\":%.1f,\"humi\":%.1f,\"light\":%d}",
        temp, humi, light_percent);

    return SendMQTTPUBRAW(MQTT_PUB_TOPIC, payload);
}

/* 上传设备状态到 STM32/data（JSON 格式） */
uint8_t MQTT_Upload_StatusData(int beep_on, int ledr_on)
{
    static char payload[256];
    sprintf(payload,
        "{\"type\":\"status\",\"beep\":%s,\"ledr\":%s}",
        beep_on ? "true" : "false",
        ledr_on ? "true" : "false");

    return SendMQTTPUBRAW(MQTT_PUB_TOPIC, payload);
}

/**
  * @brief  处理 EMQX 命令下行（STM32/command 主题）
  * @note   解析 ESP8266_RxBuff 中的 +MQTTSUBRECV 消息
  *         命令格式: {"beep":true} 或 {"ledr":false} 或同时控制两个
  *         - true  → 进入手动模式，开启设备
  *         - false → 进入手动模式，关闭设备
  *         - MCU 复位 → 恢复自动模式
  * @retval 1=处理了命令, 0=无命令或非命令消息
  */
uint8_t MQTT_ProcessCommand(void)
{
    char recv_buf[UART_RX_BUFF_LEN];
    char *buf = NULL;

    /* 优先检查是否有保存的命令（在 SendATAndWaitOK 中被暂存的） */
    if(mqtt_cmd_pending)
    {
        strncpy(recv_buf, mqtt_saved_cmd, UART_RX_BUFF_LEN - 1);
        recv_buf[UART_RX_BUFF_LEN - 1] = '\0';
        mqtt_cmd_pending = 0;
        buf = recv_buf;
    }
    else if(ESP_RX_Complete)
    {
        /* 先复制一份，立即释放接收缓冲 */
        memcpy(recv_buf, ESP8266_RxBuff, UART_RX_BUFF_LEN);
        recv_buf[UART_RX_BUFF_LEN - 1] = '\0';
        memset(ESP8266_RxBuff, 0, UART_RX_BUFF_LEN);
        ESP_RX_Complete = 0;
        buf = recv_buf;
    }
    else
    {
        return 0;
    }

    /* 检查是否是 MQTT 订阅数据 */
    if(strstr(buf, "+MQTTSUBRECV") == NULL)
        return 0;

    /* 检查是否是 STM32/command 主题 */
    if(strstr(buf, "STM32/command") == NULL)
        return 0;

    printf(">>> 收到 EMQX 命令下行: %s\r\n", buf);

    /* 定位 JSON payload */
    char *json_start = strchr(buf, '{');
    if(json_start == NULL)
    {
        printf(">>> 命令格式错误: 未找到 JSON\r\n");
        return 1;
    }

    /* 解析 beep 控制指令 */
    if(strstr(json_start, "\"beep\"") != NULL || strstr(json_start, "'beep'") != NULL)
    {
        mqtt_manual_beep = 1;
        if(strstr(json_start, "true") != NULL || strstr(json_start, ":1") != NULL)
        {
            mqtt_beep_state = 1;
            HAL_GPIO_WritePin(Beep_PA6_GPIO_Port, Beep_PA6_Pin, GPIO_PIN_SET);
            printf(">>> 命令: 蜂鸣器 -> 开启（手动模式）\r\n");
        }
        else
        {
            mqtt_beep_state = 0;
            HAL_GPIO_WritePin(Beep_PA6_GPIO_Port, Beep_PA6_Pin, GPIO_PIN_RESET);
            printf(">>> 命令: 蜂鸣器 -> 关闭（手动模式）\r\n");
        }
    }

    /* 解析 ledr 控制指令 */
    if(strstr(json_start, "\"ledr\"") != NULL || strstr(json_start, "'ledr'") != NULL)
    {
        mqtt_manual_ledr = 1;
        if(strstr(json_start, "true") != NULL || strstr(json_start, ":1") != NULL)
        {
            mqtt_ledr_state = 1;
            HAL_GPIO_WritePin(Light_LED_R_PB5_GPIO_Port, Light_LED_R_PB5_Pin, GPIO_PIN_RESET);
            printf(">>> 命令: LED -> 开启（手动模式）\r\n");
        }
        else
        {
            mqtt_ledr_state = 0;
            HAL_GPIO_WritePin(Light_LED_R_PB5_GPIO_Port, Light_LED_R_PB5_Pin, GPIO_PIN_SET);
            printf(">>> 命令: LED -> 关闭（手动模式）\r\n");
        }
    }

    return 1;
}
