/*
    LibrePods - AirPods liberated from Apple's ecosystem
    Copyright (C) 2025 LibrePods contributors

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

using System.Windows;
using System.Windows.Controls;
using LibrePods.Core.Models;
using LibrePods.Core.Utils;
using LibrePods.Windows.Services;

namespace LibrePods.Windows;

public partial class MainWindow : Window
{
    private readonly AirPodsService _airPodsService;

    public MainWindow()
    {
        InitializeComponent();
        
        _airPodsService = ((App)Application.Current).Resources["AirPodsService"] as AirPodsService 
            ?? new AirPodsService();
        
        // Subscribe to events
        _airPodsService.OnConnectionChanged += OnConnectionChanged;
        _airPodsService.OnStatusUpdate += OnStatusUpdate;
        _airPodsService.OnBatteryUpdate += OnBatteryUpdate;
        _airPodsService.OnEarDetectionChanged += OnEarDetectionChanged;
        _airPodsService.OnNoiseControlChanged += OnNoiseControlChanged;

        Loaded += MainWindow_Loaded;
        Closing += MainWindow_Closing;
    }

    private async void MainWindow_Loaded(object sender, RoutedEventArgs e)
    {
        try
        {
            await _airPodsService.InitializeAsync();
            UpdateUI(_airPodsService.CurrentStatus);
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to initialize AirPodsService", ex);
            MessageBox.Show($"Failed to initialize: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }

    private void MainWindow_Closing(object? sender, System.ComponentModel.CancelEventArgs e)
    {
        // Minimize to tray instead of closing
        e.Cancel = true;
        Hide();
    }

    private async void ConnectButton_Click(object sender, RoutedEventArgs e)
    {
        try
        {
            if (_airPodsService.IsConnected)
            {
                _airPodsService.Disconnect();
            }
            else
            {
                // Scan for devices
                var devices = await _airPodsService.ScanForAirPodsAsync();
                
                if (devices.Count == 0)
                {
                    MessageBox.Show("No AirPods found. Please make sure they are paired with Windows.", 
                        "No Devices", MessageBoxButton.OK, MessageBoxImage.Information);
                    return;
                }

                // For simplicity, connect to the first device found
                // In production, show a device selection dialog
                var device = devices[0];
                
                // Extract Bluetooth address from device
                // Get the BluetoothDevice to access the BluetoothAddress property
                string btAddress = string.Empty;
                
                // Try to get Bluetooth address from device properties first
                if (device.Properties.TryGetValue("System.Devices.Aep.DeviceAddress", out object? addressObj))
                {
                    btAddress = addressObj?.ToString() ?? string.Empty;
                    Logger.Info($"Got Bluetooth address from device properties: {btAddress}");
                }
                
                // If property not found, try to get BluetoothDevice and extract address from it
                if (string.IsNullOrEmpty(btAddress))
                {
                    Logger.Info("Bluetooth address not in properties, trying to get from BluetoothDevice");
                    
                    // Try to get the BluetoothDevice from the DeviceInformation
                    try
                    {
                        var btDevice = await Windows.Devices.Bluetooth.BluetoothDevice.FromIdAsync(device.Id);
                        if (btDevice != null)
                        {
                            // Convert the Bluetooth address to hex string format
                            btAddress = btDevice.BluetoothAddress.ToString("X12");
                            Logger.Info($"Got Bluetooth address from BluetoothDevice: {btAddress}");
                            btDevice.Dispose();
                        }
                        else
                        {
                            Logger.Error("Could not get BluetoothDevice from device ID");
                            MessageBox.Show("Failed to get device information. Please ensure AirPods are properly paired.", 
                                "Connection Error", MessageBoxButton.OK, MessageBoxImage.Error);
                            return;
                        }
                    }
                    catch (Exception ex)
                    {
                        Logger.Error("Error getting BluetoothDevice", ex);
                        MessageBox.Show($"Failed to access device: {ex.Message}", 
                            "Connection Error", MessageBoxButton.OK, MessageBoxImage.Error);
                        return;
                    }
                }
                
                await _airPodsService.ConnectAsync(btAddress);
            }
        }
        catch (Exception ex)
        {
            Logger.Error("Error connecting/disconnecting", ex);
            MessageBox.Show($"Error: {ex.Message}", "Connection Error", MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }

    private void OnConnectionChanged(bool isConnected)
    {
        Dispatcher.Invoke(() =>
        {
            ConnectionStatus.Text = isConnected ? "Connected" : "Disconnected";
            ConnectButton.Content = isConnected ? "Disconnect" : "Connect";
            
            // Enable/disable controls based on connection
            NoiseControlOff.IsEnabled = isConnected;
            NoiseControlTransparency.IsEnabled = isConnected;
            NoiseControlAdaptiveTransparency.IsEnabled = isConnected;
            NoiseControlANC.IsEnabled = isConnected;
            ConversationalAwarenessCheckBox.IsEnabled = isConnected;
        });
    }

    private void OnStatusUpdate(AirPodsStatus status)
    {
        Dispatcher.Invoke(() => UpdateUI(status));
    }

    private void OnBatteryUpdate(List<BatteryInfo> batteries)
    {
        Dispatcher.Invoke(() =>
        {
            foreach (var battery in batteries)
            {
                UpdateBatteryDisplay(battery);
            }
        });
    }

    private void OnEarDetectionChanged(EarDetectionStatus status)
    {
        Dispatcher.Invoke(() =>
        {
            LeftEarText.Text = status.LeftInEar ? "In Ear" : "Out";
            RightEarText.Text = status.RightInEar ? "In Ear" : "Out";
        });
    }

    private void OnNoiseControlChanged(NoiseControlMode mode)
    {
        Dispatcher.Invoke(() =>
        {
            // Update radio buttons without triggering event
            NoiseControlOff.Checked -= NoiseControlMode_Changed;
            NoiseControlTransparency.Checked -= NoiseControlMode_Changed;
            NoiseControlAdaptiveTransparency.Checked -= NoiseControlMode_Changed;
            NoiseControlANC.Checked -= NoiseControlMode_Changed;

            switch (mode)
            {
                case NoiseControlMode.Off:
                    NoiseControlOff.IsChecked = true;
                    break;
                case NoiseControlMode.Transparency:
                    NoiseControlTransparency.IsChecked = true;
                    break;
                case NoiseControlMode.AdaptiveTransparency:
                    NoiseControlAdaptiveTransparency.IsChecked = true;
                    break;
                case NoiseControlMode.NoiseCancellation:
                    NoiseControlANC.IsChecked = true;
                    break;
            }

            NoiseControlOff.Checked += NoiseControlMode_Changed;
            NoiseControlTransparency.Checked += NoiseControlMode_Changed;
            NoiseControlAdaptiveTransparency.Checked += NoiseControlMode_Changed;
            NoiseControlANC.Checked += NoiseControlMode_Changed;
        });
    }

    private void UpdateUI(AirPodsStatus status)
    {
        // Update battery
        if (status.LeftBattery != null)
            UpdateBatteryDisplay(status.LeftBattery);
        if (status.RightBattery != null)
            UpdateBatteryDisplay(status.RightBattery);
        if (status.CaseBattery != null)
            UpdateBatteryDisplay(status.CaseBattery);

        // Update ear detection
        LeftEarText.Text = status.EarDetection.LeftInEar ? "In Ear" : "Out";
        RightEarText.Text = status.EarDetection.RightInEar ? "In Ear" : "Out";

        // Update conversational awareness
        ConversationalAwarenessCheckBox.IsChecked = status.ConversationalAwarenessEnabled;
        ConversationalAwarenessStatus.Text = status.ConversationalAwarenessActive 
            ? "Status: Active" 
            : "Status: Inactive";

        // Update device name
        DeviceNameTextBox.Text = status.DeviceName;
    }

    private void UpdateBatteryDisplay(BatteryInfo battery)
    {
        string statusText = battery.Status switch
        {
            BatteryStatus.Charging => "Charging",
            BatteryStatus.Discharging => "",
            BatteryStatus.Disconnected => "Disconnected",
            _ => ""
        };

        switch (battery.Component)
        {
            case BatteryComponent.Left:
                LeftBatteryText.Text = $"{battery.Level}%";
                LeftStatusText.Text = statusText;
                break;
            case BatteryComponent.Right:
                RightBatteryText.Text = $"{battery.Level}%";
                RightStatusText.Text = statusText;
                break;
            case BatteryComponent.Case:
                CaseBatteryText.Text = $"{battery.Level}%";
                CaseStatusText.Text = statusText;
                break;
        }
    }

    private async void NoiseControlMode_Changed(object sender, RoutedEventArgs e)
    {
        if (sender is not RadioButton rb || rb.IsChecked != true)
            return;

        try
        {
            var mode = rb.Name switch
            {
                nameof(NoiseControlOff) => NoiseControlMode.Off,
                nameof(NoiseControlTransparency) => NoiseControlMode.Transparency,
                nameof(NoiseControlAdaptiveTransparency) => NoiseControlMode.AdaptiveTransparency,
                nameof(NoiseControlANC) => NoiseControlMode.NoiseCancellation,
                _ => NoiseControlMode.Off
            };

            await _airPodsService.SetNoiseControlModeAsync(mode);
        }
        catch (Exception ex)
        {
            Logger.Error("Error setting noise control mode", ex);
        }
    }

    private async void ConversationalAwareness_Changed(object sender, RoutedEventArgs e)
    {
        try
        {
            await _airPodsService.SetConversationalAwarenessAsync(
                ConversationalAwarenessCheckBox.IsChecked == true);
        }
        catch (Exception ex)
        {
            Logger.Error("Error setting conversational awareness", ex);
        }
    }

    private async void RenameDevice_Click(object sender, RoutedEventArgs e)
    {
        try
        {
            var newName = DeviceNameTextBox.Text?.Trim();
            if (string.IsNullOrEmpty(newName))
            {
                MessageBox.Show("Please enter a valid device name.", "Invalid Name", 
                    MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            await _airPodsService.RenameAsync(newName);
            MessageBox.Show("Device renamed successfully. You may need to re-pair for the name to take effect.", 
                "Success", MessageBoxButton.OK, MessageBoxImage.Information);
        }
        catch (Exception ex)
        {
            Logger.Error("Error renaming device", ex);
            MessageBox.Show($"Error: {ex.Message}", "Rename Error", 
                MessageBoxButton.OK, MessageBoxImage.Error);
        }
    }
}
