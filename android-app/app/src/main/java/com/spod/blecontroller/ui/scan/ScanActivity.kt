package com.spod.blecontroller.ui.scan

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.view.View
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import com.spod.blecontroller.R
import com.spod.blecontroller.databinding.ActivityScanBinding
import com.spod.blecontroller.model.BleDevice
import com.spod.blecontroller.ui.connection.ConnectionActivity
import com.spod.blecontroller.util.Logger
import com.spod.blecontroller.util.PermissionHelper
import com.spod.blecontroller.util.showSnackbar
import kotlinx.coroutines.launch

/**
 * Scan Screen — Entry point of the app.
 *
 * Scans for nearby BLE devices, displays them in a list, and allows
 * the user to select one to initiate a connection.
 */
class ScanActivity : AppCompatActivity() {

    private lateinit var binding: ActivityScanBinding
    private val viewModel: ScanViewModel by viewModels()
    private lateinit var deviceAdapter: DeviceAdapter

    private val enableBluetoothLauncher = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == RESULT_OK) {
            startScanIfReady()
        } else {
            binding.root.showSnackbar("Bluetooth is required to use this app")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityScanBinding.inflate(layoutInflater)
        setContentView(binding.root)

        setupToolbar()
        setupRecyclerView()
        setupButtons()
        observeViewModel()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.title = getString(R.string.scan_title)
    }

    private fun setupRecyclerView() {
        deviceAdapter = DeviceAdapter { device -> onDeviceSelected(device) }
        binding.rvDevices.apply {
            layoutManager = LinearLayoutManager(this@ScanActivity)
            adapter = deviceAdapter
        }
    }

    private fun setupButtons() {
        binding.btnScan.setOnClickListener {
            if (viewModel.isScanning.value) {
                viewModel.stopScan()
            } else {
                startScanIfReady()
            }
        }
    }

    private fun observeViewModel() {
        lifecycleScope.launch {
            viewModel.scannedDevices.collect { devices ->
                deviceAdapter.submitList(devices.toList())
                binding.tvEmptyState.visibility =
                    if (devices.isEmpty() && !viewModel.isScanning.value) View.VISIBLE
                    else View.GONE
            }
        }

        lifecycleScope.launch {
            viewModel.isScanning.collect { isScanning ->
                binding.apply {
                    btnScan.text = if (isScanning) getString(R.string.stop_scan) else getString(R.string.start_scan)
                    progressBar.visibility = if (isScanning) View.VISIBLE else View.GONE
                    tvScanStatus.text = if (isScanning) getString(R.string.scanning) else getString(R.string.scan_idle)
                }
            }
        }
    }

    /**
     * Check all preconditions before starting a scan:
     * 1. BLE permissions granted
     * 2. Bluetooth enabled
     */
    private fun startScanIfReady() {
        if (!PermissionHelper.hasAllPermissions(this)) {
            if (PermissionHelper.getRequiredPermissions().any { permission ->
                    androidx.core.app.ActivityCompat.shouldShowRequestPermissionRationale(this, permission)
                }) {
                PermissionHelper.showRationaleDialog(this) {
                    PermissionHelper.requestPermissions(this)
                }
            } else {
                PermissionHelper.requestPermissions(this)
            }
            return
        }

        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager
        val adapter = bluetoothManager?.adapter
        if (adapter == null || !adapter.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            enableBluetoothLauncher.launch(enableBtIntent)
            return
        }

        viewModel.startScan()
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        PermissionHelper.handlePermissionResult(
            activity = this,
            requestCode = requestCode,
            grantResults = grantResults,
            onGranted = { startScanIfReady() },
            onDenied = {
                binding.root.showSnackbar("Bluetooth permissions are required to scan for devices")
            }
        )
    }

    private fun onDeviceSelected(device: BleDevice) {
        Logger.info("ScanActivity", "Selected device: ${device.name} [${device.address}]")
        viewModel.stopScan()

        val intent = Intent(this, ConnectionActivity::class.java).apply {
            putExtra(ConnectionActivity.EXTRA_DEVICE_NAME, device.name)
            putExtra(ConnectionActivity.EXTRA_DEVICE_ADDRESS, device.address)
        }
        startActivity(intent)
    }
}
