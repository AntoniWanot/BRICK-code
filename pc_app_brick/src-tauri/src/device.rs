use serde::{Deserialize, Serialize};
use std::sync::Mutex;
use std::time::Duration;

#[allow(dead_code)]
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RobotCommand {
    pub device: String,  
    pub program: serde_json::Value,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DeviceStatus {
    pub device: String,
    pub connected: bool,
    pub port: String,
}

pub struct DeviceManager {
    devices: Mutex<Vec<DeviceStatus>>,
}

impl DeviceManager {
    pub fn new() -> Self {
        DeviceManager {
            devices: Mutex::new(Vec::new()),
        }
    }

    pub fn scan_ports() -> Result<Vec<String>, String> {
        match serialport::available_ports() {
            Ok(ports) => {
                let port_names: Vec<String> = ports
                    .iter()
                    .map(|p| p.port_name.clone())
                    .collect();
                Ok(port_names)
            }
            Err(e) => Err(format!("Failed to scan ports: {}", e)),
        }
    }

    /// Connect to a device on specified port
    pub fn connect(&self, device_name: &str, port: &str, baud_rate: u32) -> Result<(), String> {
        let available_ports = Self::scan_ports()?;
        if !available_ports.contains(&port.to_string()) {
            return Err(format!("Port {} not found", port));
        }

        // Test connection by opening and closing
        match serialport::new(port, baud_rate)
            .timeout(Duration::from_secs(2))
            .open()
        {
            Ok(_port) => {
                let mut devices = self.devices.lock().unwrap();
                devices.push(DeviceStatus {
                    device: device_name.to_string(),
                    connected: true,
                    port: port.to_string(),
                });
                Ok(())
            }
            Err(e) => Err(format!("Failed to connect to {}: {}", port, e)),
        }
    }

    /// Send command to device
    pub fn send_command(
        &self,
        device_name: &str,
        program_json: &str,
    ) -> Result<String, String> {
        let devices = self.devices.lock().unwrap();

        // Find device
        let device = devices
            .iter()
            .find(|d| d.device == device_name && d.connected)
            .ok_or_else(|| format!("Device {} not connected", device_name))?;

        // Open port and send
        match serialport::new(&device.port, 115200)
            .timeout(Duration::from_secs(5))
            .open()
        {
            Ok(mut port) => {
                // Send JSON as bytes followed by newline
                let data = format!("{}\n", program_json);
                match port.write_all(data.as_bytes()) {
                    Ok(_) => {
                        let mut response = vec![0; 1024];
                        match port.read(&mut response) {
                            Ok(n) => {
                                let resp_str =
                                    String::from_utf8_lossy(&response[..n]).to_string();
                                Ok(format!("Command sent. Response: {}", resp_str))
                            }
                            Err(_) => Ok("Command sent (no response)".to_string()),
                        }
                    }
                    Err(e) => Err(format!("Failed to send command: {}", e)),
                }
            }
            Err(e) => Err(format!("Failed to open port {}: {}", device.port, e)),
        }
    }

    pub fn get_devices(&self) -> Result<Vec<DeviceStatus>, String> {
        let devices = self.devices.lock().unwrap();
        Ok(devices.clone())
    }

    pub fn disconnect(&self, device_name: &str) -> Result<(), String> {
        let mut devices = self.devices.lock().unwrap();
        devices.retain(|d| d.device != device_name);
        Ok(())
    }
}
