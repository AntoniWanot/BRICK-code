// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
mod device;

use device::{DeviceManager, DeviceStatus};
use std::sync::Mutex;
use tauri::State;

pub struct AppState {
    device_manager: Mutex<DeviceManager>,
}

#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

#[tauri::command]
fn scan_ports() -> Result<Vec<String>, String> {
    DeviceManager::scan_ports()
}

#[tauri::command]
fn connect_device(
    state: State<'_, AppState>,
    device_name: String,
    port: String,
    baud_rate: u32,
) -> Result<String, String> {
    let manager = state.device_manager.lock().unwrap();
    manager.connect(&device_name, &port, baud_rate)?;
    Ok(format!("Connected {} on port {}", device_name, port))
}

#[tauri::command]
fn send_program(
    state: State<'_, AppState>,
    device_name: String,
    program_json: String,
) -> Result<String, String> {
    let manager = state.device_manager.lock().unwrap();
    manager.send_command(&device_name, &program_json)
}

#[tauri::command]
fn get_connected_devices(state: State<'_, AppState>) -> Result<Vec<DeviceStatus>, String> {
    let manager = state.device_manager.lock().unwrap();
    manager.get_devices()
}

#[tauri::command]
fn disconnect_device(state: State<'_, AppState>, device_name: String) -> Result<(), String> {
    let manager = state.device_manager.lock().unwrap();
    manager.disconnect(&device_name)
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .manage(AppState {
            device_manager: Mutex::new(DeviceManager::new()),
        })
        .invoke_handler(tauri::generate_handler![
            greet,
            scan_ports,
            connect_device,
            send_program,
            get_connected_devices,
            disconnect_device
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
