# sPOD BLE Controller — Android App

An Android application written in Kotlin that connects to an **ESP32-based sPOD PCM board** over Bluetooth Low Energy (BLE) to control 8 independent circuits.

---

## Features

- **BLE Device Scanner** — Discover nearby BLE devices with signal strength (RSSI) display
- **Connection Management** — Full GATT lifecycle: connect → discover services → enable notifications → ready
- **8-Circuit Dashboard** — Grid of toggle cards to control each circuit individually
- **All ON / All OFF** — Bulk control buttons
- **Auto-Reconnect** — Exponential backoff reconnection on unexpected disconnect (up to 3 attempts)
- **Debug Log Panel** — Live TX/RX packet log with hex display
- **Permission Handling** — Supports Android 12+ (`BLUETOOTH_SCAN`/`BLUETOOTH_CONNECT`) and Android ≤11 (`ACCESS_FINE_LOCATION`)

---

## Requirements

- Android Studio Hedgehog (2023.1) or newer
- Android device running API 26+ (Android 8.0 Oreo)
- ESP32 PCM board running the sPOD BLE firmware

---

## Quick Start

### 1. Configure BLE UUIDs

Before building, update the UUIDs in `BleConstants.kt` to match your PCM hardware:

```kotlin
// app/src/main/java/com/spod/blecontroller/ble/BleConstants.kt

val SERVICE_UUID: UUID = UUID.fromString("0000ffe0-0000-1000-8000-00805f9b34fb")
val CHARACTERISTIC_UUID: UUID = UUID.fromString("0000ffe1-0000-1000-8000-00805f9b34fb")
```

**To find your device's UUIDs:**
1. Install [nRF Connect for Mobile](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) on Android
2. Power on your PCM board
3. Scan and connect to it in nRF Connect
4. Note the **Service UUID** and the **writable Characteristic UUID**
5. Replace the placeholder UUIDs in `BleConstants.kt`

### 2. Build & Run

```bash
cd android-app
./gradlew assembleDebug
```

Or open in Android Studio and click **Run ▶**.

---

## BLE Protocol

### Command Packet Format

The app communicates with the PCM board using a **single-byte command**:

| Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|-------|-------|-------|-------|-------|-------|-------|-------|
| Circuit 8 | Circuit 7 | Circuit 6 | Circuit 5 | Circuit 4 | Circuit 3 | Circuit 2 | Circuit 1 |

- **1** = Circuit ON
- **0** = Circuit OFF

**Examples:**
- `0x00` (`0b00000000`) — All circuits OFF
- `0xFF` (`0b11111111`) — All circuits ON
- `0x05` (`0b00000101`) — Circuits 1 and 3 ON, rest OFF
- `0x0F` (`0b00001111`) — Circuits 1–4 ON

### BLE Flow

```
Android App                    PCM Board (ESP32)
    |                               |
    |------- BLE Scan ------------->|  (discovers device)
    |                               |
    |------- GATT Connect --------->|
    |<------ Connected -------------|
    |                               |
    |------- Discover Services ---->|
    |<------ Services List ---------|
    |                               |
    |------- Enable Notifications ->|  (write CCCD descriptor)
    |<------ Descriptor Written ----|
    |                               |
    |  *** CONNECTION READY ***     |
    |                               |
    |------- Write Characteristic ->|  (send circuit byte)
    |<------ Write Confirmed -------|
    |                               |
    |<------ Notification ---------|  (optional: PCM sends state back)
```

### Characteristic Write Type

The app uses **Write With Response** (`WRITE_TYPE_DEFAULT`) to confirm each command was received by the PCM board.

---

## Project Structure

