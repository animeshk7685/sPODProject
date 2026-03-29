package com.spod.blecontroller.ui.connection

import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.Context
import androidx.lifecycle.ViewModel
import com.spod.blecontroller.ble.BleManager
import com.spod.blecontroller.ble.ConnectionState
import kotlinx.coroutines.flow.StateFlow

/**
 * ViewModel for the Connection screen.
 *
 * Manages the connection process and exposes connection state.
 */
class ConnectionViewModel : ViewModel() {

    /** Current BLE connection state */
    val connectionState: StateFlow<ConnectionState> = BleManager.connectionState

    /**
     * Initiate a connection to the BLE device with the given MAC address.
     */
    fun connect(context: Context, deviceAddress: String) {
        val bluetoothManager = context.getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager
        val adapter = bluetoothManager?.adapter

        if (adapter == null || !adapter.isEnabled) return

        try {
            val device: BluetoothDevice = adapter.getRemoteDevice(deviceAddress)
            BleManager.connect(context, device)
        } catch (e: IllegalArgumentException) {
            // Invalid MAC address format
        }
    }

    /**
     * Disconnect from the current device.
     */
    fun disconnect() {
        BleManager.disconnect()
    }
}
