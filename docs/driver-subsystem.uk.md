# DriverSubsystem та архітектура драйверів UnityKernel

Цей документ описує задуману й поточну архітектуру драйверів у UnityKernel на основі файлів:

- `include/kernel/subsystems/driver.hpp`
- `portable/kernel/cxx/driver.cpp`
- `include/drivers/api.hpp`
- `include/drivers/loadlist.hpp`
- `include/drivers/*.hpp`

Мета системи проста й дуже важлива для ядра: забезпечити єдиний механізм завантаження, ініціалізації, зв'язування та знищення драйверів, не дозволяючи їм жити ізольовано відносно один одного.

## 1. Основна ідея

Усі драйвери є похідними класами від `Driver::BaseDriver`.

`DriverSubsystem` є центральним менеджером драйверів:

- знає список доступних драйверів;
- завантажує конкретний драйвер за категорією й типом;
- рекурсивно підтягує його залежності;
- перевіряє, чи конкретний драйвер реально може задовольнити залежність;
- прив'язує віртуальний драйвер до реального пристрою/драйвера;
- відповідає за життєвий цикл цих об'єктів.

У цьому підході:

- базовий клас описує інтерфейс, а не готову реалізацію;
- конкретні драйвери визначають поведінку;
- високорівневі драйвери (наприклад, файлові системи) можуть працювати поверх низькорівневих (наприклад, block devices);
- один підсистемний менеджер централізує ініціалізацію та очищення.

---

## 2. Архітектурна карта

Найпростіша схема системи така:

```text
+---------------------------+
| Kernel::DriverSubsystem   |
| - initialize()            |
| - loadDriver()            |
| - unloadDriver()          |
| - getLoadedDrivers()      |
| - isDriverLoaded()        |
+------------+--------------+
             |
             v
+---------------------------+
| Driver::BaseDriver         |
| + initialize()            |
| + finalize()              |
| + dependencies()          |
| + canSatisfy()            |
| + useDriver()             |
+------------+--------------+
             |
    +--------+--------+
    |                 |
    v                 v
BlockdevDriver    FSDriver
    |                 |
    v                 v
RamdiskDriver    FAT32Driver
```

Ця схема показує ключову ідею: файлова система не працює напряму з апаратурою, а через блоковий шар і логіку підбору відповідного драйвера.

---

## 3. Типи, категорії та метадані драйвера

Основний опис типів знаходиться в `include/drivers/api.hpp`.

```cpp
namespace Driver {
    enum class Category {
        BUS,
        BLK,
        FS
    };

    enum class Type {
        ANY,
        PCI,
        USB,
        PS2,
        RAMDISK,
        FAT32
    };

    struct Info {
        Category category;
        Type type;
    };
}
```

### 3.1. Category

`Category` описує рівень абстракції або клас пристрою:

- `BUS` — шини й інші комунікаційні шари;
- `BLK` — block devices, тобто пристрої, що читають/пишуть блоки даних;
- `FS` — файлові системи, які працюють поверх block-інтерфейсу.

### 3.2. Type

`Type` є більш точним типом драйвера. Наприклад:

- `RAMDISK` — драйвер RAM-диска;
- `FAT32` — файловий драйвер FAT32;
- `PCI`, `USB`, `PS2` — апаратні шляхи доступу.

### 3.3. Info

`Info` служить для опису залежності. Наприклад, якщо файловій системі потрібен будь-який блоковий пристрій, можна сформувати залежність:

```cpp
{{Category::BLK}}
```

Тут `type` не вказаний, тому для залежності використовується `Type::ANY` за замовчуванням через агрегатну ініціалізацію. Це означає: «потрібен будь-який блоковий драйвер, незалежно від конкретного підтипу».

---

## 4. Базовий інтерфейс `BaseDriver`

Клас `BaseDriver` знаходиться в тому ж файлі `include/drivers/api.hpp`.

```cpp
class BaseDriver {
public:
    BaseDriver(Category driCategory, Type driType)
        : driverInfo{driCategory, driType} {}

    virtual ~BaseDriver() = default;

    virtual bool initialize() = 0;
    virtual void finalize() = 0;

    virtual bool canSatisfy(BaseDriver* driver) { return false; };
    virtual void useDriver(BaseDriver* driver) {};

    virtual STDLib::Vector<Info> dependencies() const { return {}; }

    Category category() const { return driverInfo.category; }
    Type type() const { return driverInfo.type; }

protected:
    Info driverInfo;
};
```

