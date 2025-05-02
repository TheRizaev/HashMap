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
    static void testIntContainer(const char* containerName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "\n=== Тестирование " << containerName << " с целыми числами ===\n" << std::endl;
        Mem memory(10000);
        ContainerType container(memory);

        std::cout << "Пустой " << containerName << ": " << (container.empty() ? "да" : "нет") << std::endl;
        std::cout << "Размер " << containerName << ": " << container.size() << std::endl;

        int keys[] = { 1, 2, 3, 4, 5 };
        int values[] = { 10, 20, 30, 40, 50 };

        for (int i = 0; i < 5; i++)
        {
            int result = container.insertByKey(&keys[i], sizeof(int), &values[i], sizeof(int));
            std::cout << "Добавление ключа " << keys[i] << ", значения " << values[i]
                << ": " << (result == 0 ? "успешно" : "ошибка") << std::endl;
        }

        std::cout << "Размер " << containerName << " после добавления: " << container.size() << std::endl;
        std::cout << containerName << " пуст? " << (container.empty() ? "да" : "нет") << std::endl;

        for (int i = 0; i < 5; i++)
        {
            size_t valueSize;
            int* value = (int*)container.at(&keys[i], sizeof(int), valueSize);

            if (value != nullptr)
            {
                std::cout << "Значение для ключа " << keys[i] << ": " << *value << std::endl;
            }
            else
            {
                std::cout << "Значение для ключа " << keys[i] << " не найдено" << std::endl;
            }
        }

        int nonExistentKey = 999;
        size_t valueSize;
        int* value = (int*)container.at(&nonExistentKey, sizeof(int), valueSize);
        std::cout << "Значение для несуществующего ключа " << nonExistentKey << ": "
            << (value != nullptr ? std::to_string(*value) : "не найдено") << std::endl;

        int keyToRemove = 3;
        container.removeByKey(&keyToRemove, sizeof(int));
        std::cout << "Удален ключ " << keyToRemove << std::endl;
        std::cout << "Размер " << containerName << " после удаления: " << container.size() << std::endl;

        value = (int*)container.at(&keyToRemove, sizeof(int), valueSize);
        std::cout << "Значение для удаленного ключа " << keyToRemove << ": "
            << (value != nullptr ? std::to_string(*value) : "не найдено") << std::endl;

        std::cout << "\nПеречисление всех элементов через итератор:" << std::endl;
        Container::Iterator* iter = container.newIterator();

        if (iter != nullptr)
        {
            do
            {
                size_t elemSize;
                void* elemValue = iter->getElement(elemSize);

                if (elemValue != nullptr)
                {
                    kv_pair* pair = (kv_pair*)elemValue;
                    int* k = (int*)pair->key;
                    int* v = (int*)pair->value;
                    std::cout << "Ключ: " << *k << ", Значение: " << *v << std::endl;
                }

                if (!iter->hasNext())
                    break;

                iter->goToNext();
            } while (true);

            delete iter;
        }

        container.clear();
        std::cout << "\nПосле очистки:" << std::endl;
        std::cout << "Размер " << containerName << ": " << container.size() << std::endl;
        std::cout << containerName << " пуст? " << (container.empty() ? "да" : "нет") << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Время выполнения теста: " << duration << " мс" << std::endl;
    }

    template <typename ContainerType>
    static void testStringContainer(const char* containerName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "\n\n\n\n\n\n=== Тестирование " << containerName << " со строками ===\n" << std::endl;

        Mem memory(10000);
        ContainerType container(memory);

        const char* keys[] = { "apple", "banana", "cherry", "date", "elderberry" };
        const char* values[] = { "red",   "yellow", "red",    "brown", "purple" };

        for (int i = 0; i < 5; i++)
        {
            int result = container.insertByKey((void*)keys[i], strlen(keys[i]) + 1,
                (void*)values[i], strlen(values[i]) + 1);
            std::cout << "Добавление ключа '" << keys[i] << "', значения '" << values[i]
                << "': " << (result == 0 ? "успешно" : "ошибка") << std::endl;
        }

        const char* duplicateKey = "apple";
        const char* newValue = "green";
        int result = container.insertByKey((void*)duplicateKey, strlen(duplicateKey),
            (void*)newValue, strlen(newValue) + 1);
        std::cout << "Добавление дублирующегося ключа '" << duplicateKey << "': "
            << (result == 0 ? "успешно" : "ошибка") << std::endl;

        for (int i = 0; i < 5; i++)
        {
            size_t valueSize;
            char* value = (char*)container.at((void*)keys[i], strlen(keys[i]) + 1, valueSize);

            if (value != nullptr)
            {
                std::cout << "Значение для ключа '" << keys[i] << "': '" << value << "'" << std::endl;
            }
            else
            {
                std::cout << "Значение для ключа '" << keys[i] << "' не найдено" << std::endl;
            }
        }

        const char* keyToRemove = "banana";
        container.removeByKey((void*)keyToRemove, strlen(keyToRemove) + 1);
        std::cout << "Удален ключ '" << keyToRemove << "'" << std::endl;

        size_t valueSize;
        char* value = (char*)container.at((void*)keyToRemove, strlen(keyToRemove) + 1, valueSize);
        std::cout << "Значение для удаленного ключа '" << keyToRemove << "': "
            << (value != nullptr ? value : "не найдено") << std::endl;

        const char* valueToFind = "red";
        Container::Iterator* iter = container.find((void*)valueToFind, strlen(valueToFind) + 1);
        std::cout << "Поиск значения '" << valueToFind << "': "
            << (iter != nullptr ? "найдено" : "не найдено") << std::endl;

        if (iter != nullptr)
            delete iter;

        container.clear();
        std::cout << "После очистки размер " << containerName << ": " << container.size() << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Время выполнения теста: " << duration << " мс" << std::endl;
    }

    template <typename ContainerType>
    static void testRehashing(const char* containerName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "\n\n\n\n\n\n=== Тестирование перехеширования " << containerName << " ===\n" << std::endl;

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

    template <typename ContainerType>
    static void testStructs(const char* containerName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "\n\n\n\n\n\n=== Тестирование " << containerName << " со сложными структурами ===\n" << std::endl;

        struct Person { int id; char name[50]; int age; };

        Mem memory(10000);
        ContainerType container(memory);

        Person persons[] = {
            {1, "Alice",   25},
            {2, "Bob",     30},
            {3, "Charlie", 35},
            {4, "David",   40},
            {5, "Eve",     45}
        };

        for (int i = 0; i < 5; i++)
        {
            int result = container.insertByKey(&persons[i].id, sizeof(int),
                &persons[i], sizeof(Person));
            std::cout << "Добавление Person с id=" << persons[i].id << ": "
                << (result == 0 ? "успешно" : "ошибка") << std::endl;
        }

        for (int i = 1; i <= 5; i++)
        {
            size_t valueSize;
            Person* person = (Person*)container.at(&i, sizeof(int), valueSize);

            if (person != nullptr)
                std::cout << "Person с id=" << i << ": name='" << person->name
                << "', age=" << person->age << std::endl;
            else
                std::cout << "Person с id=" << i << " не найден" << std::endl;
        }

        int idToRemove = 3;
        container.removeByKey(&idToRemove, sizeof(int));
        std::cout << "Удален Person с id=" << idToRemove << std::endl;

        size_t valueSize;
        Person* person = (Person*)container.at(&idToRemove, sizeof(int), valueSize);
        std::cout << "Person с id=" << idToRemove
            << (person != nullptr ? " найден" : " не найден") << std::endl;

        container.clear();
        std::cout << "После очистки размер " << containerName << ": " << container.size() << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Время выполнения теста: " << duration << " мс" << std::endl;
    }
public:

    static void testTableContainer()
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::cout << "===== ТЕСТИРОВАНИЕ ХЕШ-ТАБЛИЦЫ =====\n" << std::endl;
        testIntContainer<hashTable>("таблицы");
        testStringContainer<hashTable>("таблицы");
        testRehashing<hashTable>("таблицы");
        testStructs<hashTable>("таблицы");
        std::cout << "\n===== ТЕСТИРОВАНИЕ ХЕШ-ТАБЛИЦЫ ЗАВЕРШЕНО =====\n" << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Общее время выполнения всех тестов: " << duration << " мс" << std::endl;
    }

    static void testSetContainer()
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::cout << "===== ТЕСТИРОВАНИЕ МНОЖЕСТВА =====\n" << std::endl;
        /*testIntContainer<Set>("множество");
        testStringContainer<Set>("множество");
        testRehashing<Set>("множество");
        testStructs<Set>("множество");*/
        std::cout << "\n===== ТЕСТИРОВАНИЕ МНОастиоЖЕСТВА ЗАВЕРШЕНО =====\n" << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Общее время выполнения всех тестов множества: " << duration << " мс" << std::endl;
    }
};