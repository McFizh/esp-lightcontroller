package org.mcfish.lightcontroller

import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import android.os.Bundle
import android.os.Build
import android.os.ext.SdkExtensions
import androidx.activity.enableEdgeToEdge
import androidx.appcompat.app.AppCompatActivity
import androidx.activity.viewModels
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat
import org.mcfish.lightcontroller.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {
    private lateinit var nsdManager: NsdManager
    private val viewModel: ConnectionViewModel by viewModels()
    private lateinit var binding: ActivityMainBinding
    private var isDiscoveryRunning = false

    /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
       mDNS service discovery
       :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  */
    private val discoveryListener = object : NsdManager.DiscoveryListener {
        override fun onDiscoveryStarted(regType: String) {
            isDiscoveryRunning = true
        }

        override fun onServiceFound(service: NsdServiceInfo) {

            val serviceDiscoveryListener = object : NsdManager.ResolveListener {
                override fun onResolveFailed(p0: NsdServiceInfo?, p1: Int) {
                }

                override fun onServiceResolved(service: NsdServiceInfo) {
                    val host =
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU && SdkExtensions.getExtensionVersion(
                                Build.VERSION_CODES.TIRAMISU
                            ) >= 7
                        ) {
                            service.hostAddresses.firstOrNull()
                        } else {
                            @Suppress("DEPRECATION")
                            service.host
                        }
                    val hostAddress = host?.hostAddress
                    if (hostAddress != null) {
                        viewModel.setControllerFound(hostAddress, service.port)
                    }
                }
            }

            // Fixme: This should be fixed soon(ish)
            @Suppress("DEPRECATION")
            nsdManager.resolveService(service, serviceDiscoveryListener)
        }

        override fun onServiceLost(service: NsdServiceInfo) {
        }

        override fun onDiscoveryStopped(serviceType: String) {
            isDiscoveryRunning = false
        }

        override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {
            isDiscoveryRunning = false
            nsdManager.stopServiceDiscovery(this)
        }

        override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {
            isDiscoveryRunning = false
            nsdManager.stopServiceDiscovery(this)
        }
    }

    /* :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
       Lifecycle events
       :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  */
    override fun onCreate(savedInstanceState: Bundle?) {
        // Call enableEdgeToEdge first (before super.oncreate) to make sure that screen doesn't
        // flicker on start.
        enableEdgeToEdge()
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        setSupportActionBar(binding.toolbar)

        ViewCompat.setOnApplyWindowInsetsListener(binding.root) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }
    }

    override fun onStart() {
        super.onStart()
        nsdManager = getSystemService(NSD_SERVICE) as NsdManager
        nsdManager.discoverServices(
            "_lightctrl._tcp",
            NsdManager.PROTOCOL_DNS_SD,
            discoveryListener
        )
    }

    override fun onStop() {
        if (isDiscoveryRunning) {
            nsdManager.stopServiceDiscovery(discoveryListener)
            isDiscoveryRunning = false
        }
        super.onStop()
    }
}
