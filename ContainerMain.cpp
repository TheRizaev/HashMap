#include <iostream>
#include <string>
#include <iomanip>
#include "Table.h"
#include "Mem.h"
#include <locale.h>

/**
 * Класс для тестирования ассоциативной таблицы
 * Выполняет различные операции и проверяет их корректность
 */
class TableTester {
private:
    Table* table;
    Mem* memory;

    // Вспомогательная функция для печати значения
    void printValue(void* value, size_t size) {
        if (value) {
            char* str = static_cast<char*>(value);
            std::cout << std::string(str, size - 1); // Вычитаем 1, чтобы не печатать нулевой символ
        }
        else {
            std::cout << "nullptr";
        }
    }

    // Вспомогательная функция для получения строки из значения
    std::string valueToString(void* value, size_t size) {
        if (value) {
            char* str = static_cast<char*>(value);
            return std::string(str, size - 1); // Вычитаем 1, чтобы не включать нулевой символ
        }
        return "nullptr";
    }

public:
    // Конструктор
    TableTester(size_t memorySize) {
        memory = new Mem(memorySize);
        table = new Table(*memory);
        std::cout << "Тестировщик таблицы создан с " << memorySize << " байтами памяти\n";
    }

    // Деструктор
    ~TableTester() {
        delete table;
        delete memory;
        std::cout << "Тестировщик таблицы уничтожен\n";
    }

