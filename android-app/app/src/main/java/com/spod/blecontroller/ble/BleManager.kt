package com.spod.blecontroller.ble

import android.annotation.SuppressLint
import android.bluetooth.*
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.os.Handler
import android.os.Looper
import com.spod.blecontroller.model.BleDevice
import com.spod.blecontroller.util.Logger
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

/**
 * Singleton BLE Manager that handles the full BLE lifecycle:
 * Scan → Connect → Discover Services → Read/Write → Disconnect
 *
 * All BLE state is exposed via StateFlow for reactive UI updates.
 * Thread safety is maintained using synchronized blocks and coroutines.
 */
@SuppressLint("MissingPermission") // Permissions are checked before calling BLE APIs
object BleManager {

    private const val TAG = "BleManager"

    // ── State Flows ──────────────────────────────────────────────────────────

    private val _connectionState = MutableStateFlow(ConnectionState.DISCONNECTED)
    /** Observe the current BLE connection state */
    val connectionState: StateFlow<ConnectionState> = _connectionState.asStateFlow()

    private val _scannedDevices = MutableStateFlow<List<BleDevice>>(emptyList())
    /** Observe the list of discovered BLE devices during scanning */
    val scannedDevices: StateFlow<List<BleDevice>> = _scannedDevices.asStateFlow()

    private val _circuitStates = MutableStateFlow<Byte>(0x00)
    /** Observe the current circuit states as a byte (each bit = one circuit) */
    val circuitStates: StateFlow<Byte> = _circuitStates.asStateFlow()

    private val _txLog = MutableStateFlow<List<String>>(emptyList())
    /** Observe transmitted (sent) BLE packet log */
    val txLog: StateFlow<List<String>> = _txLog.asStateFlow()

    private val _rxLog = MutableStateFlow<List<String>>(emptyList())
    /** Observe received BLE packet log */
    val rxLog: StateFlow<List<String>> = _rxLog.asStateFlow()

    private val _isScanning = MutableStateFlow(false)
    /** Observe whether BLE scanning is active */
    val isScanning: StateFlow<Boolean> = _isScanning.asStateFlow()

    // ── Internal State ───────────────────────────────────────────────────────

    private var bluetoothGatt: BluetoothGatt? = null
    private var targetCharacteristic: BluetoothGattCharacteristic? = null
    private var applicationContext: Context? = null
    private var scanner: BluetoothLeScanner? = null
    private val mainHandler = Handler(Looper.getMainLooper())
    private val coroutineScope = CoroutineScope(Dispatchers.IO + SupervisorJob())

    /** Auto-reconnect settings */
    private var autoReconnectEnabled = true
    private var reconnectAttempts = 0
    private var lastConnectedDevice: BluetoothDevice? = null

    /** Timeout job handles */
    private var scanTimeoutJob: Job? = null
    private var connectionTimeoutJob: Job? = null

    // ── Initialization ───────────────────────────────────────────────────────

    /**
     * Initialize the BleManager with an application context.
     * Call this from Application.onCreate() or before first use.
     */
    fun init(context: Context) {
        applicationContext = context.applicationContext
        Logger.info(TAG, "BleManager initialized")
    }

    // ── Scanning ─────────────────────────────────────────────────────────────

    /**
     * Start scanning for nearby BLE devices.
     * Automatically stops after [BleConstants.SCAN_TIMEOUT_MS] milliseconds.
     * Duplicate devices are filtered; RSSI is updated for known devices.
     */
    fun startScan() {
        val context = applicationContext ?: run {
            Logger.error(TAG, "BleManager not initialized")
            return
        }

        val bluetoothManager = context.getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager
        val adapter = bluetoothManager?.adapter

        if (adapter == null || !adapter.isEnabled) {
            Logger.error(TAG, "Bluetooth is disabled or unavailable")
            return
        }

        scanner = adapter.bluetoothLeScanner
        if (scanner == null) {
            Logger.error(TAG, "BLE scanner not available")
            return
        }

        _scannedDevices.value = emptyList()
        _isScanning.value = true

        Logger.info(TAG, "Starting BLE scan")
        scanner?.startScan(scanCallback)

        scanTimeoutJob?.cancel()
        scanTimeoutJob = coroutineScope.launch {
            delay(BleConstants.SCAN_TIMEOUT_MS)
            stopScan()
            Logger.info(TAG, "Scan stopped after timeout")
        }
    }

    /**
     * Stop an active BLE scan.
     */
    fun stopScan() {
        scanTimeoutJob?.cancel()
        scanner?.stopScan(scanCallback)
        scanner = null
        _isScanning.value = false
        Logger.info(TAG, "BLE scan stopped")
    }

