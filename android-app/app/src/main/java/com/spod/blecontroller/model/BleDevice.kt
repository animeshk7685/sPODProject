package com.spod.blecontroller.model

import android.bluetooth.BluetoothDevice

/**
 * Data model representing a discovered BLE device.
 *
 * @param name Device name from advertisement data, or "Unknown Device" if null
 * @param address MAC address in format "XX:XX:XX:XX:XX:XX"
 * @param rssi Received Signal Strength Indicator in dBm (e.g., -60 is stronger than -90)
 * @param bondState Current bonding state (see [BluetoothDevice] BOND_* constants)
 */
data class BleDevice(
    val name: String,
    val address: String,
    var rssi: Int,
    val bondState: Int = BluetoothDevice.BOND_NONE
) {
    /** Human-readable bond state description */
    val bondStateDescription: String
        get() = when (bondState) {
            BluetoothDevice.BOND_BONDED -> "Paired"
            BluetoothDevice.BOND_BONDING -> "Pairing..."
            else -> "Not Paired"
        }

    /** Signal strength category for display purposes */
    val signalStrength: String
        get() = when {
            rssi >= -60 -> "Strong"
            rssi >= -75 -> "Good"
            rssi >= -85 -> "Fair"
            else -> "Weak"
        }
}
