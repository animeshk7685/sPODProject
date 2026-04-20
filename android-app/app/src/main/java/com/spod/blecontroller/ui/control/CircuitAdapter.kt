package com.spod.blecontroller.ui.control

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.spod.blecontroller.R
import com.spod.blecontroller.databinding.ItemCircuitBinding
import com.spod.blecontroller.model.CircuitState

/**
 * RecyclerView adapter for displaying the 8 circuit controls in the Control Dashboard.
 * Each item shows a circuit label, toggle switch, and visual state indicator.
 */
class CircuitAdapter(
    private val onCircuitToggled: (circuitNumber: Int) -> Unit
) : ListAdapter<CircuitState, CircuitAdapter.CircuitViewHolder>(CircuitDiffCallback()) {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): CircuitViewHolder {
        val binding = ItemCircuitBinding.inflate(
            LayoutInflater.from(parent.context),
            parent,
            false
        )
        return CircuitViewHolder(binding)
    }

    override fun onBindViewHolder(holder: CircuitViewHolder, position: Int) {
        holder.bind(getItem(position))
    }

    inner class CircuitViewHolder(
        private val binding: ItemCircuitBinding
    ) : RecyclerView.ViewHolder(binding.root) {

        fun bind(circuit: CircuitState) {
            binding.apply {
                tvCircuitLabel.text = circuit.label

                // Null the listener before setting isChecked to prevent a recursive callback
                // when the RecyclerView rebinds an existing view with a new checked state.
                switchCircuit.setOnCheckedChangeListener(null)
                switchCircuit.isChecked = circuit.isOn

                val indicatorColor = if (circuit.isOn) {
                    root.context.getColor(R.color.circuit_on)
                } else {
                    root.context.getColor(R.color.circuit_off)
                }
                viewIndicator.setBackgroundColor(indicatorColor)

                tvCircuitStatus.text = if (circuit.isOn) "ON" else "OFF"
                tvCircuitStatus.setTextColor(indicatorColor)

                val cardColor = if (circuit.isOn) {
                    root.context.getColor(R.color.card_active)
                } else {
                    root.context.getColor(R.color.card_inactive)
                }
                cardView.setCardBackgroundColor(cardColor)

                switchCircuit.setOnCheckedChangeListener { _, _ ->
                    onCircuitToggled(circuit.circuitNumber)
                }

                root.setOnClickListener {
                    onCircuitToggled(circuit.circuitNumber)
                }
            }
        }
    }

    private class CircuitDiffCallback : DiffUtil.ItemCallback<CircuitState>() {
        override fun areItemsTheSame(oldItem: CircuitState, newItem: CircuitState): Boolean =
            oldItem.circuitNumber == newItem.circuitNumber

        override fun areContentsTheSame(oldItem: CircuitState, newItem: CircuitState): Boolean =
            oldItem == newItem
    }
}
