package com.spod.blecontroller

import android.app.Application
import com.spod.blecontroller.ble.BleManager
import com.spod.blecontroller.util.Logger

/**
 * Application class for the sPOD BLE Controller app.
 * Initializes global state, the Logger, and the BleManager singleton.
 */
class SpodApplication : Application() {

    override fun onCreate() {
        super.onCreate()
        Logger.init(applicationContext)
        BleManager.init(applicationContext)
        Logger.info("SpodApplication", "Application started")
    }
}
