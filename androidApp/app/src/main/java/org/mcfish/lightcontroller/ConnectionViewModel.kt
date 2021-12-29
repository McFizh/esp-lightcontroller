package org.mcfish.lightcontroller

import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.PrintWriter
import java.net.Socket
import java.net.SocketTimeoutException

class ConnectionViewModel : ViewModel() {
    private val _connectionState = MutableLiveData<AppState>()
    val connectionState: LiveData<AppState> = _connectionState

    private var serverAddr = ""
    private var serverPort = 0

    init {
        _connectionState.value = AppState.STATE_SEARCHING
    }

    fun setControllerFound(address: String, port: Int) {
        if( (address != serverAddr || port != serverPort) && _connectionState.value == AppState.STATE_SEARCHING) {
            serverAddr = address
            serverPort = port
            Log.d(Constants.LOG_TAG, "Controller found @ ${address}:${port}")
            launchConnect()
        }
    }

    fun getAddressAndPort(): String {
        return "${serverAddr}:${serverPort}"
    }

    fun launchConnect() {
        if(_connectionState.value != AppState.STATE_SEARCHING && _connectionState.value != AppState.STATE_FAILED) {
            return
        }

        viewModelScope.launch {
            Log.d(Constants.LOG_TAG, "Connecting to controller...")
            _connectionState.postValue(AppState.STATE_CONNECTING)
            connectToController(serverAddr, serverPort)
        }
    }

    private suspend fun connectToController(address: String, port: Int)
            = withContext(Dispatchers.IO) {
        val socket = Socket(address, port)
        val socketOut = PrintWriter(socket.getOutputStream())
        val socketIn = BufferedReader(InputStreamReader(socket.getInputStream()))

        Log.d(Constants.LOG_TAG, "Socker connected to: ${address}:${port}")

        socket.soTimeout = 2000
        socketOut.write("PING\n")
        socketOut.flush()

        var retryCount = 2
        var pongReceived = false

        while(retryCount > 0) {
            try {
                val text = socketIn.readLine().trim()
                if(text == "PONG") {
                    pongReceived = true
                    break
                }
            } catch(e: SocketTimeoutException) {
                Log.d(Constants.LOG_TAG, "Timeout .. Served failed to respond to PING")
            }
            retryCount--
        }

        if(!pongReceived) {
            Log.d(Constants.LOG_TAG, "Fatal .. Served failed to respond to PING")
            _connectionState.postValue(AppState.STATE_FAILED)
            socket.close()
        } else {
            Log.d(Constants.LOG_TAG, "Connected to server")
            _connectionState.postValue(AppState.STATE_CONNECTED)
        }
    }
}