```
app/src/main/
├── java/com/spod/blecontroller/
│   ├── SpodApplication.kt          # App entry, initializes Logger + BleManager
│   ├── ble/
│   │   ├── BleManager.kt           # Singleton: scan, connect, read/write
│   │   ├── BleConstants.kt         # UUIDs, timeouts, command constants
│   │   └── ConnectionState.kt      # Enum: DISCONNECTED → CONNECTING → READY
│   ├── model/
│   │   ├── BleDevice.kt            # Scanned device data model
│   │   └── CircuitState.kt         # Circuit state model + byte conversion
│   ├── ui/
│   │   ├── scan/
│   │   │   ├── ScanActivity.kt     # BLE scan UI, permission handling
│   │   │   ├── ScanViewModel.kt    # Scan state management
│   │   │   └── DeviceAdapter.kt    # RecyclerView adapter for device list
│   │   ├── connection/
│   │   │   ├── ConnectionActivity.kt  # Connection progress UI
│   │   │   └── ConnectionViewModel.kt # Connection logic
│   │   └── control/
│   │       ├── ControlActivity.kt  # 8-circuit dashboard
│   │       ├── ControlViewModel.kt # Circuit toggle + BLE write logic
│   │       └── CircuitAdapter.kt   # RecyclerView adapter for circuit grid
│   └── util/
│       ├── Logger.kt               # In-memory log buffer + Logcat
│       ├── PermissionHelper.kt     # Android 12+ / legacy BLE permissions
│       └── Extensions.kt           # Kotlin extension functions
└── res/
    ├── layout/                     # XML layouts for all screens
    ├── values/                     # strings, colors, themes, dimens
    └── drawable/                   # Vector icons
```

---

## Permissions

| Permission | Version | Purpose |
|---|---|---|
| `BLUETOOTH_SCAN` | Android 12+ | Scan for BLE devices |
| `BLUETOOTH_CONNECT` | Android 12+ | Connect to BLE devices |
| `BLUETOOTH` | ≤ Android 11 | Legacy Bluetooth access |
| `BLUETOOTH_ADMIN` | ≤ Android 11 | Enable/disable Bluetooth |
| `ACCESS_FINE_LOCATION` | ≤ Android 11 | Required for BLE scan on older Android |

---

## Troubleshooting

### Device not appearing in scan
- Ensure the PCM board is powered on and advertising
- Verify Bluetooth is enabled on your phone
- On Android ≤11, ensure Location is enabled (required by Android for BLE scanning)
- Try moving closer to the device (RSSI < -90 dBm may cause missed advertisements)

### Connection fails immediately
- Verify `SERVICE_UUID` in `BleConstants.kt` matches the PCM board's advertised service
- Check Logcat with tag `SPOD_BLE` for detailed error messages
- Some devices require pairing first — check bond state in the scan list

### Characteristic write fails
- Verify `CHARACTERISTIC_UUID` is correct and has WRITE properties
- Ensure the PCM firmware is running and accepting commands
- Check the debug log panel in the Control screen for write errors

### Auto-reconnect not working
- Ensure "Auto-Reconnect" toggle is enabled in the Control screen
- The app attempts reconnect up to 3 times with exponential backoff (1s, 2s, 4s)
- Check Logcat for reconnect attempt logs

---

## Logcat Debugging

All app events are logged to Logcat under the tag `SPOD_BLE`:

```bash
adb logcat -s SPOD_BLE
```

Log format: `[HH:mm:ss.SSS][LEVEL][TAG] message`

Log levels:
- `INFO` — Connection state changes, scan events
- `DBUG` — Device discovery, verbose details
- `ERR!` — Errors and failures
- `TX  ` — Bytes sent to PCM board
- `RX  ` — Bytes received from PCM board

---

## Testing Without Hardware

To test the UI without a real PCM board:

1. Use [nRF Connect for Mobile](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) to simulate a BLE peripheral
2. Create a custom service with UUID `0000ffe0-0000-1000-8000-00805f9b34fb`
3. Add a characteristic `0000ffe1-...` with Read/Write/Notify properties
4. Connect from the app and send circuit toggle commands

Alternatively, use the [LightBlue](https://play.google.com/store/apps/details?id=com.punchthrough.lightblueexplorer) app as a virtual peripheral.

---

## License

This project is part of the sPOD Project. See the root repository LICENSE for details.
