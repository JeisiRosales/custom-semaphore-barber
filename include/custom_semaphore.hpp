#ifndef CUSTOM_SEMAPHORE_HPP
#define CUSTOM_SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>

/*
  Semaphore - Implementación propia de un semáforo de Dijkstra
  Utiliza un mutex y una variable de condición como primitivas de bajo nivel.
  La variable de condición permite dormir y despertar hilos de forma eficiente.
*/
class Semaphore {
public:
    /*
      Constructor: inicializa el contador interno del semáforo.
      El valor inicial determina cuántos hilos pueden pasar sin bloquearse.
    */
    Semaphore(int count = 0);

    /*
      wait() - Operación P (proberen / decrementar / probar)
      Si el contador es 0, el hilo se bloquea en la variable de condición.
      Si el contador es mayor a 0, lo decrementa y continúa.
    */
    void wait();

    /*
      signal() - Operación V (verhogen / incrementar)
      Incrementa el contador y despierta a un hilo bloqueado si existe.
    */
    void signal();

private:
    int count;
    std::mutex mtx;
    std::condition_variable cv;
};

#endif
