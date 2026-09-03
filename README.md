# STM32H755 USB CDC ACM Command Test

## 1. 專案目的

本專案用於建立與驗證：

```text
Raspberry Pi CM5 ↔ STM32H755
```

之間的 **USB CDC ACM 雙向 Command 通訊**。

本專案是正式系統整合之前的獨立 Bring-Up / Proof-of-Concept 測試。

目前已成功驗證：

```text
CM5
 │
 │ USB
 │ USB CDC ACM
 ▼
STM32H755
```

可以完成：

```text
CM5 → STM32H755
Command

STM32H755 → CM5
Response
```

目前測試 Command：

```text
MOV R 1000 1000 500 0 0 0\r\n
```

STM32H755 正確接收、累積、解析 Command 後回傳：

```text
DONE\r\n
```

CM5 最終收到：

```text
Response: DONE
```

---

# 2. 整體系統背景

STM32H755 USB CDC ACM 並不是整個系統唯一的通訊介面。

最終系統架構為：

```text
                           End User
                               │
                               │ TCP/IP
                               ▼
                    ┌─────────────────────┐
                    │ Raspberry Pi CM5    │
                    │                     │
                    │ TCP Server          │
                    │ Command Parser      │
                    │ USB Transport       │
                    └──────────┬──────────┘
                               │
                               │ USB CDC ACM
                               ▼
                    ┌─────────────────────┐
                    │ STM32H755           │
                    │                     │
                    │ Command Processing  │
                    │                     │
                    │ SPI1 → EtherCAT     │
                    │ SPI2 → CM5          │
                    └─────────────────────┘
```

其中：

### USB CDC ACM

負責：

```text
CM5 → STM32H755
```

的 Command 通訊，以及：

```text
STM32H755 → CM5
```

的 Response。

### SPI2

SPI2 是另一條獨立的資料通道，主要負責後續即時 EtherCAT / motion trajectory data 傳送。

因此：

> USB Command Transport 與 SPI2 即時資料 Transport 是兩條不同的通道，不應混合設計。

---

# 3. 為什麼先做 USB 獨立測試？

正式系統預計：

```text
Windows TCP Client
        ↓
CM5 TCP Server
        ↓
Command Parser
        ↓
USB CDC ACM
        ↓
STM32H755
```

如果直接一次整合所有模組，發生錯誤時很難判斷問題來自：

* TCP Client
* TCP Server
* Command Parser
* CM5 USB Transport
* Linux TTY
* USB CDC
* STM32 Command Parser

因此先建立最小測試：

```text
CM5
 │
 │ USB CDC ACM
 ▼
STM32H755
```

先證明 USB Transport 本身可靠。

---

# 4. 硬體

## 4.1 MCU

```text
STM32H755ZIT6U
```

開發板：

```text
NUCLEO-H755ZI-Q
```

使用：

```text
CM7 Core
```

執行本 USB Device 測試。

---

# 5. USB 架構

STM32H755 使用：

```text
USB_OTG_FS
```

設定為：

```text
USB Device
```

USB Class：

```text
CDC
```

Linux CM5 端則使用：

```text
USB CDC ACM
```

因此完整資料鏈：

```text
STM32 USB CDC Device
        │
        │ USB
        ▼
Linux USB CDC ACM Driver
        │
        ▼
/dev/ttyACM0
```

---

# 6. CubeMX USB 設定

本專案的 USB Device 是透過 STM32CubeMX / STM32CubeIDE 產生。

## 6.1 USB Peripheral

啟用：

```text
USB_OTG_FS
```

模式：

```text
Device Only
```

---

## 6.2 USB Class

在 USB Device Middleware 中啟用：

```text
CDC
```

即：

```text
USB_DEVICE
    └── CDC
```

---

# 7. USB Clock

USB Full-Speed 必須使用正確的：

```text
48 MHz
```

本專案使用：

```text
HSI48
```

