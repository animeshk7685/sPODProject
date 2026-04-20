package com.spod.blecontroller.ble

/**
 * Represents the various states of a BLE connection lifecycle.
 *
 * State transitions:
 * DISCONNECTED → CONNECTING → CONNECTED → DISCOVERING_SERVICES
 *   → SERVICES_DISCOVERED → READY
 *
 * Error path: Any state → FAILED
 * Cleanup: Any state → DISCONNECTING → DISCONNECTED
 */
enum class ConnectionState {
    /** No active connection or pending connection attempt */
    DISCONNECTED,

    /** Attempting to establish a GATT connection */
    CONNECTING,

    /** GATT connection established, not yet discovered services */
    CONNECTED,

    /** Service discovery is in progress */
    DISCOVERING_SERVICES,

    /** Services have been discovered */
    SERVICES_DISCOVERED,

    /**
     * Target characteristic found and notifications enabled.
     * App is ready to send/receive circuit commands.
     */
    READY,

    /** Gracefully disconnecting */
    DISCONNECTING,

    /** Connection failed or was lost unexpectedly */
    FAILED
}
