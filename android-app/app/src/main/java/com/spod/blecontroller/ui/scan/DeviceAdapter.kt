package com.spod.blecontroller.ui.scan

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.spod.blecontroller.R
import com.spod.blecontroller.databinding.ItemDeviceBinding
import com.spod.blecontroller.model.BleDevice

/**
 * RecyclerView adapter for displaying discovered BLE devices in the Scan screen.
 * Uses DiffUtil for efficient updates.
 */
class DeviceAdapter(
    private val onDeviceClicked: (BleDevice) -> Unit
) : ListAdapter<BleDevice, DeviceAdapter.DeviceViewHolder>(DeviceDiffCallback()) {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): DeviceViewHolder {
        val binding = ItemDeviceBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return DeviceViewHolder(binding)
    }

    override fun onBindViewHolder(holder: DeviceViewHolder, position: Int) {
        holder.bind(getItem(position))
    }

    inner class DeviceViewHolder(
        private val binding: ItemDeviceBinding
    ) : RecyclerView.ViewHolder(binding.root) {

        fun bind(device: BleDevice) {
            binding.apply {
                tvDeviceName.text = device.name
                tvDeviceAddress.text = device.address
                tvRssi.text = "${device.rssi} dBm (${device.signalStrength})"
                tvBondState.text = device.bondStateDescription

                val signalColor = when {
                    device.rssi >= -60 -> root.context.getColor(R.color.signal_strong)
                    device.rssi >= -75 -> root.context.getColor(R.color.signal_good)
                    device.rssi >= -85 -> root.context.getColor(R.color.signal_fair)
                    else -> root.context.getColor(R.color.signal_weak)
                }
                ivSignal.setColorFilter(signalColor)

                root.setOnClickListener { onDeviceClicked(device) }
            }
        }
    }

    private class DeviceDiffCallback : DiffUtil.ItemCallback<BleDevice>() {
        override fun areItemsTheSame(oldItem: BleDevice, newItem: BleDevice): Boolean =
            oldItem.address == newItem.address

        override fun areContentsTheSame(oldItem: BleDevice, newItem: BleDevice): Boolean =
            oldItem == newItem
    }
}
