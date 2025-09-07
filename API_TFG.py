# Código programado en Python para la API.
# API
# Endpoints:
#   GET  /status
#   POST /motor        body: {"on": true|false}
#   POST /reset
#   POST /mode/manual
#   POST /mode/remote  body: {"pwm": 0..255}

import os
import time

from typing import Dict, Any
from flask import Flask, request, jsonify
from flask_cors import CORS
import serial

# Configuración
PORT = os.getenv("ARDUINO_PORT")  # Necesario establecer el puerto serial usado antes de encender la API.
BAUD = int(os.getenv("ARDUINO_BAUD", "9600"))
TIMEOUT = 0.2

if not PORT:
    raise SystemExit("Debes definir ARDUINO_PORT (p.ej. COM3 o /dev/ttyACM0)")

app = Flask(__name__)
CORS(app)

# Estado en memoria
state: Dict[str, Any] = {
    "objetos": 0,
    "pot": 0,
    "pwm": 0,
    "modo": "DESCONOCIDO",
    "raw": "",
    "last_update": None,
    "port": PORT,
    "baud": BAUD,
}

# Serial: abrir bajo demanda  
ser = None  # type: ignore

def open_serial():
    """Abre el puerto si no está abierto (idempotente)."""
    global ser
    if ser is None or not ser.is_open:
        ser = serial.Serial(PORT, BAUD, timeout=TIMEOUT)

def close_serial():
    global ser
    if ser is not None:
        try:
            if ser.is_open:
                ser.close()
        except Exception:
            pass
    ser = None

# Utilidades 
def clamp(x: int, lo: int, hi: int) -> int:
    x = int(x)
    if x < lo: return lo
    if x > hi: return hi
    return x

def parse_line(line: str) -> None:
    parts = [p.strip() for p in line.split(',')]
    for p in parts:
        if p.startswith("OBJ:"):
            state["objetos"] = int(p.split(":", 1)[1])
        elif p.startswith("POT:"):
            state["pot"] = int(p.split(":", 1)[1])
        elif p.startswith("PWM:"):
            state["pwm"] = int(p.split(":", 1)[1])
        elif p.startswith("MODO:"):
            state["modo"] = p.split(":", 1)[1]
    state["last_update"] = time.time()
    state["raw"] = line

def read_once_nonblocking(max_wait_s: float = 0.3) -> None:
    """Intenta leer una línea rápidamente sin bloquear (hasta max_wait_s)."""
    try:
        open_serial()
    except Exception:
        return
    deadline = time.time() + max_wait_s
    while time.time() < deadline:
        try:
            line = ser.readline().decode(errors="ignore").strip()  # type: ignore
        except Exception:
            break
        if line:
            parse_line(line)
            break
        time.sleep(0.02)

def send(cmd: str) -> bool:
    try:
        open_serial()
        data = (cmd + "\n").encode("ascii", errors="ignore")
        ser.write(data)       # type: ignore
        ser.flush()           # type: ignore
        return True
    except Exception as e:
        print("[send] error:", e)
        return False

# Endpoints 
@app.get("/status")
def status():
    read_once_nonblocking()
    return jsonify({
        "objetos": state["objetos"],
        "pot": state["pot"],
        "pwm": state["pwm"],
        "modo": state["modo"],
        "raw": state["raw"],
        "last_update": state["last_update"],
        "port": state["port"],
        "baud": state["baud"],
    })

@app.post("/motor")
def motor():
    payload = request.get_json(silent=True) or {}
    on = bool(payload.get("on"))
    ok = send("1" if on else "0")
    return (jsonify({"ok": ok}), 200 if ok else 503)

@app.post("/reset")
def reset_counter():
    ok = send("reset")
    return (jsonify({"ok": ok}), 200 if ok else 503)

@app.post("/mode/manual")
def mode_manual():
    ok = send("manual")
    return (jsonify({"ok": ok}), 200 if ok else 503)

@app.post("/mode/remote")
def mode_remote():
    payload = request.get_json(silent=True) or {}
    if "pwm" not in payload:
        return jsonify({"ok": False, "error": "Falta 'pwm' (0-255)"}), 400
    pwm = clamp(payload["pwm"], 0, 255)
    ok = send(f"r{pwm}")
    return (jsonify({"ok": ok, "pwm": pwm}), 200 if ok else 503)

if __name__ == "__main__":
    print(f"API conectada a {PORT} @ {BAUD} baudios")
    try:
        app.run(host="0.0.0.0", port=5000, debug=False, use_reloader=False)
    finally:
        close_serial()
