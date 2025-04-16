#include <iostream>
#include <cstring>
#include <string>
#include "Table.h"
#include "Mem.h"
#include <locale.h>

// ����� ��� ������������ �����������
class ContainerTester
{
public:
    // ������������ ���-������� � ������ �������
    static void testIntTable()
    {
        std::cout << "\n=== ������������ ������� � ������ ������� ===\n" << std::endl;

        // ������� �������� ������
        Mem memory(10000);

        // ������� �������
        Table table(memory);

        // ��������� ������ �������
        std::cout << "������ �������: " << (table.empty() ? "��" : "���") << std::endl;
        std::cout << "������ �������: " << table.size() << std::endl;

        // ��������� ��������
        int keys[] = { 1, 2, 3, 4, 5 };
        int values[] = { 10, 20, 30, 40, 50 };

        for (int i = 0; i < 5; i++)
        {
            int result = table.insertByKey(&keys[i], sizeof(int), &values[i], sizeof(int));
            std::cout << "���������� ����� " << keys[i] << ", �������� " << values[i]
                << ": " << (result == 0 ? "�������" : "������") << std::endl;
        }

        // ��������� ������
        std::cout << "������ ������� ����� ����������: " << table.size() << std::endl;
        std::cout << "������� �����? " << (table.empty() ? "��" : "���") << std::endl;

        // �������� �������� �� �����
        for (int i = 0; i < 5; i++)
        {
            size_t valueSize;
            int* value = (int*)table.at(&keys[i], sizeof(int), valueSize);

            if (value != nullptr)
            {
                std::cout << "�������� ��� ����� " << keys[i] << ": " << *value << std::endl;
            }
            else
            {
                std::cout << "�������� ��� ����� " << keys[i] << " �� �������" << std::endl;
            }
        }

        // ��������� ����� ��������������� �����
        int nonExistentKey = 999;
        size_t valueSize;
        int* value = (int*)table.at(&nonExistentKey, sizeof(int), valueSize);
        std::cout << "�������� ��� ��������������� ����� " << nonExistentKey << ": "
            << (value != nullptr ? std::to_string(*value) : "�� �������") << std::endl;

        // ��������� ��������
        int keyToRemove = 3;
        table.removeByKey(&keyToRemove, sizeof(int));
        std::cout << "������ ���� " << keyToRemove << std::endl;
        std::cout << "������ ������� ����� ��������: " << table.size() << std::endl;

        // ���������, ��� ������� ������
        value = (int*)table.at(&keyToRemove, sizeof(int), valueSize);
        std::cout << "�������� ��� ���������� ����� " << keyToRemove << ": "
            << (value != nullptr ? std::to_string(*value) : "�� �������") << std::endl;

        // ��������� ��������
        std::cout << "\n������������ ���� ��������� ����� ��������:" << std::endl;
        Container::Iterator* iter = table.newIterator();

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
                {
                    break;
                }

                iter->goToNext();
            } while (true);

