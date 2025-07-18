import tkinter as tk
from tkinter import ttk
import serial
import json
import threading
import time
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# Serial config — change port if needed
SERIAL_PORT = '/dev/ttyUSB0'  # For Linux/macOS
BAUD_RATE = 115200

try:
    ser = serial.Serial()
    ser.port = SERIAL_PORT
    ser.baudrate = BAUD_RATE
    ser.timeout = 1
    ser.dsrdtr = False
    ser.setRTS(False)
    ser.open()
    ser.setDTR(False)
    time.sleep(2)
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    ser = None

stop_event = threading.Event()
time_data = []
air_temp_data = []
bed_temp_data = []
humidity_data = []


def send_json(preset):
    if ser and ser.is_open:
        if preset == "OFF":
            data = {"preset": "OFF"}
        else:
            data = {"preset": preset, "status": "RUN"}
        json_str = json.dumps(data)
        ser.write((json_str + "\n").encode('utf-8'))
        print(f"Sent: {json_str}")
    else:
        print("Serial port not available")


def send_custom_values():
    if ser and ser.is_open:
        try:
            data = {}
            if stepper_check.get():
                data["isStepperOn"] = int(stepper_var.get())
            if fan1_check.get():
                data["isFan1On"] = int(fan1_var.get())
            if fan2_check.get():
                data["isFan2On"] = int(fan2_var.get())
            if debug_check.get():
                data["debug"] = int(debug_var.get())
            if dry_timer_check.get():
                data["dryTimer"] = int(dry_timer_var.get())
            if target_air_temp_check.get():
                data["targetAirTemp"] = int(float(target_air_temp_var.get()))
            if target_bed_temp_check.get():
                data["targetBedTemp"] = int(float(target_bed_temp_var.get()))

            if data:
                json_str = json.dumps(data)
                ser.write((json_str + "\n").encode('utf-8'))
                print(f"Sent custom values: {json_str}")
            else:
                print("No values selected to send.")
        except ValueError as e:
            print("Invalid input:", e)


def read_serial():
    while not stop_event.is_set():
        if ser and ser.is_open:
            try:
                if ser.in_waiting:
                    line = ser.readline().decode('utf-8').strip()
                    if line:
                        data = json.loads(line)
                        update_data(data)
            except (json.JSONDecodeError, UnicodeDecodeError):
                pass
            except serial.SerialException as e:
                print(f"Serial error: {e}")
        time.sleep(0.1)


def update_data(data):
    timestamp = time.time()
    if "currentAirTemp" in data:
        time_data.append(timestamp)
        air_temp_data.append(data["currentAirTemp"])
        bed_temp_data.append(data["currentBedTemp"])
        humidity_data.append(data["currentAbsHumidity"])
        if len(time_data) > 3600:
            time_data.pop(0)
            air_temp_data.pop(0)
            bed_temp_data.pop(0)
            humidity_data.pop(0)

        status_var.set(data.get("status", ""))
        preset_var.set(data.get("preset", ""))

        air_temp_var.set(f"{data['currentAirTemp']:.1f}")
        bed_temp_var.set(f"{data['currentBedTemp']:.1f}")
        humidity_var.set(f"{data['currentAbsHumidity']:.1f}")

        update_plot()


def update_plot():
    ax.clear()
    ax.plot(time_data, air_temp_data, label="Air Temp (°C)")
    ax.plot(time_data, bed_temp_data, label="Bed Temp (°C)")
    ax.plot(time_data, humidity_data, label="Humidity (%)")
    ax.legend(loc='upper left')
    ax.set_xlabel("Time")
    ax.set_ylabel("Value")
    canvas.draw()


def on_closing():
    stop_event.set()
    time.sleep(0.2)
    if ser and ser.is_open:
        print("Closing serial port...")
        ser.close()
    root.destroy()


