# 基于 STM32 的 DHT11 温湿度采集与 OLED 显示系统

## 项目简介

本项目基于 **STM32F103C8T6** 微控制器，使用 **DHT11** 温湿度传感器采集环境数据，并通过 **0.96 寸 OLED 显示屏**（SSD1306，I2C 接口）实时显示温湿度值。项目中利用 **DWT（Data Watchpoint and Trace）** 实现了微秒级精确延时，保证了单总线通信的时序可靠性。同时支持串口打印调试信息，便于性能分析与问题排查。

## 硬件清单

| 模块 | 型号/规格 | 连接引脚 |
|------|-----------|----------|
| 主控芯片 | STM32F103C8T6 | - |
| 温湿度传感器 | DHT11 | 数据线 → PB12 |
| OLED 显示屏 | 0.96 寸，SSD1306，I2C | SCL → PB6，SDA → PB7 |
| 调试/烧录 | DAP-Link / ST-Link | SWD 接口 |
| USB 转 TTL | - | PA9(TX), PA10(RX) |

> **注意**：DHT11 数据线需外接 4.7kΩ 上拉电阻至 3.3V（模块已内置则可省略）。

## 功能说明

- 每 2 秒读取一次 DHT11 温湿度数据。
- OLED 屏幕实时显示中文“温度”“湿度”及对应的数值。
- 通过串口（USART1）输出读取状态及数据，波特率 115200。
- 校验数据有效性，若校验失败则保留上一次显示值并输出错误提示。

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

### 3. OLED 显示

- I2C 通信，发送命令或数据前需附带控制字节（`0x00` 表示命令，`0x40` 表示数据）。
- 支持 ASCII 字符（8x16 字体）和 16x16 汉字点阵，字模数据统一保存在 `Fonts/fonts.c` 中。
- 显示位置通过页地址（0~7）和列地址（0~127）指定。

```c
// 示例：显示字符串
OLED_ShowString_F8X16(1, 2, (uint8_t*)"Hello");
```

## 使用说明

1. **编译与烧录**：使用 Keil MDK 打开工程，选择正确的芯片型号（STM32F103C8T6），编译后通过 DAP-Link 烧录。
2. **硬件连接**：按上表连接 DHT11 和 OLED，注意共地。
3. **串口调试**：打开串口助手（波特率 115200），可看到数据输出。
4. **运行**：上电后 OLED 显示“温度”“湿度”及数值，每 2 秒刷新一次。

## 注意事项

- DWT 初始化必须在主函数中调用 `DWT_Init()`，否则延时函数无效。
- 若 OLED 不显示或显示异常，检查 I2C 地址（默认 0x3C）及接线是否可靠。
- DHT11 读取失败时，串口会打印“DHT11 read error!”，请检查上拉电阻及供电。
- 编译工程前确保已勾选 Keil 的 MicroLIB 选项，否则 printf 无法输出。

## 常见问题与解决方案

| 问题 | 可能原因 | 解决方法 |
|------|----------|----------|
| OLED 全屏点亮正常，但显示汉字乱码 | 字模数据格式不一致 | 确保 PCtoLCD2002 设置为阴码、逐列式、高位在前 |
| DHT11 一直返回 HAL_ERROR | 缺少上拉电阻 / 延时不精确 | 外接 4.7kΩ 上拉电阻；确认 DWT 初始化成功 |
| 串口无输出 | 未勾选 MicroLIB 或串口初始化错误 | 勾选 MicroLIB，检查 MX_USART1_UART_Init() 是否调用 |
| 显示位置偏移 | `OLED_SetPos` 中的列地址计算错误 | 检查高位字节是否正确右移 4 位 |

## 后续扩展思路

- 增加 WiFi 模块（如 ESP8266），通过 MQTT 将数据上传至云平台。
- 添加按键切换显示界面（如历史数据、曲线图）。
- 增加低功耗模式，延长电池供电时间。

## 参考资源

- [STM32F103 数据手册](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [SSD1306 数据手册](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [DHT11 数据手册](https://www.microchip.com/wwwproducts/en/DHT11)
- [Cortex-M3 权威指南（中文版）](https://github.com/eblot/ebooks/raw/master/ARM%20Cortex-M3%20Authoritative%20Guide.pdf)

## 作者

- **刘礼鑫**
- 日期：2026 年 5 月
- GitHub：[https://github.com/2684592530llx/Project_DHT11_OLED.git]