作為 USB Clock Source。

因此需要確保：

```text
HSI48 = 48 MHz
```

並讓 USB Peripheral 使用正確的 48 MHz Clock。

---

# 8. USB GPIO 設定

本專案使用 USB OTG FS：

```text
PA8
PA9
PA11
PA12
```

相關 Alternate Function：

```text
AF10_OTG1_FS
```

USB 資料線：

```text
PA11 → USB_DM
PA12 → USB_DP
```

PA8 / PA9 則依 MCU / USB OTG FS 的 CubeMX configuration 使用。

---

# 9. USB Device Configuration

目前產生的 `usbd_conf.c` 中，USB Device 使用：

```text
PCD_SPEED_FULL
```

也就是：

```text
USB Full Speed
```

---

## 9.1 DMA

本測試：

```text
DMA Enable = DISABLE
```

也就是：

```text
dma_enable = DISABLE
```

目前 USB Bring-Up 不使用 DMA。

---

## 9.2 PHY

使用：

```text
PCD_PHY_EMBEDDED
```

也就是 MCU 內建 USB PHY。

---

## 9.3 VBUS sensing

目前：

```text
vbus_sensing_enable = DISABLE
```

---

# 10. STM32 Cube 產生的主要檔案

USB Device Middleware 產生的主要檔案包括：

```text
CM7/
├── USB_DEVICE/
│   ├── App/
│   │   ├── usb_device.c
│   │   ├── usbd_cdc_if.c
│   │   └── usbd_cdc_if.h
│   │
│   └── Target/
│       ├── usbd_conf.c
│       └── usbd_conf.h
```

實際專案中請依 CubeMX 產生的目錄結構為準。

---

# 11. `main.c` 初始化

USB Device 初始化由 CubeMX 產生：

```c
MX_USB_DEVICE_Init();
```

因此系統啟動流程包含：

```text
main()
 │
 ├── HAL initialization
 │
 ├── System Clock
 │
 ├── GPIO
 │
 ├── USB Device initialization
 │
 └── Main loop
```

USB Device 初始化完成後，STM32 就可以等待 USB Host，也就是 Raspberry Pi CM5 的連線。

---

# 12. Raspberry Pi CM5 端的 USB Device

STM32 插入 CM5 後，Linux 會辨識為：

```text
/dev/ttyACM0
```

USB Device 資訊曾確認：

```text
VID:PID
0483:5740
```

Model：

```text
STM32_Virtual_ComPort
```

因此：

```text
STM32H755
    ↓
USB CDC Device
    ↓
Linux
    ↓
cdc_acm driver
    ↓
/dev/ttyACM0
```

這一步是 USB Bring-Up 的第一個成功條件。

---

# 13. CM5 端測試程式

CM5 建立獨立專案：

```text
~/usb_command_test
```

主要結構：

```text
usb_command_test/
├── CMakeLists.txt
├── include/
│   └── usb_transport.hpp
└── src/
    ├── main.cpp
    └── usb_transport.cpp
```

CM5 端建立：

```cpp
class UsbTransport
```

負責：

```text
open()
close()
sendCommand()
receiveResponse()
```

---

# 14. CM5 `open()` 的重要設定

最初 CM5 端只使用：

```cpp
open(
    "/dev/ttyACM0",
    O_RDWR | O_NOCTTY
);
```

後來發現 Linux TTY 狀態會影響傳送的 CR/LF。

因此正式版本的 `UsbTransport::open()` 必須自行設定 `termios`。

使用：

```cpp
#include <termios.h>
```

並：

```cpp
tcgetattr()
cfmakeraw()
cfsetispeed()
cfsetospeed()
tcsetattr()
```

建立固定的 TTY environment。

目前使用：

```text
115200
8 data bits
No parity
1 stop bit
No hardware flow control
Raw mode
```

注意：

