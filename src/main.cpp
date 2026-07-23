/*
  main.cpp - Punto de entrada del programa.
  Solicita al usuario el número de sillas de la sala de espera y
  la cantidad de clientes, luego inicia la simulación.
*/

#include "barber_shop.hpp"
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>

int main() {
    int num_chairs, num_customers;

    std::cout << "Ingrese el numero de sillas de la sala de espera: ";
    std::cin >> num_chairs;
    if (num_chairs < 1) num_chairs = 3;

    std::cout << "Ingrese el numero de clientes: ";
    std::cin >> num_customers;
    if (num_customers < 1) num_customers = 10;

    BarberShop shop(num_chairs);

    std::srand(std::time(nullptr));

    std::thread barber_thread(&BarberShop::barber, &shop);

    std::vector<std::thread> customer_threads;
    for (int i = 1; i <= num_customers; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 1000 + 1500));
        customer_threads.emplace_back(&BarberShop::customer, &shop, i);
    }

    for (auto& t : customer_threads) {
        t.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::exit(0);

    return 0;
}
