package com.spod.blecontroller.ble

import java.util.UUID

/**
 * BLE UUIDs and constants for the sPOD PCM board.
 *
 * IMPORTANT: Replace SERVICE_UUID and CHARACTERISTIC_UUID with the actual
 * UUIDs from your PCM hardware before deploying.
 *
 * To find your device's UUIDs:
 * 1. Use a BLE scanner app (e.g., nRF Connect) on Android
 * 2. Connect to your PCM board
 * 3. Note the Service UUID and the writable Characteristic UUID
 */
object BleConstants {

    /**
     * The BLE service UUID advertised by the sPOD PCM board.
     * Replace with actual hardware UUID.
     */
    val SERVICE_UUID: UUID = UUID.fromString("0000ffe0-0000-1000-8000-00805f9b34fb")

    /**
     * The characteristic UUID used for reading/writing circuit commands.
     * Replace with actual hardware UUID.
     */
    val CHARACTERISTIC_UUID: UUID = UUID.fromString("0000ffe1-0000-1000-8000-00805f9b34fb")

    /**
     * Client Characteristic Configuration Descriptor UUID.
     * Standard UUID for enabling notifications/indications.
     */
    val CCCD_UUID: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")

    /** BLE scan timeout in milliseconds (10 seconds) */
    const val SCAN_TIMEOUT_MS = 10_000L

    /** Connection attempt timeout in milliseconds (15 seconds) */
    const val CONNECTION_TIMEOUT_MS = 15_000L

    /** Maximum number of auto-reconnect attempts */
    const val MAX_RECONNECT_ATTEMPTS = 3

    /** Base delay for exponential backoff reconnect (1 second) */
    const val RECONNECT_BASE_DELAY_MS = 1_000L

    /** Number of circuits controlled by the PCM board */
    const val NUM_CIRCUITS = 8

    /**
     * Command byte format:
     * Each bit represents one circuit state (0 = OFF, 1 = ON)
     * Bit 0 (LSB) = Circuit 1
     * Bit 7 (MSB) = Circuit 8
     *
     * Example: 0b00000101 = Circuits 1 and 3 ON, rest OFF
     */
    const val ALL_OFF_COMMAND: Byte = 0x00
    const val ALL_ON_COMMAND: Byte = 0xFF.toByte()
}
