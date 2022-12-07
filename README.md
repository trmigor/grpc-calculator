# Сервис-калькулятор
Простое клиент-серверное приложение для изучения основ gRPC.
## Установка gRPC
Идем в терминал. Определяем путь для установки (gRPC рекомендуется устанавливать локально, так как после установки глобально его будет крайне трудно удалить):
```sh
export MY_INSTALL_DIR=$HOME/.local
```
Удостоверимся, что эта директория существует, если нет — создаем:
```sh
mkdir -p $MY_INSTALL_DIR
```
Добавим эту директорию в переменную окружения `PATH`:
```sh
export PATH="$MY_INSTALL_DIR/bin:$PATH"
```
### Установка CMake
* Linux
	```sh
	sudo apt install -y cmake
	```
* macOS
	```sh
	brew install cmake
	```
Необходима версия минимум 3.13. Проверим версию:
```sh
cmake --version
```
### Установка зависимостей
* Linux
	```sh
	sudo apt install -y build-essential autoconf libtool pkg-config
	```
* macOS
	```sh
	brew install autoconf automake libtool pkg-config
	```
### Клонирование репозитория
Клонируем репозиторий с исходным кодом gRPC:
```sh
git clone --recurse-submodules -b v1.50.0 --depth 1 --shallow-submodules https://github.com/grpc/grpc
```
### Сборка и установка
Собираем и устанавливаем gRPC и Protocol Buffers:
```sh
cd grpc
mkdir -p cmake/build
pushd cmake/build
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=$MY_INSTALL_DIR \
      ../..
make -j 4
make install
popd
```
### Проверка
Откроем один из примеров:
```sh
cd examples/cpp/helloworld
```
Соберем его с помощью CMake:
```sh
mkdir -p cmake/build
pushd cmake/build
cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..
make -j 4
```
Запустим сервер:
```sh
./greeter_server
```
В отдельном терминале запустим клиента:
```sh
./greeter_client
```
В результате мы должны получить сообщение: `Greeter received: Hello world`.