> USB CDC ACM 並不是傳統 UART，因此這裡的 115200 並不是 USB 實體傳輸速率。它主要是建立一致的 Linux TTY configuration。

---

# 15. CM5 Command

目前測試 Command：

```text
MOV R 1000 1000 500 0 0 0\r\n
```

完整資料長度：

```text
27 bytes
```

其中：

```text
Command body = 25 bytes
CRLF         = 2 bytes
Total        = 27 bytes
```

---

# 16. STM32 `CDC_Receive_FS()`

STM32 的主要 Command Receive callback：

```c
static int8_t CDC_Receive_FS(
    uint8_t* Buf,
    uint32_t *Len
)
```

USB CDC 收到資料時，會進入：

```text
CDC_Receive_FS()
```

---

# 17. 不可以假設一次 callback 就是一個 Command

這是本專案非常重要的設計結論。

例如一個：

```text
27 bytes
```

的 Command，STM32 實際可能收到：

```text
第一次：
26 bytes

第二次：
2 bytes
```

因此：

```text
一次 write()
≠
一次 CDC_Receive_FS()
```

USB CDC callback boundary 不等於 Application Protocol boundary。

---

# 18. Command Buffer

因此 STM32 使用：

```c
static char command_buffer[128];
static uint32_t command_length = 0U;
```

收到 USB 資料後：

```text
Buf
 ↓
command_buffer
 ↓
command_length
```

持續累積。

---

# 19. CRLF 作為 Command 結束條件

目前 Application Protocol 使用：

```text
\r\n
```

作為 Command terminator。

STM32 會檢查：

```c
if ((command_length >= 2U) &&
    (command_buffer[command_length - 2U] == '\r') &&
    (command_buffer[command_length - 1U] == '\n'))
```

一旦偵測：

```text
CRLF
```

表示：

```text
完整 Command 已經收到
```

然後交給 Command Parser。

---

# 20. Command body 與 CRLF 分離

目前測試 Command：

```text
MOV R 1000 1000 500 0 0 0\r\n
```

收到：

```text
27 bytes
```

其中：

```text
27 - 2 = 25
```

因此比較：

```text
25 bytes Command body
```

而不是把 CRLF 包含在 Command body 中。

---

# 21. Command Content Validation

目前 Bring-Up 測試暫時使用固定 Command：

```c
static const char expected_command[] =
    "MOV R 1000 1000 500 0 0 0";
```

取得：

```c
const uint32_t expected_length =
    sizeof(expected_command) - 1U;
```

然後：

```c
memcmp(
    command_buffer,
    expected_command,
    expected_length
)
```

確認收到的 Command。

這只是 USB Bring-Up 用的測試 Parser。

正式產品版本將替換成真正的 Command Parser。

---

# 22. STM32 Response

Command 正確後，STM32 回傳：

```c
uint8_t response[] = "DONE\r\n";
```

並呼叫：

```c
CDC_Transmit_FS(
    response,
    sizeof(response) - 1U
);
```

因此實際 Response：

```text
DONE\r\n
```

---

# 23. `CDC_Transmit_FS()`

目前使用 CubeMX CDC interface 提供的：

```c
CDC_Transmit_FS()
```

其內部會先確認：

```c
hcdc->TxState
```

如果目前 USB TX busy：

```c
return USBD_BUSY;
```

否則：

```c
USBD_CDC_SetTxBuffer()
USBD_CDC_TransmitPacket()
```

開始傳送。

這表示未來正式產品如果需要高頻率或大量 Response，必須進一步處理：

```text
USBD_BUSY
```

以及 TX queue。

目前 Bring-Up Test 尚不需要複雜化。

---

# 24. 第一次發現的 `\r` 問題

本專案除了一般 USB Bring-Up，也發現一個非常重要的 Linux TTY 問題。

原本 STM32 應該收到：

```text
...\r\n
```

但實際 Debug 發現：

```text
...\r\r\n
```

HEX：

```text
0D 0D 0A
```

