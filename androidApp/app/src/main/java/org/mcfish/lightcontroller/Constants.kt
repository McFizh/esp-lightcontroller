package org.mcfish.lightcontroller

enum class AppState {
    STATE_SEARCHING,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_FAILED
}

class Constants {
    companion object {
        const val LOG_TAG = "LightShow"
    }
}