### 4.1. Життєвий цикл

Кожен драйвер має явно визначені точки життєвого циклу:

- `initialize()` — запуск драйвера й підготовка до використання;
- `finalize()` — зупинка й звільнення ресурсів;
- деструктор `~BaseDriver()` — остаточне очищення об'єкта.

Це важливо, оскільки ядро хоче мати суворий контроль над станом драйверів і не залишати напівінстанційовані об'єкти живими.

### 4.2. Залежності

`dependencies()` повертає список `Info` — структур, які описують, яких інших драйверів потребує поточний драйвер.

Наприклад, у `FSDriver`:

```cpp
virtual STDLib::Vector<Info> dependencies() const override {
    return {{Category::BLK}};
}
```

Тобто будь-яка файлово-системна реалізація потребує `Category::BLK` як базовий шар для роботи з даними.

### 4.3. `canSatisfy()`

Це, мабуть, найважливіша частина архітектури.

`BaseDriver::canSatisfy()` за замовчуванням повертає `false`. Це означає, що стандартна поведінка — не приймати жодну залежність без явного підтвердження.

Задум: реальний драйвер, який не є віртуальним, повинен реалізувати `canSatisfy()` так, щоб перевірити глибоко й об'єктивно, чи наданий драйвер дійсно підходить цьому залежному драйверу.

Наприклад, для файлової системи перевірка може включати:

- чи це саме block device;
- чи розмір сектора відповідає очікуваному;
- чи пристрій має достатній обсяг;
- чи в ньому є сигнатура файлової системи або допустимий суперблок;
- чи це конкретний тип носія, який відповідає своїй реалізації.

### 4.4. `useDriver()`

`useDriver()` — це точка зв'язування. Після того, як система знайшла відповідний драйвер-залежність, вона передає його поточному драйверу.

Наприклад, `FSDriver` реалізує:

```cpp
virtual void useDriver(BaseDriver* driver) override {
    this->driver = static_cast<Blockdev::BlockdevDriver*>(driver);
}
```

Це означає: файловий драйвер не тримає загальний механізм доступу, а конкретно зберігає покажчик на блоковий пристрій, поверх якого він буде працювати.

---

## 5. Віртуальні драйвери

У цій системі віртуальний драйвер — це не «порожній абстрактний клас», а драйвер, який надає інтерфейс поверх іншого драйвера.

Приклад: `Driver::FS::FSDriver`.

```cpp
class FSDriver : public BaseDriver {
public:
    FSDriver(Type driType) : BaseDriver{Category::FS, driType} {}

    virtual bool canSatisfy(BaseDriver* driver) override { return true; };
    virtual void useDriver(BaseDriver* driver) override {
        this->driver = static_cast<Blockdev::BlockdevDriver*>(driver);
    };

    virtual STDLib::Vector<Info> dependencies() const override {
        return {{Category::BLK}};
    }

protected:
    Blockdev::BlockdevDriver* driver;
};
```

### Чому це віртуальний драйвер?

Тому що він не представляє апаратний пристрій як такий, а дає логічний шар поверх блочного пристрою.

Він:

- живе у категорії `FS`;
- потребує блоковий пристрій як залежність;
- прив'язує `driver` до фактичного пристрою;
- пропонує вищий рівень абстракції: директорії, файли, FAT-структури, читання/запис тощо.

Це дуже схоже на типову модель операційної системи:

- нижній рівень: block device;
- верхній рівень: логічний файловий шар.

---

## 6. Блоковий шар

`include/drivers/blockdev/api.hpp` задає інтерфейс block device:

```cpp
namespace Driver::Blockdev {
    constexpr uint32_t sectorSize = 512;

    class BlockdevDriver : public BaseDriver {
    public:
        BlockdevDriver(Type driType) : BaseDriver{Category::BLK, driType} {}

        virtual void resize(uint32_t sectors) {}

        virtual bool read(uint32_t lba, void* buffer, uint32_t count = 1) const = 0;
        virtual bool write(uint32_t lba, const void* buffer, uint32_t count = 1) = 0;

        virtual uint32_t sectors() const = 0;
    };
}
```

### Основні властивості

- блоковий пристрій працює з секторами (`LBA`);
- `sectorSize` фіксований: 512 байт;
- API надає `read` і `write`;
- кількість секторів доступна через `sectors()`;
- драйвери можуть підтримувати `resize()`.

