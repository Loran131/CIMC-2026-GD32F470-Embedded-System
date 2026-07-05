# GD32F470 嵌入式系统开发工程总结

> **MCU**: GD32F470VE (ARM Cortex-M4, 200MHz, 512KB Flash)  
> **开发板**: GD32F470 Development Kit V2.0  
> **IDE**: Keil MDK (uVision) + ARMCLANG V6.23  
> **并不是一个特别好的实现方案，仅供参考**  
> **日期**: 2025-07-05  
> 
> 🔧 验证脚本: `000Bootloader/_final_verify.py` (12 项静态检查)  

---

## 一、工程结构

```
00000合并版/
├── 000APP/              # 应用程序工程
│   ├── CMSIS/           # ARM CMSIS + GD32F4xx 系统文件
│   ├── Driver/          # 外设驱动层 (LED/KEY/OLED/ADC/DAC/USART/SPI/I2C/RTC/SDIO/...)
│   ├── Function/        # 主逻辑代码 (Function.c/h)
│   ├── HeaderFiles/     # 顶层头文件
│   ├── Library/         # GD32F4xx 标准外设库 + USB库 + FatFS
│   ├── Protocol/        # 通信协议 (ModbusCRC)
│   ├── Startup/         # 启动文件
│   ├── User/            # main.c + 中断服务 + systick
│   └── project/         # Keil 工程文件
│
└── 000Bootloader/       # Bootloader 工程
    ├── (基本同上结构, 共享同一套 Driver/Protocol/Library)
    ├── .claude/         # AI 辅助开发配置
    ├── _final_verify.py # 代码静态验证脚本 (12项规则检查)
    └── 参考文档.pdf      # 开发参考文档
```

## 二、Flash 分区布局

两个工程共享同一 Flash 分区方案 (`flash_partition.h`)：

| 区域 | 起始地址 | 大小 | 说明 |
|------|----------|------|------|
| Bootloader | `0x0800 0000` | 64 KB | 引导程序 |
| 参数存储区 | `0x0801 0000` | 4 KB | 设备参数持久化 (含升级标志) |
| APP 程序区 | `0x0801 1000` | 128 KB | 应用程序固件 |
| APP 备份区 | `0x0803 1000` | 128 KB | **已定义，暂未使用** |
| 固件暂存区 | `0x0805 1000` | 128 KB | 升级时暂存新固件 |

GD32F470 内部 Flash 共 512KB，上述分区从 0x08000000 开始使用约 384KB。

---

## 三、000APP（应用程序）功能实现情况

### 3.1 硬件驱动 - ✅ 已集成

| 驱动模块 | 功能 | 状态 |
|----------|------|------|
| **LED** | GPIO 控制 4 路 LED (LED1~LED4) | ✅ 正常 |
| **KEY** | 按键输入检测 | ✅ 已集成驱动程序 |
| **OLED** | I2C 0.96" OLED 显示 (128x64) | ✅ 正常，显示设备信息和运行状态 |
| **RTC** | 实时时钟 (LXTAL 32.768kHz)，支持 UTC ↔ BCD 转换 | ✅ 正常，上电初始化默认时间 |
| **USART+DMA** | 485 串口通信 + DMA 收发 | ✅ 正常，配合 RingBuffer |
| **ADC CH0/CH1** | 内部 ADC 双通道采集 (12bit) | ✅ 正常，支持比例系数校准 |
| **外部 ADC** | GD30AD3344 SPI 接口，用于 PT100 温度采集 | ✅ 正常 |
| **DAC** | DAC0_OUT0 模拟电压输出 (0~4095) | ✅ 正常 |
| **TIMER3** | 0.5s 定时中断，LED2 心跳闪烁 | ✅ 正常 |
| **SysTick** | 1ms 系统滴答，用于定时和超时 | ✅ 正常 |
| **SPI Flash** | 外部 SPI Flash 读写 | 🔸 驱动已集成，初始化代码注释掉 |
| **SD Card** | SDIO 接口 + FatFS 文件系统 | 🔸 驱动已集成，初始化代码注释掉 |
| **EEPROM** | 利用 Flash 模拟参数存储 | ✅ 正常 (param 模块) |

