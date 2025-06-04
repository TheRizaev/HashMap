#pragma once
#include <iostream>
#include <cstring>
#include <string>
#include <chrono>
#include "Table.h"
#include "Mem.h"

class ContainerTester
{
private:
    template <typename ContainerType>
    static void testRehashing(const char* containerName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        Mem memory(200 * 1024 * 1024);
        ContainerType container(memory);

        const int numElements = 1000000;
        std::cout << "Добавление " << numElements << " элементов..." << std::endl;
        auto createStart = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numElements; i++)
        {
            int key = i;
            int value = i * 10;
            container.insertByKey(&key, sizeof(int), &value, sizeof(int));
        }
        auto createEnd = std::chrono::high_resolution_clock::now();
        auto createDuration = std::chrono::duration_cast<std::chrono::milliseconds>(createEnd - createStart).count();
        std::cout << "Создание: " << createDuration / 1000.0 << " секунд" << std::endl;

        auto deleteStart = std::chrono::high_resolution_clock::now();
        std::cout << "\nУдаление всех четных ключей..." << std::endl;
        for (int i = 0; i < numElements; i += 2)
        {
            container.removeByKey(&i, sizeof(int));
        }
        auto deleteEnd = std::chrono::high_resolution_clock::now();
        auto deleteDuration = std::chrono::duration_cast<std::chrono::milliseconds>(deleteEnd - deleteStart).count();
        std::cout << "Время удаления четных ключей: " << deleteDuration / 1000.0 << " секунд" << std::endl;

        // Проверка размера 
        auto sizeStart = std::chrono::high_resolution_clock::now();
        size_t tableSize = container.size();
        std::cout << "\nРазмер " << containerName << " после удаления четных ключей: " << tableSize << std::endl;
        auto sizeEnd = std::chrono::high_resolution_clock::now();
        auto sizeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(sizeEnd - sizeStart).count();
        std::cout << "Время проверки размера таблицы: " << sizeDuration / 1000.0 << " секунд" << std::endl;

        container.clear();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "\nВремя выполнения теста с " << numElements << " элементами: " << duration / 1000.0 << " секунд" << std::endl;
        std::cout << "Среднее время на операцию: " << (double)duration / numElements / 1000.0 << " секунд" << std::endl;
    }
public:

    static void testTableContainer()
    {
        auto start = std::chrono::high_resolution_clock::now();

        testRehashing<hashTable>("таблицы");
        std::cout << "\n===== ТЕСТИРОВАНИЕ ХЕШ-ТАБЛИЦЫ ЗАВЕРШЕНО =====\n" << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Общее время выполнения всех тестов: " << duration << " мс" << std::endl;
    }
};