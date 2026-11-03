// Each command the website may call has to be listed here AND allowed in capabilities/default.json,
// otherwise Tauri refuses it for a remote page ("Command ... not allowed by ACL").
fn main() {
    let commands = &["status", "pick_folder", "update_plugin", "save_key", "pick_exe", "launch", "open_url", "cred_get", "cred_save", "cred_forget", "open_nattrak", "open_popout"];
    tauri_build::try_build(tauri_build::Attributes::new().app_manifest(tauri_build::AppManifest::new().commands(commands)))
        .expect("failed to run the Tauri build step");
}
