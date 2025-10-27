Сборка
cc fifo_echo.c -o fifo_echo

Запуск
./fifo_echo [OPTIONS]

Флаги
-d - daemon режим (в случае отсутствия запускается как foreground)
-f - путь к fifo каналу
-l - путь к файлу лога (для daemon режима)
-i - интервал для alarm

Примеры:

1. Базовый запуск, проверка heartbeat сообщений и foreground режима
./fifo_echo
echo "Hello world" > /tmp/echo_fifo

Ожидание: вывод в консоли строки Hello world, heartbeat сообщения по расписанию.
Логи:
[2025-10-27 09:04:02] starting echo server; fifo='/tmp/echo_fifo', interval=5, mode=foreground
[2025-10-27 09:04:07] heartbeat: waiting for writers...
[2025-10-27 09:04:12] heartbeat: waiting for writers...
Hello world

2. Проверка сигналов
./fifo_echo
Нажатие Ctrl + c и Ctrl + \

Ожидание: вывод в консоли heartbeat сообщений 
Логи: 
[2025-10-27 09:05:37] starting echo server; fifo='/tmp/echo_fifo', interval=5, mode=foreground
[2025-10-27 09:05:42] heartbeat: waiting for writers...
[2025-10-27 09:05:47] heartbeat: waiting for writers...
[2025-10-27 09:05:52] heartbeat: waiting for writers...
[2025-10-27 09:05:57] heartbeat: waiting for writers...

3. Проверка TERM сигнала
./fifo_echo
pgrep fifo_echo
kill -TERM <PID>

Ожидание: завершение работы сервера с информированием в логе.
Логи:
[2025-10-27 09:07:33] starting echo server; fifo='/tmp/echo_fifo', interval=5, mode=foreground
[2025-10-27 09:07:38] heartbeat: waiting for writers...
[2025-10-27 09:07:43] heartbeat: waiting for writers...
[2025-10-27 09:07:48] terminating on signal 15

4. Проверка daemon режима
./fifo_echo -d
echo "Hello world" > /tmp/echo_fifo
pgrep fifo_echo
kill -TERM <PID>
tail -n 100 /tmp/echo_fifo.log

Ожидание: вывод heartbeat сообщений в файле лога, строки Hello world, завершение работы сервера.
Логи: 
[2025-10-27 09:09:45] starting echo server; fifo='/tmp/echo_fifo', interval=5, mode=foreground
[2025-10-27 09:09:50] heartbeat: waiting for writers...
[2025-10-27 09:09:55] heartbeat: waiting for writers...
Hello world
[2025-10-27 09:10:00] terminating on signal 15
