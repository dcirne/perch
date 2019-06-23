#include <chrono>
#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <thread>
#include <vector>

#include "ClientProcess.hpp"
#include "Messages_and_Services.pb.h"
#include "Messages_and_Services.grpc.pb.h"

std::vector<Perch::Product *> products;
std::vector<Perch::Product *> *productsPtr = &products;

std::vector<Perch::Customer *> customers;
std::vector<Perch::Customer *> *customersPtr = &customers;

std::vector<Perch::Display *> displays;
std::vector<Perch::Display *> *displaysPtr = &displays;

void buildProductCatalog() {
    std::vector<std::string> prodNames = {"A", "B", "C", "D", "F", "G", "H", "I", "J"};
    int i = 1;

    productsPtr->reserve(prodNames.size());

    for (const auto &prodName : prodNames) {
        Perch::Product *product = new Perch::Product();
        product->set_id(i++);
        product->set_name(prodName);
        productsPtr->push_back(std::move(product));
    }
}

void buildCustomerBase() {
    constexpr int numCustomers = 100;
    customersPtr->reserve(numCustomers);

    for (int i = 1; i < 1000; ++i) {
        Perch::Customer *customer = new Perch::Customer();
        customer->set_id(i);
        customersPtr->push_back(std::move(customer));
    }
}

void buildDisplays() {
    constexpr int numDisplays = 100;
    displaysPtr->reserve(numDisplays);

    for (int i = 0; i < numDisplays; ++i) {
        Perch::Display *display = new Perch::Display();
        display->set_id(i);
        displaysPtr->push_back(std::move(display));
    }
}

void signalHandler(int signum) {
    _exit(0);
}

void configureSignalHandler() {
    struct sigaction signalAction;
    sigemptyset(&signalAction.sa_mask);
    signalAction.sa_flags = 0;
    signalAction.sa_handler = signalHandler;
    sigaction(SIGTERM, &signalAction, NULL);
}

void childProcess() {
    configureSignalHandler();
    buildProductCatalog();
    buildCustomerBase();
    buildDisplays();

    std::srand(static_cast<unsigned>(time(NULL)));

    std::vector<std::thread> tasks;
    tasks.reserve(displaysPtr->size());

    for (const auto display : *displaysPtr) {
        std::thread task([&display]() {
            const int customerIdx = std::rand() % customersPtr->size();
            Perch::Customer *customer = new Perch::Customer();
            customer->CopyFrom(*(customersPtr->at(customerIdx)));

            const int numberOfProducts = std::rand() % 10;
            std::vector<Perch::Product *> selectedProducts;
            for (int i = 0; i < numberOfProducts; ++i) {
                const int productIdx = std::rand() % productsPtr->size();
                Perch::Product *product = new Perch::Product();
                product->CopyFrom(*(productsPtr->at(productIdx)));
                selectedProducts.push_back(product);

                const int numberOfInteractions = std::rand() % 10;
                for (int j = 0; j < numberOfInteractions; ++j) {
                    const int interactedProductIdx = std::rand() % selectedProducts.size();
                    auto *interactedProduct = selectedProducts.at(interactedProductIdx);

                    Perch::Interaction *interaction = new Perch::Interaction();
                    Perch::Display *iDisplay = new Perch::Display();
                    iDisplay->CopyFrom(*display);
                    interaction->set_allocated_display(iDisplay);
                    interaction->set_allocated_customer(customer);
                    interaction->set_allocated_product(interactedProduct);
                    interaction->set_hour(std::rand() % 24);

                    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("172.20.128.2:4242", grpc::InsecureChannelCredentials());
                    ClientProcess clientProcess(channel);
                    clientProcess.interact(*interaction);
                }
            }
        });

        tasks.push_back(std::move(task));
    }

    for (auto &task : tasks) {
        if (task.joinable()) {
            task.join();
        }
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void mainProcess(pid_t cpid) {
    sleep(14);

    if (kill(cpid, SIGTERM) == -1) {
        perror("kill");
        exit(-1);
    }

    wait(NULL); // Wait for child process to terminate
}

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        std::cout << "Error forking process" << std::endl;
        return -1;
    }

    if (pid == 0) {
        childProcess();
    } else {
        mainProcess(pid);
    }

    return 0;
}
