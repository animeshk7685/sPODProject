package com.spod.blecontroller.ui.scan

import androidx.lifecycle.ViewModel
import com.spod.blecontroller.ble.BleManager
import com.spod.blecontroller.model.BleDevice
import kotlinx.coroutines.flow.StateFlow

/**
 * ViewModel for the Scan screen.
 *
 * Manages the list of discovered BLE devices and scan state.
 * Delegates all BLE operations to [BleManager].
 */
class ScanViewModel : ViewModel() {

    /** List of discovered BLE devices from scanning */
    val scannedDevices: StateFlow<List<BleDevice>> = BleManager.scannedDevices

    /** Whether a BLE scan is currently active */
    val isScanning: StateFlow<Boolean> = BleManager.isScanning

    fun startScan() {
        BleManager.startScan()
    }

    fun stopScan() {
        BleManager.stopScan()
    }

    override fun onCleared() {
        super.onCleared()
        BleManager.stopScan()
    }
}
