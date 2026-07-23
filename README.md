# Barbero Dormilón

## Objetivo del Proyecto

Implementar una simulación multihilo del clásico problema de sincronización **"El Barbero Dormilón"** (Dijkstra, 1971) utilizando semáforos implementados manualmente. El proyecto demuestra conceptos fundamentales de **concurrencia**, **exclusión mutua** y **sincronización entre hilos** en C++.

---

## Estructura de Directorios

```
custom-semaphore-barber/
├── include/
│   ├── custom_semaphore.hpp   # Declaración de la clase Semaphore
│   └── barber_shop.hpp        # Declaración de la clase BarberShop
├── src/
│   ├── custom_semaphore.cpp   # Implementación del semáforo
│   ├── barber_shop.cpp        # Lógica del barbero y los clientes
│   └── main.cpp               # Punto de entrada del programa
├── Makefile                   # Sistema de construcción
├── .gitignore                 # Archivos ignorados por git
└── README.md                  # Documentación del proyecto
```

---

## Stack Tecnológico y Ejecución

### Stack

| Herramienta | Versión |
|-------------|---------|
| Lenguaje    | C++11 |
| Compilador  | g++ (MinGW / GCC) |
| Hilos       | std::thread (librería estándar) |
| Semáforos   | Implementación propia con std::mutex + std::condition_variable |
| Build       | GNU Make |

### Requisitos

- **Linux/Mac/WSL**: `g++` y `make` instalados.
- **Windows**: MSYS2 o WSL.

#### Instalación en Windows con MSYS2 (recomendado)

1. Descargar e instalar MSYS2 desde https://www.msys2.org/
2. Abrir "MSYS2 UCRT64" desde el menú inicio
3. Ejecutar:
   ```bash
   # Actualizar paquetes
   pacman -Syu
   ```
   ```bash
   # Instalar g++
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```
   ```bash
   # Instalar make
   pacman -S make
   ```
   ```bash
   # Verificar instalación
   g++ --version
   ```
   ```bash
   # Verificar instalación
   make --version
   ```

#### Verificar instalación en Linux/Mac/WSL

```bash
g++ --version
```
```bash
make --version
```

Si falta alguno, instalarlo con el gestor de paquetes de la distribución.

### Ejecución desde 0

```bash
# Clonar repositorio
git clone https://github.com/JeisiRosales/custom-semaphore-barber.git
```
```bash
# Seleccionar directorio
cd custom-semaphore-barber
```
```bash
# Compila el proyecto
make
```
```bash
# Ejecuta la simulación (pedirá datos por teclado)
./build/barber_shop
```
```bash
# Limpia archivos objeto y ejecutable
make clean
```

Al ejecutar, el programa solicitará por teclado la cantidad de sillas de espera y la cantidad de clientes:

```
Ingrese el numero de sillas de la sala de espera: 5
Ingrese el numero de clientes: 15
```

---

## Lógica del Programa Detallada

### Contexto del Problema

Una barbería tiene una sala de espera con **n sillas** (configurable por el usuario al ejecutar) y una sala de barbería con la silla del barbero. El barbero y los clientes son **hilos concurrentes** que deben coordinarse sin condiciones de carrera. El programa pide al usuario la cantidad de sillas y la cantidad de clientes, luego inicia la simulación.

### Condiciones del Problema

| Condición | Comportamiento |
|-----------|---------------|
| No hay clientes | El barbero se duerme (`customers_ready.wait()`) e imprime "No hay clientes, durmiendo..." |
| Cliente entra y hay sillas libres | El cliente se sienta, despierta al barbero y espera su turno |
| Cliente entra y todas las sillas ocupadas | El cliente se va de la barbería |
| Barbero está dormido | El cliente lo despierta (`customers_ready.signal()`) y el barbero imprime "Despertado por un cliente" |
| Barbero ocupado pero hay sillas | El cliente se sienta y espera |
| Se acaban los clientes | El barbero se duerme permanentemente esperando el próximo cliente que nunca llega |

### Semáforo Personalizado (`custom_semaphore`)

Implementa las operaciones clásicas de Dijkstra usando un `std::mutex` y un `std::condition_variable` como primitivas de bajo nivel:

- **`wait()` (operación P)**: Adquiere el mutex. Si el contador es 0, el hilo se bloquea en la variable de condición. Si el contador es > 0, lo decrementa y continúa.
- **`signal()` (operación V)**: Adquiere el mutex, incrementa el contador y despierta a un hilo bloqueado (si hay alguno).