Цей інтерфейс є природнім базовим шаром для:

- RAM-дисків;
- ATA/AHCI/NVMe-драйверів;
- USB mass storage;
- файлових систем.

---

## 7. Конкретні драйвери

### 7.1. `RamdiskDriver`

Файл: `include/drivers/blockdev/ramdisk.hpp`

```cpp
class RamdiskDriver : public BlockdevDriver {
public:
    RamdiskDriver();

    bool initialize() override;
    void finalize() override;

    void resize(uint32_t sectors) override;

    bool read(uint32_t lba, void* buffer, uint32_t count = 1) const override;
    bool write(uint32_t lba, const void* buffer, uint32_t count = 1) override;

    uint32_t sectors() const override;

private:
    STDLib::Vector<uint8_t> storage;
};
```

`RamdiskDriver` — це повноцінний block device у пам'яті. Він не прив'язаний до фізичного обладнання, але реалізує потрібний інтерфейс:

- `read()` читає блоки зі сховища;
- `write()` записує блоки в сховище;
- `resize()` змінює розмір RAM-диска;
- `sectors()` повертає кількість секторів.

Це дуже корисно для ранньої розробки та тестування файлових систем, оскільки дозволяє вставити віртуальний диск без реального контролера диска.

### 7.2. `FAT32Driver`

Файл: `include/drivers/fs/fat32.hpp`

```cpp
class FAT32Driver : public FSDriver {
public:
    FAT32Driver() : FSDriver{Type::FAT32} {};

    bool initialize() override;
    void finalize() override;

    std::string_view cwd() override;

    Status changeDirectory(std::string_view path) override;
    Status listDirectory(std::string_view path, ListCallback callback, void* context) override;

    Status readFile(std::string_view path, ChunkCallback callback, void* context) override;
    Status writeFile(std::string_view path, std::string_view data, bool append) override;

    Status createFile(std::string_view path) override;
    Status createDirectory(std::string_view path) override;

    Status remove(std::string_view path) override;

    Status stat(std::string_view path, EntryInfo& info) override;

    Status format() override;

private:
    bool mounted = false;
    uint32_t cwdCluster = FAT32::rootCluster;
    STDLib::String cwdPath;
};
```

`FAT32Driver` показує, як вузька, конкретна реалізація піднімається над загальною абстракцією.

Він:

- є драйвером категорії `FS`;
- прив'язаний до `Blockdev::BlockdevDriver` через `FSDriver::driver`;
- реалізує логіку FAT32: директорії, файли, форматування, статуси, робота з current working directory.

Тобто `FAT32Driver` не знає, чи саме це RAM-диск, ATA, або інший block-носій. Він працює через абстракцію `BlockdevDriver` і лише вимагає, що драйвер дійсно може бути його залежністю.

---

## 8. DriverSubsystem: завантаження й ініціалізація

Файл `include/kernel/subsystems/driver.hpp` оголошує API підсистеми:

```cpp
namespace Kernel::DriverSubsystem {
    void initialize();
    void finalize();

    Driver::BaseDriver* loadDriver(Driver::Category category, Driver::Type type);
    void unloadDriver(Driver::BaseDriver* driver);

    const STDLib::Vector<Driver::BaseDriver*> getLoadedDrivers();
    const STDLib::Vector<Driver::BaseDriver*> getLoadedDrivers(Driver::Category category, Driver::Type type);

    bool isDriverLoaded(Driver::BaseDriver* driver);
}
```

### 8.1. Реєстр драйверів

У `portable/kernel/cxx/driver.cpp` є два вектори:

```cpp
static STDLib::Vector<Driver::BaseDriver*> drivers;
static STDLib::Vector<Driver::BaseDriver*> loadedDrivers;
```

Це дуже важливо:

- `drivers` — список усіх зареєстрованих драйверів;
- `loadedDrivers` — список тих, які зараз активні в системі.

Ця модель дозволяє відокремити «можливі драйвери» від «активно завантажених драйверів».

### 8.2. Початкова ініціалізація

```cpp
void initialize() {
    drivers = Driver::loadDriverList();
}
```

Тобто підсистема просто заповнює реєстр тим, що повертає `Driver::loadDriverList()`.

### 8.3. Завершення

```cpp
void finalize() {
    for (auto driver : drivers) delete driver;
    drivers.clear();
}
```

Це означає, що підсистема є єдиним власником об'єктів драйверів і відповідає за останнє знищення.

