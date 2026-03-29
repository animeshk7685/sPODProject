package com.spod.blecontroller.ui.control

import android.content.Intent
import android.os.Bundle
import android.view.HapticFeedbackConstants
import android.view.View
import androidx.activity.OnBackPressedCallback
import androidx.activity.viewModels
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.GridLayoutManager
import com.spod.blecontroller.R
import com.spod.blecontroller.ble.BleManager
import com.spod.blecontroller.ble.ConnectionState
import com.spod.blecontroller.databinding.ActivityControlBinding
import com.spod.blecontroller.ui.scan.ScanActivity
import com.spod.blecontroller.util.showSnackbar
import kotlinx.coroutines.launch

/**
 * Control Dashboard — The main screen for controlling the 8 PCM circuits.
 *
 * Features:
 * - 2-column grid of circuit cards with toggle switches
 * - "All ON" and "All OFF" buttons
 * - Connection status in toolbar
 * - Toggleable debug log panel showing TX/RX packets
 * - Handles disconnection gracefully
 */
class ControlActivity : AppCompatActivity() {

    companion object {
        const val EXTRA_DEVICE_NAME = "extra_device_name"
        const val EXTRA_DEVICE_ADDRESS = "extra_device_address"
    }

    private lateinit var binding: ActivityControlBinding
    private val viewModel: ControlViewModel by viewModels()
    private lateinit var circuitAdapter: CircuitAdapter

    private var deviceName: String = "Unknown Device"
    private var deviceAddress: String = ""
    private var isDebugPanelVisible = false
    private var disconnectionDialogShown = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityControlBinding.inflate(layoutInflater)
        setContentView(binding.root)

        deviceName = intent.getStringExtra(EXTRA_DEVICE_NAME) ?: "Unknown Device"
        deviceAddress = intent.getStringExtra(EXTRA_DEVICE_ADDRESS) ?: ""

        setupToolbar()
        setupRecyclerView()
        setupButtons()
        observeViewModel()
        handleBackPress()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.title = getString(R.string.control_title)
        binding.tvDeviceInfo.text = "$deviceName • $deviceAddress"
    }

    private fun setupRecyclerView() {
        circuitAdapter = CircuitAdapter { circuitNumber ->
            binding.root.performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY)
            viewModel.toggleCircuit(circuitNumber)
        }
        binding.rvCircuits.apply {
            layoutManager = GridLayoutManager(this@ControlActivity, 2)
            adapter = circuitAdapter
        }
    }

    private fun setupButtons() {
        binding.btnAllOn.setOnClickListener { viewModel.setAllOn() }
        binding.btnAllOff.setOnClickListener { viewModel.setAllOff() }

        binding.btnToggleDebug.setOnClickListener {
            isDebugPanelVisible = !isDebugPanelVisible
            binding.layoutDebugPanel.visibility =
                if (isDebugPanelVisible) View.VISIBLE else View.GONE
            binding.btnToggleDebug.text =
                if (isDebugPanelVisible) getString(R.string.hide_debug) else getString(R.string.show_debug)
        }

        binding.switchAutoReconnect.setOnCheckedChangeListener { _, isChecked ->
            viewModel.setAutoReconnect(isChecked)
        }
    }

    private fun observeViewModel() {
        lifecycleScope.launch {
            viewModel.circuits.collect { circuits ->
                circuitAdapter.submitList(circuits.toList())
            }
        }

        lifecycleScope.launch {
            viewModel.connectionState.collect { state ->
                updateConnectionStatus(state)
                when (state) {
                    ConnectionState.DISCONNECTED, ConnectionState.FAILED -> showDisconnectionDialog()
                    else -> {}
                }
            }
        }

        lifecycleScope.launch {
            viewModel.txLog.collect { logs ->
                binding.tvTxLog.text = logs.takeLast(10).joinToString("\n")
            }
        }

        lifecycleScope.launch {
            viewModel.rxLog.collect { logs ->
                binding.tvRxLog.text = logs.takeLast(10).joinToString("\n")
            }
        }
    }

    private fun updateConnectionStatus(state: ConnectionState) {
        val (statusText, color) = when (state) {
            ConnectionState.READY -> Pair("Connected", getColor(R.color.status_connected))
            ConnectionState.DISCONNECTED -> Pair("Disconnected", getColor(R.color.status_disconnected))
            ConnectionState.FAILED -> Pair("Failed", getColor(R.color.status_failed))
            ConnectionState.CONNECTING -> Pair("Connecting...", getColor(R.color.status_connecting))
            else -> Pair(state.name, getColor(R.color.status_connecting))
        }
        binding.tvConnectionStatus.text = statusText
        binding.tvConnectionStatus.setTextColor(color)
    }

    private fun showDisconnectionDialog() {
        if (disconnectionDialogShown) return
        disconnectionDialogShown = true

        binding.root.showSnackbar(
            message = "Device disconnected",
            actionLabel = "Reconnect"
        ) {
            disconnectionDialogShown = false
            val intent = Intent(this, ScanActivity::class.java).apply {
                flags = Intent.FLAG_ACTIVITY_CLEAR_TOP
            }
            startActivity(intent)
            finish()
        }
    }

    private fun handleBackPress() {
        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                val state = viewModel.connectionState.value
                if (state == ConnectionState.READY) {
                    AlertDialog.Builder(this@ControlActivity)
                        .setTitle("Disconnect?")
                        .setMessage("Going back will disconnect from the device. Continue?")
                        .setPositiveButton("Disconnect") { _, _ ->
                            BleManager.disconnect()
                            isEnabled = false
                            onBackPressedDispatcher.onBackPressed()
                        }
                        .setNegativeButton("Stay", null)
                        .show()
                } else {
                    isEnabled = false
                    onBackPressedDispatcher.onBackPressed()
                }
            }
        })
    }
}
