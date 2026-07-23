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
  barber() - Lógica del hilo del barbero (ciclo infinito).

  Paso 1: customers_ready.wait()
    Espera a que un cliente se siente en la sala. Si no hay clientes,
    el barbero se queda dormido aquí (el semáforo está en 0).

  Paso 2: Incrementa free_chairs
    Adquiere chair_mutex, libera una silla de espera (porque el cliente
    va a pasar a la silla de corte) y libera el mutex.

  Paso 3: barber_ready.signal()
    Notifica al cliente que está esperando que ya puede pasar
    a la silla de barbería para cortarse el pelo.

  Paso 4: Corta el pelo
    Simula el corte durmiendo el hilo 1 segundo. Luego vuelve al paso 1.
*/
void BarberShop::barber() {
    while (true) {
        std::cout << "[Barbero] No hay clientes, durmiendo...\n";
        customers_ready.wait();
        std::cout << "[Barbero] Despertado por un cliente.\n";

        // El cliente pasa de la sala de espera a la silla de corte.
        // Se libera una silla de la sala de espera.
        chair_mutex.lock();
        ++free_chairs;
        chair_mutex.unlock();

        // El barbero notifica al cliente que ya puede acercarse.
        barber_ready.signal();

        std::cout << "[Barbero] Cortando el cabello...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
