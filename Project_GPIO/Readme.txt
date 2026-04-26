用STM32CubeMX生成GPIO模板

GPIO简介

GPIO 是通用输入输出端口的简称，简单来说就是STM32/可控制的引脚，STM32芯片的GPIO引脚与外部设备连接起来，从而实现与外部通讯、控制以及数据采集的功能。所有的GPIO引脚都有基本的输入输出功能。
最基本的输出功能是由STM32控制引脚输出高、低电平，实现开关控制，如把GPIO 引脚接入到LED 灯，那就可以控制LED 灯的亮灭，引脚接入到继电器或三极管，那就可以通过继电器或三极管控制外部大功率电路的通断。最基本的输入功能是检测外部输入电平，如把GPIO引脚连接到按键，通过电平高低区分按键是否被按下。

1.打开 STM32CubeMX 选择 C8T6芯片 ，新建工程。

2.在左侧的 System Core 中：
  选择 SYS 将 Debug 设置为 Serial Wire
  选择RCC 将High Speed Clock (HSE)  设置为 Crystal/Ceramic Resonator 
                   Low Speed Clock (LSE)    设置为 Crystal/Ceramic Resonator

3.Clock Configuration  时钟部分：
  选择 HSE -> /1 -> HSE -> 8 -> x9 -> PLLCLK -> 72 -> /1 ->72 -> /2

4.在 C8T6 芯片上面找到 PA1（red），PA2(green)，PA3(blue) 将它们设置为 GPIO-Output

5.点击左侧的GPIO，将三个引脚设置为 低电平 Low（亮），推挽输出 Output Push Pull ,  命名。（其他默认）

6.Project Manager ：输入工程名称 ， 选择好路径 ， Toolchain /IDE：MDK-ARM 
  Project Manager -> Code Generator ：勾选Copy only the necessary library files ， Generate peripheral initialization as a pair of '.c/.h' files per peripheral

7.生成代码，打开项目
