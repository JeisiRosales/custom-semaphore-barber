/*
  main.cpp - Punto de entrada del programa.
  Genera aleatoriamente el número de sillas de la sala de espera (1-10),
  la cantidad de clientes (10-29) y el tiempo de llegada de cada cliente
  (2000-4999 ms), luego inicia la simulación.
*/

#include "barber_shop.hpp"
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>

int main() {
    std::srand(std::time(nullptr));

    // Generar valores aleatorios
    int num_chairs    = std::rand() % 10 + 1;   // 1 a 10 sillas
    int num_customers = std::rand() % 20 + 10;  // 10 a 29 clientes

    std::cout << "Numero de sillas de la sala de espera: " << num_chairs
              << " (aleatorio)" << std::endl;
    std::cout << "Numero de clientes: " << num_customers
              << " (aleatorio)" << std::endl;

    BarberShop shop(num_chairs);

    std::thread barber_thread(&BarberShop::barber, &shop);

    std::vector<std::thread> customer_threads;
    for (int i = 1; i <= num_customers; ++i) {
        // Tiempo de llegada aleatorio entre 2000 y 4999 ms
        // (intervalo mayor que el corte de 1.5s para que el barbero alterne
        //  entre atender y dormir, con acumulación ocasional de clientes)
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 3000 + 2000));
        customer_threads.emplace_back(&BarberShop::customer, &shop, i);
    }

    for (auto& t : customer_threads) {
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::exit(0);

    return 0;
}
