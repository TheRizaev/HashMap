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
    }

    template <typename ContainerType>
    static void testKeyPresence(const char* containerName)
    {
        std::cout << "\n===== ТЕСТ ПРОВЕРКИ НАЛИЧИЯ КЛЮЧЕЙ =====\n" << std::endl;

        Mem memory(100 * 1024 * 1024);
        ContainerType container(memory);

        const int numElements = 10000; // Меньше элементов для более быстрого теста

        // Добавление элементов
        std::cout << "Добавление " << numElements << " элементов..." << std::endl;
        for (int i = 0; i < numElements; i++)
        {
            int key = i;
            int value = i * 10;
            container.insertByKey(&key, sizeof(int), &value, sizeof(int));
        }
        std::cout << "Размер до удаления: " << container.size() << std::endl;

        // Удаление четных ключей
        std::cout << "\nУдаление всех четных ключей..." << std::endl;
        for (int i = 0; i < numElements; i += 2)
        {
            container.removeByKey(&i, sizeof(int));
        }
        std::cout << "Размер после удаления: " << container.size() << std::endl;

        // Проверка наличия четных ключей (должны отсутствовать)
        std::cout << "\nПроверка отсутствия четных ключей..." << std::endl;
        int evenKeysFound = 0;
        int evenKeysChecked = 0;
        for (int i = 0; i < numElements; i += 2)
        {
            evenKeysChecked++;
            size_t valueSize;
            void* value = container.at(&i, sizeof(int), valueSize);
            if (value != nullptr)
            {
                evenKeysFound++;
                if (evenKeysFound <= 10) // Выводим только первые 10 найденных
                {
                    std::cout << "  ОШИБКА: Найден четный ключ " << i << " со значением " << *(int*)value << std::endl;
                }
            }
        }

        if (evenKeysFound > 0)
        {
            std::cout << "ОШИБКА: Найдено " << evenKeysFound << " четных ключей из " << evenKeysChecked << " проверенных!" << std::endl;
        }
        else
        {
            std::cout << "ОК: Все " << evenKeysChecked << " четных ключей успешно удалены" << std::endl;
        }

        // Проверка наличия нечетных ключей (должны присутствовать)
        std::cout << "\nПроверка наличия нечетных ключей..." << std::endl;
        int oddKeysFound = 0;
        int oddKeysMissing = 0;
        int oddKeysChecked = 0;
        for (int i = 1; i < numElements; i += 2)
        {
            oddKeysChecked++;
            size_t valueSize;
            void* value = container.at(&i, sizeof(int), valueSize);
            if (value != nullptr)
            {
                oddKeysFound++;
                // Проверяем корректность значения
                if (*(int*)value != i * 10)
                {
                    std::cout << "  ОШИБКА: Ключ " << i << " имеет неправильное значение: "
                        << *(int*)value << " (ожидалось " << i * 10 << ")" << std::endl;
                }
            }
            else
            {
                oddKeysMissing++;
                if (oddKeysMissing <= 10) // Выводим только первые 10 отсутствующих
                {
                    std::cout << "  ОШИБКА: Отсутствует нечетный ключ " << i << std::endl;
                }
            }
        }

        std::cout << "Найдено " << oddKeysFound << " нечетных ключей из " << oddKeysChecked << " ожидаемых" << std::endl;
        if (oddKeysMissing > 0)
        {
            std::cout << "ОШИБКА: Отсутствует " << oddKeysMissing << " нечетных ключей!" << std::endl;
        }
        else
        {
            std::cout << "ОК: Все нечетные ключи на месте" << std::endl;
        }

        // Проверка 10 случайных ключей
        std::cout << "\n===== ПРОВЕРКА 10 СЛУЧАЙНЫХ КЛЮЧЕЙ =====\n";
        srand(static_cast<unsigned>(time(nullptr)));

        for (int i = 0; i < 10; i++)
        {
            int randomKey = rand() % numElements;
            size_t valueSize;
            void* value = container.at(&randomKey, sizeof(int), valueSize);

            std::cout << "Ключ " << randomKey << ": ";

            if (value != nullptr)
            {
                int intValue = *(int*)value;
                std::cout << "найден, значение = " << intValue;

                if (randomKey % 2 == 0)
                {
                    std::cout << " [ОШИБКА: четный ключ не должен существовать!]";
                }
                else
                {
                    std::cout << " [OK: нечетный]";
                    if (intValue != randomKey * 10)
                    {
                        std::cout << " [ОШИБКА: ожидалось " << randomKey * 10 << "]";
                    }
                }
            }
            else
            {
                std::cout << "не найден";

                if (randomKey % 2 == 0)
                {
                    std::cout << " [OK: четный ключ удален]";
                }
                else
                {
                    std::cout << " [ОШИБКА: нечетный ключ должен существовать!]";
                }
            }

            std::cout << std::endl;
        }

        // Вывод 10 случайных элементов из таблицы
        std::cout << "\n===== СЛУЧАЙНЫЕ ЭЛЕМЕНТЫ ИЗ ТАБЛИЦЫ =====\n";
        Container::Iterator* iter = container.newIterator();
        if (iter)
        {
            int elementsToShow = 10;
            int currentElement = 0;
            int totalElements = container.size();

            // Определяем шаг для равномерного распределения
            int step = totalElements / elementsToShow;
            if (step < 1) step = 1;

            int nextShowAt = 0;

            std::cout << "Вывод " << elementsToShow << " элементов из " << totalElements << ":\n";

            do {
                if (currentElement == nextShowAt)
                {
                    size_t elemSize;
                    void* element = iter->getElement(elemSize);
                    if (element)
                    {
                        kv_pair* pair = static_cast<kv_pair*>(element);
                        if (pair && pair->key && pair->value)
                        {
                            int key = *(int*)pair->key;
                            int value = *(int*)pair->value;
                            std::cout << "  Элемент #" << currentElement
                                << ": Ключ = " << key
                                << ", Значение = " << value;

                            // Проверяем четность ключа
                            if (key % 2 == 0)
                            {
                                std::cout << " [ОШИБКА: ЧЕТНЫЙ КЛЮЧ!]";
                            }
                            else
                            {
                                std::cout << " [OK: нечетный]";
                            }

                            // Проверяем корректность значения
                            if (value != key * 10)
                            {
                                std::cout << " [ОШИБКА: неправильное значение!]";
                            }

                            std::cout << std::endl;
                        }
                    }

                    nextShowAt += step;
                    elementsToShow--;
                    if (elementsToShow == 0) break;
                }

                currentElement++;

                if (!iter->hasNext()) break;
                iter->goToNext();

            } while (true);

            delete iter;
        }
        else
        {
            std::cout << "Не удалось создать итератор для вывода элементов" << std::endl;
        }

        // Итоговая проверка
        std::cout << "\n===== ИТОГ ТЕСТА =====\n";
        int expectedSize = numElements / 2; // Половина элементов должна остаться
        int actualSize = container.size();

        if (actualSize == expectedSize && evenKeysFound == 0 && oddKeysMissing == 0)
        {
            std::cout << "ТЕСТ ПРОЙДЕН: Все проверки выполнены успешно!" << std::endl;
        }
        else
        {
            std::cout << "ТЕСТ ПРОВАЛЕН:" << std::endl;
            std::cout << "  - Ожидаемый размер: " << expectedSize << ", фактический: " << actualSize << std::endl;
            std::cout << "  - Найдено четных ключей (должно быть 0): " << evenKeysFound << std::endl;
            std::cout << "  - Отсутствует нечетных ключей (должно быть 0): " << oddKeysMissing << std::endl;
        }

        container.clear();
    }
public:
    static void testTableContainer()
    {
        auto start = std::chrono::high_resolution_clock::now();

        // Основной тест производительности
        testRehashing<hashTable>("таблицы");

        // Тест проверки наличия ключей
        testKeyPresence<hashTable>("таблицы");;

        std::cout << "\n===== ТЕСТИРОВАНИЕ ХЕШ-ТАБЛИЦЫ ЗАВЕРШЕНО =====\n" << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Общее время выполнения всех тестов: " << duration << " мс" << std::endl;
    }
};