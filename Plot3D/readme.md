# Software Magnetómetro LSM303AGR

Interfaz gráfica para visualizar y registrar datos de un magnetómetro LSM303AGR. Permite conectar al puerto serial, enviar comandos, recibir datos, graficarlos en 3D y exportarlos a CSV.

---

## Requisitos

- Python 3.12 o superior
- Librerías Python:
  - PyQt5
  - matplotlib
  - pyserial
  - numpy

---

## Clonado del proyecto

```bash
git clone <url_del_repositorio>
cd Plot3D
```

## Instalación de dependencias
Se recomienda crear un entorno virtual antes de instalar las dependencias:

### Windows
```powershell
python -m venv venv
venv\Scripts\activate
```

### Linux / Mac
```bash
python -m venv venv
source venv/bin/activate
```

## Instalar todas las dependencias:
```powershell
pip install -r requirements.txt
```

## Ejecución desde código fuente
```powershell
python magnetometro_gui.py
```
## Ejecución desde archivo compilado (.exe)
```powershell
dist\magnetometro_gui.exe
```