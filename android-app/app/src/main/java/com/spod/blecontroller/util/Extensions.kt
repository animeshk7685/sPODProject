package com.spod.blecontroller.util

import android.content.Context
import android.view.View
import android.widget.Toast
import com.google.android.material.snackbar.Snackbar

/** Show a short Toast message */
fun Context.showToast(message: String) {
    Toast.makeText(this, message, Toast.LENGTH_SHORT).show()
}

/** Show a long Toast message */
fun Context.showLongToast(message: String) {
    Toast.makeText(this, message, Toast.LENGTH_LONG).show()
}

/** Show a Snackbar with an optional action */
fun View.showSnackbar(
    message: String,
    actionLabel: String? = null,
    action: (() -> Unit)? = null
) {
    val snackbar = Snackbar.make(this, message, Snackbar.LENGTH_SHORT)
    if (actionLabel != null && action != null) {
        snackbar.setAction(actionLabel) { action() }
    }
    snackbar.show()
}

/** Make a View visible */
fun View.show() {
    visibility = View.VISIBLE
}

/** Make a View invisible (still occupies space) */
fun View.hide() {
    visibility = View.INVISIBLE
}

/** Make a View gone (no space occupied) */
fun View.gone() {
    visibility = View.GONE
}

/** Convert a byte to a hex string */
fun Byte.toHexString(): String = "0x%02X".format(this)

/** Convert a ByteArray to a space-separated hex string */
fun ByteArray.toHexString(): String = joinToString(" ") { "0x%02X".format(it) }

/** Check if a specific bit is set in a byte */
fun Byte.isBitSet(bitPosition: Int): Boolean {
    require(bitPosition in 0..7) { "Bit position must be 0-7" }
    return (toInt() and (1 shl bitPosition)) != 0
}