---

## 9. Розв'язання залежностей

Розв'язання залежностей — центр логіки підсистеми. Воно реалізоване в `portable/kernel/cxx/driver.cpp`.

### 9.1. Пошук драйвера

```cpp
Driver::BaseDriver* findDriver(
    Driver::Category category,
    Driver::Type type,
    uint8_t excludeFlag,
    const STDLib::Vector<Driver::BaseDriver*>& excludeList = {}
)
```

Функція знаходить драйвер, який:

- належить до потрібної категорії;
- має потрібний тип або `ANY`;
- не потрапляє в список виключень;
- відповідає критеріям завантаженості/невантаженості.

Флаги `excludeLoadedFlag` і `excludeUnloadedFlag` дають можливість відфільтровувати уже завантажені або ще не завантажені драйвери.

### 9.2. Пошук набору драйверів

```cpp
STDLib::Vector<Driver::BaseDriver*> findDrivers(...)
```

Ця функція складає список усіх відповідних кандидатів; при цьому вона повторює пошук, додаючи знайдені драйвери до `excludeList` і продовжуючи пошук наступного елемента.

### 9.3. Рекурсивне завантаження

Найважливішою частиною є:

```cpp
bool loadInitDriver(Driver::BaseDriver* driver) {
    if (!driver) return false;

    loadedDrivers.push_back(driver);

    for (const auto& dep : driver->dependencies()) {
        auto depDrivers = findDrivers(dep.category, dep.type, excludeNoneFlag);

        for (auto depDri : depDrivers) {
            if (driver->canSatisfy(depDri) && !isDriverLoaded(depDri))
                if (!loadInitDriver(depDri)) continue;

            driver->useDriver(depDri);
        }
    }

    return driver->initialize();
}
```

Це дає такі кроки:

1. Драйвер додається до `loadedDrivers`.
2. Для кожної залежності треба знайти всі можливі кандидати.
3. Для кожного кандидата перевіряється `canSatisfy()`.
4. Якщо кандидат не завантажений, він рекурсивно завантажується.
5. Після цього викликається `useDriver()`, тобто залежність фіксується у фреймі активного драйвера.
6. Після того, як усі залежності відпрацювали, викликається `initialize()` поточного драйвера.

Це не просто «пошук по назвах», а справжня рекурсивна іерархія побудови драйверного стеку.

---

## 10. Реальний сценарій зв'язування залежностей

Розглянемо сценарій:

```cpp
Kernel::DriverSubsystem::loadDriver(Driver::Category::FS, Driver::Type::FAT32);
```

### Крок 1: пошук кандидата

`findDriver(Category::FS, Type::FAT32, ...)` знаходить `FAT32Driver` у реєстрі.

### Крок 2: розв'язання залежностей

`FAT32Driver::dependencies()` повертає:

```cpp
{{Category::BLK}}
```

Тобто підсистема шукає всі блочні драйвери: наприклад, `RamdiskDriver`.

### Крок 3: перевірка відповідності

Для кожного кандидата викликається:

```cpp
driver->canSatisfy(depDri)
```

Для `FAT32Driver`, якщо це працює як файловий шар, він має перевірити:

- це справді block device;
- дані об'єкта є достатньо придатними для FAT32;
- пристрій має підтримку читання/запису секторів;
- при необхідності перевірити сигнатуру файлової системи або логічний формат.

### Крок 4: прив'язка

Після того, як вибрано вузол, викликається:

```cpp
driver->useDriver(depDri)
```

`FSDriver::useDriver()` зберігає вказівник на блоковий пристрій, який стане базовою платформою для файлової системи.

### Крок 5: ініціалізація

Після того, як вся залежність зібрана, викликається:

```cpp
FAT32Driver::initialize()
```

Це дає потужну модель: драйвери не просто «створюються», а «збираються в робочий стек».

---

## 11. Власність і життєвий цикл об'єктів

Основна ідея системи полягає в тому, що `DriverSubsystem` є єдиним власником драйверів.

- `Driver::loadDriverList()` створює всі драйвери через `new`.
- `DriverSubsystem::initialize()` зберігає їх у реєстрі.
- `loadDriver()` завантажує конкретні об'єкти й додає їх до `loadedDrivers`.
- `finalize()` проходиться по реєстру й видаляє кожен драйвер.
- `unloadDriver()` знімає конкретний драйвер з активного списку й викликає `finalize()` на ньому.

