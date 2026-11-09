#!/usr/bin/env python3
"""
ELO312 - Sesión Evaluada 3: GUI de control del juego (Nucleo-L476RG).

Envía al microcontrolador (Tabla 1):
    S\\n  Comenzar el juego
    R\\n  Reiniciar el juego
    E\\n  Nivel fácil
    M\\n  Nivel medio
    H\\n  Nivel difícil

Recibe del microcontrolador (Tabla 2):
    SCORE:<puntaje>\\n
    LIFE:<vidas>\\n

Dependencias: biblioteca estándar (tkinter) + pyserial.
    pip install pyserial

Uso:
    python elo312_gui.py                       # elegir puerto en la GUI
    python elo312_gui.py --port COM5           # conectar al iniciar
    python elo312_gui.py --port /dev/ttyACM0 --baud 115200
"""

import argparse
import queue
import re
import threading
import time
import tkinter as tk
from tkinter import messagebox, ttk
from tkinter.scrolledtext import ScrolledText

import serial
import serial.tools.list_ports

BAUDRATES = (9600, 19200, 38400, 57600, 115200, 230400)
DEFAULT_BAUD = 115200
LEVELS = (("Fácil", "E"), ("Medio", "M"), ("Difícil", "H"))
MSG_RE = re.compile(r"\b(SCORE|LIFE)\s*:\s*(-?\d+)", re.IGNORECASE)
MAX_LOG_LINES = 2000
POLL_MS = 30
ST_VID = 0x0483  # STMicroelectronics (ST-LINK Virtual COM Port)


class SerialReader(threading.Thread):
    """Lee el puerto serial en segundo plano y entrega líneas completas por una cola."""

    def __init__(self, ser, q):
        super().__init__(daemon=True)
        self.ser = ser
        self.q = q
        self._stop_evt = threading.Event()

    def run(self):
        buf = bytearray()
        while not self._stop_evt.is_set():
            try:
                data = self.ser.read(self.ser.in_waiting or 1)
            except (serial.SerialException, OSError, TypeError) as e:
                if not self._stop_evt.is_set():
                    self.q.put((self, "error", str(e)))
                return
            if not data:
                continue
            buf.extend(data)
            while b"\n" in buf:
                line, _, buf = buf.partition(b"\n")
                text = line.decode("ascii", errors="replace").strip()
                self.q.put((self, "line", text))

    def stop(self):
        self._stop_evt.set()


