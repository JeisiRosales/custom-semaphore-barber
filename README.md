# Barbero Dormilón

## Objetivo del Proyecto

Implementar una simulación multihilo del clásico problema de sincronización **"El Barbero Dormilón"** (Dijkstra, 1971) utilizando semáforos implementados manualmente. El proyecto demuestra conceptos fundamentales de **concurrencia**, **exclusión mutua** y **sincronización entre hilos** en C++. Los parámetros de la simulación (sillas, clientes y tiempos de llegada) se generan **aleatoriamente** en cada ejecución.

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
   pacman -Syu
   ```
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```
   ```bash
   pacman -S make
   ```
   ```bash
   g++ --version
   ```
   ```bash
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
git clone https://github.com/JeisiRosales/custom-semaphore-barber.git
```
```bash
cd custom-semaphore-barber
```
```bash
make
```
```bash
# Ejecuta la simulación (pedirá datos por teclado)
./build/barber_shop
```
```bash
make clean
```

Al ejecutar, el programa solicitará por teclado la cantidad de sillas de espera y la cantidad de clientes:

```
Numero de sillas de la sala de espera: 5 (aleatorio)
Numero de clientes: 15 (aleatorio)
```

---

## Lógica del Programa Detallada

### Contexto del Problema

Una barbería tiene una sala de espera con **n sillas** (configurable por el usuario al ejecutar) y una sala de barbería con la silla del barbero. El barbero y los clientes son **hilos concurrentes** que deben coordinarse sin condiciones de carrera. El programa pide al usuario la cantidad de sillas y la cantidad de clientes, luego inicia la simulación.

### Condiciones del Problema

| Condición | Comportamiento |
|-----------|---------------|
| No hay clientes | El barbero se duerme (`customers_ready.wait()`) e imprime "No hay clientes, durmiendo..." |
| Cliente entra y hay sillas libres | El cliente se sienta, señaliza al barbero y espera su turno |
| Cliente entra y todas las sillas ocupadas | El cliente se va de la barbería |
| Barbero está dormido | El cliente lo despierta (`customers_ready.signal()`) y se imprime "Despertado por un cliente" |
| Barbero ocupado pero hay sillas | El cliente se sienta y espera; el barbero atiende la cola sin mensajes de despertar |
| Barbero atiende cola | Imprime "Atendiendo al siguiente cliente." sin el mensaje falso de despertar |
| Se acaban los clientes | El barbero se duerme permanentemente esperando el próximo cliente que nunca llega |

### Semáforo Personalizado (`custom_semaphore`)

Implementa las operaciones clásicas de Dijkstra usando un `std::mutex` y un `std::condition_variable` como primitivas de bajo nivel:

- **`wait()` (operación P)**: Adquiere el mutex. Si el contador es 0, el hilo se bloquea en la variable de condición. Si el contador es > 0, lo decrementa y continúa.
- **`signal()` (operación V)**: Adquiere el mutex, incrementa el contador y despierta a un hilo bloqueado (si hay alguno).
- **`get_count()`**: Retorna el valor actual del contador interno. El barbero lo usa para consultar si hay clientes en espera y decidir si imprime el mensaje de "durmiendo" o continúa atendiendo la cola.

### Hilo del Barbero (con detección de estado)

El barbero ejecuta un **ciclo infinito** con una variable booleana `sleeping` que rastrea si está dormido o despierto, evitando mensajes engañosos:

1. Inicia con `sleeping = true` e imprime `"[Barbero] No hay clientes, durmiendo..."`.
2. Guarda `was_sleeping = sleeping` y ejecuta **`customers_ready.wait()`** — se bloquea hasta que llegue un cliente o retorna inmediato si ya hay clientes en cola.
3. **Si `was_sleeping == true`**: imprime `"[Barbero] Despertado por un cliente."` y pone `sleeping = false`.
   **Si `was_sleeping == false`**: no imprime mensaje de despertar (ya estaba atendiendo la cola).
4. Adquiere `chair_mutex` → incrementa `free_chairs` (libera la silla de espera que ocupaba el cliente) → libera `chair_mutex`.
5. Ejecuta **`barber_ready.signal()`** — notifica al cliente que puede pasar a la silla de corte.
6. **Si `was_sleeping == true`**: imprime `"[Barbero] Cortando el cabello..."`.
   **Si `was_sleeping == false`**: imprime `"[Barbero] Atendiendo al siguiente cliente."`.
