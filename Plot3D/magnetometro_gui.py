import sys
import os
import csv
import threading
import serial
import serial.tools.list_ports
from PyQt5.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton, QTextEdit,
    QLabel, QComboBox, QMessageBox, QSpinBox, QGroupBox, QFormLayout, QFileDialog,
    QTableWidget, QTableWidgetItem
)
from PyQt5.QtCore import pyqtSignal, QObject
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure

# --- Hilo de comunicación serial ---
class SerialThread(QObject):
    data_received = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.serial_port = None
        self.running = False

    def connect(self, port, baudrate=115200, parity='N', stopbits=1, bytesize=8):
        try:
            parity_map = {'N': serial.PARITY_NONE, 'E': serial.PARITY_EVEN, 'O': serial.PARITY_ODD}
            stop_map = {1: serial.STOPBITS_ONE, 2: serial.STOPBITS_TWO}
            self.serial_port = serial.Serial(
                port, baudrate=baudrate, parity=parity_map[parity],
                stopbits=stop_map[stopbits], bytesize=bytesize, timeout=1
            )
            self.running = True
            threading.Thread(target=self.read_loop, daemon=True).start()
            return True
        except Exception as e:
            print(f"Error al conectar: {e}")
            return False

    def disconnect(self):
        self.running = False
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()

    def send(self, msg):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.write(msg.encode())

    def read_loop(self):
        while self.running and self.serial_port and self.serial_port.is_open:
            try:
                line = self.serial_port.readline().decode(errors='ignore')
                if line:
                    self.data_received.emit(line.strip())
            except Exception:
                break

# --- Ventana de graficación ---
class PlotWindow(QWidget):
    def __init__(self, data):
        super().__init__()
        self.setWindowTitle("Gráfico 3D de datos almacenados")
        self.data = data
        self.figure = Figure()
        self.canvas = FigureCanvas(self.figure)
        self.toolbar = NavigationToolbar(self.canvas, self)
        layout = QVBoxLayout()
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        self.setLayout(layout)
        self.plot_data()

    def plot_data(self):
        self.figure.clear()
        ax = self.figure.add_subplot(111, projection="3d")
        if self.data:
            xs, ys, zs = zip(*self.data)
            ax.scatter(xs, ys, zs, c='r', marker='o')
            ax.set_title(f"{len(self.data)} mediciones del campo magnético")
        else:
            ax.set_title("No hay datos para mostrar")
        ax.set_xlabel("Eje X")
        ax.set_ylabel("Eje Y")
        ax.set_zlabel("Eje Z")
        self.canvas.draw()