### 3.2 通信协议 - ✅ 完整实现

基于 485 总线的 ASCII 十六进制帧协议：

```
帧格式: A5 B6 | DevID(2B) | FrameType(1B) | CmdWord(2B) | Version(1B) | DataLen(1B) | Content(NB) | CRC(2B) | B6 A5
```

#### 系统命令帧 (FrameType 0x01)
| 命令字 | 功能 | 状态 |
|--------|------|------|
| `0x0101` | 设备复位 | ✅ 正常 |
| `0x0104` | 查询固件版本 (v2.0.1.0) | ✅ 正常 |
| `0x0105` | 设置设备时间 (UTC 时间戳) | ✅ 正常 |
| `0x0106` | 查询设备时间 (返回 UTC) | ✅ 正常 |
| `0x01A1` | 设置设备 ID | ✅ 正常 |
| `0x01A2` | 设置波特率 (需重启生效) | ✅ 正常 |
| `0x0111` | 查询设备 ID | ✅ 正常 |
| `0x0112` | 查询波特率 | ✅ 正常 |

#### 数据采集帧 (FrameType 0x02)
| 命令字 | 功能 | 状态 |
|--------|------|------|
| `0x0201` | 查询 CH0 采集值 (比例换算后) | ✅ 正常 |
| `0x0202` | 查询 CH1 采集值 (比例换算后) | ✅ 正常 |
| `0x0221` | 查询外部 ADC (PT100) 温度 | ✅ 正常 |
| `0x0241` | 设置 CH0 比例系数 (IEEE 754 float) | ✅ 正常 |
| `0x0242` | 设置 CH1 比例系数 (IEEE 754 float) | ✅ 正常 |
| `0x0261` | 设置自动上报间隔 (1s/3s/5s) | ✅ 正常 |

#### 控制命令帧 (FrameType 0x03)
| 命令字 | 功能 | 状态 |
|--------|------|------|
| `0x0301` | 设置 DAC 输出电压 | ✅ 正常 |
| `0x0302` | 开启定时自动上报 | ✅ 正常 |
| `0x0303` | 停止定时自动上报 | ✅ 正常 |
| `0x03AA` | 进入睡眠模式 (RTC 闹钟唤醒) | ⚠️ 已实现, 见下文 |

#### 系统管理帧 (FrameType 0x05) - 升级相关
| 命令字 | 功能 | 状态 |
|--------|------|------|
| `0x0501` | 进入固件升级模式 | ✅ APP 侧设置标志位并复位 |
| `0x0502` | 准备接收固件数据包 | 🔸 APP 侧仅有桩代码 |
| `0x0503` | 执行固件升级 | 🔸 APP 侧仅有桩代码 |
| `0xFFFF→0x8888` | 广播寻址/应答 | ✅ 正常 |

#### 阈值与报警 (FrameType 0x04 / 0x06)
| 命令字 | 功能 | 状态 |
|--------|------|------|
| `0x0400~0x0403` | 查询 CH0/CH1/CH2 阈值 | ✅ 正常 |
| `0x0411~0x0413` | 设置 CH0/CH1/CH2 阈值 | ✅ 正常 |
| `0x0601` | 启用/禁用报警主动上推 | ✅ 正常 |
| `0x0602` | 查询报警记录 | ✅ 正常 |
| `0x0603` | 清除报警记录 | ✅ 正常 |

### 3.3 自动上报功能 - ✅ 完整实现

- 支持 1s / 3s / 5s 三种时间间隔
- 上报内容: UTC时间戳(4B) + CH0数据(4B float) + CH1数据(4B float) = 12字节
- 开启自动上报期间会屏蔽其他命令的响应 (仅响应停止命令 0x0303)

### 3.4 参数持久化 - ✅ 完整实现