    /** Callback for BLE scan results */
    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val device = result.device
            val name = device.name ?: "Unknown Device"
            val address = device.address
            val rssi = result.rssi
            val bondState = device.bondState

            val bleDevice = BleDevice(name = name, address = address, rssi = rssi, bondState = bondState)

            val currentList = _scannedDevices.value.toMutableList()
            val existingIndex = currentList.indexOfFirst { it.address == address }
            if (existingIndex >= 0) {
                currentList[existingIndex] = bleDevice
            } else {
                currentList.add(bleDevice)
                Logger.debug(TAG, "Discovered device: $name [$address] RSSI: $rssi")
            }
            _scannedDevices.value = currentList
        }

        override fun onScanFailed(errorCode: Int) {
            Logger.error(TAG, "Scan failed with error code: $errorCode")
            _isScanning.value = false
        }
    }

    // ── Connection ───────────────────────────────────────────────────────────

    /**
     * Connect to a BLE device using GATT.
     *
     * @param context Activity or application context
     * @param device The BluetoothDevice to connect to
     */
    fun connect(context: Context, device: BluetoothDevice) {
        if (_connectionState.value == ConnectionState.CONNECTING ||
            _connectionState.value == ConnectionState.CONNECTED) {
            Logger.warn(TAG, "Already connecting or connected")
            return
        }

        lastConnectedDevice = device
        reconnectAttempts = 0
        performConnect(context, device)
    }

    private fun performConnect(context: Context, device: BluetoothDevice) {
        Logger.info(TAG, "Connecting to ${device.address}")
        _connectionState.value = ConnectionState.CONNECTING

        closeGatt()

        bluetoothGatt = device.connectGatt(
            context,
            false,
            gattCallback,
            BluetoothDevice.TRANSPORT_LE
        )

        connectionTimeoutJob?.cancel()
        connectionTimeoutJob = coroutineScope.launch {
            delay(BleConstants.CONNECTION_TIMEOUT_MS)
            if (_connectionState.value == ConnectionState.CONNECTING) {
                Logger.error(TAG, "Connection timed out")
                _connectionState.value = ConnectionState.FAILED
                closeGatt()
            }
        }
    }

    /**
     * Disconnect from the current BLE device gracefully.
     */
    fun disconnect() {
        autoReconnectEnabled = false
        _connectionState.value = ConnectionState.DISCONNECTING
        Logger.info(TAG, "Disconnecting...")
        bluetoothGatt?.disconnect()
    }

    private fun closeGatt() {
        bluetoothGatt?.close()
        bluetoothGatt = null
        targetCharacteristic = null
    }

    // ── GATT Callback ────────────────────────────────────────────────────────

    private val gattCallback = object : BluetoothGattCallback() {

        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            connectionTimeoutJob?.cancel()

            when (newState) {
                BluetoothProfile.STATE_CONNECTED -> {
                    Logger.info(TAG, "GATT connected, discovering services...")
                    _connectionState.value = ConnectionState.CONNECTED
                    reconnectAttempts = 0

                    mainHandler.post {
                        gatt.discoverServices()
                        _connectionState.value = ConnectionState.DISCOVERING_SERVICES
                    }
                }

                BluetoothProfile.STATE_DISCONNECTED -> {
                    Logger.info(TAG, "GATT disconnected, status: $status")
                    val wasReady = _connectionState.value == ConnectionState.READY ||
                            _connectionState.value == ConnectionState.CONNECTED ||
                            _connectionState.value == ConnectionState.SERVICES_DISCOVERED

                    closeGatt()

                    if (_connectionState.value == ConnectionState.DISCONNECTING) {
                        _connectionState.value = ConnectionState.DISCONNECTED
                    } else {
                        _connectionState.value = ConnectionState.DISCONNECTED
                        if (wasReady && autoReconnectEnabled) {
                            scheduleReconnect()
                        }
                    }
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Logger.error(TAG, "Service discovery failed with status: $status")
                _connectionState.value = ConnectionState.FAILED
                return
            }

            Logger.info(TAG, "Services discovered")
            _connectionState.value = ConnectionState.SERVICES_DISCOVERED

            val service = gatt.getService(BleConstants.SERVICE_UUID)
            if (service == null) {
                Logger.error(TAG, "Target service not found: ${BleConstants.SERVICE_UUID}")
                _connectionState.value = ConnectionState.FAILED
                return
            }

            val characteristic = service.getCharacteristic(BleConstants.CHARACTERISTIC_UUID)
            if (characteristic == null) {
                Logger.error(TAG, "Target characteristic not found: ${BleConstants.CHARACTERISTIC_UUID}")
                _connectionState.value = ConnectionState.FAILED
                return
            }

            targetCharacteristic = characteristic
            Logger.info(TAG, "Target characteristic found")

            enableNotifications(gatt, characteristic)
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                val hex = characteristic.value?.joinToString(" ") { "0x%02X".format(it) } ?: ""
                Logger.tx(TAG, "Write success: $hex")
                addTxLog(hex)
            } else {
                Logger.error(TAG, "Write failed with status: $status")
            }
        }

        @Suppress("DEPRECATION")
        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic
        ) {
            val data = characteristic.value ?: return
            val hex = data.joinToString(" ") { "0x%02X".format(it) }
            Logger.rx(TAG, "Notification received: $hex")
            addRxLog(hex)

            if (data.isNotEmpty()) {
                _circuitStates.value = data[0]
            }
        }

        override fun onDescriptorWrite(
            gatt: BluetoothGatt,
            descriptor: BluetoothGattDescriptor,
            status: Int
        ) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Logger.info(TAG, "Notifications enabled successfully")
                _connectionState.value = ConnectionState.READY
            } else {
                Logger.error(TAG, "Failed to enable notifications, status: $status")
                _connectionState.value = ConnectionState.READY
            }
        }
    }

    // ── Notifications ────────────────────────────────────────────────────────

    @Suppress("DEPRECATION")
    private fun enableNotifications(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic) {
        val success = gatt.setCharacteristicNotification(characteristic, true)
        if (!success) {
            Logger.error(TAG, "Failed to register for notifications locally")
            _connectionState.value = ConnectionState.READY
            return
        }

        val descriptor = characteristic.getDescriptor(BleConstants.CCCD_UUID)
        if (descriptor != null) {
            descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            gatt.writeDescriptor(descriptor)
        } else {
            Logger.warn(TAG, "CCCD descriptor not found, skipping notification enable")
            _connectionState.value = ConnectionState.READY
        }
    }

    // ── Writing Commands ─────────────────────────────────────────────────────

    /**
     * Send circuit ON/OFF commands to the PCM board.
     *
     * The firmware (pcm/ble.cpp :: processCOMMData / processCANPacket) expects
     * one COMM CAN-packet per circuit, packed into a single BLE write.
     * Each 14-byte packet has the structure:
     *
     *   [0]     = 0x55          — packet delimiter
     *   [1]     = 0x0C (12)     — length field  (total bytes − 2)
     *   [2]     = 0x00          — CAN_PACKET type
     *   [3]     = 0x00          — unused (spacer)
     *   [4]     = 0x00          — unused (spacer)
     *   [5]     = 0x80 | addr   — SWITCH_PACKET flag | PCM address
     *   [6]     = canEncoding   — circuit index in CAN bus encoding (see CAN_CIRCUIT_ENCODING)
     *   [7]     = outCmd        — 0x00 = OFF, 0xFF = ON
     *   [8]     = 0x00          — blinkOn  (0 = no blinking)
     *   [9]     = 0x00          — blinkOff (0 = no blinking)
     *   [10–13] = CRC32         — IEEE 802.3 CRC of bytes [0–9], stored little-endian
     *
     * All 8 circuit packets are concatenated into one write; the firmware's parser
     * iterates through the buffer finding each 0x55 delimiter independently.
     *
     * @param circuitStateByte A byte where each bit represents a circuit's state:
     *   - Bit 0 (LSB) = Circuit 1 (index 0)
     *   - Bit 7 (MSB) = Circuit 8 (index 7)
     *   - 1 = ON, 0 = OFF
     */
    @Suppress("DEPRECATION")
    @Synchronized
    fun writeCircuitCommand(circuitStateByte: Byte) {
        val gatt = bluetoothGatt
        val characteristic = targetCharacteristic

        if (gatt == null || characteristic == null) {
            Logger.error(TAG, "Cannot write: not connected or characteristic not found")
            return
        }

        if (_connectionState.value != ConnectionState.READY) {
            Logger.error(TAG, "Cannot write: connection not ready (${_connectionState.value})")
            return
        }

        // Build 8 individual CAN packets (one per circuit) concatenated into one write.
        val packetSize = 14          // bytes per circuit packet
        val payload = ByteArray(packetSize * BleConstants.NUM_CIRCUITS)
        val stateInt = circuitStateByte.toInt() and 0xFF

        for (circuit in 0 until BleConstants.NUM_CIRCUITS) {
            val offset = circuit * packetSize
            val isOn = (stateInt shr circuit) and 0x01 == 1
            val outCmd = if (isOn) BleConstants.OUT_CMD_ON else BleConstants.OUT_CMD_OFF
            val canByte = BleConstants.CAN_CIRCUIT_ENCODING[circuit].toByte()
            val addrByte = (BleConstants.SWITCH_PACKET or BleConstants.PCM_ADDRESS).toByte()

            // Bytes 0–9 (the portion covered by CRC)
            payload[offset + 0] = BleConstants.PACKET_DELIMITER          // 0x55
            payload[offset + 1] = 0x0C.toByte()                          // length field = 14 - 2
            payload[offset + 2] = BleConstants.CAN_PACKET_TYPE           // 0x00
            payload[offset + 3] = 0x00                                   // spacer
            payload[offset + 4] = 0x00                                   // spacer
            payload[offset + 5] = addrByte                               // SWITCH_PACKET | addr
            payload[offset + 6] = canByte                                // CAN circuit encoding
            payload[offset + 7] = outCmd                                 // ON or OFF
            payload[offset + 8] = 0x00                                   // blinkOn  (no blink)
            payload[offset + 9] = 0x00                                   // blinkOff (no blink)

            // CRC32 over bytes [offset+0 .. offset+9] — IEEE 802.3 (same as java.util.zip.CRC32)
            val crc = computeCrc32(payload, offset, 10)
            payload[offset + 10] = (crc and 0xFF).toByte()              // LSB
            payload[offset + 11] = ((crc shr 8) and 0xFF).toByte()
            payload[offset + 12] = ((crc shr 16) and 0xFF).toByte()
            payload[offset + 13] = ((crc shr 24) and 0xFF).toByte()    // MSB
        }

        characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
        characteristic.value = payload

        val success = gatt.writeCharacteristic(characteristic)
        if (success) {
            _circuitStates.value = circuitStateByte
            val hex = payload.joinToString(" ") { "0x%02X".format(it) }
            Logger.tx(TAG, "Sent circuit command: 0x${"%02X".format(circuitStateByte)} → $hex")
        } else {
            Logger.error(TAG, "Failed to initiate characteristic write")
        }
    }

    /**
     * Compute IEEE 802.3 (standard Ethernet / zlib) CRC32 of [length] bytes
     * starting at [offset] within [data].
     *
     * This matches the firmware's crc32() implementation in crc32.cpp.
     */
    private fun computeCrc32(data: ByteArray, offset: Int, length: Int): Long {
        val crc = java.util.zip.CRC32()
        crc.update(data, offset, length)
        return crc.value
    }

    // ── Auto-Reconnect ───────────────────────────────────────────────────────

    /**
     * Enable or disable automatic reconnection on unexpected disconnection.
     */
    fun setAutoReconnect(enabled: Boolean) {
        autoReconnectEnabled = enabled
        Logger.info(TAG, "Auto-reconnect ${if (enabled) "enabled" else "disabled"}")
    }

    private fun scheduleReconnect() {
        if (reconnectAttempts >= BleConstants.MAX_RECONNECT_ATTEMPTS) {
            Logger.error(TAG, "Max reconnect attempts reached ($reconnectAttempts)")
            _connectionState.value = ConnectionState.FAILED
            return
        }

        val device = lastConnectedDevice ?: return
        val context = applicationContext ?: return
        reconnectAttempts++

        val delayMs = BleConstants.RECONNECT_BASE_DELAY_MS * (1L shl (reconnectAttempts - 1))
        Logger.info(TAG, "Reconnect attempt $reconnectAttempts in ${delayMs}ms")

        coroutineScope.launch {
            delay(delayMs)
            performConnect(context, device)
        }
    }

    // ── Log Helpers ──────────────────────────────────────────────────────────

    private val maxLogEntries = Logger.MAX_LOG_ENTRIES

    private fun addTxLog(message: String) {
        val timestamp = java.text.SimpleDateFormat("HH:mm:ss.SSS", java.util.Locale.US)
            .format(java.util.Date())
        val entry = "[$timestamp] TX: $message"
        val updated = (_txLog.value + entry).takeLast(maxLogEntries)
        _txLog.value = updated
    }

    private fun addRxLog(message: String) {
        val timestamp = java.text.SimpleDateFormat("HH:mm:ss.SSS", java.util.Locale.US)
            .format(java.util.Date())
        val entry = "[$timestamp] RX: $message"
        val updated = (_rxLog.value + entry).takeLast(maxLogEntries)
        _rxLog.value = updated
    }

    // ── Cleanup ──────────────────────────────────────────────────────────────

    /**
     * Clean up all BLE resources. Call when the app is closing.
     */
    fun cleanup() {
        stopScan()
        disconnect()
        closeGatt()
        coroutineScope.cancel()
        Logger.info(TAG, "BleManager cleaned up")
    }
}