7. Duerme el hilo **1.5 segundos** simulando el corte, luego imprime `"[Barbero] Corte terminado."`.
8. Consulta **`customers_ready.get_count()`**:
   - Si `== 0`: imprime `"[Barbero] No hay clientes, durmiendo..."` y marca `sleeping = true`.
   - Si `> 0`: mantiene `sleeping = false` (hay cola de espera).
9. Vuelve al paso 2.

### Hilo del Cliente

Cada cliente se ejecuta una vez y termina:

1. Adquiere `chair_mutex` para acceder a `free_chairs` de forma segura.
2. **Si `free_chairs > 0`** (hay silla disponible):
   - Decrementa `free_chairs` (el cliente se sienta en la sala de espera).
   - Libera `chair_mutex`.
   - Ejecuta **`customers_ready.signal()`** — incrementa el semáforo para que el barbero sepa que hay clientes.
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
[Cliente 2] Se sienta en una silla. (Libres: 2)        ← llega durante el corte
[Cliente 3] Se sienta en una silla. (Libres: 1)        ← sala de espera llenándose
[Barbero] Corte terminado.
[Barbero] Atendiendo al siguiente cliente.              ← sin mensaje falso de despertar
[Cliente 2] Le estan cortando el pelo...
[Barbero] Corte terminado.
[Barbero] Atendiendo al siguiente cliente.              ← atiende la cola
[Cliente 3] Le estan cortando el pelo...
[Cliente 4] Se sienta en una silla. (Libres: 2)
[Barbero] Corte terminado.
[Barbero] Atendiendo al siguiente cliente.
[Cliente 4] Le estan cortando el pelo...
[Barbero] Corte terminado.
[Barbero] No hay clientes, durmiendo...
[Cliente 5] Se sienta en una silla. (Libres: 2)
[Barbero] Despertado por un cliente.
[Barbero] Cortando el cabello...
[Cliente 5] Le estan cortando el pelo...
mak[Barbero] Corte terminado.
[Barbero] No hay clientes, durmiendo...                 ← se queda dormido para siempre
```

### Variables Compartidas

| Variable | Tipo | Propósito |
|----------|------|-----------|
| `free_chairs` | `int` | Sillas de espera disponibles (inicia en el valor generado aleatoriamente) |
| `chair_mutex` | `std::mutex` | Protege `free_chairs` contra accesos simultáneos |
| `customers_ready` | `Semaphore` | Cuenta clientes esperando; `get_count()` permite al barbero decidir si duerme o sigue atendiendo |
| `barber_ready` | `Semaphore` | Sincroniza al cliente para pasar a la silla de corte (inicia en 0) |

### Diagrama de Flujo

```
CLIENTE                              BARBERO
   │                                    │
   ├─ Adquiere chair_mutex              │
   │                                    │
   ├─ ¿free_chairs > 0?                 │
   │  ├─ No → se va                     │
   │  └─ Sí →                           │
   │     ├─ free_chairs--               │
   │     ├─ Libera chair_mutex          ├─ sleeping = true
   │     │                              ├─ "No hay clientes, durmiendo..."
   │     ├─ customers_ready.signal() ──→│
   │     │                              ├─ customers_ready.wait()
   │     │                              ├─ Si was_sleeping: "Despertado por un cliente."
   │     │                              ├─ chair_mutex.lock()
   │     │                              ├─ free_chairs++
   │     │                              ├─ chair_mutex.unlock()
   │     │                              ├─ barber_ready.signal()
   │     ├─ barber_ready.wait() ←───────┘
   │     │                              ├─ Si was_sleeping: "Cortando el cabello..."
   │     │                              │  Sino: "Atendiendo al siguiente cliente."
   │     │                              ├─ Corte (1.5 segundos)
   │     │                              ├─ "Corte terminado."
   │     │                              ├─ get_count() == 0?
   │     │                              │  Sí → "No hay clientes, durmiendo..."
   │     │                              │        sleeping = true
   │     │                              │  No → sleeping = false (sigue atendiendo cola)
   │     └─ Sale de la barbería         └─ Vuelve al wait()
```

## Autores

- **Jeisi Rosales**
- **Santiago Velasquez**
- **Orlando Zabala**
