package com.spod.blecontroller.model

/**
 * Represents the state of a single circuit on the PCM board.
 *
 * @param circuitNumber Circuit identifier (1-8)
 * @param label Display label (e.g., "Circuit 1")
 * @param isOn Current ON/OFF state
 */
data class CircuitState(
    val circuitNumber: Int,
    val label: String,
    var isOn: Boolean = false
) {
    companion object {
        /**
         * Create the default list of 8 circuit states (all OFF).
         */
        fun createDefault(): List<CircuitState> {
            return (1..8).map { i ->
                CircuitState(
                    circuitNumber = i,
                    label = "Circuit $i",
                    isOn = false
                )
            }
        }

        /**
         * Update a list of circuit states from a command byte.
         * Bit 0 = Circuit 1, Bit 7 = Circuit 8.
         */
        fun fromByte(stateByte: Byte): List<CircuitState> {
            val stateInt = stateByte.toInt() and 0xFF
            return (1..8).map { i ->
                val bitMask = 1 shl (i - 1)
                CircuitState(
                    circuitNumber = i,
                    label = "Circuit $i",
                    isOn = (stateInt and bitMask) != 0
                )
            }
        }

        /**
         * Convert a list of circuit states to a command byte.
         * Returns a byte where each bit represents the circuit's ON/OFF state.
         */
        fun toByte(circuits: List<CircuitState>): Byte {
            var result = 0
            for (circuit in circuits) {
                if (circuit.isOn) {
                    result = result or (1 shl (circuit.circuitNumber - 1))
                }
            }
            return result.toByte()
        }
    }
}
