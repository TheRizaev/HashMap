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
        std::cout << "===== ������������ ���-������� =====\n" << std::endl;

        // ���� 1: ������
        testStrings();

        // ���� 2: ������� ���������
        testMillion();

        std::cout << "\n===== ������������ ��������� =====\n" << std::endl;
    }

private:
    static void testStrings()
    {
        std::cout << "=== ���� �� �������� ===" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        Mem memory(10000);
        hashTable table(memory);

        // �������� ������
        const char* keys[] = { "apple", "banana", "cherry", "date", "elderberry" };
        const char* values[] = { "red", "yellow", "red", "brown", "purple" };

        // �������
        for (int i = 0; i < 5; i++) {
            table.insertByKey((void*)keys[i], strlen(keys[i]) + 1,
                (void*)values[i], strlen(values[i]) + 1);
        }

        // ��������
        std::cout << "�������� ����������� ��������:" << std::endl;
        for (int i = 0; i < 5; i++) {
            size_t valueSize;
            char* value = (char*)table.at((void*)keys[i], strlen(keys[i]) + 1, valueSize);
            std::cout << keys[i] << " -> " << (value ? value : "�� �������") << std::endl;
        }

        // �������� � ��������
        table.removeByKey((void*)keys[1], strlen(keys[1]) + 1);
        size_t valueSize;
        char* value = (char*)table.at((void*)keys[1], strlen(keys[1]) + 1, valueSize);
        std::cout << "\n����� �������� '" << keys[1] << "': "
            << (value ? "�������" : "�� �������") << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end - start).count();
        std::cout << "����� ����������: " << duration << " ������\n" << std::endl;
    }

    static void testMillion()
    {
        std::cout << "=== ���� � ��������� ��������� ===" << std::endl;

        Mem memory(200 * 1024 * 1024); // 200 ��
        hashTable table(memory);

        const int numElements = 1000000;

        // ������� �������� ���������
        std::cout << "������� " << numElements << " ���������..." << std::endl;
        auto insertStart = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < numElements; i++) {
            table.insertByKey(&i, sizeof(int), &i, sizeof(int));
        }

        auto insertEnd = std::chrono::high_resolution_clock::now();
        auto insertTime = std::chrono::duration<double>(insertEnd - insertStart).count();
        std::cout << "����� �������: " << insertTime << " ������" << std::endl;
        std::cout << "��������� � �������: " << table.size() << std::endl;

        // ���� ������ ������ ��� ��������
        std::cout << "\n���� ������ ��� ��������..." << std::endl;
        std::vector<int> keysToDelete;
        keysToDelete.reserve(numElements / 2);

        for (int i = 0; i < numElements; i += 2) {
            keysToDelete.push_back(i);
        }

        // �������� �������� ��������� (������)
        std::cout << "�������� " << keysToDelete.size() << " ���������..." << std::endl;
        auto deleteStart = std::chrono::high_resolution_clock::now();

        for (int key : keysToDelete) {
            table.removeByKey(&key, sizeof(int));
        }

        auto deleteEnd = std::chrono::high_resolution_clock::now();
        auto deleteTime = std::chrono::duration<double>(deleteEnd - deleteStart).count();
        std::cout << "����� ��������: " << deleteTime << " ������" << std::endl;
        std::cout << "��������� � ������� ����� ��������: " << table.size() << std::endl;

        // �������� ��������� ���������
        std::cout << "\n�������� ��������� ���������..." << std::endl;
        auto checkStart = std::chrono::high_resolution_clock::now();

        int found = 0;
        int notFound = 0;

        // ��������� ������ 100 ������ ������
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

        std::cout << "�� ������ 100 ��������� ������:" << std::endl;
        std::cout << "- �������: " << found << std::endl;
        std::cout << "- �� �������: " << notFound << std::endl;

        // ��������� �������� ����� (������ ��������)
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

        std::cout << "\n�� ������ 100 ���������� ������ (��������):" << std::endl;
        std::cout << "- �������: " << found << std::endl;
        std::cout << "- �� �������: " << notFound << std::endl;

        auto checkEnd = std::chrono::high_resolution_clock::now();
        auto checkTime = std::chrono::duration<double>(checkEnd - checkStart).count();
        std::cout << "\n����� ��������: " << checkTime << " ������" << std::endl;

        // ����� �����
        auto totalTime = insertTime + deleteTime + checkTime;
        std::cout << "\n����� ����� �����: " << totalTime << " ������" << std::endl;
        std::cout << "������� ����� �� ��������: " << totalTime / (numElements * 1.5) * 1000000
            << " �����������" << std::endl;
    }
};