也就是多了一個：

```text
0D
```

即：

```text
\r
```

---

# 25. 問題的初步懷疑

檢查 CM5：

```bash
stty -F /dev/ttyACM0 -a
```

發現：

```text
onlcr
```

存在。

因此初步懷疑 Linux TTY output processing 可能將：

```text
\n
```

轉換為：

```text
\r\n
```

而 Application 原本已經傳：

```text
\r\n
```

就可能形成：

```text
\r\r\n
```

---

# 26. 第一次除錯方法

嘗試：

```bash
stty -F /dev/ttyACM0 -opost
```

確認：

```text
-opost
```

確實存在。

但是再次測試，STM32 仍然收到：

```text
0D 0D 0A
```

因此：

> 單純依靠 Shell 中的 `stty -opost` 不是可靠的正式解決方案。

---

# 27. 找到真正的工程問題

進一步檢查 CM5 的：

```text
usb_transport.cpp
```

發現原始：

```cpp
UsbTransport::open()
```

只負責：

```cpp
open("/dev/ttyACM0", O_RDWR | O_NOCTTY)
```

卻沒有自行設定：

```text
termios
```

因此 USB Transport 實際上依賴：

```text
當時 Linux TTY 的狀態
```

這是一個不應該存在的隱性 dependency。

---

# 28. 正式解決方法

CM5 `UsbTransport::open()` 改為：

```cpp
struct termios tty;

tcgetattr(fd_, &tty);

cfmakeraw(&tty);

cfsetispeed(&tty, B115200);
cfsetospeed(&tty, B115200);

tty.c_cflag |= (CLOCAL | CREAD);
tty.c_cflag &= ~CSTOPB;
tty.c_cflag &= ~CRTSCTS;
tty.c_cflag &= ~PARENB;
tty.c_cflag &= ~CSIZE;
tty.c_cflag |= CS8;

tcsetattr(fd_, TCSANOW, &tty);
```

核心是：

```cpp
cfmakeraw(&tty);
```

讓 USB Transport 使用 raw TTY mode。

---

# 29. 修正前後比較

## 修正前

Application：

```text
MOV R 1000 1000 500 0 0 0\r\n
```

STM32：

```text
... 0D 0D 0A
```

也就是：

```text
\r\r\n
```

結果：

```text
Command length / framing
受到干擾
```

---

## 修正後

Application：

```text
MOV R 1000 1000 500 0 0 0\r\n
```

STM32：

```text
... 0D 0A
```

也就是：

```text
\r\n
```

結果：

```text
command_length = 27
received_command_length = 25
expected_length = 25
```

完全正確。

---

# 30. 最終 CM5 → STM32 測試結果

STM32 COM3：

```text
[USB RX] Len=27
[USB RX] Data HEX:
4D 4F 56 20 52 20 31 30 30 30 20 31 30 30 30 20
35 30 30 20 30 20 30 20 30 0D 0A

[USB RX] CRLF detected, command_length=27
[USB RX] expected_length=25, received_command_length=25
```

這證明 STM32 收到的資料為：

```text
MOV R 1000 1000 500 0 0 0\r\n
```

沒有多出的：

```text
\r
```

---

# 31. 最終 STM32 → CM5 測試結果

STM32 回傳：

```text
DONE\r\n
```

CM5：

```text
Command sent
Response: DONE
```

因此雙向通訊：

```text
CM5 → STM32
        PASS

STM32 → CM5
        PASS
```

---

# 32. 完整測試資料流程