# --- Ventana principal ---
class MagnetometroGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Interfaz Magnetómetro LSM303AGR")
        self.serial_thread = SerialThread()
        self.serial_thread.data_received.connect(self.on_data_received)
        self.mediciones = []
        self.port_map = {}
        self.plot_windows = []
        self.init_ui()

    def init_ui(self):
        main_layout = QHBoxLayout()
        left_layout = QVBoxLayout()
        right_layout = QVBoxLayout()

        # --- Configuración y conexión serial ---
        serial_layout = QHBoxLayout()
        self.port_combo = QComboBox()
        self.refresh_btn = QPushButton("Refrescar")
        self.refresh_btn.clicked.connect(self.refresh_ports)
        serial_layout.addWidget(self.refresh_btn)

        serial_layout.addWidget(QLabel("Puerto:"))
        serial_layout.addWidget(self.port_combo)

        self.connect_btn = QPushButton("Conectar")
        self.connect_btn.clicked.connect(self.toggle_connection)
        serial_layout.addWidget(self.connect_btn)
        left_layout.addLayout(serial_layout)

        serial_layout.addWidget(QLabel("Baudios:"))
        self.baud_box = QSpinBox()
        self.baud_box.setRange(1200, 115200)
        self.baud_box.setValue(115200)
        serial_layout.addWidget(self.baud_box)

        serial_layout.addWidget(QLabel("Paridad:"))
        self.parity_combo = QComboBox()
        self.parity_combo.addItems(['N', 'E', 'O'])
        serial_layout.addWidget(self.parity_combo)

        serial_layout.addWidget(QLabel("Stop bits:"))
        self.stop_combo = QComboBox()
        self.stop_combo.addItems(['1','2'])
        serial_layout.addWidget(self.stop_combo)

        # --- Botones de comando ---
        cmd_layout = QHBoxLayout()
        self.btn_cmd_a = QPushButton("Enviar comando A")
        self.btn_cmd_a.clicked.connect(lambda: self.send_command("A\n"))
        cmd_layout.addWidget(self.btn_cmd_a)
        self.btn_cmd_b = QPushButton("Enviar comando B")
        self.btn_cmd_b.clicked.connect(lambda: self.send_command("B\n"))
        cmd_layout.addWidget(self.btn_cmd_b)
        left_layout.addLayout(cmd_layout)

        # --- Consola ---
        self.console = QTextEdit()
        self.console.setReadOnly(True)
        left_layout.addWidget(QLabel("Consola de comunicación:"))
        left_layout.addWidget(self.console)

        # --- Panel derecho ---
        right_layout = QVBoxLayout()

        # --- Botón About arriba ---
        self.about_btn = QPushButton("About")
        self.about_btn.clicked.connect(self.show_about)
        right_layout.addWidget(self.about_btn)

        # --- Panel de manejo de datos ---
        data_layout = QHBoxLayout()
        self.btn_export = QPushButton("Exportar CSV")
        self.btn_export.clicked.connect(self.export_csv)
        data_layout.addWidget(self.btn_export)
        self.btn_plot = QPushButton("Graficar datos")
        self.btn_plot.clicked.connect(self.plot_stored_data)
        data_layout.addWidget(self.btn_plot)
        self.btn_clear = QPushButton("Borrar datos")
        self.btn_clear.clicked.connect(self.clear_data)
        data_layout.addWidget(self.btn_clear)
        left_layout.addLayout(data_layout)

        main_layout.addLayout(left_layout)

        # --- Panel derecho: info de datos ---
        self.data_panel = QGroupBox("Datos almacenados")
        panel_layout = QFormLayout()
        
        self.label_count = QLabel("0")
        panel_layout.addRow("Total datos:", self.label_count)
        self.spin_index = QSpinBox()
        self.spin_index.setMinimum(0)
        self.spin_index.setMaximum(0)
        self.spin_index.valueChanged.connect(self.update_selected_data)
        panel_layout.addRow("Ir al índice:", self.spin_index)
        self.label_x = QLabel("-")
        self.label_y = QLabel("-")
        self.label_z = QLabel("-")
        panel_layout.addRow("X:", self.label_x)
        panel_layout.addRow("Y:", self.label_y)
        panel_layout.addRow("Z:", self.label_z)
        self.data_panel.setLayout(panel_layout)
        right_layout.addWidget(self.data_panel)
        main_layout.addLayout(right_layout)

        # --- Panel derecho: tabla datos ---
        self.data_table = QTableWidget()
        self.data_table.setColumnCount(3)
        self.data_table.setHorizontalHeaderLabels(["X", "Y", "Z"])
        self.data_table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.data_table.setSelectionBehavior(QTableWidget.SelectRows)
        self.data_table.setSelectionMode(QTableWidget.SingleSelection)
        self.data_table.horizontalHeader().setDefaultSectionSize(62) 
        self.data_table.verticalHeader().setVisible(False)
        self.data_table.setAlternatingRowColors(True)
        self.data_table.setSortingEnabled(False)
        right_layout.addWidget(self.data_table)

        self.setLayout(main_layout)
        self.refresh_ports()

    # --- Funciones serial ---
    def refresh_ports(self):
        ports = serial.tools.list_ports.comports()
        self.port_combo.clear()
        self.port_map.clear()
        for port in ports:
            display_text = f"{port.device} – {port.description}"
            if port.manufacturer:
                display_text += f" ({port.manufacturer})"
            self.port_combo.addItem(display_text)
            self.port_map[display_text] = port.device
        if not ports:
            self.port_combo.addItem("No se detectan puertos")

    def show_about(self):
        QMessageBox.information(
            self,
            "Acerca de",
            "Software Magnetómetro LSM303AGR\n"
            "Versión 1.0\n"
            "Desarrollado por Rodrigo Jimenez para la asignatura ELO312 - UTFSM\n\n"
            "Uso educativo únicamente.\n\n"
            "Icono: 'Magnetómetro' de Flaticon (Freepik Company) - https://www.flaticon.es/icono-gratis/magnetometro_10614176\n\n"
            "Bibliotecas de terceros utilizadas:\n"
            "- PyQt5 (GPL v3)\n"
            "- matplotlib (PSF License)\n"
            "- pyserial (BSD License)\n\n"
            "Consulte las respectivas licencias para más detalles."
        )


    def toggle_connection(self):
        if not self.serial_thread.running:
            display_text = self.port_combo.currentText()
            if display_text not in self.port_map:
                QMessageBox.warning(self, "Advertencia", "Seleccione un puerto válido.")
                return
            port = self.port_map[display_text]
            baud = self.baud_box.value()
            parity = self.parity_combo.currentText()
            stop = int(self.stop_combo.currentText())
            if self.serial_thread.connect(port, baudrate=baud, parity=parity, stopbits=stop):
                self.console.append(f"Conectado a {port} a {baud} baudios\n")
                self.connect_btn.setText("Desconectar")
                self.port_combo.setEnabled(False)
                self.refresh_btn.setEnabled(False)
            else:
                QMessageBox.critical(self, "Error", "No se pudo abrir el puerto.")
        else:
            self.serial_thread.disconnect()
            self.console.append("Desconectado.\n")
            self.connect_btn.setText("Conectar")
            self.port_combo.setEnabled(True)
            self.refresh_btn.setEnabled(True)

    def send_command(self, cmd):
        if not self.serial_thread.running:
            QMessageBox.warning(self, "Advertencia", "Debe conectar primero el puerto serial.")
            return
        self.console.append(f">>> {cmd.strip()}")
        self.serial_thread.send(cmd)

    # --- Recepción de datos ---
    def on_data_received(self, data):
        self.console.append(data)
        # Comando B: x,y,z
        new_entry = None
        if "," in data:
            try:
                x_str, y_str, z_str = data.split(",")
                x, y, z = float(x_str), float(y_str), float(z_str)
                new_entry = (x, y, z)
            except:
                pass
        # Comando A: mx:.. my:.. mz:..
        elif "mx:" in data and "my:" in data and "mz:" in data:
            try:
                parts = data.split()
                x = float(parts[0].split(":")[1])
                y = float(parts[1].split(":")[1])
                z = float(parts[2].split(":")[1])
                new_entry = (x, y, z)
            except:
                pass

        if new_entry:
            self.mediciones.append(new_entry)
            # Agregar fila a la tabla
            row_idx = self.data_table.rowCount()
            self.data_table.insertRow(row_idx)
            for col, val in enumerate(new_entry):
                self.data_table.setItem(row_idx, col, QTableWidgetItem(f"{val:.3f}"))
            self.data_table.currentCellChanged.connect(
                lambda row, col, prev_row, prev_col: self.spin_index.setValue(row)
            )

        # Actualizar panel derecho
        self.label_count.setText(str(len(self.mediciones)))
        self.spin_index.setMaximum(max(0, len(self.mediciones)-1))
        self.update_selected_data()

    def update_selected_data(self):
        idx = self.spin_index.value()
        if 0 <= idx < len(self.mediciones):
            x, y, z = self.mediciones[idx]
            self.label_x.setText(f"{x:.3f}")
            self.label_y.setText(f"{y:.3f}")
            self.label_z.setText(f"{z:.3f}")
        else:
            self.label_x.setText("-")
            self.label_y.setText("-")
            self.label_z.setText("-")

    # --- Manejo de datos ---
    def export_csv(self):
        start_path = os.getcwd()
        default_name = os.path.join(start_path, "rx_data.csv")
        filename, _ = QFileDialog.getSaveFileName(
            self, "Guardar CSV", default_name, "CSV files (*.csv)"
        )
        if filename:
            with open(filename, "w", newline='') as f:
                writer = csv.writer(f)
                writer.writerow(["X", "Y", "Z"])
                writer.writerows(self.mediciones)
            QMessageBox.information(self, "Exportar CSV", f"Datos exportados a {filename}.")

    def plot_stored_data(self):
        plot_win = PlotWindow(self.mediciones)
        plot_win.resize(700, 600)
        plot_win.show()
        self.plot_windows.append(plot_win)
        plot_win.destroyed.connect(lambda: self.plot_windows.remove(plot_win))

    def clear_data(self):
        self.mediciones.clear()
        self.label_count.setText("0")
        self.spin_index.setMaximum(0)
        self.update_selected_data()
        
        # Vaciar la tabla de datos
        self.data_table.setRowCount(0)
        
        QMessageBox.information(self, "Datos", "Datos almacenados borrados.")

# --- Main ---
if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MagnetometroGUI()
    window.resize(1100, 700)
    window.show()
    sys.exit(app.exec_())