## Напишем теперь свой сервис
Подготовим рабочее пространство:
```sh
cd
mkdir calculator
cd calculator
mkdir proto src build
```
### Описание сервиса
Создадим описание нашего сервиса в формате Protocol Buffers:
```sh
vi proto/calculator.proto
```
```protobuf
// Определим версию синтаксиса
syntax = "proto3";

// Обозначим пакет и соответствующее пространство
package calculator;

// Опишем наш сервис:
service Calculator {
    // В нем будет всего один метод
    // Он принимает запрос, считает и возвращает ответ
    rpc Calculate(Request) returns (Response) {};
};

// Опишем принимаемый запрос
message Request {
    // Обозначим доступные операции над числами
    enum OperationType {
        UNSPECIFIED = 0;    // Неизвестная/отсутствующая операция
        ADD = 1;            // Сложение
        SUBSTRACT = 2;      // Вычитание
        MULTIPLY = 3;       // Умножение
        DIVIDE = 4;         // Целочисленное деление
        MOD = 5;            // Остаток от деления
    };

    // Из каких полей состоит запрос:
    optional OperationType operation_type = 1;  // Тип операции
    optional int32 first = 2;                   // Первый аргумент
    optional int32 second = 3;                  // Второй аргумент
};

// Опишем возвращаемый ответ
message Response {
    // Из каких полей состоит ответ:
    int32 result = 1;   // Результат операции
};
```
### Сервер
Теперь создадим сервер:
```sh
vi src/server.cpp
```
```cpp
// Добавим наш пакет с описанием сервиса
// Этот файл будет сгенерирован во время сборки на основе calculator.proto
#include <calculator.grpc.pb.h>
// Подключим основные возможности gRPC
#include <grpc++/grpc++.h>
// Используемые системные библиотеки
#include <memory>
#include <iostream>

// Обычно так делать не хорошо
// Но в этот раз можно, для простоты
using namespace grpc;
using namespace calculator;

// Определим реализацию нашего сервиса
// Этот класс должен публично наследовать Calculator::Service
// Calculator::Service сгенерирован по calculator.proto
class CalculatorServiceImpl : public Calculator::Service {
public:
    // Единственный метод нашего сервиса
    // Он возвращает grpc::Status, показывающий успешность запроса
    // Как аргумент он принимает:
    // context - серверный контекст, мы его не используем
    // request - собственно, пришедший запрос
    // response - плейсхолдер для записи результата
    Status Calculate(ServerContext* context, const Request* request, Response* response) {
        // Если в запросе отсутствует тип операции, возвращаем ошибку
        // Для проверки наличия поля используем метод has_<название поля>()
        if (!request->has_operation_type()) {
            return Status(INVALID_ARGUMENT, "No operation specified");
        }

        // Если отсутствует хотя бы один из аргументоа, возвращаем ошибку
        if (!request->has_first() || !request->has_second()) {
            return Status(INVALID_ARGUMENT, "Absent argument");
        }

        // Выполним требуемую операцию
        // Для чтения поля используем метод <название поля>()
        switch (request->operation_type()) {
        // Сложение
        case Request::ADD:
            // Для установки поля используем метод set_<название поля>(<значение>)
            response->set_result(request->first() + request->second());
            return Status::OK;

        // Вычитание
        case Request::SUBSTRACT:
            response->set_result(request->first() - request->second());
            return Status::OK;

        // Умножение
        case Request::MULTIPLY:
            response->set_result(request->first() * request->second());
            return Status::OK;

        // Целочисленное деление
        case Request::DIVIDE:
            // Если делим на ноль, возвращаем ошибку
            if (request->second() == 0) {
                return Status(INVALID_ARGUMENT, "Division by zero");
            }
            response->set_result(request->first() / request->second());
            return Status::OK;

        // Остаток от деления
        case Request::MOD:
            // Если делим на ноль, возвращаем ошибку
            if (request->second() == 0) {
                return Status(INVALID_ARGUMENT, "Division by zero");
            }
            response->set_result(request->first() % request->second());
            return Status::OK;

        // Если запрошена операция типа UNSPECIFIED, отменяем запрос
        default:
            return Status(CANCELLED, "No operation specified");
        }
    }
};

// Функция для запуска сервера
void RunServer() {
    // Адрес и порт сервера
    std::string server_address = "localhost:2510";
    // Объект реализации нашего сервиса
    CalculatorServiceImpl service;

    // Создадим сервер с помощью билдера (см. раздел README "Что почитать")
    ServerBuilder builder;
    // Установим адрес и порт сервера
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Зарегистрируем наш сервис 
    builder.RegisterService(&service);
    // Построим и запустим сервер
    std::unique_ptr<Server> server(builder.BuildAndStart());

    std::cout << "Server listening on " << server_address << std::endl;
    // Дожидаемся завершения работы сервера
    server->Wait();
}

int main() {
    RunServer();
    return 0;
}
```
### Клиент
Теперь создадим клиента:
```sh
vi src/server.cpp
```
```cpp
// Тут все так же:
#include <calculator.grpc.pb.h>
#include <grpc++/grpc++.h>
#include <memory>
#include <iostream>

using namespace grpc;
using namespace calculator;

// Класс клиента
class Client {
public:
    // Конструктор принимает на вход grpc::Channel в обертке из std::shared_ptr
    // grpc::Channel - канал для передачи сообщений gRPC
    // Создает stub ("огрызок")
    // По сути stub - это обертка для передачи сообщений
    // stub притворяется обычным объектом, который сам выполняет все методы
    // На самом деле пересылает их сервису
    // Потому и огрызок :)
    Client(std::shared_ptr<Channel> channel) : stub(Calculator::NewStub(channel)) {}

    // Подготавливает запрос, отсылает его и принимает ответ
    int Calculate(int first, int second, char operation) {
        // Создадим запрос
        Request request;
        // Поля устанавливаются точно так же
        // Метод set_<название поля>(<значение>)
        request.set_first(first);
        request.set_second(second);

        // Выберем подходящую операцию
        switch (operation) {
        case '+':
            request.set_operation_type(Request::ADD);
            break;
        case '-':
            request.set_operation_type(Request::SUBSTRACT);
            break;
        case '*':
            request.set_operation_type(Request::MULTIPLY);
            break;
        case '/':
            request.set_operation_type(Request::DIVIDE);
            break;
        case '%':
            request.set_operation_type(Request::MOD);
            break;
        default:
            request.set_operation_type(Request::UNSPECIFIED);
            break;
        }

        // Подготовим плейсхолдер для ответа
        Response response;
        // Клиентский контекст, в данной ситуации он нам особо не нужен
        ClientContext context;
        // Посылаем сообщение и получаем ответ
        // Очень похоже на наш метод CalculatorServiceImpl::Calculate
        // Только вместо серверного контекста идет клиентский
        Status status = stub->Calculate(&context, request, &response);

        // Проверяем статус
        if (status.ok()) {
            // Если все хорошо, возвращаем результат
            return response.result();
        } else {
            // Если пришла ошибка, бросаем исключение
            throw std::runtime_error(std::to_string(status.error_code()) + ": " + status.error_message());
        }
    }

private:
    // stub в обертке из unique_ptr
    std::unique_ptr<calculator::Calculator::Stub> stub;
};

int main() {
    // Адрес и порт сервера
    std::string server_address = "localhost:2510";
    // Создаем канал без аутентификации и прочих защитных мер
    // По нему создаем нашего клиента
    Client client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    // Тестируем:
    int first = 10;
    int second = 2;

    std::cout << client.Calculate(first, second, '+') << std::endl;
    std::cout << client.Calculate(first, second, '-') << std::endl;
    std::cout << client.Calculate(first, second, '*') << std::endl;
    std::cout << client.Calculate(first, second, '/') << std::endl;
    std::cout << client.Calculate(first, second, '%') << std::endl;

    std::cout << client.Calculate(first, 0, '/') << std::endl;

    return 0;
}
```
### CMake
Тут немножко магии CMake, ибо если обсуждать, как он работает, нужен еще один воркшоп.
В файле `./CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.13) # Обозначим минимальную версию CMake
project(calculator LANGUAGES CXX) # Название и языки проекта

set(CMAKE_CXX_STANDARD 14) # Стандарт C++

# Поддиректории со своими CMakeLists.txt
add_subdirectory(proto)
add_subdirectory(src)
```
В файле `proto/CMakeLists.txt`:
```cmake
# Подключаем необходимые пакеты
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

# Обозначаем файл с описанием
set(PROTO_FILES
    calculator.proto
)

# Создаем библиотеку и определяем связи с другими библиотеками
add_library(proto ${PROTO_FILES})
target_link_libraries(proto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Генерируем заголовочные и исходные файлы по описанию сервиса
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET proto LANGUAGE cpp)
protobuf_generate(TARGET proto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
```
В файле `src/CMakeLists.txt`:
```cmake
# Подключаем необходимые пакеты
find_package(Threads)

# Обозначаем группу исходных файлов
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

# Добавляем файлы
add_executable(client client.cpp)
add_executable(server server.cpp)

# Подключаем нашу библиотеку
target_link_libraries(client
    PRIVATE
        proto
)
target_link_libraries(server
    PRIVATE
        proto
)
```
### Сборка
Соберем проект.
```sh
cd build
cmake ..
make -j 4
```
### Запуск
Запустим сервер:
```sh
src/server
```
В отдельном терминале запустим клиента:
```sh
src/client
```
Если все сделано правильно, клиент должен вывести что-то такое:
```
12
8
20
5
0
libc++abi: terminating with uncaught exception of type std::runtime_error: 3: Division by zero
[1]    5843 abort      src/client
```
## Что почитать дальше
* [Документация gRPC](https://grpc.io/docs/)
* [Быстрый старт с Protocol Buffers](https://developers.google.com/protocol-buffers/docs/overview)
* [Документация Protocol Buffers](https://developers.google.com/protocol-buffers/docs/reference/overview)
* [Документация CMake](https://cmake.org/documentation/)
* [Шаблон проектирования "Строитель" (builder, билдер)](https://ru.wikipedia.org/wiki/%D0%A1%D1%82%D1%80%D0%BE%D0%B8%D1%82%D0%B5%D0%BB%D1%8C_(%D1%88%D0%B0%D0%B1%D0%BB%D0%BE%D0%BD_%D0%BF%D1%80%D0%BE%D0%B5%D0%BA%D1%82%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D1%8F))
* [Умные указатели](https://learn.microsoft.com/en-us/cpp/cpp/smart-pointers-modern-cpp?view=msvc-170)
	* [std::shared_ptr](https://en.cppreference.com/w/cpp/memory/shared_ptr)
	* [std::unique_ptr](https://en.cppreference.com/w/cpp/memory/unique_ptr)

## С чем поиграться
Чтобы посмотреть на сгенерированный из `calculator.proto` код на разных языках, используйте компилятор Protocol Buffers, который установился вместе с gRPC:
```sh
protoc --cpp_out=. --java_out=. --python_out=. --csharp_out=. --kotlin_out=. --objc_out=. --php_out=. --ruby_out=. calculator.proto
```

