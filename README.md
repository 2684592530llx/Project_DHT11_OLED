# 基于 STM32 的环境监测报警系统

<img width="1440" height="1920" alt="image" src="https://github.com/user-attachments/assets/8de066c3-1b00-4338-a8e2-425d4ed50393" />


## 项目简介

本项目基于 **STM32F103C8T6** 微控制器，集成了 **DHT11** 温湿度传感器、**光敏电阻**（或光敏传感器模块）以及 **有源蜂鸣器**，构成一个低成本的环境监测与报警系统。系统实时采集环境温度、湿度和光照强度，并通过 **0.96 寸 OLED 显示屏**（SSD1306，I2C 接口）进行本地显示。当温度或光照强度超过预设阈值时，蜂鸣器自动发出警报，OLED 屏幕上给出醒目的警告提示。

项目中利用 **DWT（Data Watchpoint and Trace）** 实现了微秒级精确延时，保证了 DHT11 单总线通信的时序可靠性。同时支持串口打印调试信息，便于性能分析与问题排查。

## 硬件清单

| 模块 | 型号/规格 | 连接引脚 |
|------|-----------|----------|
| 主控芯片 | STM32F103C8T6 | - |
| 温湿度传感器 | DHT11 | 数据线 → PB12 |
| 光敏电阻模块（模拟输出） | 光敏传感器模块 | AO → PA0（ADC1_IN0） |
| 有源蜂鸣器 | 3.3V 有源蜂鸣器模块 | 信号线 → PA6 |
| OLED 显示屏 | 0.96 寸，SSD1306，I2C | SCL → PB6，SDA → PB7 |
| 调试/烧录 | DAP-Link / ST-Link | SWD 接口 |
| USB 转 TTL | - | PA9(TX), PA10(RX) |

## 功能说明

- 每 2 秒读取一次 DHT11 温湿度数据，同时读取光敏电阻的 ADC 值，换算为相对光照强度（0~100%）。
- OLED 屏幕实时显示：
  - 温度（单位：℃）
  - 湿度（单位：%RH）
  - 光照强度（单位：% 或 ADC 原始值）
- 当温度超过 **温度阈值**（例如 30℃）或光照强度超过 **光照阈值**（例如 80%）时：
  - 蜂鸣器持续发出报警声。
  - OLED 屏幕显示“TEMP HIGH”或“LIGHT HIGH”警告信息。
- 当所有参数恢复正常后，蜂鸣器停止报警，OLED 恢复常规显示。
- 数据校验机制：DHT11 校验失败时保留上一次显示值，并通过串口输出错误提示。

## 代码结构

```
Project/
├── Core/                    # 核心文件（启动文件、main.c 等）
├── User/
│   ├── APP/                 # 应用层
│   │   └── dht11/
│   │       ├── app_dht11.c
│   │       └── app_dht11.h
│   ├── Fonts/               # 字模文件
│   │   └── fonts.c          # 存放 ASCII 字库和汉字字模数组
│   ├── led/                 # LED 控制模块（bsp_led.c/h）
│   ├── dwt/                 # DWT 微秒延时模块（bsp_dwt.c/h）
│   ├── usart/               # 串口通信模块（bsp_usart.c/h）
│   ├── adc/                 # ADC 驱动（读取光敏电阻）(bsp_adc.c/h)
│   ├── beep/                # 蜂鸣器控制模块（bsp_beep.c/h）
│   ├── dht11/               # DHT11 底层驱动（bsp_dht11.c/h）
│   └── oled/                # OLED 底层驱动（bsp_oled.c/h）
├── Libraries/               # HAL 库和 CMSIS
└── Doc/                     # 项目文档（数据手册等）
```

## 关键算法与实现细节

### 1. DHT11 读取时序

- 主机拉低总线 20ms → 拉高 30μs → 切换为输入。
- 等待 DHT11 响应（低电平 80μs → 高电平 80μs）。
- 每 bit 以 50μs 低电平起始，主机延时 40μs 后检测电平：高则为 1，低则为 0。
- 一次通信接收 40 位数据（5 字节），校验通过后更新温湿度。

### 2. 微秒级延时（DWT）

利用 Cortex-M3 内核的 DWT 计数器 `CYCCNT`，精确延时函数 `DWT_DelayUs()`，避免了使用 HAL_Delay 的毫秒级误差。

```c
void DWT_DelayUs(uint32_t us) {
    uint32_t duration = (uint32_t)((uint64_t)us * SystemCoreClock / 1000000ULL);
    uint32_t start = DWT_GetTick();
    while ((DWT_GetTick() - start) < duration);
}
```

### 3. 光敏电阻 ADC 采样与滤波

- 使用 STM32 的 ADC1 对 PA0（通道0）进行单次转换，采样时间设为最大（239.5 周期）。
- 为提高稳定性，连续采样 10 次取算术平均值。
- 根据 ADC 值计算相对光照强度：`light_percent = (float)adc_value / 4095.0f * 100.0f`。

