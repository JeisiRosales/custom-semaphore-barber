#include "barber_shop.hpp"
#include <iostream>
#include <thread>
#include <chrono>

/*
  Constructor: inicializa la cantidad total de sillas,
  todas disponibles inicialmente (free_chairs = num_chairs).
  customers_ready empieza en 0 (barbero dormido).
  barber_ready empieza en 0 (cliente esperando turno).
*/
BarberShop::BarberShop(int num_chairs)
    : num_chairs(num_chairs),
      free_chairs(num_chairs),
      customers_ready(0),
      barber_ready(0) {}

/*
  barber() - Lógica del hilo del barbero (ciclo infinito con detección de estado).

  Utiliza una variable booleana 'sleeping' para distinguir dos escenarios:
    - Barbero dormido (sleeping == true): se imprime "Despertado por un cliente"
      y "Cortando el cabello...".
    - Barbero despierto atendiendo cola (sleeping == false): se imprime
      "Atendiendo al siguiente cliente.", sin mensajes falsos de despertar.

  Paso 1: customers_ready.wait()
    Espera a que un cliente se siente en la sala. Si no hay clientes,
    el barbero se queda bloqueado aquí (el semáforo está en 0).
    Si ya hay clientes en cola, el wait() retorna inmediatamente.

  Paso 2: Libera una silla de espera (chair_mutex + ++free_chairs).

  Paso 3: barber_ready.signal()
    Notifica al cliente que ya puede pasar a la silla de corte.

  Paso 4: Corta el pelo (1.5 segundos).

  Paso 5: Consulta customers_ready.get_count()
    Si == 0: no hay más clientes → imprime "No hay clientes, durmiendo..."
             y marca sleeping = true.
    Si > 0:  hay clientes esperando → duerme sleeping = false para que
             la siguiente iteración use "Atendiendo al siguiente cliente".
*/
void BarberShop::barber() {
    bool sleeping = true;

    std::cout << "[Barbero] No hay clientes, durmiendo...\n";

    while (true) {
        bool was_sleeping = sleeping;

        customers_ready.wait();

        if (was_sleeping) {
            std::cout << "[Barbero] Despertado por un cliente.\n";
            sleeping = false;
        }

        chair_mutex.lock();
        ++free_chairs;
        chair_mutex.unlock();

        barber_ready.signal();

        if (was_sleeping) {
            std::cout << "[Barbero] Cortando el cabello...\n";
        } else {
            std::cout << "[Barbero] Atendiendo al siguiente cliente.\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        std::cout << "[Barbero] Corte terminado.\n";

        if (customers_ready.get_count() == 0) {
            std::cout << "[Barbero] No hay clientes, durmiendo...\n";
            sleeping = true;
        }
    }
}

/*
  customer(id) - Lógica de cada hilo de cliente.

  Paso 1: Adquiere chair_mutex
    Para acceder a free_chairs de forma exclusiva (evitar condiciones de carrera).

  Paso 2: Verifica si hay sillas libres
    - Si free_chairs > 0: el cliente se sienta (decrementa free_chairs),
      libera el mutex, señaliza customers_ready (despierta al barbero
      si estaba dormido), y espera en barber_ready su turno para cortarse.
    - Si free_chairs == 0: libera el mutex y se va de la barbería.
*/
void BarberShop::customer(int id) {
    // Acceso exclusivo a las sillas de espera
    chair_mutex.lock();

    if (free_chairs > 0) {
        // Hay silla disponible: el cliente se sienta a esperar
        --free_chairs;
        std::cout << "[Cliente " << id << "] Se sienta en una silla. "
                  << "(Libres: " << free_chairs << ")\n";
        chair_mutex.unlock();

        // Despierta al barbero (o le notifica que hay un cliente esperando)
        customers_ready.signal();

        // Espera a que el barbero lo llame para cortarse el pelo
        barber_ready.wait();

        std::cout << "[Cliente " << id << "] Le estan cortando el pelo...\n";
    } else {
        // No hay sillas disponibles: el cliente se va
        chair_mutex.unlock();
        std::cout << "[Cliente " << id << "] No hay sillas disponibles, se va.\n";
    }
}
