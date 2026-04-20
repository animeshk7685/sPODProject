package com.spod.blecontroller.ble

import java.util.UUID

/**
 * BLE UUIDs and protocol constants for the sPOD PCM board (ESP32 / NimBLE firmware).
 *
 * ── UUID Notes ────────────────────────────────────────────────────────────────
 * These 128-bit UUIDs are derived directly from the firmware source
 * (pcm/ble.cpp, ble_attUuid128 array). They are stored in the firmware as
 * little-endian byte arrays and reconstructed here in standard UUID string
 * format (big-endian groups) so that Android's UUID.fromString() matches
 * what NimBLE advertises on-air.
 *
 * ── Authentication ────────────────────────────────────────────────────────────
 * The PCM firmware requires BLE bonding with MITM protection (passkey 123456).
 * When Android prompts for a passkey during pairing, enter 123456.
 * All writes to COMM_CHARACTERISTIC_UUID are silently dropped by the firmware
 * until the BLE-level pairing (authGood flag) is complete.
 *
 * If the device is in "unsecured" mode (status.isUnsecured = true), the app
 * must instead write a passkey packet to UNSECURED_CHARACTERISTIC_UUID before
 * commands are accepted.
 */
object BleConstants {

    // ── Service & Characteristic UUIDs ───────────────────────────────────────
    // Derived from firmware pcm/ble.cpp :: ble_attUuid128[]:
    //   [0] = sPOD service
    //   [1] = COMM characteristic  (primary circuit-control channel)
    //   [5] = Unsecured-status characteristic (used for passkey auth in unsecured mode)

    /**
     * Primary sPOD service UUID.
     * Firmware source bytes (little-endian): 5B 68 56 60 9F E9 04 84 7B 44 0D 8C EC F9 3A 7E
     */
    val SERVICE_UUID: UUID = UUID.fromString("7e3af9ec-8c0d-447b-8404-e99f6056685b")

    /**
     * COMM characteristic UUID — used to send circuit-control (CAN) packets and
     * receive status notifications.
     * Firmware source bytes (little-endian): F4 C6 6F 64 02 EC 5B A6 6F 4C 00 4C 64 47 06 B9
     */
    val CHARACTERISTIC_UUID: UUID = UUID.fromString("b9064764-4c00-4c6f-a65b-ec02646fc6f4")

    /**
     * Unsecured-status characteristic UUID — used for passkey exchange when the
     * device is in unsecured mode (status.isUnsecured = true).
     * Firmware source bytes (little-endian): DE 6C FD CB A1 CC E8 82 36 45 58 7B D9 57 BA A0
     */
    val UNSECURED_CHARACTERISTIC_UUID: UUID = UUID.fromString("a0ba57d9-7b58-4536-82e8-cca1cbfd6cde")

    /**
     * Client Characteristic Configuration Descriptor UUID.
     * Standard 16-bit UUID for enabling notifications/indications (0x2902).
     */
    val CCCD_UUID: UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")

    // ── Timing Constants ─────────────────────────────────────────────────────

    /** BLE scan timeout in milliseconds (10 seconds) */
    const val SCAN_TIMEOUT_MS = 10_000L

    /** Connection attempt timeout in milliseconds (15 seconds) */
    const val CONNECTION_TIMEOUT_MS = 15_000L

    /** Maximum number of auto-reconnect attempts */
    const val MAX_RECONNECT_ATTEMPTS = 3

    /** Base delay for exponential backoff reconnect (1 second) */
    const val RECONNECT_BASE_DELAY_MS = 1_000L

    // ── Protocol Constants ───────────────────────────────────────────────────

    /** Number of circuits controlled by the PCM board */
    const val NUM_CIRCUITS = 8

    /**
     * Default PCM board RS-485 address (0 for a single-PCM setup).
     * In multi-PCM systems, change this to match your board's dip-switch address.
     * The firmware uses this to determine whether to execute commands locally or
     * forward them to the RS-485 bus.
     */
    const val PCM_ADDRESS: Int = 0

    /**
     * Packet delimiter byte — every COMM packet starts with 0x55.
     * The firmware's processCOMMData() uses this to frame individual packets
     * within a single BLE characteristic write.
     */
    const val PACKET_DELIMITER: Byte = 0x55.toByte()

    /**
     * CAN packet type identifier (bleRxBuffer[2] == 0x00 → CAN_PACKET).
     * Used for circuit switch commands routed over the internal CAN/RS-485 bus.
     */
    const val CAN_PACKET_TYPE: Byte = 0x00

    /**
     * SWITCH_PACKET flag (upper nibble 0x80) ORed with the PCM address (lower nibble).
     * The firmware checks (bleRxBuffer[5] & 0xF0) == SWITCH_PACKET to identify
     * circuit-toggle commands.
     */
    const val SWITCH_PACKET: Int = 0x80

    /**
     * CAN encoding for circuits 0–7.
     * The firmware uses a non-sequential bit-mask encoding (see common.cpp :: iToCan()).
     * Index in this array = circuit index (0-based).
     */
    val CAN_CIRCUIT_ENCODING = intArrayOf(
        0x08,   // Circuit 0 (Circuit 1 in UI) — CAN_SW0
        0x10,   // Circuit 1 (Circuit 2 in UI) — CAN_SW1
        0x20,   // Circuit 2 (Circuit 3 in UI) — CAN_SW2
        0x40,   // Circuit 3 (Circuit 4 in UI) — CAN_SW3
        0x80,   // Circuit 4 (Circuit 5 in UI) — CAN_SW4
        0x01,   // Circuit 5 (Circuit 6 in UI) — CAN_SW5
        0x02,   // Circuit 6 (Circuit 7 in UI) — CAN_SW6
        0x04    // Circuit 7 (Circuit 8 in UI) — CAN_SW7
    )

    /**
     * Output command for fully ON (100 % PWM duty cycle).
     * The firmware interprets outCmd > 0 as "on"; 0xFF is the standard full-on value.
     */
    const val OUT_CMD_ON: Byte = 0xFF.toByte()

    /** Output command for fully OFF. */
    const val OUT_CMD_OFF: Byte = 0x00
}