```text
┌──────────────────────────────────────────┐
│ Raspberry Pi CM5                         │
│                                          │
│ main.cpp                                 │
│     │                                    │
│     ▼                                    │
│ UsbTransport::sendCommand()              │
│     │                                    │
│     ▼                                    │
│ write(/dev/ttyACM0)                      │
│     │                                    │
│     ▼                                    │
│ Linux CDC ACM                            │
└───────────────┬──────────────────────────┘
                │ USB
                ▼
┌──────────────────────────────────────────┐
│ STM32H755                                │
│                                          │
│ USB CDC                                  │
│     │                                    │
│     ▼                                    │
│ CDC_Receive_FS()                         │
│     │                                    │
│     ▼                                    │
│ command_buffer                           │
│     │                                    │
│     ▼                                    │
│ CRLF detection                           │
│     │                                    │
│     ▼                                    │
│ Command Parser                           │
│     │                                    │
│     ▼                                    │
│ CDC_Transmit_FS()                        │
└───────────────┬──────────────────────────┘
                │ USB
                ▼
┌──────────────────────────────────────────┐
│ Raspberry Pi CM5                         │
│                                          │
│ /dev/ttyACM0                             │
│     │                                    │
│     ▼                                    │
│ UsbTransport::receiveResponse()          │
│     │                                    │
│     ▼                                    │
│ Response: DONE                           │
└──────────────────────────────────────────┘
```

---

# 33. 本專案的重要工程結論

## 33.1 USB CDC Callback 不等於 Application Packet

永遠不要假設：

```text
write()
=
CDC_Receive_FS()
```

Command 必須透過 Application Layer framing 組合。

---

## 33.2 CRLF 是 Application Protocol 的一部分

目前 Command Protocol 使用：

```text
CRLF = \r\n
```

因此 Transport Layer 必須保持 byte transparency。

---

## 33.3 CM5 TTY 必須由 Transport 自己設定

不能要求使用者先執行：

```bash
stty -F /dev/ttyACM0 -opost
```

Transport 應自行：

```text
open
 ↓
termios
 ↓
raw mode
 ↓
communication
```

這樣才能避免環境依賴。

---

## 33.4 USB 與 SPI2 不應混合

本 USB 通道：

```text
Command / Response
```

SPI2：

```text
Real-time trajectory data
```

兩者具有不同的用途與時間特性。

---

# 34. 目前測試限制

目前 STM32 程式是 Bring-Up Test，因此尚未完成正式 Command System。

目前：

```text
Command = 固定測試 Command
Response = 固定 DONE\r\n
```

尚未加入完整：

* Command syntax parser
* Parameter validation
* Command queue
* Timeout handling
* Invalid command response
* Multiple command processing
* USB disconnect/reconnect handling
* TX queue
* `USBD_BUSY` retry mechanism
* 長時間壓力測試

這些應留到正式 Command Parser 階段處理。

---

# 35. 與 CM5 `usb_command_test` 的關係

CM5 端：

```text
usb_command_test
```

負責驗證：

```text
Linux
USB CDC ACM
/dev/ttyACM0
UsbTransport
Command TX
Response RX
```

STM32 端：

```text
STM32H755 USB CDC Command Test
```

負責驗證：

```text
USB Device
CDC
CDC_Receive_FS()
Command Buffer
CRLF detection
Command parsing
CDC_Transmit_FS()
```

兩邊組合起來：

```text
CM5 usb_command_test
        │
        │ USB CDC ACM
        ▼
STM32H755 USB CDC Test
```

共同完成完整的 End-to-End Test。

---

# 36. 最終驗證項目

| 項目                         | 結果       |
| -------------------------- | -------- |
| STM32 USB_OTG_FS           | PASS     |
| USB Device Only            | PASS     |
| USB CDC Class              | PASS     |
| HSI48 / USB 48 MHz         | PASS     |
| USB Full Speed             | PASS     |
| CM5 USB Enumeration        | PASS     |
| `/dev/ttyACM0`             | PASS     |
| CM5 → STM32 Command        | PASS     |
| STM32 Command Buffer       | PASS     |
| CRLF detection             | PASS     |
| Command content validation | PASS     |
| STM32 → CM5 Response       | PASS     |
| `DONE\r\n`                 | PASS     |
| Linux 額外 `\r` 問題           | 已定位      |
| `termios` raw mode 修正      | PASS     |
| End-to-End USB Command     | **PASS** |

