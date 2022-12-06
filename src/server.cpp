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

    // Создадим сервер с помощью билдера
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
