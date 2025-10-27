from flask import Flask, render_template, jsonify
import serial
import json
import threading

app = Flask(__name__)

# Serial port configuration
SERIAL_PORT = 'COM4'  # Replace with your Arduino's serial port
BAUD_RATE = 9600
ser = None

data_lock = threading.Lock()
sensor_data = {'slot1': 'vacant', 'slot2': 'vacant', 'slot3': 'vacant'}

# Function to open serial port
def open_serial_port():
    global ser
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Serial port {SERIAL_PORT} opened successfully.")
    except serial.SerialException as e:
        print(f"Failed to open serial port {SERIAL_PORT}: {str(e)}")
        ser = None

# Function to read from serial port and update sensor_data
def read_serial():
    global sensor_data
    while True:
        if ser and ser.is_open:
            try:
                line = ser.readline().decode('utf-8').strip()
                if line.startswith('D:'):
                    distances = [int(d) for d in line[2:].split(',')]
                    with data_lock:
                        sensor_data = parse_led_data(distances)
            except UnicodeDecodeError as e:
                print(f"UnicodeDecodeError: {e}")
            except Exception as e:
                print(f"Error reading serial data: {str(e)}")

# Function to parse distances and update LED status
def parse_led_data(distances):
    result = {}
    for i in range(3):
        sensor1 = distances[2 * i]
        sensor2 = distances[2 * i + 1]
        if sensor1 > 4 and sensor2 > 4:
            result[f'slot{i+1}'] = 'vacant'
        elif abs(sensor1 - sensor2) <= 1 and sensor1 <= 4 and sensor2 <= 4:
            result[f'slot{i+1}'] = 'occupied'
        elif (sensor1 > 4 and sensor2 <= 4) or (sensor2 > 4 and sensor1 <= 4):
            result[f'slot{i+1}'] = 'misaligned'
        else:
            result[f'slot{i+1}'] = 'error'
    return result

# Route to serve the index.html template
@app.route('/')
def index():
    return render_template('index.html')

# Route to fetch LED statuses from Arduino
@app.route('/led_status')
def get_led_status():
    with data_lock:
        return jsonify(sensor_data)

if __name__ == '__main__':
    open_serial_port()  # Open serial port before starting Flask app
    threading.Thread(target=read_serial, daemon=True).start()
    app.run(debug=True)  # Run Flask app
