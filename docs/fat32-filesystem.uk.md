# FAT32 RAM-диск UnityKernel і файлові команди оболонки

Цей документ пояснює FAT32-зміни в UnityKernel: нову абстракцію блокового пристрою, RAM-диск із FAT32-подібною структурою, API файлової системи та Unix-подібні команди оболонки для роботи з файлами й директоріями.

Поточна реалізація навмисно невелика й зручна для ядра. Вона дає UnityKernel реальні файлові й директорійні семантики без потреби одразу мати драйвер ATA, AHCI, NVMe, USB або virtio. Сховище є тимчасовим і знаходиться в пам'яті ядра, але внутрішня організація повторює основні ідеї FAT32: boot sector, FAT-таблицю, кластери, короткі 8.3 directory entries, ланцюжки кластерів і обхід директорій.

## Що Було Додано

Додано чотири нові файли:

- `include/kernel/block.hpp`
- `portable/kernel/cxx/block.cpp`
- `include/kernel/fat32.hpp`
- `portable/kernel/cxx/fat32.cpp`

Також змінено:

- `portable/kernel/CMakeLists.txt`
- `portable/kernel/cxx/kernel.cpp`
- `portable/kernel/cxx/shell.cpp`

Загальна картина:

- `Kernel::Block::MemoryDevice` дає sector-based read/write доступ до буфера пам'яті.
- `Kernel::FAT32` створює й керує невеликим RAM-диском у FAT32-стилі.
- `kernelMain()` монтує RAM-диск під час старту ядра.
- Оболонка отримала файлові команди: `ls`, `cd`, `cat`, `touch`, `write`, `append`, `mkdir`, `rm`, `stat`.

## Команди Для Користувача

Оболонка тепер підтримує такі файлові команди:

| Команда | Призначення |
| --- | --- |
| `pwd` | Показати поточну робочу директорію. |
| `ls [path]` | Показати вміст директорії або один файловий запис. |
| `cd <path>` | Змінити поточну директорію. |
| `cat <file>` | Вивести вміст файлу. |
| `touch <file>` | Створити порожній файл, якщо його ще немає. |
| `write <file> <text>` | Замінити вміст файлу текстом. Якщо файлу немає, він створюється. |
| `append <file> <text>` | Додати текст у кінець файлу. Якщо файлу немає, він створюється. |
| `mkdir <dir>` | Створити директорію. |
| `rm <path>` | Видалити файл або порожню директорію. |
| `stat <path>` | Показати базову метаінформацію: тип, ім'я, розмір і перший кластер. |

Prompt тепер містить поточну директорію:

```text
/ $
```

Якщо виконати `cd tmp`, prompt стане:

```text
/tmp $
```

## Приклад Сесії В Оболонці

Після старту файлова система містить:

- `/welcome.txt`
- `/tmp`

Приклад:

```text
/ $ ls
- 37 welcome.txt
d 0 tmp
/ $ cat welcome.txt
UnityKernel FAT32 ram disk is online.
/ $ mkdir docs
/ $ cd docs
/docs $ write note.txt hello from UnityKernel
/docs $ cat note.txt
hello from UnityKernel
/docs $ append note.txt  again
/docs $ cat note.txt
hello from UnityKernel again
/docs $ stat note.txt
file note.txt
size: 28
cluster: 5
/docs $ cd ..
/ $ rm docs
rm: docs: directory not empty
/ $ rm docs/note.txt
/ $ rm docs
```

`write` і `append` сприймають усе після шляху до файлу як текст. Цитування ще не реалізоване, тому імена файлів не можуть містити пробіли, але вміст файлу може.

## Послідовність Старту

Важлива частина старту така:

1. `kernelMain()` очищає консоль.
2. Пам'ять ініціалізується через `Memory::initialize()`.
3. C++ runtime-конструктори запускаються через `CXXRuntime::initialize()`.
4. Архітектурна ініціалізація виконується через `Arch::initialize()`.
5. `FAT32::initialize()` створює й монтує RAM-backed FAT32 volume.
6. Ядро друкує `Mounted FAT32 ram disk`.
7. Запускається інтерактивна оболонка.

Файлова система готова ще до появи shell prompt.

## Блоковий Шар

Блоковий шар зараз дуже маленький:

```cpp
namespace Kernel::Block {
    constexpr uint32_t sectorSize = 512;

    class MemoryDevice {
    public:
        bool initialize(uint8_t* storage, uint32_t sectorCount);
        bool read(uint32_t lba, void* buffer, uint32_t count = 1) const;
        bool write(uint32_t lba, const void* buffer, uint32_t count = 1);
        uint32_t sectors() const;
    };
}
```

