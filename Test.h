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
    static void testKeyPresence(const char* containerName)
    {
        std::cout << "\n===== ���� �������� ������� ������ =====\n" << std::endl;

        Mem memory(100 * 1024 * 1024);
        ContainerType container(memory);

        const int numElements = 10000; // ������ ��������� ��� ����� �������� �����

        // ���������� ���������
        std::cout << "���������� " << numElements << " ���������..." << std::endl;
        for (int i = 0; i < numElements; i++)
        {
            int key = i;
            int value = i * 10;
            container.insertByKey(&key, sizeof(int), &value, sizeof(int));
        }
        std::cout << "������ �� ��������: " << container.size() << std::endl;

        // �������� ������ ������
        std::cout << "\n�������� ���� ������ ������..." << std::endl;
        for (int i = 0; i < numElements; i += 2)
        {
            container.removeByKey(&i, sizeof(int));
        }
        std::cout << "������ ����� ��������: " << container.size() << std::endl;

        // �������� ������� ������ ������ (������ �������������)
        std::cout << "\n�������� ���������� ������ ������..." << std::endl;
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
                if (evenKeysFound <= 10) // ������� ������ ������ 10 ���������
                {
                    std::cout << "  ������: ������ ������ ���� " << i << " �� ��������� " << *(int*)value << std::endl;
                }
            }
        }

        if (evenKeysFound > 0)
        {
            std::cout << "������: ������� " << evenKeysFound << " ������ ������ �� " << evenKeysChecked << " �����������!" << std::endl;
        }
        else
        {
            std::cout << "��: ��� " << evenKeysChecked << " ������ ������ ������� �������" << std::endl;
        }

        // �������� ������� �������� ������ (������ ��������������)
        std::cout << "\n�������� ������� �������� ������..." << std::endl;
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
                // ��������� ������������ ��������
                if (*(int*)value != i * 10)
                {
                    std::cout << "  ������: ���� " << i << " ����� ������������ ��������: "
                        << *(int*)value << " (��������� " << i * 10 << ")" << std::endl;
                }
            }
            else
            {
                oddKeysMissing++;
                if (oddKeysMissing <= 10) // ������� ������ ������ 10 �������������
                {
                    std::cout << "  ������: ����������� �������� ���� " << i << std::endl;
                }
            }
        }

        std::cout << "������� " << oddKeysFound << " �������� ������ �� " << oddKeysChecked << " ���������" << std::endl;
        if (oddKeysMissing > 0)
        {
            std::cout << "������: ����������� " << oddKeysMissing << " �������� ������!" << std::endl;
        }
        else
        {
            std::cout << "��: ��� �������� ����� �� �����" << std::endl;
        }

        // �������� 10 ��������� ������
        std::cout << "\n===== �������� 10 ��������� ������ =====\n";
        srand(static_cast<unsigned>(time(nullptr)));

        for (int i = 0; i < 10; i++)
        {
            int randomKey = rand() % numElements;
            size_t valueSize;
            void* value = container.at(&randomKey, sizeof(int), valueSize);

            std::cout << "���� " << randomKey << ": ";

            if (value != nullptr)
            {
                int intValue = *(int*)value;
                std::cout << "������, �������� = " << intValue;

                if (randomKey % 2 == 0)
                {
                    std::cout << " [������: ������ ���� �� ������ ������������!]";
                }
                else
                {
                    std::cout << " [OK: ��������]";
                    if (intValue != randomKey * 10)
                    {
                        std::cout << " [������: ��������� " << randomKey * 10 << "]";
                    }
                }
            }
            else
            {
                std::cout << "�� ������";

                if (randomKey % 2 == 0)
                {
                    std::cout << " [OK: ������ ���� ������]";
                }
                else
                {
                    std::cout << " [������: �������� ���� ������ ������������!]";
                }
            }

            std::cout << std::endl;
        }

        // ����� 10 ��������� ��������� �� �������
        std::cout << "\n===== ��������� �������� �� ������� =====\n";
        Container::Iterator* iter = container.newIterator();
        if (iter)
        {
            int elementsToShow = 10;
            int currentElement = 0;
            int totalElements = container.size();

            // ���������� ��� ��� ������������ �������������
            int step = totalElements / elementsToShow;
            if (step < 1) step = 1;

            int nextShowAt = 0;

            std::cout << "����� " << elementsToShow << " ��������� �� " << totalElements << ":\n";

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
                            std::cout << "  ������� #" << currentElement
                                << ": ���� = " << key
                                << ", �������� = " << value;

                            // ��������� �������� �����
                            if (key % 2 == 0)
                            {
                                std::cout << " [������: ������ ����!]";
                            }
                            else
                            {
                                std::cout << " [OK: ��������]";
                            }

                            // ��������� ������������ ��������
                            if (value != key * 10)
                            {
                                std::cout << " [������: ������������ ��������!]";
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
            std::cout << "�� ������� ������� �������� ��� ������ ���������" << std::endl;
        }

        // �������� ��������
        std::cout << "\n===== ���� ����� =====\n";
        int expectedSize = numElements / 2; // �������� ��������� ������ ��������
        int actualSize = container.size();

        if (actualSize == expectedSize && evenKeysFound == 0 && oddKeysMissing == 0)
        {
            std::cout << "���� �������: ��� �������� ��������� �������!" << std::endl;
        }
        else
        {
            std::cout << "���� ��������:" << std::endl;
            std::cout << "  - ��������� ������: " << expectedSize << ", �����������: " << actualSize << std::endl;
            std::cout << "  - ������� ������ ������ (������ ���� 0): " << evenKeysFound << std::endl;
            std::cout << "  - ����������� �������� ������ (������ ���� 0): " << oddKeysMissing << std::endl;
        }

        container.clear();
    }
public:
    static void testTableContainer()
    {
        auto start = std::chrono::high_resolution_clock::now();

        // �������� ���� ������������������
        testRehashing<hashTable>("�������");

        // ���� �������� ������� ������
        testKeyPresence<hashTable>("�������");;

        std::cout << "\n===== ������������ ���-������� ��������� =====\n" << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "����� ����� ���������� ���� ������: " << duration << " ��" << std::endl;
    }
};