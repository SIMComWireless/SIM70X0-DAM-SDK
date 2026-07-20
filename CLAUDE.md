# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

SIM70X0 DAM SDK (TX1.0) — a C embedded SDK for building downloadable application modules (DAM) that run natively on SIMCom SIM7070/SIM7080/SIM7090 LTE Cat-M1/NB-IoT/GSM cellular modules (Qualcomm MDM9205 chipset, ARM Cortex-A7). The SDK uses ThreadX (Azure RTOS) and Qualcomm's QAPI hardware abstraction layer.

## Build Commands

**Windows:**
```bash
# Build (default project name: app)
build_llvm_mk.bat

# Build with custom project name
build_llvm_mk.bat -p myproject

# Clean build
build_llvm_mk.bat -c
```

**Linux:**
```bash
./build_llvm_mk.sh
./build_llvm_mk.sh -p myproject
./build_llvm_mk.sh -c
```

**Output:** `bin/cust_app.bin` — flashed to the module via SIM7070 MultiUpdateTool over UART/USB AT port.

**Prerequisites:**
- Snapdragon LLVM 4.0.11 toolchain (path configured in `tools/opt.mak`)
- Python 2.7 (required for ELF-to-binary conversion)
- Bundled GNU utilities in `tools/` on Windows (no MinGW/Cygwin needed)

## Build System

- **Top-level:** `Makefile` includes `tools/opt.mak` (toolchain paths) and `src/app/cust.mak` (source/include paths)
- **Compiler:** `clang.exe` targeting `armv7m-none-musleabi` with flags: `-marm -target armv7m-none-musleabi -mfloat-abi=softfp -mfpu=none -mcpu=cortex-a7 -Osize`
- **Linker:** `clang++.exe` with script at `src/build/cust_app.ld`
- **Binary conversion:** `llvm-elf-to-hex.py` produces raw binary from ELF

**Adding new source files:** Edit `src/app/cust.mak` — add directories to `CUST_SRC_PATH` and include paths to `CUST_INCLUDE_PATH`.

## Architecture

**Boot sequence:**
1. Module firmware loads `cust_app.bin`, parses assembly preamble (`src/app/asm/txm_module_preamble_llvm.S`)
2. Entry point `qcli_dam_app_start()` in `src/app/application/pal_module.c` is called
3. `pal_module.c` initializes debug UART, allocates 128 KB byte pool, creates semaphores, spawns demo threads

**Thread model:** Each demo runs as a separate ThreadX thread (4-12 KB stack, priority 150-164). Enable/disable demos via `#define` switches in `pal_module.c`. Thread entry functions are in `src/app/application/ThreadX.c`.

**Key source layout:**
| Path | Purpose |
|---|---|
| `src/app/application/pal_module.c` | Entry point, resource allocation, thread creation |
| `src/app/application/ThreadX.c/.h` | Thread entry functions calling demo modules |
| `src/app/demo/` | Self-contained demo modules (GPIO, I2C, ADC, UART, MQTT, sockets, HTTP, GPS, OTA, etc.) |
| `src/app/Easylogger/` | Logging framework ported to ThreadX |
| `src/app/asm/` | ARM assembly preamble (module metadata) |
| `include/qapi/` | Qualcomm QAPI headers (~60+ — GPIO, UART, SPI, I2C, ADC, sockets, SSL, HTTP, MQTT, GNSS, etc.) |
| `include/threadx_api/` | ThreadX RTOS API headers |
| `include/qmi/` | QMI client headers |
| `include/src/` | QAPI user-layer glue source files |
| `libs/` | Prebuilt binary libraries (ThreadX, QAPI DAM, QCCI, diagnostics, etc.) |

**API layers:**
- **QAPI** — Qualcomm hardware abstraction (GPIO, UART, SPI, I2C, ADC, sockets, SSL, HTTP, MQTT, filesystem, GNSS)
- **ThreadX** — RTOS primitives (threads, mutexes, semaphores, queues, event flags, memory pools)
- **AT Commands** — Modem operations via `IOT_Visual_AT_Input()`/`IOT_Visual_AT_Output()`

**Network demos pattern:** Network-dependent demos (socket, MQTT, HTTP, GPS, OTA) all wait on `Net_Active_semaphore` for PDN activation before operating.

## Key Constraints

- The `libs/` directory and `include/qapi/` headers are provided by Qualcomm/SIMCom — do not modify prebuilt libraries
- The assembly preamble must declare `qcli_dam_app_start` as the entry point
- Thread stacks are limited (4-12 KB) — watch stack usage in demo threads
- The SDK is Windows-first; Linux support requires adjusting toolchain paths in `tools/opt.mak`
