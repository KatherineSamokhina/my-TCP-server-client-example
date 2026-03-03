# Key-Value Server & Client

Два приложения (сервер на C++, клиент на Python), взаимодействующие по TCP.

## Требования

- **Сервер:** C++17, CMake 3.14+, Boost 1.70+ (header-only, Asio)
- **Клиент:** Python 3.7+

## Сборка сервера

```bash
cd server
mkdir build
cd build
cmake .. -DBOOST_ROOT=/path/to/boost
cmake --build .
```

На Windows (если Boost в `C:\path\to\boost`):

```powershell
cd server
mkdir build
cd build
cmake .. -DBOOST_ROOT="C:\path\to\boost"
cmake --build .
```

Исполняемый файл: `build/Debug/server.exe` (Windows) или `build/server` (Linux).

Файлы `config.txt` и `server_config.txt` копируются в папку с exe при сборке.

## Тестовые скрипты

### run_reconnect_test.py

Тест работоспособности и переподключения (Windows, отдельные окна консоли):

1. Запуск сервера
2. Запуск 10 клиентов
3. Через 5 сек — ещё один клиент и остановка сервера
4. Через 1 сек — перезапуск сервера

Клиенты продолжают работу (переподключение при обрыве). Окна остаются открытыми для ручного закрытия.

```bash
python run_reconnect_test.py
```

## Запуск

### 1. Сервер

```bash
cd server/build/Debug   # или server/build на Linux
./server.exe            # или ./server
```

Сервер читает `config.txt` и `server_config.txt` из текущей директории. Остановка — Ctrl+C.

### 2. Клиент

```bash
python client/client.py
python client/client.py --host 127.0.0.1 --port 12345
python client/client.py --help
```

## Конфигурация

### server_config.txt

```
host=0.0.0.0
port=12345
```

### config.txt (хранилище key-value)

```
tree=Blue
sky=Gray
water=Green
```

## Протокол

**Команды (клиент → сервер):**
```
$get <key>
$set <key>=<value>
```

**Ответы (сервер → клиент):** три строки:
```
<результат>        # key=value для get, OK для set, ERROR=... при ошибке
reads=<N>
writes=<M>
```