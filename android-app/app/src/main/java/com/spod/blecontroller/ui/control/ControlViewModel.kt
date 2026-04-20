package com.spod.blecontroller.ui.control

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.spod.blecontroller.ble.BleManager
import com.spod.blecontroller.ble.ConnectionState
import com.spod.blecontroller.model.CircuitState
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

/**
 * ViewModel for the Control Dashboard screen.
 *
 * Manages the state of 8 circuit switches and sends BLE commands
 * to the PCM board when circuit states change.
 */
class ControlViewModel : ViewModel() {

    /** Current BLE connection state */
    val connectionState: StateFlow<ConnectionState> = BleManager.connectionState

    /** TX log for debug panel */
    val txLog: StateFlow<List<String>> = BleManager.txLog

    /** RX log for debug panel */
    val rxLog: StateFlow<List<String>> = BleManager.rxLog

    private val _autoReconnectEnabled = MutableStateFlow(true)
    val autoReconnectEnabled: StateFlow<Boolean> = _autoReconnectEnabled.asStateFlow()

    private val _circuits = MutableStateFlow(CircuitState.createDefault())
    /** List of 8 circuit states */
    val circuits: StateFlow<List<CircuitState>> = _circuits.asStateFlow()

    init {
        viewModelScope.launch {
            BleManager.circuitStates.collect { stateByte ->
                _circuits.value = CircuitState.fromByte(stateByte)
            }
        }
    }

    /**
     * Toggle the ON/OFF state of a specific circuit and send the BLE command.
     *
     * @param circuitNumber 1-8
     */
    fun toggleCircuit(circuitNumber: Int) {
        val currentCircuits = _circuits.value.toMutableList()
        val index = circuitNumber - 1
        if (index < 0 || index >= currentCircuits.size) return

        currentCircuits[index] = currentCircuits[index].copy(isOn = !currentCircuits[index].isOn)
        _circuits.value = currentCircuits
        sendCurrentState()
    }

    /** Turn all circuits ON and send the BLE command. */
    fun setAllOn() {
        _circuits.value = _circuits.value.map { it.copy(isOn = true) }
        sendCurrentState()
    }

    /** Turn all circuits OFF and send the BLE command. */
    fun setAllOff() {
        _circuits.value = _circuits.value.map { it.copy(isOn = false) }
        sendCurrentState()
    }

    fun setAutoReconnect(enabled: Boolean) {
        _autoReconnectEnabled.value = enabled
        BleManager.setAutoReconnect(enabled)
    }

    private fun sendCurrentState() {
        val stateByte = CircuitState.toByte(_circuits.value)
        BleManager.writeCircuitCommand(stateByte)
    }
}