class App:
    def __init__(self, root, port=None, baud=DEFAULT_BAUD):
        self.root = root
        self.ser = None
        self.reader = None
        self.q = queue.Queue()
        self.port_map = {}

        root.title("ELO312 - Juego Nucleo-L476RG")
        root.minsize(540, 480)
        self._build_ui(baud)
        self.refresh_ports(prefer=port)

        root.protocol("WM_DELETE_WINDOW", self.on_close)
        root.after(POLL_MS, self._poll)
        if port:
            self.connect()

    # ------------------------------------------------------------------ UI
    def _build_ui(self, baud):
        pad = {"padx": 6, "pady": 4}

        self.status_var = tk.StringVar(value="Desconectado")
        ttk.Label(self.root, textvariable=self.status_var, relief="sunken",
                  anchor="w").pack(side="bottom", fill="x")

        # Conexión
        conn = ttk.LabelFrame(self.root, text="Conexión")
        conn.pack(fill="x", **pad)
        ttk.Label(conn, text="Puerto:").grid(row=0, column=0, sticky="w", **pad)
        self.port_var = tk.StringVar()
        self.port_cb = ttk.Combobox(conn, textvariable=self.port_var,
                                    state="readonly", width=42)
        self.port_cb.grid(row=0, column=1, sticky="ew", **pad)
        self.refresh_btn = ttk.Button(conn, text="Actualizar",
                                      command=self.refresh_ports)
        self.refresh_btn.grid(row=0, column=2, **pad)

        ttk.Label(conn, text="Baudios:").grid(row=1, column=0, sticky="w", **pad)
        self.baud_var = tk.StringVar(value=str(baud))
        self.baud_cb = ttk.Combobox(conn, textvariable=self.baud_var, width=10,
                                    values=[str(b) for b in BAUDRATES])
        self.baud_cb.grid(row=1, column=1, sticky="w", **pad)
        self.conn_btn = ttk.Button(conn, text="Conectar",
                                   command=self.toggle_connection)
        self.conn_btn.grid(row=1, column=2, **pad)
        conn.columnconfigure(1, weight=1)

        # Juego
        game = ttk.LabelFrame(self.root, text="Juego")
        game.pack(fill="x", **pad)
        self.start_btn = ttk.Button(game, text="Comenzar (S)",
                                    command=lambda: self.send("S"))
        self.reset_btn = ttk.Button(game, text="Reiniciar (R)",
                                    command=lambda: self.send("R"))
        self.start_btn.grid(row=0, column=0, sticky="ew", **pad)
        self.reset_btn.grid(row=0, column=1, sticky="ew", **pad)

        lvl = ttk.Frame(game)
        lvl.grid(row=1, column=0, columnspan=2, sticky="w", **pad)
        ttk.Label(lvl, text="Nivel:").pack(side="left")
        self.level_var = tk.StringVar(value="")
        self.level_btns = []
        for name, cmd in LEVELS:
            b = ttk.Radiobutton(lvl, text=f"{name} ({cmd})", value=cmd,
                                variable=self.level_var, command=self.on_level)
            b.pack(side="left", padx=8)
            self.level_btns.append(b)
        game.columnconfigure((0, 1), weight=1)

        # Marcador
        board = ttk.LabelFrame(self.root, text="Marcador")
        board.pack(fill="x", **pad)
        big = ("Helvetica", 28, "bold")
        self.score_var = tk.StringVar(value="–")
        self.life_var = tk.StringVar(value="–")
        ttk.Label(board, text="Puntaje").grid(row=0, column=0, **pad)
        ttk.Label(board, text="Vidas").grid(row=0, column=1, **pad)
        ttk.Label(board, textvariable=self.score_var, font=big).grid(row=1, column=0, **pad)
        ttk.Label(board, textvariable=self.life_var, font=big).grid(row=1, column=1, **pad)
        board.columnconfigure((0, 1), weight=1)

        # Registro
        logf = ttk.LabelFrame(self.root, text="Registro serial")
        logf.pack(fill="both", expand=True, **pad)
        self.log = ScrolledText(logf, height=10, state="disabled",
                                font=("Courier", 10))
        self.log.pack(fill="both", expand=True, padx=4, pady=4)
        ttk.Button(logf, text="Limpiar", command=self.clear_log).pack(
            anchor="e", padx=4, pady=(0, 4))

        self._set_connected(False)

    def _set_connected(self, connected, msg=None):
        state = "normal" if connected else "disabled"
        for w in (self.start_btn, self.reset_btn, *self.level_btns):
            w.configure(state=state)
        self.port_cb.configure(state="disabled" if connected else "readonly")
        self.baud_cb.configure(state="disabled" if connected else "normal")
        self.refresh_btn.configure(state="disabled" if connected else "normal")
        self.conn_btn.configure(text="Desconectar" if connected else "Conectar")
        if msg:
            self.status_var.set(msg)

    # -------------------------------------------------------------- Puertos
    def refresh_ports(self, prefer=None):
        ports = sorted(serial.tools.list_ports.comports(), key=lambda p: p.device)
        self.port_map = {}
        labels = []
        selected = None
        for p in ports:
            desc = p.description if p.description and p.description != "n/a" else ""
            label = f"{p.device} — {desc}" if desc else p.device
            self.port_map[label] = p.device
            labels.append(label)
            if prefer and p.device == prefer:
                selected = label
            elif selected is None and (
                p.vid == ST_VID
                or "stlink" in desc.lower().replace("-", "")
                or "stm" in (p.manufacturer or "").lower()
            ):
                selected = label
        if prefer and prefer not in self.port_map.values():
            labels.insert(0, prefer)
            self.port_map[prefer] = prefer
            selected = prefer
        if selected is None and labels:
            selected = labels[0]
        self.port_cb["values"] = labels
        self.port_var.set(selected or "")

    # ------------------------------------------------------------- Conexión
    def toggle_connection(self):
        if self.ser:
            self.disconnect()
        else:
            self.connect()

    def connect(self):
        label = self.port_var.get()
        device = self.port_map.get(label, label)
        if not device:
            messagebox.showwarning("Puerto", "Seleccione un puerto serial.")
            return
        try:
            baud = int(self.baud_var.get())
        except ValueError:
            messagebox.showwarning("Baudios", "Tasa de baudios inválida.")
            return
        try:
            self.ser = serial.Serial(device, baud, timeout=0.1, write_timeout=1)
            self.ser.reset_input_buffer()
        except (serial.SerialException, OSError, ValueError) as e:
            self.ser = None
            messagebox.showerror("Error", f"No se pudo abrir {device}:\n{e}")
            return
        self.reader = SerialReader(self.ser, self.q)
        self.reader.start()
        self._set_connected(True, f"Conectado a {device} @ {baud} baud (8N1)")
        self._log(f"--- Conectado a {device} @ {baud} baud ---")

    def disconnect(self):
        if self.reader:
            self.reader.stop()
            self.reader.join(timeout=0.5)
            self.reader = None
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
            self.ser = None
            self._log("--- Desconectado ---")
        self._set_connected(False, "Desconectado")

    # ---------------------------------------------------------------- Envío
    def send(self, cmd):
        if not self.ser:
            return
        try:
            self.ser.write(f"{cmd}\n".encode("ascii"))
            self.ser.flush()
        except (serial.SerialException, OSError) as e:
            self._log(f"!! Error al enviar '{cmd}': {e}")
            self.disconnect()
            messagebox.showerror("Error", f"Error al enviar:\n{e}")
            return
        self._log(f"TX > {cmd}")

    def on_level(self):
        cmd = self.level_var.get()
        if cmd:
            self.send(cmd)

    # ------------------------------------------------------------- Recepción
    def _poll(self):
        try:
            while True:
                src, kind, payload = self.q.get_nowait()
                if kind == "line":
                    self._handle_line(payload)
                elif kind == "error" and src is self.reader:
                    self._log(f"!! Error de lectura: {payload}")
                    self.disconnect()
                    messagebox.showerror("Error", f"Se perdió la conexión:\n{payload}")
        except queue.Empty:
            pass
        self.root.after(POLL_MS, self._poll)

    def _handle_line(self, line):
        if not line:
            return
        matches = MSG_RE.findall(line)
        self._log(f"RX < {line}" + ("" if matches else "   (no reconocido)"))
        for key, val in matches:
            n = int(val)
            if key.upper() == "SCORE":
                self.score_var.set(str(n))
            else:
                self.life_var.set(self._fmt_lives(n))

    @staticmethod
    def _fmt_lives(n):
        return f"{n}  {'♥' * n}" if 0 < n <= 10 else str(n)

    # -------------------------------------------------------------- Registro
    def _log(self, text):
        self.log.configure(state="normal")
        self.log.insert("end", f"[{time.strftime('%H:%M:%S')}] {text}\n")
        lines = int(self.log.index("end-1c").split(".")[0])
        if lines > MAX_LOG_LINES:
            self.log.delete("1.0", f"{lines - MAX_LOG_LINES}.0")
        self.log.see("end")
        self.log.configure(state="disabled")

    def clear_log(self):
        self.log.configure(state="normal")
        self.log.delete("1.0", "end")
        self.log.configure(state="disabled")

    def on_close(self):
        self.disconnect()
        self.root.destroy()


def main():
    ap = argparse.ArgumentParser(description="GUI ELO312 - Sesión Evaluada 3")
    ap.add_argument("--port", help="Puerto serial (ej. COM5, /dev/ttyACM0)")
    ap.add_argument("--baud", type=int, default=DEFAULT_BAUD,
                    help=f"Tasa de baudios (por defecto {DEFAULT_BAUD})")
    args = ap.parse_args()

    root = tk.Tk()
    App(root, port=args.port, baud=args.baud)
    root.mainloop()


if __name__ == "__main__":
    main()