    // Тест 1: Проверка создания пустой таблицы
    void testEmptyTable() {
        std::cout << "\n=== Тест 1: Проверка создания пустой таблицы ===\n";
        std::cout << "Ожидается: Таблица создана и пуста. Размер = 0\n";

        bool isEmpty = table->empty();
        int size = table->size();

        std::cout << "Результат: Таблица пуста: " << (isEmpty ? "да" : "нет")
            << ", размер: " << size << "\n";
        std::cout << "Тест " << (isEmpty && size == 0 ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";
    }

    // Тест 2: Вставка элементов в таблицу
    void testInsert() {
        std::cout << "\n=== Тест 2: Вставка элементов в таблицу ===\n";
        std::cout << "Ожидается: 3 элемента успешно вставлены. Размер = 3\n";

        std::string key1 = "имя";
        std::string value1 = "Иван";
        std::string key2 = "фамилия";
        std::string value2 = "Иванов";
        std::string key3 = "возраст";
        std::string value3 = "25";

        int result1 = table->insertByKey(
            (void*)key1.c_str(), key1.size() + 1,
            (void*)value1.c_str(), value1.size() + 1
        );

        int result2 = table->insertByKey(
            (void*)key2.c_str(), key2.size() + 1,
            (void*)value2.c_str(), value2.size() + 1
        );

        int result3 = table->insertByKey(
            (void*)key3.c_str(), key3.size() + 1,
            (void*)value3.c_str(), value3.size() + 1
        );

        std::cout << "Результат вставки:\n";
        std::cout << "Ключ '" << key1 << "': " << (result1 == 0 ? "успешно" : "ошибка") << "\n";
        std::cout << "Ключ '" << key2 << "': " << (result2 == 0 ? "успешно" : "ошибка") << "\n";
        std::cout << "Ключ '" << key3 << "': " << (result3 == 0 ? "успешно" : "ошибка") << "\n";
        std::cout << "Размер таблицы: " << table->size() << "\n";

        bool insertSuccess = (result1 == 0 && result2 == 0 && result3 == 0);
        bool sizeCorrect = (table->size() == 3);

        std::cout << "Тест " << (insertSuccess && sizeCorrect ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";
    }

    // Тест 3: Получение элементов по ключу
    void testRetrieve() {
        std::cout << "\n=== Тест 3: Получение элементов по ключу ===\n";
        std::cout << "Ожидается: Получение 3 значений по ключам с правильными значениями\n";

        std::string key1 = "имя";
        std::string expected1 = "Иван";
        std::string key2 = "фамилия";
        std::string expected2 = "Иванов";
        std::string key3 = "возраст";
        std::string expected3 = "25";

        size_t valueSize1, valueSize2, valueSize3;
        void* value1 = table->at((void*)key1.c_str(), key1.size() + 1, valueSize1);
        void* value2 = table->at((void*)key2.c_str(), key2.size() + 1, valueSize2);
        void* value3 = table->at((void*)key3.c_str(), key3.size() + 1, valueSize3);

        std::string actual1 = valueToString(value1, valueSize1);
        std::string actual2 = valueToString(value2, valueSize2);
        std::string actual3 = valueToString(value3, valueSize3);

        std::cout << "Результат получения:\n";
        std::cout << "Ключ '" << key1 << "': ожидалось '" << expected1 << "', получено '" << actual1 << "'\n";
        std::cout << "Ключ '" << key2 << "': ожидалось '" << expected2 << "', получено '" << actual2 << "'\n";
        std::cout << "Ключ '" << key3 << "': ожидалось '" << expected3 << "', получено '" << actual3 << "'\n";

        bool retrieveSuccess = (
            actual1 == expected1 &&
            actual2 == expected2 &&
            actual3 == expected3
            );

        std::cout << "Тест " << (retrieveSuccess ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";
    }

    // Тест 4: Получение несуществующего ключа
    void testNonExistentKey() {
        std::cout << "\n=== Тест 4: Получение несуществующего ключа ===\n";
        std::cout << "Ожидается: При запросе несуществующего ключа должен вернуться nullptr\n";

        std::string nonExistentKey = "город";
        size_t valueSize;
        void* value = table->at((void*)nonExistentKey.c_str(), nonExistentKey.size() + 1, valueSize);

        std::cout << "Результат получения:\n";
        std::cout << "Ключ '" << nonExistentKey << "': ";
        printValue(value, valueSize);
        std::cout << "\n";

        bool isNull = (value == nullptr);
        std::cout << "Тест " << (isNull ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";
    }

    // Тест 5: Вставка элемента с существующим ключом
    void testDuplicateKey() {
        std::cout << "\n=== Тест 5: Вставка элемента с существующим ключом ===\n";
        std::cout << "Ожидается: Вставка должна завершиться ошибкой (результат не 0)\n";

        std::string key = "имя";
        std::string newValue = "Петр";

        int result = table->insertByKey(
            (void*)key.c_str(), key.size() + 1,
            (void*)newValue.c_str(), newValue.size() + 1
        );

        std::cout << "Результат вставки:\n";
        std::cout << "Ключ '" << key << "': "
            << (result == 0 ? "успешно (ошибка теста)" : "ошибка (верно)") << "\n";

        // Проверяем, что значение не изменилось
        size_t valueSize;
        void* value = table->at((void*)key.c_str(), key.size() + 1, valueSize);
        std::string actualValue = valueToString(value, valueSize);

        std::cout << "Значение ключа '" << key << "' после попытки вставки: " << actualValue << "\n";

        bool insertFailed = (result != 0);
        bool valueUnchanged = (actualValue == "Иван");

        std::cout << "Тест " << (insertFailed && valueUnchanged ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";
    }

    // Тест 6: Удаление элемента
    void testRemove() {
        std::cout << "\n=== Тест 6: Удаление элемента ===\n";
        std::cout << "Ожидается: Элемент удален, размер уменьшен на 1\n";

        std::string keyToRemove = "фамилия";
        int sizeBefore = table->size();

        // Проверяем, что элемент существует
        size_t valueSizeBefore;
        void* valueBefore = table->at((void*)keyToRemove.c_str(), keyToRemove.size() + 1, valueSizeBefore);

        std::cout << "Значение перед удалением: ";
        printValue(valueBefore, valueSizeBefore);
        std::cout << "\n";

        // Удаляем элемент
        table->removeByKey((void*)keyToRemove.c_str(), keyToRemove.size() + 1);

        // Проверяем, что элемент удален
        size_t valueSizeAfter;
        void* valueAfter = table->at((void*)keyToRemove.c_str(), keyToRemove.size() + 1, valueSizeAfter);

        std::cout << "Значение после удаления: ";
        printValue(valueAfter, valueSizeAfter);
        std::cout << "\n";

        int sizeAfter = table->size();

        std::cout << "Размер до удаления: " << sizeBefore << "\n";
        std::cout << "Размер после удаления: " << sizeAfter << "\n";

        bool elementRemoved = (valueAfter == nullptr);
        bool sizeDecreased = (sizeAfter == sizeBefore - 1);

        std::cout << "Тест " << (elementRemoved && sizeDecreased ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";
    }

    // Тест 7: Итерация по таблице
    void testIteration() {
        std::cout << "\n=== Тест 7: Итерация по таблице ===\n";
        std::cout << "Ожидается: Перебор всех элементов таблицы через итератор\n";

        Container::Iterator* iter = table->newIterator();
        if (!iter) {
            std::cout << "Ошибка: не удалось создать итератор!\n";
            std::cout << "Тест ПРОВАЛЕН\n";
            return;
        }

        std::cout << "Элементы таблицы:\n";
        int count = 0;

        do {
            size_t valueSize;
            void* value = iter->getElement(valueSize);
            if (!value) {
                break;
            }

            std::cout << "Элемент " << count++ << ": ";
            printValue(value, valueSize);
            std::cout << "\n";

            if (!iter->hasNext()) {
                break;
            }

            iter->goToNext();
        } while (true);

        delete iter;

        std::cout << "Всего элементов через итератор: " << count << "\n";
        std::cout << "Размер таблицы: " << table->size() << "\n";

        bool iterationCorrect = (count == table->size());

        std::cout << "Тест " << (iterationCorrect ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";
    }

    // Тест 8: Очистка таблицы
    void testClear() {
        std::cout << "\n=== Тест 8: Очистка таблицы ===\n";
        std::cout << "Ожидается: Таблица очищена, размер = 0\n";

        int sizeBefore = table->size();
        std::cout << "Размер до очистки: " << sizeBefore << "\n";

        table->clear();

        int sizeAfter = table->size();
        bool isEmpty = table->empty();

        std::cout << "Размер после очистки: " << sizeAfter << "\n";
        std::cout << "Таблица пуста: " << (isEmpty ? "да" : "нет") << "\n";

        // Проверяем, что все элементы удалены
        size_t valueSize;
        std::string key = "имя";
        void* value = table->at((void*)key.c_str(), key.size() + 1, valueSize);
        std::cout << "Попытка получить элемент 'имя': ";
        printValue(value, valueSize);
        std::cout << "\n";

        bool clearSuccess = (sizeAfter == 0 && isEmpty && value == nullptr);

        std::cout << "Тест " << (clearSuccess ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";
    }

    // Тест 9: Стресс-тест (много элементов)
    void testStress() {
        std::cout << "\n=== Тест 9: Стресс-тест (много элементов) ===\n";
        std::cout << "Ожидается: Успешная вставка большого количества элементов и автоматическое перехеширование\n";

        const int numElements = 1000;
        std::cout << "Вставка " << numElements << " элементов...\n";

        // Очищаем таблицу перед тестом
        table->clear();

        int successCount = 0;
        for (int i = 0; i < numElements; i++) {
            std::string key = "key" + std::to_string(i);
            std::string value = "value" + std::to_string(i);

            int result = table->insertByKey(
                (void*)key.c_str(), key.size() + 1,
                (void*)value.c_str(), value.size() + 1
            );

            if (result == 0) {
                successCount++;
            }
        }

        int actualSize = table->size();

        std::cout << "Успешно вставлено элементов: " << successCount << "/" << numElements << "\n";
        std::cout << "Размер таблицы: " << actualSize << "\n";

        // Проверяем несколько случайных элементов
        int checkCount = 5;
        std::cout << "Проверка " << checkCount << " случайных элементов:\n";

        int correctValues = 0;
        for (int j = 0; j < checkCount; j++) {
            int index = j * (numElements / checkCount);
            std::string key = "key" + std::to_string(index);
            std::string expectedValue = "value" + std::to_string(index);

            size_t valueSize;
            void* value = table->at((void*)key.c_str(), key.size() + 1, valueSize);
            std::string actualValue = valueToString(value, valueSize);

            std::cout << "Ключ '" << key << "': ожидалось '" << expectedValue
                << "', получено '" << actualValue << "' - "
                << (actualValue == expectedValue ? "верно" : "ошибка") << "\n";

            if (actualValue == expectedValue) {
                correctValues++;
            }
        }

        bool stressSuccess = (successCount == numElements &&
            actualSize == numElements &&
            correctValues == checkCount);

        std::cout << "Тест " << (stressSuccess ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";

        // Очищаем таблицу после теста
        table->clear();
    }

    // Тест 10: Поиск элемента по значению
    void testFindByValue() {
        std::cout << "\n=== Тест 10: Поиск элемента по значению ===\n";
        std::cout << "Ожидается: Элемент успешно найден по значению\n";

        // Сначала добавим несколько элементов
        table->clear();

        std::string key1 = "имя";
        std::string value1 = "Иван";
        std::string key2 = "фамилия";
        std::string value2 = "Иванов";
        std::string key3 = "возраст";
        std::string value3 = "25";

        table->insertByKey(
            (void*)key1.c_str(), key1.size() + 1,
            (void*)value1.c_str(), value1.size() + 1
        );

        table->insertByKey(
            (void*)key2.c_str(), key2.size() + 1,
            (void*)value2.c_str(), value2.size() + 1
        );

        table->insertByKey(
            (void*)key3.c_str(), key3.size() + 1,
            (void*)value3.c_str(), value3.size() + 1
        );

        // Теперь ищем элемент по значению
        std::string searchValue = "Иванов";
        Container::Iterator* iter = table->find(
            (void*)searchValue.c_str(), searchValue.size() + 1
        );

        bool found = (iter != nullptr);
        std::cout << "Поиск значения '" << searchValue << "': "
            << (found ? "найдено" : "не найдено") << "\n";

        if (found) {
            // Получаем значение из итератора и проверяем, совпадает ли оно с искомым
            size_t valueSize;
            void* foundValue = iter->getElement(valueSize);
            std::string foundValueStr = valueToString(foundValue, valueSize);

            std::cout << "Найденное значение: '" << foundValueStr << "'\n";

            bool valueCorrect = (foundValueStr == searchValue);
            std::cout << "Тест " << (valueCorrect ? "УСПЕШЕН" : "ПРОВАЛЕН") << "\n";

            delete iter;
        }
        else {
            std::cout << "Тест ПРОВАЛЕН\n";
        }
    }

    // Запуск всех тестов
    void runAllTests() {
        std::cout << "======= НАЧАЛО ТЕСТИРОВАНИЯ АССОЦИАТИВНОЙ ТАБЛИЦЫ =======\n";
        testEmptyTable();
        testInsert();
        testRetrieve();
        testNonExistentKey();
        testDuplicateKey();
        testRemove();
        testIteration();
        testClear();
        testStress();
        testFindByValue();
        std::cout << "======= ОКОНЧАНИЕ ТЕСТИРОВАНИЯ АССОЦИАТИВНОЙ ТАБЛИЦЫ =======\n";
    }
};

int main() {
    setlocale(LC_ALL, "Russian");

    std::cout << "=== Тестирование класса Table (Ассоциативная таблица) ===\n";

    // Создаем тестировщик с 2МБ памяти
    TableTester tester(2 * 1024 * 1024);

    // Запускаем все тесты
    tester.runAllTests();

    std::cout << "\nТестирование завершено.\n";

    return 0;
}