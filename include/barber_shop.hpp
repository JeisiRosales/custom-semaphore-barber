#ifndef BARBER_SHOP_HPP
#define BARBER_SHOP_HPP

#include "custom_semaphore.hpp"
#include <mutex>

/*
  BarberShop - Modela la barbería compartida entre el barbero y los clientes.
  Encapsula todos los recursos compartidos (sillas, semáforos, mutex) y
  expone dos métodos que se ejecutan en hilos separados.
*/
class BarberShop {
public:
    /*
      Constructor: recibe el número de sillas de espera disponibles.
    */
    BarberShop(int num_chairs);

    /*
      barber() - Método ejecutado por el hilo del barbero.
      Ciclo infinito con detección de estado (sleeping/was_sleeping):
      espera clientes, libera sillas, notifica al cliente y simula
      el corte de pelo. Distingue entre despertar y atender en cola
      mediante customers_ready.get_count().
    */
    void barber();

    /*
      customer(id) - Método ejecutado por cada hilo de cliente.
      Intenta sentarse; si hay silla, espera su turno; si no, se va.
    */
    void customer(int id);

private:
    int num_chairs;          // Capacidad máxima de sillas de espera
    int free_chairs;         // Sillas disponibles actualmente

    std::mutex chair_mutex;  // Mutex estándar para proteger free_chairs
    Semaphore customers_ready;  // Semáforo: despierta al barbero; get_count() consulta cola
    Semaphore barber_ready;     // Semáforo: barbero notifica al cliente que pase
};

#endif