---

# 37. Bring-Up 完成狀態

目前可以正式認定：

```text
STM32H755 USB CDC ACM Command Transport
```

已完成基本雙向驗證。

成功資料流：

```text
CM5
 │
 │ MOV R 1000 1000 500 0 0 0\r\n
 ▼
STM32H755
 │
 │ Command Parser
 │
 │ DONE\r\n
 ▼
CM5
```

CM5 最終：

```text
Command sent
Response: DONE
```

---

# 38. 下一階段

USB CDC Bring-Up 完成後，不重新設計這條已驗證的 USB Transport。

下一階段將進入：

```text
Gmt_CMD_Parser
```

整合。

目標架構：

```text
Windows TCP Client
        │
        │ TCP/IP
        ▼
Raspberry Pi CM5
        │
        ▼
TCP Server
        │
        ▼
Command Parser
        │
        ▼
UsbTransport
        │
        ▼
/dev/ttyACM0
        │
        ▼
USB CDC ACM
        │
        ▼
STM32H755
```

STM32 端則逐步將目前固定測試：

```text
MOV R 1000 1000 500 0 0 0
```

替換成正式的 GMT Command Parser。

---

# 39. 最重要的除錯紀錄

本專案最值得保留的問題是：

```text
預期：

...\r\n

實際：

...\r\r\n
```

最終確認：

```text
不是 Application 自己多送一個 \r
不是 Command Parser 自己產生 \r
不是 USB CDC Device 自己產生 \r
```

真正需要修正的是：

```text
CM5 Linux TTY configuration
```

而正式解法是讓：

```text
UsbTransport::open()
```

自行建立：

```text
raw TTY
```

而不是依賴：

```bash
stty
```

這項修正對後續：

```text
Gmt_CMD_Parser
```

的正式整合非常重要。

---

# 40. GitHub 保存目的

本 Repository 應保留為：

```text
STM32H755 USB CDC ACM Bring-Up Reference
```

用途包括：

1. 保存 CubeMX USB 設定。
2. 保存 STM32 CDC 程式修改。
3. 保存 Command Buffer / CRLF parser 的實作。
4. 保存 CM5 ↔ STM32 的測試方式。
5. 保存 Linux TTY `\r` 問題的完整除錯過程。
6. 保存已驗證成功的 USB Transport 架構。
7. 作為未來 `Gmt_CMD_Parser` 整合時的參考版本。

---

# 41. 最終結論

本專案已完成 STM32H755 與 Raspberry Pi CM5 之間的：

```text
USB CDC ACM Bidirectional Command Transport
```

驗證。

最終結果：

```text
CM5
 │
 │ "MOV R 1000 1000 500 0 0 0\r\n"
 ▼
STM32H755
 │
 │ Command Buffer
 │ CRLF Detection
 │ Command Validation
 │
 │ "DONE\r\n"
 ▼
CM5
```

測試成功：

```text
Command sent
Response: DONE
```

因此目前 USB CDC ACM 可以視為：

> **已驗證成功、可供後續 `Gmt_CMD_Parser` 整合的 STM32H755 ↔ Raspberry Pi CM5 Command Transport。**

後續整合應遵守：

> **不重新設計已驗證的 USB CDC Transport，只在其上層加入正式 TCP Server、Command Parser 與 GMT Command Protocol。**

---
## 圖片

![STM32CubeMX_1](images/USB_OTG_FS.png)

![STM32CubeMX_1](images/USB_DEVICE_M7.png)

![USB_Connection](images/STM32H7_CM5_USB_Connection.png)

![Micro-B_USB](images/Micro-B_USB.png)

---
## 關建檔案：
- CM7\Core\Src\main.c
- CM7\USB_DEVICE\App\usbd_cdc_if.c