`MemoryDevice` поводиться як простий дисковий пристрій:

- Увесь доступ sector-based.
- Розмір сектора фіксований: 512 байтів.
- Read/write використовують LBA-номери секторів.
- Межі перевіряються перед копіюванням.
- Фактичне сховище - це raw `uint8_t*` буфер, яким володіє реалізація файлової системи.

Цей інтерфейс спеціально схожий на реальний block device, щоб пізніше RAM-бекенд можна було замінити на драйвер диска. Наприклад, майбутній ATA-драйвер може мати такий самий стиль `read`/`write`, але працювати з реальними секторами диска.

## Геометрія FAT32 Volume

Поточний RAM-диск має фіксовану compile-time геометрію:

| Поле | Значення |
| --- | --- |
| Розмір сектора | 512 байтів |
| Секторів на кластер | 1 |
| Розмір кластера | 512 байтів |
| Reserved sectors | 32 |
| FAT sectors | 2 |
| Кількість FAT у BPB | 1 |
| Data clusters | 128 |
| Root cluster | 2 |
| Total sectors | 162 |
| Total disk size | 82,944 байти |

Загальна кількість секторів:

```text
reservedSectors + fatSectors + clusterCount * sectorsPerCluster
= 32 + 2 + 128 * 1
= 162 sectors
```

Загальний розмір у байтах:

```text
162 * 512 = 82,944 bytes
```

Це невеликий обсяг, але його достатньо для базового тестування файлів і директорій через оболонку.

## Внутрішня Розмітка RAM-диска

Хоча сховище знаходиться в пам'яті, воно організоване як disk image:

```text
+------------------------------+
| Reserved region              |
| - sector 0: boot sector      |
| - інші reserved sectors      |
+------------------------------+
| FAT region                   |
| - 2 sectors                  |
+------------------------------+
| Data region                  |
| - cluster 2: root directory  |
| - cluster 3..: files/dirs    |
+------------------------------+
```

Перетворення cluster number у LBA:

```text
LBA = reservedSectors + fatSectors + (cluster - rootCluster) * sectorsPerCluster
```

З поточними константами:

```text
LBA = 32 + 2 + (cluster - 2)
```

Тобто:

- cluster 2 відповідає sector 34
- cluster 3 відповідає sector 35
- cluster 4 відповідає sector 36

## Boot Sector

`writeBootSector()` створює FAT32-style BIOS Parameter Block у секторі 0.

Основні поля:

- jump instruction
- OEM name: `UNITYKRN`
- bytes per sector: `512`
- sectors per cluster: `1`
- reserved sector count: `32`
- FAT count: `1`
- total sectors: `162`
- FAT size: `2`
- root cluster: `2`
- volume serial number
- volume label: `UNITYKERNEL`
- filesystem type text: `FAT32`
- boot signature: `0x55AA`

Цей boot sector потрібен, щоб структура volume була FAT32-подібною. Наразі ядро не читає зовнішній FAT32 boot sector; воно створює відому in-memory структуру.

## FAT-Таблиця

FAT-таблиця зберігає один 32-bit entry на кластер. Використовуються лише молодші 28 бітів, як у FAT32.

Спеціальні значення:

| Значення | Сенс |
| --- | --- |
| `0` | Вільний кластер |
| `0x0FFFFFF8` і більше | Кінець ланцюжка кластерів |
| Номер кластера | Наступний кластер у ланцюжку файлу або директорії |

Під час ініціалізації:

- FAT entry 0 позначається reserved.
- FAT entry 1 позначається reserved.
- FAT entry 2 позначається end-of-chain для root directory.

Виділення кластера сканує FAT від cluster 3 угору. Коли знаходиться вільний кластер:

1. Його FAT entry стає `endOfChain`.
2. Кластер зануляється.
3. Номер кластера повертається виклику.

## Directory Entries

Директорії зберігаються як кластери з 32-байтними FAT directory entries.

Структура:

```cpp
struct DirectoryEntry {
    char name[11];
    uint8_t attr;
    uint8_t ntReserved;
    uint8_t creationTenths;
    uint16_t creationTime;
    uint16_t creationDate;
    uint16_t accessDate;
    uint16_t firstClusterHigh;
    uint16_t writeTime;
    uint16_t writeDate;
    uint16_t firstClusterLow;
    uint32_t fileSize;
} __attribute__((packed));
```

Один 512-байтний кластер містить:

```text
512 / 32 = 16 directory entries
```

Реалізація пропускає:

- видалені entries, де перший байт `0xE5`
- long-file-name entries
- `.` і `..` під час звичайного `ls`

Директорії, створені через `mkdir`, отримують:

- `.` із посиланням на саму директорію
- `..` із посиланням на parent directory

