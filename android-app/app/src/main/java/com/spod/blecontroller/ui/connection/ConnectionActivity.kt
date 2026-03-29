package com.spod.blecontroller.ui.connection

import android.content.Intent
import android.os.Bundle
import android.os.CountDownTimer
import android.view.View
import androidx.activity.OnBackPressedCallback
import androidx.activity.viewModels
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.spod.blecontroller.R
import com.spod.blecontroller.ble.BleConstants
import com.spod.blecontroller.ble.ConnectionState
import com.spod.blecontroller.databinding.ActivityConnectionBinding
import com.spod.blecontroller.ui.control.ControlActivity
import com.spod.blecontroller.util.Logger
import kotlinx.coroutines.launch

/**
 * Connection Screen — shown while connecting to a selected BLE device.
 *
 * Displays the connection progress and handles:
 * - GATT connection
 * - Service discovery
 * - Navigation to ControlActivity on success
 * - Retry on failure
 */
class ConnectionActivity : AppCompatActivity() {

    companion object {
        const val EXTRA_DEVICE_NAME = "extra_device_name"
        const val EXTRA_DEVICE_ADDRESS = "extra_device_address"
    }

    private lateinit var binding: ActivityConnectionBinding
    private val viewModel: ConnectionViewModel by viewModels()

    private var deviceName: String = "Unknown Device"
    private var deviceAddress: String = ""
    private var connectionTimeoutTimer: CountDownTimer? = null
    private var hasNavigatedToControl = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityConnectionBinding.inflate(layoutInflater)
        setContentView(binding.root)

        deviceName = intent.getStringExtra(EXTRA_DEVICE_NAME) ?: "Unknown Device"
        deviceAddress = intent.getStringExtra(EXTRA_DEVICE_ADDRESS) ?: ""

        setupToolbar()
        setupDeviceInfo()
        setupButtons()
        observeConnectionState()
        handleBackPress()

        initiateConnection()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.apply {
            title = getString(R.string.connection_title)
            setDisplayHomeAsUpEnabled(true)
        }
    }

    private fun setupDeviceInfo() {
        binding.tvDeviceName.text = deviceName
        binding.tvDeviceAddress.text = deviceAddress
    }

    private fun setupButtons() {
        binding.btnRetry.setOnClickListener { initiateConnection() }
        binding.btnDisconnect.setOnClickListener {
            viewModel.disconnect()
            finish()
        }
    }

    private fun observeConnectionState() {
        lifecycleScope.launch {
            viewModel.connectionState.collect { state ->
                updateUI(state)

                when (state) {
                    ConnectionState.READY -> {
                        if (!hasNavigatedToControl) {
                            hasNavigatedToControl = true
                            connectionTimeoutTimer?.cancel()
                            navigateToControl()
                        }
                    }
                    ConnectionState.FAILED -> {
                        connectionTimeoutTimer?.cancel()
                        showFailureState()
                    }
                    ConnectionState.DISCONNECTED -> {
                        if (!hasNavigatedToControl) {
                            connectionTimeoutTimer?.cancel()
                        }
                    }
                    else -> { /* Ongoing states handled by updateUI */ }
                }
            }
        }
    }

    private fun updateUI(state: ConnectionState) {
        val (statusText, progressVisible) = when (state) {
            ConnectionState.CONNECTING -> Pair(getString(R.string.status_connecting), true)
            ConnectionState.CONNECTED -> Pair(getString(R.string.status_connected), true)
            ConnectionState.DISCOVERING_SERVICES -> Pair(getString(R.string.status_discovering), true)
            ConnectionState.SERVICES_DISCOVERED -> Pair(getString(R.string.status_services_found), true)
            ConnectionState.READY -> Pair(getString(R.string.status_ready), false)
            ConnectionState.FAILED -> Pair(getString(R.string.status_failed), false)
            ConnectionState.DISCONNECTING -> Pair(getString(R.string.status_disconnecting), true)
            ConnectionState.DISCONNECTED -> Pair(getString(R.string.status_disconnected), false)
        }

        binding.tvConnectionStatus.text = statusText
        binding.progressBar.visibility = if (progressVisible) View.VISIBLE else View.GONE
        binding.btnRetry.visibility = if (state == ConnectionState.FAILED) View.VISIBLE else View.GONE
        binding.btnDisconnect.visibility = if (state == ConnectionState.CONNECTING ||
            state == ConnectionState.CONNECTED ||
            state == ConnectionState.DISCOVERING_SERVICES) View.VISIBLE else View.GONE
    }

    private fun showFailureState() {
        binding.tvConnectionStatus.text = getString(R.string.status_failed)
        binding.btnRetry.visibility = View.VISIBLE
    }

    private fun initiateConnection() {
        hasNavigatedToControl = false
        binding.btnRetry.visibility = View.GONE
        startConnectionTimeout()
        viewModel.connect(this, deviceAddress)
    }

    private fun startConnectionTimeout() {
        connectionTimeoutTimer?.cancel()
        connectionTimeoutTimer = object : CountDownTimer(BleConstants.CONNECTION_TIMEOUT_MS, 1_000L) {
            override fun onTick(millisUntilFinished: Long) {}

            override fun onFinish() {
                if (viewModel.connectionState.value == ConnectionState.CONNECTING ||
                    viewModel.connectionState.value == ConnectionState.CONNECTED ||
                    viewModel.connectionState.value == ConnectionState.DISCOVERING_SERVICES) {
                    Logger.error("ConnectionActivity", "Connection timed out")
                    showFailureState()
                    binding.tvConnectionStatus.text = getString(R.string.status_timeout)
                }
            }
        }.start()
    }

    private fun navigateToControl() {
        val intent = Intent(this, ControlActivity::class.java).apply {
            putExtra(ControlActivity.EXTRA_DEVICE_NAME, deviceName)
            putExtra(ControlActivity.EXTRA_DEVICE_ADDRESS, deviceAddress)
        }
        startActivity(intent)
        finish()
    }

    private fun handleBackPress() {
        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                val state = viewModel.connectionState.value
                if (state == ConnectionState.CONNECTING || state == ConnectionState.CONNECTED) {
                    AlertDialog.Builder(this@ConnectionActivity)
                        .setTitle("Cancel Connection?")
                        .setMessage("Are you sure you want to cancel the connection attempt?")
                        .setPositiveButton("Yes") { _, _ ->
                            viewModel.disconnect()
                            isEnabled = false
                            onBackPressedDispatcher.onBackPressed()
                        }
                        .setNegativeButton("No", null)
                        .show()
                } else {
                    isEnabled = false
                    onBackPressedDispatcher.onBackPressed()
                }
            }
        })
    }

    override fun onSupportNavigateUp(): Boolean {
        onBackPressedDispatcher.onBackPressed()
        return true
    }

    override fun onDestroy() {
        super.onDestroy()
        connectionTimeoutTimer?.cancel()
    }
}
