package com.mywatch.companion

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.mywatch.companion.databinding.ActivityMainBinding
import java.io.IOException
import java.io.InputStream
import java.nio.charset.StandardCharsets
import java.util.UUID
import java.util.concurrent.Executors

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        (getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager).adapter
    }
    private var socket: BluetoothSocket? = null
    private var readThread: Thread? = null
    private val executor = Executors.newSingleThreadExecutor()

    private val sppUuid: UUID =
        UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")

    private val requestPerms = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { granted ->
        if (granted.values.all { it }) {
            refreshDevices()
        } else {
            appendLog("\n缺少蓝牙权限，无法列出已配对设备。\n")
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        binding.btnRefresh.setOnClickListener { ensurePermsThenRefresh() }
        binding.btnConnect.setOnClickListener { connectSelected() }
        binding.btnDisconnect.setOnClickListener { disconnect() }
        binding.btnOv.setOnClickListener { sendLine("OV") }
        binding.btnOvSend.setOnClickListener { sendLine("OV+SEND") }
        binding.btnOvVersion.setOnClickListener { sendLine("OV+VERSION") }
        binding.btnCustomSend.setOnClickListener {
            val line = binding.editCustom.text?.toString()?.trim().orEmpty()
            if (line.isNotEmpty()) {
                sendLine(line)
            }
        }
        binding.btnClear.setOnClickListener { binding.logText.text = "" }

        ensurePermsThenRefresh()
    }

    private fun bluetoothPermissions(): Array<String> {
        return if (Build.VERSION.SDK_INT >= 31) {
            arrayOf(
                Manifest.permission.BLUETOOTH_CONNECT,
                Manifest.permission.BLUETOOTH_SCAN
            )
        } else {
            arrayOf(
                Manifest.permission.BLUETOOTH,
                Manifest.permission.BLUETOOTH_ADMIN,
                Manifest.permission.ACCESS_FINE_LOCATION
            )
        }
    }

    private fun havePerms(): Boolean {
        return bluetoothPermissions().all {
            ContextCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
        }
    }

    private fun ensurePermsThenRefresh() {
        if (havePerms()) {
            refreshDevices()
        } else {
            requestPerms.launch(bluetoothPermissions())
        }
    }

    private fun refreshDevices() {
        val adapter = bluetoothAdapter
        if (adapter == null) {
            binding.statusText.text = "状态：本机无蓝牙适配器"
            return
        }
        if (!adapter.isEnabled) {
            binding.statusText.text = "状态：请先打开手机蓝牙"
            Toast.makeText(this, "请先打开手机蓝牙", Toast.LENGTH_SHORT).show()
            return
        }
        val list: List<BluetoothDevice> = try {
            adapter.bondedDevices?.toList().orEmpty()
        } catch (se: SecurityException) {
            appendLog("\n读取已配对列表失败(权限): ${se.message}\n")
            emptyList()
        }
        val labels = list.map { d ->
            val name = try {
                d.name
            } catch (_: SecurityException) {
                null
            }.orEmpty().ifBlank { "未知名称" }
            "$name  ${d.address}"
        }
        binding.deviceSpinner.adapter = ArrayAdapter(
            this,
            android.R.layout.simple_spinner_dropdown_item,
            if (labels.isEmpty()) {
                listOf("(无已配对设备，请先在系统蓝牙里配对)")
            } else {
                labels
            }
        )
        @Suppress("UNCHECKED_CAST")
        binding.deviceSpinner.tag = if (list.isEmpty()) emptyList<BluetoothDevice>() else list
        binding.statusText.text = "状态：已配对 ${list.size} 台，请选择后点连接"
    }

    private fun selectedDevice(): BluetoothDevice? {
        @Suppress("UNCHECKED_CAST")
        val list = binding.deviceSpinner.tag as? List<BluetoothDevice> ?: return null
        if (list.isEmpty()) {
            return null
        }
        val idx = binding.deviceSpinner.selectedItemPosition
        if (idx < 0 || idx >= list.size) {
            return null
        }
        return list[idx]
    }

    private fun connectSelected() {
        val dev = selectedDevice()
        if (dev == null) {
            Toast.makeText(this, "请选择已配对设备", Toast.LENGTH_SHORT).show()
            return
        }
        disconnect()
        executor.execute {
            postStatus("状态：连接中…")
            try {
                bluetoothAdapter?.cancelDiscovery()
            } catch (_: SecurityException) {
            }
            val sock = tryOpenSocket(dev)
            if (sock == null) {
                postStatus("状态：连接失败（见下方日志）")
                return@execute
            }
            socket = sock
            postStatus("状态：已连接 ${dev.address}")
            startReadLoop(sock)
        }
    }

    private fun tryOpenSocket(dev: BluetoothDevice): BluetoothSocket? {
        return try {
            val sock = dev.createRfcommSocketToServiceRecord(sppUuid)
            sock.connect()
            sock
        } catch (e: IOException) {
            appendLog("\n标准 SPP UUID 失败: ${e.message}，尝试 insecure 通道 1…\n")
            try {
                @Suppress("DiscouragedPrivateApi")
                val m = dev.javaClass.getMethod("createRfcommSocket", Int::class.javaPrimitiveType)
                val sock = m.invoke(dev, 1) as BluetoothSocket
                sock.connect()
                sock
            } catch (e2: Exception) {
                appendLog("通道 1 仍失败: ${e2.message}\n")
                null
            }
        } catch (se: SecurityException) {
            appendLog("权限不足: ${se.message}\n")
            null
        }
    }

    private fun startReadLoop(sock: BluetoothSocket) {
        readThread?.interrupt()
        readThread = Thread {
            val input: InputStream = try {
                sock.inputStream
            } catch (e: IOException) {
                appendLog("\n打开输入流失败: ${e.message}\n")
                return@Thread
            }
            val buf = ByteArray(2048)
            try {
                while (!Thread.currentThread().isInterrupted) {
                    val n = input.read(buf)
                    if (n <= 0) {
                        break
                    }
                    val chunk = String(buf, 0, n, StandardCharsets.UTF_8)
                    runOnUiThread { appendLog(chunk) }
                }
            } catch (_: IOException) {
                // 正常关闭或断线
            } finally {
                runOnUiThread {
                    if (socket === sock) {
                        postStatus("状态：连接已关闭")
                    }
                }
            }
        }.also { it.start() }
    }

    private fun disconnect() {
        readThread?.interrupt()
        readThread = null
        try {
            socket?.close()
        } catch (_: IOException) {
        }
        socket = null
        binding.statusText.text = "状态：未连接"
    }

    private fun sendLine(cmd: String) {
        val sock = socket
        if (sock == null) {
            Toast.makeText(this, "请先连接", Toast.LENGTH_SHORT).show()
            return
        }
        val line = if (cmd.endsWith("\r") || cmd.endsWith("\n")) cmd else "$cmd\r\n"
        executor.execute {
            try {
                sock.outputStream.write(line.toByteArray(StandardCharsets.UTF_8))
                sock.outputStream.flush()
                runOnUiThread { appendLog("\n>>> $line") }
            } catch (e: IOException) {
                runOnUiThread {
                    appendLog("\n发送失败: ${e.message}\n")
                }
            }
        }
    }

    private fun appendLog(text: String) {
        binding.logText.append(text)
    }

    private fun postStatus(s: String) {
        runOnUiThread { binding.statusText.text = s }
    }

    override fun onDestroy() {
        disconnect()
        executor.shutdown()
        super.onDestroy()
    }
}
