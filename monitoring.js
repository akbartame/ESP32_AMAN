// Function to update sensor data
function updateSensorData() {
    fetch('/sensor-data')
      .then(response => response.json())
      .then(data => {
        // Update gas and vibration sensor values
        document.getElementById('gasValue').textContent = data.gas;
        document.getElementById('vibration1').textContent = data.vibration1;
        document.getElementById('vibration2').textContent = data.vibration2;
        document.getElementById('vibration3').textContent = data.vibration3;
        document.getElementById('vibration4').textContent = data.vibration4;
        document.getElementById('vibration1Total').textContent = data.vibration1Total;
        document.getElementById('vibration2Total').textContent = data.vibration2Total;
        document.getElementById('vibration3Total').textContent = data.vibration3Total;
        document.getElementById('vibration4Total').textContent = data.vibration4Total;

        // Update safety status
        let statusText = '';
        switch (data.status) {
          case 0:
            statusText = 'Aman';
            break;
          case 1:
            statusText = 'Rawan';
            break;
          case 2:
            statusText = 'Bahaya';
            break;
          default:
            statusText = 'Unknown';
        }
        document.getElementById('safetyStatus').textContent = statusText;
      })
      .catch(error => console.error('Error fetching sensor data:', error));
  }

  // Refresh data every 5 seconds
  setInterval(updateSensorData, 5000);
  // Initial data load
  updateSensorData();