## Імена Файлів

Поточна реалізація підтримує тільки класичні FAT short names.

Форма:

```text
BASE.EXT
```

Правила:

- Base name: від 1 до 8 символів.
- Extension: від 0 до 3 символів.
- Імена зберігаються uppercase у directory entries.
- Імена показуються lowercase в оболонці.
- `.` і `..` зарезервовані.
- Long file names не підтримуються.
- Пробіли в іменах файлів не підтримуються.

Дозволені символи:

- `A-Z`
- `a-z`
- `0-9`
- `_`
- `-`
- `$`
- `~`

Приклади:

| Input | Stored short name | Displayed |
| --- | --- | --- |
| `note.txt` | `NOTE    TXT` | `note.txt` |
| `KERNEL.BIN` | `KERNEL  BIN` | `kernel.bin` |
| `tmp` | `TMP        ` | `tmp` |

Некоректні приклади:

- `verylongname.txt`
- `archive.tar.gz`
- `.hidden`
- `my file.txt`

## Обробка Шляхів

Файлова система підтримує absolute і relative paths.

Приклади:

```text
/welcome.txt
tmp/file.txt
./note.txt
../other.txt
```

Path resolution відбувається покомпонентно:

1. Обирається стартовий кластер:
   - `/...` стартує з root cluster 2.
   - relative path стартує з current working directory cluster.
2. Шлях розбивається за `/`.
3. `.` означає залишитися в поточній директорії.
4. `..` означає перейти через directory entry `..`.
5. Звичайні компоненти перетворюються у FAT 8.3 short names.
6. Поточна директорія сканується в пошуку такого short name.
7. Якщо залишилися інші компоненти шляху, знайдений entry має бути директорією.

Є два важливі resolver-шляхи:

- `resolveExisting()` знаходить уже наявний файл або директорію.
- `resolveParent()` знаходить parent directory і фінальне short name для create/update операцій.

Наприклад:

```text
write /tmp/log.txt hello
```

використовує `resolveParent()`:

- parent path: `/tmp`
- leaf name: `log.txt`
- parent cluster: кластер `/tmp`
- short name: `LOG     TXT`

Потім код створює або оновлює entry у цій parent directory.

## Створення Та Запис Файлів

`writeFile(path, data, append)` обслуговує і `write`, і `append`.

Якщо файлу немає:

1. Resolve parent directory.
2. Перевірити leaf name як FAT 8.3.
3. Виділити достатньо кластерів для даних.
4. Записати дані у cluster chain.
5. Створити directory entry з archive attribute і file size.

Якщо файл існує:

- Якщо це директорія, повертається `isDirectory`.
- Для overwrite mode:
  - старий cluster chain звільняється
  - новий chain виділяється за потреби
  - запис починається з offset 0
  - file size оновлюється
- Для append mode:
  - старі дані зберігаються
  - cluster chain збільшується за потреби
  - запис починається зі старого file size
  - file size оновлюється

`append` не додає автоматично пробіл або newline. Команда записує саме той текст, який іде після шляху до файлу.

## Читання Файлів

`readFile(path, callback, context)` читає файл кластер за кластером.

Вона не повертає великий буфер. Замість цього вона викликає callback для кожного шматка:

```cpp
using ChunkCallback = void (*)(std::string_view chunk, void* context);
```

Оболонка використовує це, щоб друкувати вміст файлу прямо в консоль. Для ядра це зручний патерн, бо він уникає зайвих heap allocations і в майбутньому добре підходить для streaming.

## Перегляд Директорій

`list(path, callback, context)` показує:

- один file entry, якщо `path` веде до файлу
- усі видимі entries директорії, якщо `path` веде до директорії

Дані передаються через:

```cpp
using ListCallback = void (*)(const EntryInfo& entry, void* context);
```

`EntryInfo` містить:

- display name
- ознаку directory/file
- file size
- first cluster

Оболонка друкує компактний список:

```text
- 37 welcome.txt
d 0 tmp
```

Перший стовпчик:

- `-` для файлу
- `d` для директорії

## Видалення Файлів І Директорій

`remove(path)` підтримує:

- видалення файлів
- видалення порожніх директорій

Для файлів:

1. Resolve path.
2. Звільнити cluster chain, якщо файл має data clusters.
3. Позначити directory entry видаленим, записавши `0xE5` у перший байт імені.

Для директорій:

1. Resolve path.
2. Відхилити root, `.`, і `..`.
3. Перевірити, що директорія порожня, окрім `.` і `..`.
4. Звільнити cluster chain директорії.
5. Позначити parent directory entry видаленим.

Якщо директорія не порожня, повертається `directoryNotEmpty`.

## Status Та Помилки

