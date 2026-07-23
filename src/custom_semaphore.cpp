#include "custom_semaphore.hpp"

/*
  Constructor: guarda el valor inicial del contador.
  Por defecto count=0, lo que significa que el primer wait() se bloqueará.
*/
Semaphore::Semaphore(int count) : count(count) {}

/*
  wait() - Operación P del semáforo.
  1. Adquiere el mutex exclusivamente.
  2. Mientras el contador sea 0, el hilo se duerme en la cv (condition_variable).
     El while es necesario para manejar "despertares espurios".
  3. Cuando el contador > 0, lo decrementa y continúa.
  4. Libera el mutex.
*/
void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(mtx);
    while (count == 0) {
        cv.wait(lock);
    }
    --count;
}

/*
  signal() - Operación V del semáforo.
  1. Adquiere el mutex exclusivamente.
  2. Incrementa el contador.
  3. Notifica a uno de los hilos bloqueados (si hay) para que se despierte.
  4. Libera el mutex.
*/
void Semaphore::signal() {
    std::unique_lock<std::mutex> lock(mtx);
    ++count;
    cv.notify_one();
}
