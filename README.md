# 循迹与航向控制小车底层框架 · MSPM0G3507

基于 TI MSPM0G3507 的分层小车底层框架，循迹环 + 航向环，可直接打开 Keil 工程编译烧录。
本工程为演示版：上电只做各模块初始化与状态显示，PID 闭环默认关闭。

## 工程信息

| 项 | 内容 |
| --- | --- |
| 硬件 | TI MSPM0G3507 / LP-MSPM0G3507 |
| SDK | MSPM0 SDK 2.09.00.01 |
| 工具链 | MDK-ARM ARMCLANG V6.22 |
| 入口工程 | `keil/empty_LP_MSPM0G3507_nortos_keil.uvprojx` |
| 配置工具 | SysConfig（`empty.syscfg`） |

## 目录说明

| 目录 | 内容 |
| --- | --- |
| `main.c` | 程序入口。 |
| `driver/` | 硬件驱动：串口、编码器、电机、按键、IMU、PID。 |
| `library/` | 第三方组件：ringbuffer、OLED 驱动与字库。 |
| `logic/` | 应用层 |
| `keil/` `gcc/` `iar/` `ticlang/` | 各工具链工程文件。 |

## 板级功能

- UART1 八路串口红外传感器，帧格式 `$D,x1:0,...#`，1=黑线
- 双路直流电机，PWM + 方向控制
- 四路 GPIO 中断编码器测速
- ICM42688， I2C
- 0.96 英寸 OLED，I2C
- 多路按键

## 串口

| 串口 | 用途 | 接收方式 |
| --- | --- | --- |
| UART0 | 调试打印 | 超时轮询断帧 |
| UART1 | 八路红外传感器 | DMA + RX 超时中断 |
| UART2 | 空闲，可接外设 | DMA + RX 超时中断 |
| UART3 | 空闲，可接外设 | DMA + RX 超时中断 |

UART2、UART3 已在 `empty.syscfg` 中使能并配好 DMA，接入新外设只需在 `logic/uart_app.c` 的 `UartX_Task` 里写解析。

## 编译和下载

1. 安装 MSPM0 SDK 2.09.00.01 与 SysConfig。
2. 打开 `keil/empty_LP_MSPM0G3507_nortos_keil.uvprojx`。
3. 执行 `Project -> Rebuild all target files`。
4. 下载并运行。

编译时自动运行 SysConfig 钩子，由 `empty.syscfg` 重新生成 `ti_msp_dl_config.c/h`。

> **注意**：工程内三处路径均硬编码为 SDK 的绝对路径（SysConfig 命令、Include Paths、Linker 的 `driverlib.a`）。复制到其它机器后若编译报错，优先检查这三处，改成自己机器上的 SDK 安装路径，详见本目录下 [Config_README.md](Config_README.md)。

## 运行说明

上电后 `System_Init()` 依次调用各模块初始化，串口打印结果：

```
==== System Init ====
uart init ok
oled init ok
motor init ok
encoder init ok
pid init ok
gray init ok
==== Finish ====
```

OLED 显示按键、八路灰度位图、左右编码器速度、灰度在线状态和 PID 开关。

`logic/user_pid.c` 中 `pid_running` 默认为 `false`，改 `true` 即可跑循迹闭环；
`heading_contrl_enable` 置 `true` 切换航向闭环。速度上限和 PID 参数在同文件顶部。

## 两种 IMU 接法

- `logic/imu_imu_app.c`：直接驱动 ICM42688 并解算姿态，当前在用。
- `logic/imu_app.c`：串口外挂 IMU，接口 `imu_app_task`，当前未接入，二者二选一。

## 第三方组件

| 文件 | 来源 | 许可 |
| --- | --- | --- |
| `library/ringbuffer.c/h` | RT-Thread | Apache License 2.0 |
| `library/OLED.c/h`、`OLED_FONT.h` | 第三方 OLED 驱动（无版权头，来源待补） | —— —— |

## 许可

MIT License，详见 [LICENSE.txt](LICENSE.txt)。