Функції файлової системи повертають `FAT32::Status`.

Поточні статуси:

| Status | Значення |
| --- | --- |
| `ok` | Операція успішна. |
| `notReady` | Файлова система ще не ініціалізована. |
| `invalidName` | Компонент шляху не є коректним FAT 8.3 ім'ям або операція націлена на некоректний entry. |
| `notFound` | Entry не існує. |
| `alreadyExists` | Create operation знайшла вже наявний entry. |
| `notDirectory` | Компонент шляху мав бути директорією, але нею не є. |
| `isDirectory` | Операція очікувала файл, але отримала директорію. |
| `directoryNotEmpty` | Спроба видалити непорожню директорію. |
| `noSpace` | Немає вільних кластерів. |
| `ioError` | Помилка block operation або внутрішньої metadata operation. |

Оболонка перетворює статуси на повідомлення через `statusText()`.

Приклади:

```text
cat: tmp: is a directory
rm: docs: directory not empty
write: verylongname.txt: invalid FAT32 8.3 name
```

## Поточні Обмеження

Це практичний перший filesystem layer, а не повний FAT32-драйвер.

Важливі обмеження:

- Storage знаходиться в RAM і є volatile.
- Файли зникають після reboot.
- Реальний disk driver ще не підключений.
- Ядро створює фіксовану відому FAT32 layout, а не монтує довільний зовнішній FAT32 image.
- Long file names не підтримуються.
- Підтримуються тільки FAT 8.3 short names.
- Timestamps не ведуться.
- Permission model ще немає.
- File descriptors ще немає.
- Seek API ще немає.
- Binary shell input ще немає.
- `write` і `append` орієнтовані на текст.
- Немає `cp`, `mv`, `rmdir`, `hexdump`, `find`, `tree`.
- У shell parser ще немає quoting та escaping.
- Volume size фіксований і невеликий.
- Реалізація має одну FAT region і не дзеркалить кілька FAT.
- Немає fsck-style consistency checker.

## Чому Така Архітектура Корисна

Навіть з обмеженнями ця реалізація дає ядру кілька важливих основ:

- filesystem namespace
- current working directory
- directory traversal
- створення й видалення файлів
- allocation і freeing кластерів
- sector-based block abstraction
- shell workflow для перевірки файлової поведінки ядра

Найважливіше: shell не змінює FAT-структури напряму. Він викликає API `Kernel::FAT32`. Це розділення полегшує:

- майбутню заміну RAM-диска на реальний block device
- додавання іншої файлової системи
- створення VFS layer над FAT32
- додавання file descriptors і process-like APIs

## Наступні Кроки

Логічні наступні кроки:

1. Додати реальний block-device driver.
   - Почати з ATA PIO або virtio-blk для QEMU.
   - Зберегти sector-based інтерфейс.

2. Парсити наявний FAT32 boot sector.
   - Читати BPB values замість compile-time constants.
   - Валідовувати FAT count, sector size, cluster size, root cluster і FAT size.

3. Додати VFS layer.
   - Визначити generic file operations незалежно від FAT32.
   - Монтувати FAT32 під `/`.
   - Пізніше монтувати інші файлові системи або device nodes.

4. Додати file descriptors.
   - `open`
   - `read`
   - `write`
   - `seek`
   - `close`

5. Покращити shell parser.
   - Quoted strings.
   - Escapes.
   - Multiple arguments.
   - Кращі повідомлення про помилки.

6. Додати більше Unix-like commands.
   - `cp`
   - `mv`
   - `rmdir`
   - `hexdump`
   - `tree`
   - `more`

7. Додати тести або host-side harness.
   - Збирати FAT32 logic у host test binary.
   - Тестувати path resolution, allocation, deletion, append і directory behavior.

## Build І Run Через Docker

Зібрати kernel image:

```sh
docker build -t unitykernel .
```

Експортувати ISO:

```sh
mkdir -p out
docker run --rm -v "$PWD/out:/out" unitykernel \
    sh -c 'cp build/kernel.iso /out/kernel.iso'
```

Запустити QEMU з Docker без графічного вікна:

```sh
docker run --rm -it unitykernel \
    qemu-system-x86_64 \
    -cdrom /workspace/build/kernel.iso \
    -display none \
    -serial stdio
```

Для короткого smoke test із таймаутом:

```sh
docker run --rm unitykernel \
    timeout 8 qemu-system-x86_64 \
    -cdrom /workspace/build/kernel.iso \
    -display none \
    -serial stdio \
    -no-reboot
```

Exit code `124` від `timeout` означає, що QEMU ще працював, коли час завершився. У цьому випадку це очікувано, бо ядро входить в інтерактивну оболонку й чекає на введення.