```c
uint16_t Get_ADC_Average(uint8_t times) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
            sum += HAL_ADC_GetValue(&hadc1);
        }
    }
    return (uint16_t)(sum / times);
}
```

### 4. 报警逻辑（状态机）

- 定义温度阈值 `TEMP_ALARM` 和光照阈值 `LIGHT_ALARM`。
- 主循环中读取温湿度和光照，若任一参数超限则：
  - 蜂鸣器引脚置高（有源蜂鸣器鸣叫）。
  - OLED 清屏并在指定行显示报警信息。
  - 报警标志位 `alarm_active` 置 1。
- 当所有参数恢复正常且 `alarm_active` 为 1 时：
  - 蜂鸣器关、报警标志清零、恢复常规显示界面。
- 报警期间不更新常规显示，避免屏幕闪烁。

```c
if (temp > TEMP_ALARM || light_percent > LIGHT_ALARM) {
    if (!alarm_active) {
        alarm_active = 1;
        BEEP_ON();
        OLED_Clear();
        OLED_ShowString_F8X16(1, 2, (uint8_t*)"*** ALARM ***");
        if (temp > TEMP_ALARM)
            OLED_ShowString_F8X16(3, 2, (uint8_t*)"TEMP HIGH!");
        if (light_percent > LIGHT_ALARM)
            OLED_ShowString_F8X16(4, 2, (uint8_t*)"LIGHT HIGH!");
    }
} else {
    if (alarm_active) {
        alarm_active = 0;
        BEEP_OFF();
        OLED_Clear();
        // 后续正常刷新显示温湿度和光照
    }
    // 常规显示...
}
```

### 5. OLED 显示

- I2C 通信，发送命令或数据前需附带控制字节（`0x00` 表示命令，`0x40` 表示数据）。
- 支持 ASCII 字符（8x16 字体）和 16x16 汉字点阵，字模数据统一保存在 `Fonts/fonts.c` 中。
- 显示位置通过页地址（0~7）和列地址（0~127）指定。

## 使用说明

1. **编译与烧录**：使用 Keil MDK 打开工程，选择正确的芯片型号（STM32F103C8T6），编译后通过 DAP-Link 烧录。
2. **硬件连接**：按上表连接 DHT11、光敏电阻模块、蜂鸣器及 OLED，注意共地。
3. **串口调试**：打开串口助手（波特率 115200），可看到温湿度和 ADC 数据输出。
4. **阈值设置**：在代码中修改 `TEMP_ALARM` 和 `LIGHT_ALARM` 宏定义，重新编译烧录即可。
5. **运行**：上电后 OLED 显示实时数据；当环境参数超限时蜂鸣器报警，屏幕显示警告。

## 注意事项

- DWT 初始化必须在主函数中调用 `DWT_Init()`，否则延时函数无效。
- 若 OLED 不显示或显示异常，检查 I2C 地址（默认 0x3C）及接线是否可靠。
- DHT11 读取失败时，串口会打印“DHT11 read error!”，请检查上拉电阻及供电。
- 光敏电阻数值若跳动明显，请增加采样时间或多次平均滤波。
- 编译工程前确保已勾选 Keil 的 MicroLIB 选项，否则 printf 无法输出。

## 常见问题与解决方案

| 问题 | 可能原因 | 解决方法 |
|------|----------|----------|
| OLED 全屏点亮正常，但显示汉字乱码 | 字模数据格式不一致 | 确保 PCtoLCD2002 设置为阴码、逐列式、高位在前 |
| DHT11 一直返回 HAL_ERROR | 延时不精确 | 确认 DWT 初始化成功 |
| 串口无输出 | 未勾选 MicroLIB 或串口初始化错误 | 勾选 MicroLIB，检查 MX_USART1_UART_Init() 是否调用 |
| 报警无法解除 | 阈值判断逻辑错误 | 检查浮点数比较时的数据类型；确保阈值宏定义为 `float` 类型 |

## 后续扩展思路

- 增加按键设置报警阈值，无需重新编译烧录。
- 加入 **ESP8266 WiFi 模块**，将温湿度、光照数据上传至云平台（如 OneNET、阿里云），实现远程监控和报警。
- 增加 **RTC 实时时钟**，记录历史数据并制作温湿度变化曲线。
- 增加 **低功耗模式**，使用电池供电，延长待机时间。

## 参考资源

- [STM32F103 数据手册](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [SSD1306 数据手册](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [DHT11 数据手册](https://www.microchip.com/wwwproducts/en/DHT11)
- [Cortex-M3 权威指南（中文版）](https://github.com/eblot/ebooks/raw/master/ARM%20Cortex-M3%20Authoritative%20Guide.pdf)

## 作者

- **刘礼鑫**
- 日期：2026 年 5 月
- GitHub：[https://github.com/2684592530llx/Project_DHT11_OLED.git](https://github.com/2684592530llx/Project_DHT11_OLED.git)
