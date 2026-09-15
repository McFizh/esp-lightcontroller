package org.mcfish.lightcontroller

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.activityViewModels
import androidx.lifecycle.Observer
import androidx.navigation.fragment.findNavController
import org.mcfish.lightcontroller.databinding.FragmentInitBinding

/*
 Default fragment, which kicks of the connection process
 */
class InitFragment : Fragment() {
    private val viewModel: ConnectionViewModel by activityViewModels()
    private var _binding: FragmentInitBinding? = null
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentInitBinding.inflate(inflater, container, false)
        return binding.root
    }

    private fun onConnectingState() {
        val serverAddr = viewModel.getAddressAndPort()
        binding.infoTextRow1.text = getString(R.string.controller_found, serverAddr)
        binding.infoTextRow1.visibility = View.VISIBLE
        binding.infoTextRow2.text = getString(R.string.connecting)
        binding.reconnButton.visibility = View.GONE
    }

    private fun onFailedState() {
        binding.infoTextRow2.text = getString(R.string.connection_failed)
        binding.progressBar.visibility = View.GONE
        binding.reconnButton.visibility = View.VISIBLE
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
        binding.reconnButton.setOnClickListener {
            binding.progressBar.visibility = View.VISIBLE
            binding.reconnButton.visibility = View.GONE
            binding.infoTextRow2.text = getString(R.string.connecting)
            viewModel.launchConnect()
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

    override fun onStart() {
        super.onStart()
    }

    override fun onStop() {
        super.onStop()
    }
}