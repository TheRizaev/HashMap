#pragma once
#include <iostream>
#include <cstring>
#include <chrono>
#include <vector>
#include "Table.h"
#include "Mem.h"

class ContainerTester
{
public:
    static void testTableContainer()
    {
        std::cout << "===== ТЕСТИРОВАНИЕ ХЕШ-ТАБЛИЦЫ =====\n" << std::endl;

        // Тест 1: Строки
        testStrings();

        // Тест 2: Миллион элементов
        testMillion();

        std::cout << "\n===== ТЕСТИРОВАНИЕ ЗАВЕРШЕНО =====\n" << std::endl;
    }

private:
    static void testStrings()
    {
        std::cout << "=== Тест со строками ===" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        Mem memory(10000);
        hashTable table(memory);

        // Тестовые данные
        const char* keys[] = { "apple", "banana", "cherry", "date", "elderberry" };
        const char* values[] = { "red", "yellow", "red", "brown", "purple" };

        // Вставка
        for (int i = 0; i < 5; i++) {
            table.insertByKey((void*)keys[i], strlen(keys[i]) + 1,
                (void*)values[i], strlen(values[i]) + 1);
        }

        // Проверка
        std::cout << "Проверка вставленных значений:" << std::endl;
        for (int i = 0; i < 5; i++) {
            size_t valueSize;
            char* value = (char*)table.at((void*)keys[i], strlen(keys[i]) + 1, valueSize);
            std::cout << keys[i] << " -> " << (value ? value : "не найдено") << std::endl;
        }

        // Удаление и проверка
        table.removeByKey((void*)keys[1], strlen(keys[1]) + 1);
        size_t valueSize;
        char* value = (char*)table.at((void*)keys[1], strlen(keys[1]) + 1, valueSize);
        std::cout << "\nПосле удаления '" << keys[1] << "': "
            << (value ? "найдено" : "не найдено") << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end - start).count();
        std::cout << "Время выполнения: " << duration << " секунд\n" << std::endl;
    }

    static void testMillion()
    {
        std::cout << "=== Тест с миллионом элементов ===" << std::endl;

        Mem memory(200 * 1024 * 1024); // 200 МБ
        hashTable table(memory);

        const int numElements = 1000000;

        // Вставка миллиона элементов
        std::cout << "Вставка " << numElements << " элементов..." << std::endl;
        auto insertStart = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numElements; i++) {
            table.insertByKey(&i, sizeof(int), &i, sizeof(int));
        }

        auto insertEnd = std::chrono::high_resolution_clock::now();
        auto insertTime = std::chrono::duration<double>(insertEnd - insertStart).count();
        std::cout << "Время вставки: " << insertTime << " секунд" << std::endl;
        std::cout << "Элементов в таблице: " << table.size() << std::endl;

        // Сбор четных ключей для удаления
        std::cout << "\nСбор ключей для удаления..." << std::endl;
        std::vector<int> keysToDelete;
        keysToDelete.reserve(numElements / 2);

        for (int i = 0; i < numElements; i += 2) {
            keysToDelete.push_back(i);
        }

        // Удаление половины элементов (четные)
        std::cout << "Удаление " << keysToDelete.size() << " элементов..." << std::endl;
        auto deleteStart = std::chrono::high_resolution_clock::now();

        for (int key : keysToDelete) {
            table.removeByKey(&key, sizeof(int));
        }

        auto deleteEnd = std::chrono::high_resolution_clock::now();
        auto deleteTime = std::chrono::duration<double>(deleteEnd - deleteStart).count();
        std::cout << "Время удаления: " << deleteTime << " секунд" << std::endl;
        std::cout << "Элементов в таблице после удаления: " << table.size() << std::endl;

        // Проверка удаленных элементов
        std::cout << "\nПроверка удаленных элементов..." << std::endl;
        auto checkStart = std::chrono::high_resolution_clock::now();

        int found = 0;
        int notFound = 0;

        // Проверяем первые 100 четных ключей
        for (int i = 0; i < 200; i += 2) {
            size_t valueSize;
            void* value = table.at(&i, sizeof(int), valueSize);
            if (value) {
                found++;
            }
            else {
                notFound++;
            }
        }

        std::cout << "Из первых 100 удаленных ключей:" << std::endl;
        std::cout << "- Найдено: " << found << std::endl;
        std::cout << "- Не найдено: " << notFound << std::endl;

        // Проверяем нечетные ключи (должны остаться)
        found = 0;
        notFound = 0;
        for (int i = 1; i < 200; i += 2) {
            size_t valueSize;
            void* value = table.at(&i, sizeof(int), valueSize);
            if (value) {
                found++;
            }
            else {
                notFound++;
            }
        }

        std::cout << "\nИз первых 100 оставшихся ключей (нечетные):" << std::endl;
        std::cout << "- Найдено: " << found << std::endl;
        std::cout << "- Не найдено: " << notFound << std::endl;

        auto checkEnd = std::chrono::high_resolution_clock::now();
        auto checkTime = std::chrono::duration<double>(checkEnd - checkStart).count();
        std::cout << "\nВремя проверки: " << checkTime << " секунд" << std::endl;

        // Общее время
        auto totalTime = insertTime + deleteTime + checkTime;
        std::cout << "\nОбщее время теста: " << totalTime << " секунд" << std::endl;
        std::cout << "Среднее время на операцию: " << totalTime / (numElements * 1.5) * 1000000
            << " микросекунд" << std::endl;
    }
};