package com.spod.blecontroller.util

import android.content.Context
import android.util.Log
import java.text.SimpleDateFormat
import java.util.*
import java.util.concurrent.ConcurrentLinkedDeque

/**
 * Custom logger for the sPOD BLE Controller app.
 *
 * Features:
 * - Logs to Logcat with tag "SPOD_BLE"
 * - Maintains in-memory ring buffer of last 100 log entries
 * - Timestamps each entry
 * - Categorizes entries: TX, RX, INFO, ERROR, DEBUG
 * - Debug mode toggle (when disabled, DEBUG logs are suppressed)
 */
object Logger {

    private const val LOGCAT_TAG = "SPOD_BLE"
    const val MAX_LOG_ENTRIES = 100

    @Volatile
    var isDebugEnabled: Boolean = true

    private val logBuffer = ConcurrentLinkedDeque<LogEntry>()
    private val timeFormat = SimpleDateFormat("HH:mm:ss.SSS", Locale.US)

    /**
     * Initialize the logger. Call from Application.onCreate().
     */
    fun init(context: Context) {
        info("Logger", "Logger initialized (debug=${isDebugEnabled})")
    }

    // ── Public Logging Methods ───────────────────────────────────────────────

    fun info(tag: String, message: String) {
        log(LogLevel.INFO, tag, message)
        Log.i(LOGCAT_TAG, "[$tag] $message")
    }

    fun debug(tag: String, message: String) {
        if (!isDebugEnabled) return
        log(LogLevel.DEBUG, tag, message)
        Log.d(LOGCAT_TAG, "[$tag] $message")
    }

    fun error(tag: String, message: String, throwable: Throwable? = null) {
        log(LogLevel.ERROR, tag, message)
        Log.e(LOGCAT_TAG, "[$tag] $message", throwable)
    }

    /** Log a transmitted (sent) BLE packet */
    fun tx(tag: String, message: String) {
        log(LogLevel.TX, tag, message)
        Log.d(LOGCAT_TAG, "[$tag] TX: $message")
    }

    fun warn(tag: String, message: String) {
        log(LogLevel.INFO, tag, message)
        Log.w(LOGCAT_TAG, "[$tag] WARN: $message")
    }

    /** Log a received BLE packet */
    fun rx(tag: String, message: String) {
        log(LogLevel.RX, tag, message)
        Log.d(LOGCAT_TAG, "[$tag] RX: $message")
    }

    // ── Buffer Access ────────────────────────────────────────────────────────

    /** Get all log entries as a formatted list (most recent last) */
    fun getLogEntries(): List<LogEntry> = logBuffer.toList()

    /** Get log entries as formatted strings for display */
    fun getFormattedLog(): List<String> = logBuffer.map { it.formatted }

    /** Clear the in-memory log buffer */
    fun clearLog() {
        logBuffer.clear()
    }

    // ── Internal ─────────────────────────────────────────────────────────────

    private fun log(level: LogLevel, tag: String, message: String) {
        val timestamp = timeFormat.format(Date())
        val entry = LogEntry(timestamp = timestamp, level = level, tag = tag, message = message)
        logBuffer.addLast(entry)
        while (logBuffer.size > MAX_LOG_ENTRIES) {
            logBuffer.pollFirst()
        }
    }

    // ── Models ───────────────────────────────────────────────────────────────

    enum class LogLevel(val prefix: String) {
        INFO("INFO"),
        DEBUG("DBUG"),
        ERROR("ERR!"),
        TX("TX  "),
        RX("RX  ")
    }

    data class LogEntry(
        val timestamp: String,
        val level: LogLevel,
        val tag: String,
        val message: String
    ) {
        val formatted: String
            get() = "[$timestamp][${level.prefix}][$tag] $message"
    }
}
