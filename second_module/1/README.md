# Broadcast local chat

### Задание
Напишите простой консольный клиент чата, позволяющий обмениваться текстовыми сообщениями с такими же клиентами в локальной сети. Для передачи сообщений используйте широковещательные UDP дейтаграммы. Порт укажите в аргументе командной строки.

### Примечание
На маке работает не так, как ожидается, на маке лучше через docker =(

---

# Запуск

## Локально

0. Подготовка утилит
- \[MacOs\] brew install gcc
- \[Linux\] sudo apt-get update && sudo apt-get install gcc make
1. make build
2. make run

## Docker

1. `make docker-network`
2. `make docker`
3. `make docker-run` (запускаем 2 раза, чтобы посмотреть на работу чата)
4. `make run` (В каждом контейнере)
5. Пробуем писать сообщения там и там