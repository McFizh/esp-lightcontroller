package org.mcfish.lightcontroller

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.fragment.app.activityViewModels
import androidx.lifecycle.Observer
import androidx.navigation.fragment.findNavController
import kotlinx.android.synthetic.main.fragment_init.*

class InitFragment : Fragment() {
    private val viewModel: ConnectionViewModel by activityViewModels()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.fragment_init, container, false)
    }

    private fun onConnectingState() {
        val serverAddr = viewModel.getAddressAndPort()
        infoTextRow1.text = "Ohjain löydetty osoitteesta:\n" + serverAddr + "\n\n"
        infoTextRow1.visibility = View.VISIBLE
        infoTextRow2.text = "Yhdistetään..."
        reconnButton.visibility = View.GONE
    }

    private fun onFailedState() {
        infoTextRow2.text = "Ohjaimeen ei saatu yhteyttä"
        progressBar.visibility = View.GONE
        reconnButton.visibility = View.VISIBLE
    }

    private fun onConnectedState() {
        findNavController().navigate(R.id.action_FirstFragment_to_SecondFragment)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        // Handle state changes
        viewModel.connectionState.observe(viewLifecycleOwner, Observer { state ->
            when(state) {
                AppState.STATE_CONNECTING -> onConnectingState()
                AppState.STATE_FAILED -> onFailedState()
                AppState.STATE_CONNECTED -> onConnectedState()
                AppState.STATE_SEARCHING -> {}
            }
        })

        // Reconnect button action
        view.findViewById<Button>(R.id.reconnButton).setOnClickListener {
            progressBar.visibility = View.VISIBLE
            reconnButton.visibility = View.GONE
            infoTextRow2.text = "Yhdistetään..."
            viewModel.launchConnect()
        }
    }

    override fun onStart() {
        super.onStart()
    }

    override fun onStop() {
        super.onStop()
    }
}