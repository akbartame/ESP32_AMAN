function confirmResetWifi() {
    if (confirm("Apakah Anda yakin ingin mereset WiFi?")) {
        resetWifi();
    }
}

function resetWifi() {
    // Fungsi ini akan mengirim permintaan untuk mereset WiFi
    fetch('/wifi-reset', { method: 'POST' })
        .then(response => response.text())
        .then(result => alert("WiFi berhasil direset"))
        .catch(error => alert("Gagal mereset WiFi: " + error));
}