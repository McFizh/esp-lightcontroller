package org.mcfish.lightcontroller

import android.content.Context
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.activity.viewModels
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {
    private lateinit var nsdManager : NsdManager
    private val viewModel: ConnectionViewModel by viewModels()

    /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
       mDNS service discovery
       :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  */
    private val serviceDiscoveryListener = object : NsdManager.ResolveListener {
        override fun onResolveFailed(p0: NsdServiceInfo?, p1: Int) {
        }

        override fun onServiceResolved(service: NsdServiceInfo) {
            viewModel.setControllerFound(service.host.hostAddress, service.port)
        }
    }

    private val discoveryListener = object : NsdManager.DiscoveryListener {
        override fun onDiscoveryStarted(regType: String) {
        }

        override fun onServiceFound(service: NsdServiceInfo) {
            nsdManager.resolveService(service, serviceDiscoveryListener)
        }

        override fun onServiceLost(service: NsdServiceInfo) {
        }

        override fun onDiscoveryStopped(serviceType: String) {
        }

        override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {
            nsdManager.stopServiceDiscovery(this)
        }

        override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {
            nsdManager.stopServiceDiscovery(this)
        }
    }

    /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
       Lifecycle events
       :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  */
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)
        setSupportActionBar(toolbar)
    }

    override fun onStart() {
        super.onStart()
        nsdManager = getSystemService(Context.NSD_SERVICE) as NsdManager
        nsdManager.discoverServices(
            "_lightctrl._tcp",
            NsdManager.PROTOCOL_DNS_SD,
            discoveryListener
        )
    }

    override fun onStop() {
        nsdManager.stopServiceDiscovery(discoveryListener)
        super.onStop()
    }
}
