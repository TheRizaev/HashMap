#include <iostream>
#include <string>
#include "Table.h"
#include "Mem.h"
#include <locale.h>

void printValue(void* value, size_t size) {
    if (value) {
        char* str = static_cast<char*>(value);
        std::cout << std::string(str, size - 1); // Вычитаем 1, чтобы не печатать нулевой символ
    }
    else {
        std::cout << "nullptr";
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    // Создаем менеджер памяти
    Mem memory(1024 * 1024); // 1MB памяти

    // Создаем таблицу
    Table table(memory);

    std::cout << "Создана пустая таблица. Размер: " << table.size() << std::endl;
    std::cout << "Таблица пуста: " << (table.empty() ? "да" : "нет") << std::endl;

    // Добавляем элементы в таблицу
    std::string key1 = "имя";
    std::string value1 = "Иван";
    std::string key2 = "фамилия";
    std::string value2 = "Иванов";
    std::string key3 = "возраст";
    std::string value3 = "25";

    std::cout << "\nДобавляем элементы в таблицу...\n";

    int result = table.insertByKey(
        (void*)key1.c_str(), key1.size() + 1,
        (void*)value1.c_str(), value1.size() + 1
    );
    std::cout << "Добавление '" << key1 << "': "
        << (result == 0 ? "успешно" : "ошибка") << std::endl;

    result = table.insertByKey(
        (void*)key2.c_str(), key2.size() + 1,
        (void*)value2.c_str(), value2.size() + 1
    );
    std::cout << "Добавление '" << key2 << "': "
        << (result == 0 ? "успешно" : "ошибка") << std::endl;

    result = table.insertByKey(
        (void*)key3.c_str(), key3.size() + 1,
        (void*)value3.c_str(), value3.size() + 1
    );
    std::cout << "Добавление '" << key3 << "': "
        << (result == 0 ? "успешно" : "ошибка") << std::endl;

    std::cout << "\nПосле добавления элементов. Размер: " << table.size() << std::endl;
    std::cout << "Таблица пуста: " << (table.empty() ? "да" : "нет") << std::endl;

    // Получаем элементы по ключу
    std::cout << "\nПолучаем элементы по ключу:\n";

    size_t valueSize;
    void* value;

    value = table.at((void*)key1.c_str(), key1.size() + 1, valueSize);
    std::cout << key1 << ": ";
    printValue(value, valueSize);
    std::cout << std::endl;

    value = table.at((void*)key2.c_str(), key2.size() + 1, valueSize);
    std::cout << key2 << ": ";
    printValue(value, valueSize);
    std::cout << std::endl;

    value = table.at((void*)key3.c_str(), key3.size() + 1, valueSize);
    std::cout << key3 << ": ";
    printValue(value, valueSize);
    std::cout << std::endl;

    // Пробуем получить несуществующий ключ
    std::string nonExistentKey = "город";
    value = table.at((void*)nonExistentKey.c_str(), nonExistentKey.size() + 1, valueSize);
    std::cout << nonExistentKey << ": ";
    printValue(value, valueSize);
    std::cout << std::endl;

    // Пробуем добавить элемент с уже существующим ключом
    std::cout << "\nПробуем добавить элемент с существующим ключом:\n";
    std::string newValue = "Петр";
    result = table.insertByKey(
        (void*)key1.c_str(), key1.size() + 1,
        (void*)newValue.c_str(), newValue.size() + 1
    );
    std::cout << "Добавление '" << key1 << "': "
        << (result == 0 ? "успешно" : "ошибка (ключ уже существует)") << std::endl;

    // Удаляем элемент
    std::cout << "\nУдаляем элемент с ключом '" << key2 << "'\n";
    table.removeByKey((void*)key2.c_str(), key2.size() + 1);
    std::cout << "После удаления. Размер: " << table.size() << std::endl;

    // Проверяем, что элемент удален
    size_t checkValueSize = 0;
    void* checkValue = table.at((void*)key2.c_str(), key2.size() + 1, checkValueSize);
    std::cout << key2 << ": ";
    printValue(checkValue, checkValueSize);
    std::cout << std::endl;

    // Перебираем все элементы с помощью итератора
    std::cout << "\nПеребираем все элементы таблицы:\n";
    Container::Iterator* iter = table.newIterator();
    if (iter) {
        int index = 0;
        do {
            size_t elemSize;
            void* elemValue = iter->getElement(elemSize);
            if (elemValue == nullptr) break;

            std::cout << "Элемент " << index++ << ": ";
            printValue(elemValue, elemSize);
            std::cout << std::endl;

            if (!iter->hasNext()) break;
            iter->goToNext();
        } while (true);

        delete iter;
    }

    // Очищаем таблицу
    std::cout << "\nОчищаем таблицу\n";
    table.clear();
    std::cout << "После очистки. Размер: " << table.size() << std::endl;
    std::cout << "Таблица пуста: " << (table.empty() ? "да" : "нет") << std::endl;

    return 0;
}