def create_gui():
    global status_var, preset_var, air_temp_var, bed_temp_var, humidity_var
    global stepper_var, fan1_var, fan2_var, debug_var, dry_timer_var, target_air_temp_var, target_bed_temp_var
    global stepper_check, fan1_check, fan2_check, debug_check, dry_timer_check, target_air_temp_check, target_bed_temp_check
    global fig, ax, canvas, root

    root = tk.Tk()
    root.title("Dryer Control GUI")

    root.protocol("WM_DELETE_WINDOW", on_closing)

    status_var = tk.StringVar()
    preset_var = tk.StringVar()
    air_temp_var = tk.StringVar()
    bed_temp_var = tk.StringVar()
    humidity_var = tk.StringVar()

    # Variable inputs
    stepper_var = tk.StringVar(value="0")
    fan1_var = tk.StringVar(value="0")
    fan2_var = tk.StringVar(value="0")
    debug_var = tk.StringVar(value="1")
    dry_timer_var = tk.StringVar(value="0")
    target_air_temp_var = tk.StringVar(value="0")
    target_bed_temp_var = tk.StringVar(value="0")

    # Checkboxes
    stepper_check = tk.IntVar()
    fan1_check = tk.IntVar()
    fan2_check = tk.IntVar()
    debug_check = tk.IntVar()
    dry_timer_check = tk.IntVar()
    target_air_temp_check = tk.IntVar()
    target_bed_temp_check = tk.IntVar()

    # Preset Buttons
    button_frame = ttk.Frame(root)
    button_frame.pack(pady=10)
    for preset in ["PLA", "PET", "TPU", "OFF"]:
        btn = ttk.Button(button_frame, text=preset, width=10,
                         command=lambda p=preset: threading.Thread(target=send_json, args=(p,)).start())
        btn.pack(side=tk.LEFT, padx=5)

    # Status
    info_frame = ttk.Frame(root)
    info_frame.pack(pady=5)
    ttk.Label(info_frame, text="Status:").grid(row=0, column=0, sticky="e")
    ttk.Label(info_frame, textvariable=status_var, width=10).grid(row=0, column=1)
    ttk.Label(info_frame, text="Preset:").grid(row=1, column=0, sticky="e")
    ttk.Label(info_frame, textvariable=preset_var, width=10).grid(row=1, column=1)

    # Live values
    values_frame = ttk.Frame(root)
    values_frame.pack(pady=5)
    ttk.Label(values_frame, text="Air Temp (°C):").grid(row=0, column=0, sticky="e")
    ttk.Label(values_frame, textvariable=air_temp_var, width=10).grid(row=0, column=1)
    ttk.Label(values_frame, text="Bed Temp (°C):").grid(row=1, column=0, sticky="e")
    ttk.Label(values_frame, textvariable=bed_temp_var, width=10).grid(row=1, column=1)
    ttk.Label(values_frame, text="Humidity (%):").grid(row=2, column=0, sticky="e")
    ttk.Label(values_frame, textvariable=humidity_var, width=10).grid(row=2, column=1)

    # Plot
    fig, ax = plt.subplots(figsize=(6, 3))
    canvas = FigureCanvasTkAgg(fig, master=root)
    canvas.get_tk_widget().pack(pady=10)
    canvas.draw()

    # Custom control inputs
    control_frame = ttk.LabelFrame(root, text="Manual Controls")
    control_frame.pack(pady=10)

    variables = [
        ("Stepper On (0/1):", stepper_var, stepper_check),
        ("Fan1 On (0/1):", fan1_var, fan1_check),
        ("Fan2 On (0/1):", fan2_var, fan2_check),
        ("Debug (0/1):", debug_var, debug_check),
        ("Dry Timer (s):", dry_timer_var, dry_timer_check),
        ("Target Air Temp:", target_air_temp_var, target_air_temp_check),
        ("Target Bed Temp:", target_bed_temp_var, target_bed_temp_check)
    ]

    for idx, (label, var, check_var) in enumerate(variables):
        ttk.Checkbutton(control_frame, variable=check_var).grid(row=idx, column=0)
        ttk.Label(control_frame, text=label).grid(row=idx, column=1, sticky="e", padx=2)
        ttk.Entry(control_frame, textvariable=var, width=10).grid(row=idx, column=2, padx=5)

    send_btn = ttk.Button(control_frame, text="SEND", command=send_custom_values)
    send_btn.grid(row=len(variables), column=0, columnspan=3, pady=10)

    threading.Thread(target=read_serial, daemon=True).start()

    root.mainloop()


if __name__ == "__main__":
    create_gui()