- 使用 Flash 模拟 EEPROM (4KB 参数区)
- CRC32 校验保证数据完整性
- 支持: 设备ID、波特率、通道比例系数、报警阈值、报警推送开关、报警记录(10条)
- 支持: `upgrade_requested` 升级标志位

---

## 四、000Bootloader（引导程序）功能实现情况

### 4.1 Bootloader 核心流程 - ✅ 基本完成

```
上电 → 系统初始化 → OLED显示"Bootloader"
     → 检查 upgrade_requested 标志
         ├─ 已请求升级: 10秒倒计时等待 0x0502 命令
         │    ├─ 收到 0x0502 → 接收固件分片 → 收到 0x0503 → 执行升级 → 跳转APP
         │    └─ 超时 → 跳转APP
         └─ 正常运行: 5秒等待 → 跳转APP
```

### 4.2 Bootloader 命令处理

| 命令字 | 功能 | 状态 |
|--------|------|------|
| `0x0501` | 查询 Bootloader 状态 | ✅ 正常 |
| `0x0502` | 准备接收固件数据 (512片 × 256B = 128KB) | ⚠️ 框架完成，待联调 |
| `0x0503` | 执行固件升级 (擦除APP区 → 拷贝暂存区 → 清除标志 → 跳转) | ⚠️ 框架完成，待联调 |

### 4.3 固件升级数据流

```
PC端 → 485总线 → Bootloader → Flash暂存区(0x08051000)
                                    ↓ Magic校验(0xABCD0001)
                                    ↓ 擦除APP区(0x08011000)
                                    ↓ 逐字编程
                                    ↓ 清除 upgrade_requested 标志
                                    ↓ Jump to APP
```

### 4.4 APP 跳转逻辑 - ✅ 完整实现

- SP/PC 合法性校验 (SP∈SRAM, PC∈APP Flash)
- 关闭 SysTick，清除所有 NVIC 中断
- 设置 SCB->VTOR = APP 基地址
- 设置 MSP，__DSB + __ISB 屏障
- 跳转到 APP Reset_Handler

---

## 五、⚠️ 待完成功能

### 5.1 睡眠唤醒功能 (部分完成)

**已完成**:
- APP 中实现了 `deepsleep_with_rtc_alarm()` 函数
- 通过命令 `0x03AA` 可触发 MCU 进入 Deep Sleep 模式
- 使用 RTC Alarm 0 定时唤醒 (默认 10 秒)
- 唤醒后恢复 HXTAL + PLL 时钟配置

**待完成**:

| 唤醒源 | 参考实现 | 集成状态 |
|--------|----------|----------|
| RTC 闹钟唤醒 | `26_WakeUp_RTC` | ✅ 已集成 |
| EXIT 外部中断唤醒 (KEY) | `24_WakeUp_EXIT` | ❌ 未集成 |
| UART 中断唤醒 (485) | `25_WakeUp_UART_IT` | ❌ 未集成 |
| 多唤醒源联合配置 | -- | ❌ 未实现 |

**说明**: 参考示例 `24_WakeUp_EXIT`, `25_WakeUp_UART_IT`, `26_WakeUp_RTC` 三个独立工程提供了不同的睡眠唤醒方案。当前合并版仅集成了 RTC 闹钟唤醒，EXIT 和 UART 中断唤醒尚未合并到 000APP 工程中。

**建议实现方案**:
1. 在 `System_Init()` 中配置 EXIT 中断线 (KEY 引脚) 作为唤醒源
2. 配置 USART 的 RX 引脚为 EXTI 唤醒源 (或使用 UART 自身的唤醒功能)
3. 进入睡眠前使能多个唤醒源，任意一个事件均可唤醒
4. 唤醒后在中断/主循环中判断唤醒源类型并执行对应处理

### 5.2 Bootloader 固件下载 (框架完成，待联调打通)

