package com.spod.blecontroller.util

import android.Manifest
import android.app.Activity
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Build
import android.provider.Settings
import androidx.appcompat.app.AlertDialog
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat

/**
 * Helper class to manage BLE-related runtime permissions.
 *
 * Android 12+ (API 31+) requires BLUETOOTH_SCAN and BLUETOOTH_CONNECT.
 * Android 11 and below require ACCESS_FINE_LOCATION for BLE scanning.
 */
object PermissionHelper {

    const val REQUEST_CODE_BLE_PERMISSIONS = 1001

    /**
     * Returns the list of BLE permissions required for the current Android version.
     */
    fun getRequiredPermissions(): Array<String> {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            arrayOf(
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT
            )
        } else {
            arrayOf(
                Manifest.permission.ACCESS_FINE_LOCATION
            )
        }
    }

    /**
     * Check if all required BLE permissions are granted.
     */
    fun hasAllPermissions(context: Context): Boolean {
        return getRequiredPermissions().all { permission ->
            ContextCompat.checkSelfPermission(context, permission) == PackageManager.PERMISSION_GRANTED
        }
    }

    /**
     * Request all required BLE permissions from the user.
     */
    fun requestPermissions(activity: Activity) {
        ActivityCompat.requestPermissions(
            activity,
            getRequiredPermissions(),
            REQUEST_CODE_BLE_PERMISSIONS
        )
    }

    /**
     * Handle the result of a permission request.
     */
    fun handlePermissionResult(
        activity: Activity,
        requestCode: Int,
        grantResults: IntArray,
        onGranted: () -> Unit,
        onDenied: () -> Unit
    ) {
        if (requestCode != REQUEST_CODE_BLE_PERMISSIONS) return

        if (grantResults.isNotEmpty() && grantResults.all { it == PackageManager.PERMISSION_GRANTED }) {
            onGranted()
        } else {
            val shouldShowRationale = getRequiredPermissions().any { permission ->
                ActivityCompat.shouldShowRequestPermissionRationale(activity, permission)
            }

            if (!shouldShowRationale) {
                showSettingsDialog(activity)
            } else {
                onDenied()
            }
        }
    }

    /**
     * Show a rationale dialog explaining why BLE permissions are needed.
     */
    fun showRationaleDialog(activity: Activity, onPositive: () -> Unit) {
        AlertDialog.Builder(activity)
            .setTitle("Bluetooth Permissions Required")
            .setMessage(
                "This app requires Bluetooth permissions to scan for and connect to your PCM board. " +
                "Please grant the required permissions to continue."
            )
            .setPositiveButton("Grant Permissions") { _, _ -> onPositive() }
            .setNegativeButton("Cancel") { dialog, _ -> dialog.dismiss() }
            .setCancelable(false)
            .show()
    }

    /**
     * Show a dialog directing the user to app Settings when permissions
     * have been permanently denied.
     */
    private fun showSettingsDialog(activity: Activity) {
        AlertDialog.Builder(activity)
            .setTitle("Permissions Required")
            .setMessage(
                "Bluetooth permissions are required for this app to function. " +
                "Please enable them in App Settings."
            )
            .setPositiveButton("Open Settings") { _, _ ->
                val intent = Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS).apply {
                    data = Uri.fromParts("package", activity.packageName, null)
                }
                activity.startActivity(intent)
            }
            .setNegativeButton("Cancel") { dialog, _ -> dialog.dismiss() }
            .setCancelable(false)
            .show()
    }
}
