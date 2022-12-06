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