**已完成**:
- Flash 分区方案定义完成
- 固件升级通信协议帧定义完成 (0x0501/0x0502/0x0503)
- 暂存区接收机制已实现 (`receive_firmware_slices`)
- 固件搬运和跳转逻辑已实现 (`execute_upgrade`, `jump_to_app`)
- Magic Number 校验 (0xABCD0001) 用于验证固件完整性
- 参数区 `upgrade_requested` 标志位触发升级流程机制已就绪

**静态验证**: Bootloader 目录下有 `_final_verify.py` Python 脚本，对源码进行 12 项静态规则检查，包括: RingBuffer 提取行为、ASCII 解析器复位、帧字段顺序、转义字符正确性、文件作用域静态变量、参数 getter 正确性、LED GPIO 引脚正确性等。

**待解决/验证**:

| 问题 | 说明 |
|------|------|
| **二进制接收与协议解析冲突** | 当前 `receive_firmware_slices()` 使用 `recv_bytes()` 直接读取原始字节，绕过了 ASCII 协议帧解析器（关闭了 idle 中断）。与 PC 端联调时需约定固件数据是否走裸二进制传输 |
| **端到端联调未完成** | PC 上位机 → 485 → Bootloader 的完整固件下载流程尚未完成实际测试 |
| **无校验签名机制** | 仅有简单的 magic number (0xABCD0001) 校验，缺少 SHA/CRC 完整性校验 |
| **无回滚机制** | APP 备份区 (0x08031000) 已分配但未使用，升级失败无法回退 |
| **升级状态机简单** | 异常情况下（如传输中断）缺少超时恢复和断点续传 |

**建议**:
1. 与 PC 端上位机约定固件传输格式（裸二进制 vs 编码帧）
2. 增加 CRC32 或 SHA 完整性校验
3. 考虑使用 APP 备份区实现 A/B 分区升级，支持回滚
4. 完善升级状态机，增加超时重试和错误恢复

---

## 六、技术要点总结

### 6.1 通信协议设计
- 基于 RS-485 总线的 ASCII 十六进制帧编码
- 支持单播 (指定 DevID) 和广播 (DevID=0xFFFF)
- 帧类型区分: 系统命令/数据采集/控制命令/阈值管理/系统管理/报警管理
- 错误响应帧 (0xFF/0xEEEE) 统一错误码
- CRC16 校验保证数据完整性

### 6.2 参数存储方案
- 利用内部 Flash 4KB 扇区模拟 EEPROM
- 结构体对齐 + CRC32 校验
- 支持运行时读写和掉电保存
- 升级标志位实现 APP ↔ Bootloader 通信握手

### 6.3 ADC 采集与校准
- 双通道 (CH0/CH1) + 外部 ADC (PT100)
- 比例系数校准 (float IEEE 754)
- 报警阈值判断 + 主动上推机制

### 6.4 时钟系统
- HXTAL 25MHz → PLL → 200MHz 系统时钟
- LXTAL 32.768kHz → RTC 时钟源
- 睡眠唤醒后需重新配置 PLL

---

## 七、编译与烧录

### 7.1 编译顺序
1. 先编译 `000Bootloader`，烧录到 `0x08000000`
2. 再编译 `000APP`，烧录到 `0x08011000`

### 7.2 Keil 工程
- Bootloader: `000Bootloader/project/CIMC_GD32_fatfs_driver.uvprojx`
- APP: `000APP/project/CIMC_GD32_fatfs_driver.uvprojx`

### 7.3 APP 中断向量偏移
APP 工程中 `System_Init()` 必须调用:
```c
nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x11000);
```
将中断向量表映射到 APP 起始地址。

---

## 八、参考示例工程

以下参考工程位于 `04template/` 目录下，提供了独立的外设驱动示例：