### Hilo del Barbero

El barbero ejecuta un **ciclo infinito** con los siguientes pasos:

1. Imprime `"[Barbero] No hay clientes, durmiendo..."` y ejecuta **`customers_ready.wait()`** — se bloquea hasta que llegue un cliente.
2. Al ser despertado, imprime `"[Barbero] Despertado por un cliente."`.
3. Adquiere `chair_mutex` → incrementa `free_chairs` (libera la silla de espera que ocupaba el cliente) → libera `chair_mutex`.
4. Ejecuta **`barber_ready.signal()`** — notifica al cliente que puede pasar a la silla de corte.
5. Imprime `"[Barbero] Cortando el cabello..."` y duerme el hilo 1 segundo simulando el corte.
6. Vuelve al paso 1.

### Hilo del Cliente

Cada cliente se ejecuta una vez y termina:

1. Adquiere `chair_mutex` para acceder a `free_chairs` de forma segura.
2. **Si `free_chairs > 0`** (hay silla disponible):
   - Decrementa `free_chairs` (el cliente se sienta en la sala de espera).
   - Libera `chair_mutex`.
   - Ejecuta **`customers_ready.signal()`** — despierta al barbero si está dormido.
   - Ejecuta **`barber_ready.wait()`** — espera a que el barbero lo llame para cortarse el pelo.
   - Imprime `"[Cliente N] Le estan cortando el pelo..."`.
   - El hilo termina (el cliente sale de la barbería).
3. **Si `free_chairs == 0`** (todas las sillas ocupadas):
   - Libera `chair_mutex`.
   - Imprime `"[Cliente N] No hay sillas disponibles, se va."`.
   - El hilo termina sin cortarse el pelo.

### Flujo en Consola (ejemplo con 3 sillas, 3 clientes)

```
Ingrese el numero de sillas de la sala de espera: 3
Ingrese el numero de clientes: 3
[Barbero] No hay clientes, durmiendo...
[Cliente 1] Se sienta en una silla. (Libres: 2)
[Barbero] Despertado por un cliente.
[Barbero] Cortando el cabello...
[Cliente 1] Le estan cortando el pelo...
[Barbero] No hay clientes, durmiendo...
[Cliente 2] Se sienta en una silla. (Libres: 2)
[Barbero] Despertado por un cliente.
[Barbero] Cortando el cabello...
[Cliente 2] Le estan cortando el pelo...
[Barbero] No hay clientes, durmiendo...
[Cliente 3] Se sienta en una silla. (Libres: 2)
[Barbero] Despertado por un cliente.
[Barbero] Cortando el cabello...
[Cliente 3] Le estan cortando el pelo...
[Barbero] No hay clientes, durmiendo...  ← se queda dormido para siempre
```

### Variables Compartidas

| Variable | Tipo | Propósito |
|----------|------|-----------|
| `free_chairs` | `int` | Sillas de espera disponibles (inicia en el valor ingresado por el usuario) |
| `chair_mutex` | `std::mutex` | Protege `free_chairs` contra accesos simultáneos |
| `customers_ready` | `Semaphore` | Despierta al barbero; cuenta clientes esperando (inicia en 0) |
| `barber_ready` | `Semaphore` | Sincroniza al cliente para pasar a la silla de corte (inicia en 0) |

### Diagrama de Flujo

```
CLIENTE                         BARBERO
   │                               │
   ├─ Adquiere chair_mutex         │
   │                               │
   ├─ ¿free_chairs > 0?            │
   │  ├─ No → se va                │
   │  └─ Sí →                      │
   │     ├─ free_chairs--          │
   │     ├─ Libera chair_mutex     ├─ "No hay clientes, durmiendo..."
   │     │                         ├─ customers_ready.wait()
   │     ├─ customers_ready.signal() ──→  "Despertado por un cliente."
   │     │                               ├─ chair_mutex.lock()
   │     │                               ├─ free_chairs++
   │     │                               ├─ chair_mutex.unlock()
   │     │                               ├─ barber_ready.signal()
   │     ├─ barber_ready.wait() ←────────┘
   │     │                               ├─ "Cortando el cabello..."
   │     └─ Sale de la barbería          └─ Vuelve a dormir
```

## Autores

- **Jeisi Rosales**
- **Santiago Velasquez**
- **Orlando Zabala**
