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
        std::cout << "\n=== ������������ " << containerName << " � ������ ������� ===\n" << std::endl;
        Mem memory(10000);
        ContainerType container(memory);

        std::cout << "������ " << containerName << ": " << (container.empty() ? "��" : "���") << std::endl;
        std::cout << "������ " << containerName << ": " << container.size() << std::endl;

        int keys[] = { 1, 2, 3, 4, 5 };
        int values[] = { 10, 20, 30, 40, 50 };

        for (int i = 0; i < 5; i++)
        {
            int result = container.insertByKey(&keys[i], sizeof(int), &values[i], sizeof(int));
            std::cout << "���������� ����� " << keys[i] << ", �������� " << values[i]
                << ": " << (result == 0 ? "�������" : "������") << std::endl;
        }

        std::cout << "������ " << containerName << " ����� ����������: " << container.size() << std::endl;
        std::cout << containerName << " ����? " << (container.empty() ? "��" : "���") << std::endl;

        for (int i = 0; i < 5; i++)
        {
            size_t valueSize;
            int* value = (int*)container.at(&keys[i], sizeof(int), valueSize);

            if (value != nullptr)
            {
                std::cout << "�������� ��� ����� " << keys[i] << ": " << *value << std::endl;
            }
            else
            {
                std::cout << "�������� ��� ����� " << keys[i] << " �� �������" << std::endl;
            }
        }

        int nonExistentKey = 999;
        size_t valueSize;
        int* value = (int*)container.at(&nonExistentKey, sizeof(int), valueSize);
        std::cout << "�������� ��� ��������������� ����� " << nonExistentKey << ": "
            << (value != nullptr ? std::to_string(*value) : "�� �������") << std::endl;

        int keyToRemove = 3;
        container.removeByKey(&keyToRemove, sizeof(int));
        std::cout << "������ ���� " << keyToRemove << std::endl;
        std::cout << "������ " << containerName << " ����� ��������: " << container.size() << std::endl;

        value = (int*)container.at(&keyToRemove, sizeof(int), valueSize);
        std::cout << "�������� ��� ���������� ����� " << keyToRemove << ": "
            << (value != nullptr ? std::to_string(*value) : "�� �������") << std::endl;

        std::cout << "\n������������ ���� ��������� ����� ��������:" << std::endl;
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
                    std::cout << "����: " << *k << ", ��������: " << *v << std::endl;
                }

                if (!iter->hasNext())
                    break;

                iter->goToNext();
            } while (true);

            delete iter;
        }

        container.clear();
        std::cout << "\n����� �������:" << std::endl;
        std::cout << "������ " << containerName << ": " << container.size() << std::endl;
        std::cout << containerName << " ����? " << (container.empty() ? "��" : "���") << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "����� ���������� �����: " << duration << " ��" << std::endl;
    }

    template <typename ContainerType>
    static void testStringContainer(const char* containerName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "\n\n\n\n\n\n=== ������������ " << containerName << " �� �������� ===\n" << std::endl;

        Mem memory(10000);
        ContainerType container(memory);

        const char* keys[] = { "apple", "banana", "cherry", "date", "elderberry" };
        const char* values[] = { "red",   "yellow", "red",    "brown", "purple" };

        for (int i = 0; i < 5; i++)
        {
            int result = container.insertByKey((void*)keys[i], strlen(keys[i]) + 1,
                (void*)values[i], strlen(values[i]) + 1);
            std::cout << "���������� ����� '" << keys[i] << "', �������� '" << values[i]
                << "': " << (result == 0 ? "�������" : "������") << std::endl;
        }

        const char* duplicateKey = "apple";
        const char* newValue = "green";
        int result = container.insertByKey((void*)duplicateKey, strlen(duplicateKey),
            (void*)newValue, strlen(newValue) + 1);
        std::cout << "���������� �������������� ����� '" << duplicateKey << "': "
            << (result == 0 ? "�������" : "������") << std::endl;

        for (int i = 0; i < 5; i++)
        {
            size_t valueSize;
            char* value = (char*)container.at((void*)keys[i], strlen(keys[i]) + 1, valueSize);

            if (value != nullptr)
            {
                std::cout << "�������� ��� ����� '" << keys[i] << "': '" << value << "'" << std::endl;
            }
            else
            {
                std::cout << "�������� ��� ����� '" << keys[i] << "' �� �������" << std::endl;
            }
        }

        const char* keyToRemove = "banana";
        container.removeByKey((void*)keyToRemove, strlen(keyToRemove) + 1);
        std::cout << "������ ���� '" << keyToRemove << "'" << std::endl;

        size_t valueSize;
        char* value = (char*)container.at((void*)keyToRemove, strlen(keyToRemove) + 1, valueSize);
        std::cout << "�������� ��� ���������� ����� '" << keyToRemove << "': "
            << (value != nullptr ? value : "�� �������") << std::endl;

        const char* valueToFind = "red";
        Container::Iterator* iter = container.find((void*)valueToFind, strlen(valueToFind) + 1);
        std::cout << "����� �������� '" << valueToFind << "': "
            << (iter != nullptr ? "�������" : "�� �������") << std::endl;

        if (iter != nullptr)
            delete iter;

        container.clear();
        std::cout << "����� ������� ������ " << containerName << ": " << container.size() << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "����� ���������� �����: " << duration << " ��" << std::endl;
    }

    template <typename ContainerType>
    static void testRehashing(const char* containerName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "\n\n\n\n\n\n=== ������������ ��������������� " << containerName << " ===\n" << std::endl;

        Mem memory(200 * 1024 * 1024);
        ContainerType container(memory);

        const int numElements = 1000000;
        std::cout << "���������� " << numElements << " ���������..." << std::endl;
        auto createStart = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numElements; i++)
        {
            int key = i;
            int value = i * 10;
            container.insertByKey(&key, sizeof(int), &value, sizeof(int));
        }
        auto createEnd = std::chrono::high_resolution_clock::now();
        auto createDuration = std::chrono::duration_cast<std::chrono::milliseconds>(createEnd - createStart).count();
        std::cout << "��������: " << createDuration / 1000.0 << " ������" << std::endl;

        auto deleteStart = std::chrono::high_resolution_clock::now();
        std::cout << "\n�������� ���� ������ ������..." << std::endl;
        for (int i = 0; i < numElements; i += 2)
        {
            container.removeByKey(&i, sizeof(int));
        }
        auto deleteEnd = std::chrono::high_resolution_clock::now();
        auto deleteDuration = std::chrono::duration_cast<std::chrono::milliseconds>(deleteEnd - deleteStart).count();
        std::cout << "����� �������� ������ ������: " << deleteDuration / 1000.0 << " ������" << std::endl;

        // �������� ������� 
        auto sizeStart = std::chrono::high_resolution_clock::now();
        size_t tableSize = container.size();
        std::cout << "\n������ " << containerName << " ����� �������� ������ ������: " << tableSize << std::endl;
        auto sizeEnd = std::chrono::high_resolution_clock::now();
        auto sizeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(sizeEnd - sizeStart).count();
        std::cout << "����� �������� ������� �������: " << sizeDuration / 1000.0 << " ������" << std::endl;

        container.clear();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "\n����� ���������� ����� � " << numElements << " ����������: " << duration / 1000.0 << " ������" << std::endl;
        std::cout << "������� ����� �� ��������: " << (double)duration / numElements / 1000.0 << " ������" << std::endl;
    }

    template <typename ContainerType>
    static void testStructs(const char* containerName)
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "\n\n\n\n\n\n=== ������������ " << containerName << " �� �������� ����������� ===\n" << std::endl;

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
            std::cout << "���������� Person � id=" << persons[i].id << ": "
                << (result == 0 ? "�������" : "������") << std::endl;
        }

        for (int i = 1; i <= 5; i++)
        {
            size_t valueSize;
            Person* person = (Person*)container.at(&i, sizeof(int), valueSize);

            if (person != nullptr)
                std::cout << "Person � id=" << i << ": name='" << person->name
                << "', age=" << person->age << std::endl;
            else
                std::cout << "Person � id=" << i << " �� ������" << std::endl;
        }

        int idToRemove = 3;
        container.removeByKey(&idToRemove, sizeof(int));
        std::cout << "������ Person � id=" << idToRemove << std::endl;

        size_t valueSize;
        Person* person = (Person*)container.at(&idToRemove, sizeof(int), valueSize);
        std::cout << "Person � id=" << idToRemove
            << (person != nullptr ? " ������" : " �� ������") << std::endl;

        container.clear();
        std::cout << "����� ������� ������ " << containerName << ": " << container.size() << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "����� ���������� �����: " << duration << " ��" << std::endl;
    }
public:

    static void testTableContainer()
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::cout << "===== ������������ ���-������� =====\n" << std::endl;
        testIntContainer<hashTable>("�������");
        testStringContainer<hashTable>("�������");
        testRehashing<hashTable>("�������");
        testStructs<hashTable>("�������");
        std::cout << "\n===== ������������ ���-������� ��������� =====\n" << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "����� ����� ���������� ���� ������: " << duration << " ��" << std::endl;
    }

    static void testSetContainer()
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::cout << "===== ������������ ��������� =====\n" << std::endl;
        /*testIntContainer<Set>("���������");
        testStringContainer<Set>("���������");
        testRehashing<Set>("���������");
        testStructs<Set>("���������");*/
        std::cout << "\n===== ������������ �������������� ��������� =====\n" << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "����� ����� ���������� ���� ������ ���������: " << duration << " ��" << std::endl;
    }
};