| 编号 | 工程名 | 功能 |
|------|--------|------|
| 01 | `LED_driver` | LED 驱动 |
| 02 | `KEY_driver` | 按键驱动 |
| 03 | `EXTInterrupt_driver` | 外部中断 |
| 04-07 | `SerialPort*_driver` | 串口收发/DMA/232/485 |
| 08 | `ADC_driver` | ADC 采集 |
| 09 | `DAC_driver` | DAC 输出 |
| 10-16 | `Timer*_driver` | 定时器/PWM/脉冲/频率测量 |
| 17 | `IndependentWatchdog_driver` | 独立看门狗 |
| 18 | `RTC_driver` | 实时时钟 |
| 19 | `OLED_driver` | OLED 显示 |
| 20 | `extFlash_driver` | 外部 Flash |
| 21 | `fatfs_driver` | FatFS 文件系统 |
| 22 | `EEPROM` | EEPROM |
| 23 | `OLED_EEPROM` | OLED + EEPROM |
| **24** | **`WakeUp_EXIT`** | **外部中断唤醒 (待集成)** |
| **25** | **`WakeUp_UART_IT`** | **UART 中断唤醒 (待集成)** |
| **26** | **`WakeUp_RTC`** | **RTC 闹钟唤醒 (已集成)** |
| 27 | `BootLoader_Two_Stage` | 两阶段 Bootloader (Bootloader+APP, 含参数区规划文档) |
| 28 | `BootLoader_Three_Stage` | 三阶段 Bootloader (含 APP 备份与版本回滚, 含参数区规划文档) |

> 📝 参考 bootloader 目录下各有 `Bootloader 参数区规划.md` 设计文档，记录了 Flash 分区与升级策略的详细设计。  
> ⚠️ 注意：参考 bootloader (27/28) 的 Flash 分区方案 (Bootloader 48KB + Params 4KB + APP 76KB) 与合并版 (Bootloader 64KB + Params 4KB + APP 128KB + Backup 128KB + Staging 128KB) **不一致**，合并版采用了更大的分区以支持更完善的升级机制。

---

## 十、合并版 vs 参考 Bootloader 对比

| 特性 | 两阶段参考 (27) | 三阶段参考 (28) | 合并版 (000Bootloader) |
|------|:--:|:--:|:--:|
| Bootloader 大小 | 48KB | 48KB | 64KB |
| APP 大小 | 76KB | 76KB | 128KB |
| APP 备份区 | ❌ | 128KB (B分区) | 128KB (已分配) |
| 固件暂存区 | 52KB (下载缓存) | 52KB (下载缓存) | 128KB (FW Staging) |
| 升级方式 | 上电检测 BIN→拷贝 | 上电检测 BIN→拷贝 | 实时 RS485 分片接收 |
| 版本回滚 | ❌ | ✅ (5次启动失败回滚) | ❌ (未实现) |
| CRC32 校验 | ✅ | ✅ | ❌ (仅 magic number) |
| 参数区结构 | Main+Backup+Log | Main+Backup+Log+User+Calib | 单一 ParameterBlock |
| 升级触发 | Flash 标志位 | Flash 标志位 | 485 命令 + Flash 标志位 |

---

## 九、总结

### 已完成 ✅
- 完整的硬件驱动层 (LED/KEY/OLED/ADC/DAC/USART/SPI/I2C/RTC/SDIO/TIMER)
- 完整的 485 通信协议栈 (6 类帧类型, 30+ 命令字)
- RTC 实时时钟 + UTC 时间戳转换
- 双通道 ADC 采集 + 比例校准 + 阈值报警
- 定时自动上报功能
- 参数持久化存储 (Flash 模拟 EEPROM + CRC)
- Bootloader 框架 (Flash 分区/升级握手/固件搬运/APP 跳转)
- RTC 闹钟睡眠唤醒 (`deepsleep_with_rtc_alarm`)

### 待完成 ⚠️
1. **睡眠唤醒功能扩展**: EXIT 外部中断唤醒和 UART 中断唤醒尚未集成到 APP
2. **Bootloader 固件下载**: 框架代码已完成，但 PC 端到 MCU 的端到端联调尚未打通
3. **固件完整性校验**: 仅有 magic number，缺少 CRC/SHA 校验
4. **升级回滚机制**: APP 备份区已分配但未使用

---

> 📅 最后更新: 2025-07-05  
> 🏷️ 版本: FW v2.0.1.0
