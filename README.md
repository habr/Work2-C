2) В программу добавить 1 поток (всего будет 2 потока включая 1 главный
поток).
В потоке должен быть свой event loop. Перенести TCP сокет в этот поток.
Приняв пакет, нужно переслать его на обработку в главный поток.
Главный поток должен что-то сделать с пакетом, например перевернуть все
данные в нем задом наперед.
После этого пакет передается обратно в поток и там отсылается обратно
клиенту.

Компиляция этого кода выполняется таким образом:
gcc -pthread -o main2 main2.c

Запуск производится следующим образом:
./main2 8080

Проверка работы с помощью telnet:
telnet localhost 8080
