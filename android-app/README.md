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

### 1. BLE UUIDs (already configured)

The app uses the UUIDs derived directly from the PCM firmware source
(`pcm/ble.cpp`, `ble_attUuid128` array). No changes needed unless the
firmware is modified.

| Constant | UUID | Purpose |
|---|---|---|
| `SERVICE_UUID` | `7e3af9ec-8c0d-447b-8404-e99f6056685b` | sPOD primary service |
| `CHARACTERISTIC_UUID` | `b9064764-4c00-4c6f-a65b-ec02646fc6f4` | COMM characteristic (circuit control) |
| `UNSECURED_CHARACTERISTIC_UUID` | `a0ba57d9-7b58-4536-82e8-cca1cbfd6cde` | Security/passkey characteristic |

### 2. PCM Address (multi-board setups)

If you have more than one PCM board on the RS-485 bus, update the address in
`BleConstants.kt`:

```kotlin
const val PCM_ADDRESS: Int = 0   // Change to 1, 2, or 3 for additional boards
```

### 3. Build & Run

```bash
cd android-app
./gradlew assembleDebug
```

Or open in Android Studio and click **Run ▶**.

---

## Authentication / Pairing

**The PCM firmware enforces BLE security.** All writes to the COMM characteristic are
silently dropped unless `authGood` is set in the firmware, which only happens after
a successful BLE pairing or passkey exchange.

### Secured mode (default firmware config)

The firmware boots with:
```cpp
BLEDevice::setSecurityAuth(true /*bonding*/, true /*MITM*/, true /*SC*/);
BLEDevice::setSecurityPasskey(123456);
BLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
```

When you connect for the first time:
1. Android will show a system pairing dialog
2. **Enter passkey `123456`** when prompted
3. After pairing completes, `authGood` is set and circuit commands work

### Unsecured mode

If the device has `status.isUnsecured = true` (set via device configuration), the
app must write a passkey packet to the `UNSECURED_CHARACTERISTIC_UUID` before
commands are accepted. See `processSecurityPacket()` in `pcm/ble.cpp` for the
packet format.

> **Note:** If circuit toggle commands appear to be sent (the app shows TX logs)
> but the PCM board does not respond, the most likely cause is that pairing has
> not been completed and `authGood` is still `false` on the firmware side.

---

## BLE Protocol

### Complete BLE Flow

```
Android App                        PCM Board (ESP32 / NimBLE)
    |                                       |
    |------- BLE Scan ─────────────────────>|  advertises "sPOD Link #XXXXXXXX"
    |<------ Advertisement (service UUID) --|
    |                                       |
    |------- GATT Connect ─────────────────>|  TRANSPORT_LE
    |<------ Connected ─────────────────────|
    |                                       |
    |      *** BLE PAIRING ***              |
    |------- Pair Request ─────────────────>|  Android initiates
    |        (passkey = 123456)             |
    |<------ Auth Complete ─────────────────|  authGood = true in firmware
    |                                       |
    |------- Discover Services ────────────>|
    |<------ Services List ─────────────────|
    |                                       |
    |------- Find COMM Characteristic ─────>|  UUID: b9064764-...
    |<------ Characteristic Found ──────────|
    |                                       |
    |------- Write CCCD (enable notify) ───>|  0x01 0x00 → CCCD descriptor
    |<------ Descriptor Written ────────────|
    |                                       |
    |  *** CONNECTION READY — COMMANDS ACCEPTED ***
    |                                       |
    |------- Write Characteristic ─────────>|  14 bytes × 8 circuits per write
    |<------ Write Response ────────────────|  confirms receipt
    |                                       |
    |<------ Notification ──────────────────|  PCM sends status updates
```

### Circuit Command Packet Format

Each circuit toggle generates one 14-byte CAN packet. All 8 circuit packets
are concatenated and sent in a single BLE write (the firmware's `processCOMMData()`
parses multiple `0x55`-delimited packets from one write).

```
Offset  Value        Description
──────  ───────────  ─────────────────────────────────────────────────────
  [0]   0x55         Packet delimiter — signals start of packet
  [1]   0x0C (12)    Length field = total packet length − 2
  [2]   0x00         Packet type = CAN_PACKET
  [3]   0x00         Unused spacer byte
  [4]   0x00         Unused spacer byte
  [5]   0x80 | addr  SWITCH_PACKET flag (0x80) OR'd with PCM address (0–3)
  [6]   canEncoding  Circuit number in CAN bus encoding (see table below)
  [7]   outCmd       0xFF = ON,  0x00 = OFF
  [8]   0x00         blinkOn  — 0 = no blinking
  [9]   0x00         blinkOff — 0 = no blinking
[10]    CRC & 0xFF   CRC32 LSB  ┐
[11]    CRC >> 8     CRC32      │  IEEE 802.3 CRC32 of bytes [0..9],
[12]    CRC >> 16    CRC32      │  stored little-endian
[13]    CRC >> 24    CRC32 MSB  ┘
```

### CAN Circuit Encoding

The firmware uses a non-sequential bit-mask encoding for circuit indices
(see `libraries/spod_library/src/common.cpp :: iToCan()`):

| Circuit (0-based) | UI Label | CAN encoding byte |
|---|---|---|
| 0 | Circuit 1 | `0x08` |
| 1 | Circuit 2 | `0x10` |
| 2 | Circuit 3 | `0x20` |
| 3 | Circuit 4 | `0x40` |
| 4 | Circuit 5 | `0x80` |
| 5 | Circuit 6 | `0x01` |
| 6 | Circuit 7 | `0x02` |
| 7 | Circuit 8 | `0x04` |

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
- Check Logcat with tag `SPOD_BLE` for detailed error messages
- Some Android versions may show a pairing dialog — accept it and enter passkey `123456`

### Commands sent but circuits do not respond
This is almost always an authentication issue. The firmware silently drops all
COMM writes if `authGood` is false:
1. Check that BLE pairing completed successfully (bond state should be BONDED)
2. If previously bonded but not working, try removing the device from Android's
   Bluetooth settings and re-pair
3. Enable Logcat (`adb logcat -s SPOD_BLE`) and look for
   `"comm::onWrite() -- authGood=0"` — if present, pairing is needed

### Characteristic write fails
- Ensure `CHARACTERISTIC_UUID` matches the COMM characteristic
- Verify connection state is `READY` (after notification enable completes)
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

1. Use [nRF Connect for Mobile](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) to simulate a BLE peripheral
2. Create a custom service with UUID `7e3af9ec-8c0d-447b-8404-e99f6056685b`
3. Add a characteristic `b9064764-4c00-4c6f-a65b-ec02646fc6f4` with Read/Write/Notify properties
4. Connect from the app — note: simulated peripherals do not require pairing, so `authGood`
   enforcement is bypassed in hardware testing only

---

## License

This project is part of the sPOD Project. See the root repository LICENSE for details.
