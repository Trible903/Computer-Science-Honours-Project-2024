import serial
import csv
import time
from datetime import datetime

# Set up the serial connection (make sure the port matches your Arduino)
ser = serial.Serial('COM3', 9600, timeout=1)  # Change 'COM3' to your Arduino port

# Real World Coordinate
constant_x = 2
constant_y = 1

# CSV file with a unique timestamp in the filename, including constant_x and constant_y
timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
csv_file = f'measurement_data_(0-3)_{constant_x}_{constant_y}_{timestamp}.csv'

try:
    # Open the CSV file once at the beginning
    with open(csv_file, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Write header to the CSV
        writer.writerow(['Timestamp', 'X Coordinate', 'Y Coordinate', 'Distance', 'Button State',
                         'StationOne Distance', 'StationTwo Distance', 'StationThree Distance'])

        # Start reading from Arduino
        while True:
            if ser.in_waiting > 0:
                # Read the line from the serial port
                line = ser.readline().decode('utf-8').strip()

                # Split the line into x and y coordinates, button state, and distances
                try:
                    # Expecting format: x,y,button_state,stationOneDistance,stationTwoDistance,stationThreeDistance
                    x, y, button_state, stationOneDist, stationTwoDist, stationThreeDist = line.split(',')
                    x, y = map(float, (x, y))  # Convert x and y to float
                    button_state = button_state.strip()  # Button state (e.g., "HIGH" or "LOW")
                    stationOneDist = float(stationOneDist)
                    stationTwoDist = float(stationTwoDist)
                    stationThreeDist = float(stationThreeDist)
                    current_time = time.strftime("%Y-%m-%d %H:%M:%S")

                    # Calculate Euclidean distance for the latest coordinate
                    distance = ((constant_x - x)**2 + (constant_y - y)**2)**0.5

                    # Write to CSV file
                    writer.writerow([current_time, x, y, distance, button_state,
                                     stationOneDist, stationTwoDist, stationThreeDist])
                    print("Success: Read Value")
                except ValueError:
                    print("Error: Could not convert data to float or parse button state.")

except KeyboardInterrupt:
    print("Data collection stopped.")

finally:
    ser.close()  # Ensure the serial connection is closed