У цьому сенсі підсистема повністю контролює планування, оголошення, порядок ініціалізації та очищення ресурсів.

---

## 12. Поточний стан реалізації

У поточному коді реалізована мінімальна, але цілком показова версія системи.

### Поточні драйвери

Файл `include/drivers/loadlist.hpp` містить:

```cpp
STDLib::Vector<BaseDriver*> loadDriverList() {
    STDLib::Vector<BaseDriver*> drivers;
    drivers.push_back(new Blockdev::RamdiskDriver);
    // drivers.push_back(new FS::FAT32Driver);

    return drivers;
}
```

Це означає, що наразі в реєстрі є лише:

- `Blockdev::RamdiskDriver`

Файлова система `FAT32Driver` вже спроектована у загальному інтерфейсі, але не є активним елементом реєстру за замовчуванням.

Це корисно для ранніх етапів розробки: можна перевірити block device без складної файлової системи, а потім додати FS-шар, коли він буде готовий.

---

## 13. Рекомендована модель `canSatisfy()` для реальних драйверів

У дизайнерській моделі, яку ви описали, `canSatisfy()` має бути реалізований справжнім драйвером, а не лише загальним fallback.

### Хороша реалізація включає:

- перевірку категорії та типу;
- перевірку сумісного мінімального інтерфейсу;
- перевірку фізичних/логічних властивостей пристрою;
- перевірку чи відповідний драйвер справді може обслужити конкретну залежність.

Псевдокод:

```cpp
bool FAT32Driver::canSatisfy(BaseDriver* candidate) {
    if (!candidate) return false;

    if (candidate->category() != Driver::Category::BLK)
        return false;

    auto* block = dynamic_cast<Driver::Blockdev::BlockdevDriver*>(candidate);
    if (!block)
        return false;

    // deep validation
    if (block->sectors() == 0)
        return false;

    if (Driver::Blockdev::sectorSize != 512)
        return false;

    // optional: parse filesystem signature, partition table, superblock, etc.
    return true;
}
```

Це не просто перевірка типу. Це перевірка реальної придатності драйвера до конкретного використання.

---

## 14. Плюси архітектури

Ця архітектура має кілька сильних сторін:

- модульність: кожен драйвер має чіткий інтерфейс;
- гнучкість: віртуальні драйвери працюють поверх фізичних;
- рекурсивна розв'язка залежностей;
- централізована власність: ядро управляє життєвим циклом;
- можливість підключення нових драйверів без розриву основної системи.

### Ключова властивість

`DriverSubsystem` є не просто контейнером, а диспетчером картографії драйверів: він не лише знає, що є, але й формує правильну графову ієрархію залежностей.

---

## 15. Обмеження поточної реалізації

Незважаючи на чітку архітектуру, поточна кодова база все ще є базовою. До основних обмежень належать:

- `canSatisfy()` у базовому класі за замовчуванням повертає `false`; це добре як безпечний default, але вимагає явної реалізації реальними драйверами;
- `findDriver()` і `findDrivers()` базуються на простому пошуку, без багаторівневої маршрутизації або політики вибору;
- `loadInitDriver()` додає драйвер до `loadedDrivers` перед повною перевіркою його залежностей і ініціалізації;
- поточний реєстр містить лише RAM-диск; файлові системи ще не повністю інтегровані в список завантаження;
- `getLoadedDrivers(Category, Type)` зараз фактично працює не як фільтр активних завантажених драйверів, а як пошук за каталогом/типом серед реєстру.

Це нормально для ранньої версії ядра, але важливо враховувати при розширенні системи.

---

## 16. Висновок

Архітектура драйверів UnityKernel побудована на простій і потужній ідеї:

- базовий клас задає інтерфейс;
- підсистема керує реєстром і життєвим циклом;
- залежності розв'язуються рекурсивно;
- віртуальні драйвери працюють поверх конкретних пристроїв;
- `canSatisfy()` є критично важливим місцем, де реалізується справжня логіка сумісності.

Найширше узагальнення таке:

> `DriverSubsystem` — це не просто список драйверів, а графовий менеджер ресурсів ядра, який збирає, перевіряє й підтримує стабільну ієрархію драйверів від апаратного шару до логічних абстракцій.

Ця структура гарно підходить для розвитку ядра: від RAM-диска та простих block-інтерфейсів до реальних PCI/USB/AHCI/FS-стеків без повної переробки базової моделі.