            delete iter;
        }

        // ������� �������
        table.clear();
        std::cout << "\n����� �������:" << std::endl;
        std::cout << "������ �������: " << table.size() << std::endl;
        std::cout << "������� �����? " << (table.empty() ? "��" : "���") << std::endl;
    }

    // ������������ ���-������� �� ��������
    static void testStringTable()
    {
        std::cout << "\n=== ������������ ������� �� �������� ===\n" << std::endl;

        // ������� �������� ������
        Mem memory(10000);

        // ������� �������
        Table table(memory);

        // ��������� ��������
        const char* keys[] = { "apple", "banana", "cherry", "date", "elderberry" };
        const char* values[] = { "red", "yellow", "red", "brown", "purple" };

        for (int i = 0; i < 5; i++)
        {
            int result = table.insertByKey((void*)keys[i], strlen(keys[i]) + 1,
                (void*)values[i], strlen(values[i]) + 1);
            std::cout << "���������� ����� '" << keys[i] << "', �������� '" << values[i]
                << "': " << (result == 0 ? "�������" : "������") << std::endl;
        }

        // ������� �������� ������������� ����
        const char* duplicateKey = "apple";
        const char* newValue = "green";
        int result = table.insertByKey((void*)duplicateKey, strlen(duplicateKey) + 1,
            (void*)newValue, strlen(newValue) + 1);
        std::cout << "���������� �������������� ����� '" << duplicateKey << "': "
            << (result == 0 ? "�������" : "������") << std::endl;

        // �������� �������� �� �����
        for (int i = 0; i < 5; i++)
        {
            size_t valueSize;
            char* value = (char*)table.at((void*)keys[i], strlen(keys[i]) + 1, valueSize);

            if (value != nullptr)
            {
                std::cout << "�������� ��� ����� '" << keys[i] << "': '" << value << "'" << std::endl;
            }
            else
            {
                std::cout << "�������� ��� ����� '" << keys[i] << "' �� �������" << std::endl;
            }
        }

        // ��������� ��������
        const char* keyToRemove = "banana";
        table.removeByKey((void*)keyToRemove, strlen(keyToRemove) + 1);
        std::cout << "������ ���� '" << keyToRemove << "'" << std::endl;

        // ���������, ��� ������� ������
        size_t valueSize;
        char* value = (char*)table.at((void*)keyToRemove, strlen(keyToRemove) + 1, valueSize);
        std::cout << "�������� ��� ���������� ����� '" << keyToRemove << "': "
            << (value != nullptr ? value : "�� �������") << std::endl;

        // ��������� ����� �� ��������
        const char* valueToFind = "red";
        Container::Iterator* iter = table.find((void*)valueToFind, strlen(valueToFind) + 1);
        std::cout << "����� �������� '" << valueToFind << "': "
            << (iter != nullptr ? "�������" : "�� �������") << std::endl;

        if (iter != nullptr)
        {
            delete iter;
        }

        // ������� �������
        table.clear();
        std::cout << "����� ������� ������ �������: " << table.size() << std::endl;
    }

    // ������������ ��������������� ��� ���������� ���������� ��������
    static void testRehashing()
    {
        std::cout << "\n=== ������������ ��������������� ===\n" << std::endl;

        // ������� �������� ������
        Mem memory(100000);

        // ������� �������
        Table table(memory);

        const int numElements = 2000; // ���������� ��� ������������ ���������������

        std::cout << "���������� " << numElements << " ���������..." << std::endl;

        // ��������� ��������
        for (int i = 0; i < numElements; i++)
        {
            int key = i;
            int value = i * 10;
            table.insertByKey(&key, sizeof(int), &value, sizeof(int));

            // ������� ���������� � ������� ����� ��������� ���������
            if (i % 500 == 0 || i == numElements - 1)
            {
                std::cout << "��������� " << i + 1 << " ���������, ������ �������: "
                    << table.size() << std::endl;
            }
        }

        // ��������� ������ � ��������� ���������
        const int numChecks = 20;
        std::cout << "\n��������� " << numChecks << " ��������� ��������� ����� ���������������:" << std::endl;

        for (int i = 0; i < numChecks; i++)
        {
            int key = rand() % numElements;
            size_t valueSize;
            int* value = (int*)table.at(&key, sizeof(int), valueSize);

            std::cout << "���� " << key << ": "
                << (value != nullptr ? std::to_string(*value) : "�� �������") << std::endl;
        }

        // ������� �������
        table.clear();
    }

    // ������������ ������� ��������
    static void testStructs()
    {
        std::cout << "\n=== ������������ �� �������� ����������� ===\n" << std::endl;

        // ���������� ���������
        struct Person
        {
            int id;
            char name[50];
            int age;
        };

        // ������� �������� ������
        Mem memory(10000);

        // ������� �������
        Table table(memory);

        // ������� ��������� �������� Person
        Person persons[] = {
            {1, "Alice", 25},
            {2, "Bob", 30},
            {3, "Charlie", 35},
            {4, "David", 40},
            {5, "Eve", 45}
        };

        // ��������� �������� � �������
        for (int i = 0; i < 5; i++)
        {
            int result = table.insertByKey(&persons[i].id, sizeof(int), &persons[i], sizeof(Person));
            std::cout << "���������� Person � id=" << persons[i].id << ": "
                << (result == 0 ? "�������" : "������") << std::endl;
        }

        // �������� �������� �� �����
        for (int i = 1; i <= 5; i++)
        {
            size_t valueSize;
            Person* person = (Person*)table.at(&i, sizeof(int), valueSize);

            if (person != nullptr)
            {
                std::cout << "Person � id=" << i << ": name='" << person->name
                    << "', age=" << person->age << std::endl;
            }
            else
            {
                std::cout << "Person � id=" << i << " �� ������" << std::endl;
            }
        }

        // ������� ���� �������
        int idToRemove = 3;
        table.removeByKey(&idToRemove, sizeof(int));
        std::cout << "������ Person � id=" << idToRemove << std::endl;

        // ���������, ��� ������� ������
        size_t valueSize;
        Person* person = (Person*)table.at(&idToRemove, sizeof(int), valueSize);
        std::cout << "Person � id=" << idToRemove << (person != nullptr ? " ������" : " �� ������") << std::endl;

        // ������� �������
        table.clear();
        std::cout << "����� ������� ������ �������: " << table.size() << std::endl;
    }
};

int main()
{
    setlocale(LC_ALL, "Russian");

    std::cout << "===== ������������ ���-������� =====\n" << std::endl;

    // ��������� ���-������� � ������� ������ ������
    ContainerTester::testIntTable();
    ContainerTester::testStringTable();
    ContainerTester::testRehashing();
    ContainerTester::testStructs();

    std::cout << "\n===== ������������ ��������� =====\n" << std::endl;

    return 0;
}