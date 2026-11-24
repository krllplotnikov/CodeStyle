# CodeStyle

Стандарт проектирования для встраиваемых проектов на C/C++: прошивки микроконтроллеров, драйверы периферии, библиотеки.

Базовый принцип — **аппаратная независимость прикладного кода.** Весь платформозависимый код локализован в слое аппаратной абстракции `Board/` и в точке сборки `main.c`. Остальные слои взаимодействуют с периферией исключительно через указатели на функции, передаваемые при инициализации (внедрение зависимостей). Это обеспечивает:

- переносимость библиотек и модулей между МК без модификации исходного кода;
- возможность host-тестирования логики (unit-тесты на ПК с подменой аппаратных функций заглушками — stubs/mocks);
- явное разделение бизнес-логики и драйверного кода.

Эталонная реализация находится в директории `Reference/`. Все примеры в этом документе взяты из неё.

## Содержание

1. [Архитектура проекта](#1-архитектура-проекта)
   - [Эталонный проект](#эталонный-проект)
   - [Объект и дескриптор](#объект-и-дескриптор)
2. [Соглашения об именовании](#2-соглашения-об-именовании)
3. [Заголовочный файл (.h)](#3-заголовочный-файл-h)
4. [Файл реализации (.c)](#4-файл-реализации-c)
   - [Жизненный цикл объекта](#жизненный-цикл-объекта)
   - [Время и переполнение счётчика тиков](#время-и-переполнение-счётчика-тиков)
   - [События: колбэки и context](#события-колбэки-и-context)
   - [Обработка ошибок](#обработка-ошибок)
5. [Аппаратный слой: Board и main.c](#5-аппаратный-слой-board-и-mainc)
   - [Board](#board)
   - [main.c](#mainc)
6. [Прерывания и разделяемые данные](#6-прерывания-и-разделяемые-данные)
   - [Допустимые операции в ISR](#допустимые-операции-в-isr)
   - [Передача данных из ISR](#передача-данных-из-isr)
   - [Требования к разделяемым полям](#требования-к-разделяемым-полям)
   - [RTOS](#rtos)
7. [Форматирование](#7-форматирование)

---

## 1. Архитектура проекта

Базой проекта служат файлы, сгенерированные конфигуратором (например, STM32CubeMX). Поверх них вводятся четыре каталога:

```
Project/
├── Board/          ← слой аппаратной абстракции: объекты физических устройств и аппаратные функции поверх HAL
├── Libraries/      ← аппаратно-независимые библиотеки, git submodules
├── Modules/        ← модули: фрагменты бизнес-логики, построенные на библиотеках
├── Applications/   ← приложение: композиция модулей в конечный продукт
└── main.c          ← composition root: создание платы и приложения, связывание, запуск
```

| Слой | Назначение | Зависимость от платформы |
|---|---|---|
| `Board/` | Содержит экземпляры библиотек, соответствующие физическим устройствам платы (`buttonMain`, `ledMain`), и инициализирует их. Реализует аппаратные функции (`getButtonState`, `setLedState`, `getTick`) поверх HAL. Бизнес-логики не содержит | **Есть** |
| `Libraries/` | Решает одну изолированную задачу (управление светодиодом, опрос кнопки). Подключается как **git submodule** для переиспользования между проектами | Нет |
| `Modules/` | Реализует фрагмент бизнес-логики на основе библиотек (например, распознавание клика и удержания кнопки) | Нет |
| `Applications/` | Реализует бизнес-логику продукта. Содержит модули как поля своей структуры и инициализирует их | Нет |
| `main.c` | Создаёт экземпляры платы и приложения, передаёт приложению объекты платы через дескриптор, запускает систему | **Есть** (только на этапе инициализации) |

Граф зависимостей строго однонаправленный: `Applications → Modules → Libraries`; `Board/` зависит от `Libraries/` (содержит их объекты), но не наоборот. Портирование на другой МК или плату сводится к перегенерации файлов конфигуратора и переписыванию `Board.c`; остальные слои не изменяются.

### Эталонный проект

`Reference/STM32` — проект для STM32F103C8 с одной кнопкой и одним светодиодом. Клик по кнопке включает светодиод, удержание — выключает.

```
Reference/STM32/
├── Board/
│   ├── Board.h                 ← Board_st: buttonMain, ledMain; Board_Init
│   └── Board.c                 ← аппаратные функции поверх HAL, инициализация объектов платы
├── Libraries/                  ← git submodules
│   ├── Button/src/Button.h/.c  ← опрос кнопки: антидребезг, клики, удержание
│   └── LED/src/LED.h/.c        ← управление светодиодом: индикации, повторы
├── Modules/
│   ├── ModuleButton/           ← преобразование событий кнопки в CLICK / HOLD
│   └── ModuleLED/              ← включение / выключение светодиода
├── Applications/
│   └── Device/                 ← связывает ModuleButton и ModuleLED
└── Core/Src/main.c             ← создание board и device, запуск
```

Цепочка вызовов в суперцикле: `Device_Run → ModuleButton_Run → Button_Run` и `Device_Run → ModuleLED_Run → LED_Run`.

### Объект и дескриптор

Каждая сущность (светодиод, кнопка, модуль, приложение) представлена **структурой-объектом**, инкапсулирующей её состояние. При инициализации объекту передаётся **дескриптор** — структура с внешними зависимостями и аппаратными функциями:

```c
LED_Descriptor_st ledDescriptor = {
	.setState                 = setLedState,   // управление выходом
	.getTick                  = getTick,       // источник системного времени
	.indicationIsOverCallback = NULL,
	.context                  = NULL,
};
LED_Init(&_board->ledMain, &ledDescriptor);
```

Все функции объекта принимают указатель на экземпляр первым аргументом: `ModuleLED_TurnOn(&device->moduleLed)`. За счёт этого код библиотеки реентерабелен по отношению к экземплярам и обслуживает произвольное их число.

Дескриптор обязателен для всех объектов — библиотек, модулей и приложений. Он содержит поля трёх категорий:
- **зависимости** — аппаратные функции (`setState`, `getTick`) либо указатели на объекты нижележащего слоя (`LED_st* led`, `Button_st* button`);
- **колбэки событий** — механизм уведомления вышележащего слоя (`eventCallback`);
- **`void* context`** — непрозрачный указатель на объект-подписчик. Объект хранит его, но не разыменовывает.

Объект, который является частью другого объекта, хранится как поле его структуры, а не создаётся в `main.c`. Так, модули — поля приложения:

```c
struct Device{
	Device_Descriptor_st descriptor;

	ModuleButton_st      moduleButton;
	ModuleLED_st         moduleLed;
};
```

В `Libraries/`, `Modules/` и `Applications/` **глобальные переменные запрещены**: всё состояние хранится в экземпляре объекта.

---

## 2. Соглашения об именовании

Идентификатор должен однозначно указывать на принадлежность к модулю и категорию сущности. Для этого используются префикс модуля и суффикс типа.

| Сущность | Правило | Пример |
|---|---|---|
| Файлы | `PascalCase`, аббревиатуры в верхнем регистре | `Board.h`, `ModuleLED.h` |
| Макроконстанты `#define` | `UPPER_SNAKE_CASE` с префиксом модуля | `MODULE_BUTTON_HOLD_COUNT` |
| Перечисление | `Модуль_Имя` + суффикс `_et` | `ModuleButton_Event_et` |
| Константы перечисления | `UPPER_SNAKE_CASE` с префиксом | `MODULE_BUTTON_EVENT_CLICK` |
| Структура | `Модуль_Имя` + суффикс `_st` | `ModuleLED_Descriptor_st` |
| Тип указателя на функцию | `Модуль_Имя`, без суффикса | `LED_SetState` |
| Тип колбэка события | `Модуль_Событие` + `Callback` | `ModuleButton_EventCallback` |
| Публичная функция | `Модуль_Действие` | `ModuleLED_TurnOn` |
| Функция, вызываемая из ISR | базовое имя + `FromISR` | `[Модуль]_[Действие]FromISR` |
| Тип кода результата | `Модуль_Result_et`, константы `МОДУЛЬ_RESULT_...` | `[Модуль]_Result_et`, `[МОДУЛЬ]_RESULT_OK` |
| Функция с внутренним связыванием (`static`) | `camelCase`, без префикса | `sendEvent`, `getButtonState` |
| Обработчик события нижележащего объекта | `static`, `camelCase`: источник + `Event` | `moduleButtonEvent`, `buttonEvent` |
| Переменные и поля | `camelCase`, семантически значимые имена | `moduleLed`, `indicationRepeatCounter` |
| Параметры функций | `camelCase` с префиксом `_` | `_moduleButton`, `_event` |

Префикс `_` у параметров визуально отделяет входные данные функции от полей структуры и локальных переменных.

```c
void ModuleButton_Init(ModuleButton_st* _moduleButton, ModuleButton_Descriptor_st* _descriptor);
```

**Модули и приложения:**
- модули получают префикс `Module`: каталог `Modules/ModuleLED/`, файлы `ModuleLED.h/.c`, тип `ModuleLED_st`, функции `ModuleLED_TurnOn(...)`;
- приложения именуются собственным именем: `Applications/Device/Device.c`, `Device_st`, `Device_Run`.

**Функции без параметров объявляются со списком `(void)`.** В C пустой список `()` означает объявление без прототипа (число и типы аргументов не заданы), что отключает проверку вызовов компилятором:

```c
static uint32_t getTick(void);
```

---

## 3. Заголовочный файл (`.h`)

Все заголовочные файлы имеют единую структуру. Порядок секций:

1. Include guard — `MODULEBUTTON_H_`.
2. `#include` — все зависимости модуля.
3. `#define` — макроконстанты.
4. Определения `enum`.
5. Псевдонимы перечислений: `typedef enum ModuleButton_Event ModuleButton_Event_et;`
6. Предварительные объявления структур: `typedef struct ModuleButton ModuleButton_st;`
7. Типы указателей на функции.
8. Определения `struct`.
9. Прототипы функций в блоке `extern "C"` — для совместимости с C++ (подавление name mangling).

Предварительные объявления структур делают все типы модуля видимыми до их определения: например, тип колбэка `ModuleButton_EventCallback` принимает `ModuleButton_st*` (неполный тип) до определения структуры `ModuleButton`.

Перечисления, в отличие от структур, определяются **до** своего `typedef`. Предварительное объявление `enum` без определения запрещено и стандартом C (ISO C forbids forward references to 'enum' types), и C++ (без явного базового типа). Такой заголовок не компилируется в C++, например в unit-тестах на GoogleTest, несмотря на блок `extern "C"`. Перечисления не зависят от остальных типов модуля, поэтому ранее определение ничего не ограничивает.

<details>
<summary>Пример заголовочного файла: <code>Modules/ModuleButton/ModuleButton.h</code></summary>

```c
#ifndef MODULEBUTTON_H_
#define MODULEBUTTON_H_

#include <stddef.h>
#include <stdint.h>
#include "Button.h"

#define MODULE_BUTTON_HOLD_COUNT 4

enum ModuleButton_Event{
	MODULE_BUTTON_EVENT_CLICK,
	MODULE_BUTTON_EVENT_HOLD
};

typedef enum ModuleButton_Event ModuleButton_Event_et;

typedef struct ModuleButton_Descriptor ModuleButton_Descriptor_st;
typedef struct ModuleButton            ModuleButton_st;

typedef void (*ModuleButton_EventCallback)(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event);

struct ModuleButton_Descriptor{
	Button_st*                 button;
	ModuleButton_EventCallback eventCallback;
	void*                      context;
};

struct ModuleButton{
	ModuleButton_Descriptor_st descriptor;
};

#ifdef __cplusplus
extern "C" {
#endif

void ModuleButton_Init(ModuleButton_st* _moduleButton, ModuleButton_Descriptor_st* _descriptor);
void ModuleButton_Run(ModuleButton_st* _moduleButton);

#ifdef __cplusplus
}
#endif

#endif
```

</details>

---

## 4. Файл реализации (`.c`)

Порядок секций (отсутствующие секции опускаются):

1. `#include` **только собственного** заголовка — транзитивные зависимости подключены в `.h`. Исключение — `Board.c`, который дополнительно подключает `main.h` (см. [раздел 5](#board)).
2. Данные модуля — константные таблицы, предопределённые наборы.
3. Прототипы `static` функций.
4. Публичные функции — в порядке объявления в `.h`; первыми идут `_Init` и `_Run`.
5. Определения `static` функций: сначала обработчики событий нижележащих объектов, затем вспомогательные функции.

<details>
<summary>Пример файла реализации: <code>Modules/ModuleButton/ModuleButton.c</code></summary>

```c
#include "ModuleButton.h"

static void buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter);
static void sendEvent(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event);

void ModuleButton_Init(ModuleButton_st* _moduleButton, ModuleButton_Descriptor_st* _descriptor){
	_moduleButton->descriptor = *_descriptor;

	Button_SetEventCallback(_moduleButton->descriptor.button, buttonEvent, _moduleButton);
}

void ModuleButton_Run(ModuleButton_st* _moduleButton){
	Button_Run(_moduleButton->descriptor.button);
}

static void buttonEvent(Button_st* _button, Button_Event_et _event, uint8_t _counter){
	ModuleButton_st* moduleButton = (ModuleButton_st*)_button->descriptor.context;

	switch(_event){
	case BUTTON_EVENT_CLICK:{
		if(_counter == 1){
			sendEvent(moduleButton, MODULE_BUTTON_EVENT_CLICK);
		}
		break;
	}

	case BUTTON_EVENT_HOLD:{
		if(_counter == MODULE_BUTTON_HOLD_COUNT){
			sendEvent(moduleButton, MODULE_BUTTON_EVENT_HOLD);
		}
		break;
	}

	default: break;
	}
}

static void sendEvent(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event){
	if(_moduleButton->descriptor.eventCallback != NULL){
		_moduleButton->descriptor.eventCallback(_moduleButton, _event);
	}
}
```

</details>

### Жизненный цикл объекта

- **`_Init(obj, descriptor)`** — копирует дескриптор в объект (`_moduleLed->descriptor = *_descriptor;`) и явно инициализирует **все** остальные поля. Полагаться на неявное обнуление (`.bss`, `= {0}` в точке определения) запрещено. Если объект содержит вложенные объекты (приложение — модули), `_Init` формирует для них дескрипторы и вызывает их `_Init`.
- **`_Run(obj)`** — вызывается циклически из суперцикла или задачи RTOS. Функция **обязана быть неблокирующей**: запрещены `HAL_Delay`, busy-wait и любые ожидания. Отсчёт времени ведётся по `getTick()`. `_Run` вышележащего объекта вызывает `_Run` своих зависимостей:

  ```c
  void Device_Run(Device_st* _device){
  	ModuleButton_Run(&_device->moduleButton);
  	ModuleLED_Run(&_device->moduleLed);
  }
  ```

- Опциональные колбэки перед вызовом проверяются на `NULL` (см. `sendEvent` выше).

### Время и переполнение счётчика тиков

32-битный счётчик миллисекунд `getTick()` переполняется через 2³² мс ≈ **49,7 суток** и возобновляет отсчёт с нуля. При некорректном сравнении времени после переполнения таймеры срабатывают немедленно либо не срабатывают никогда. Дефект проявляется только при длительной непрерывной работе и не воспроизводится при стендовой отладке.

**Корректно — фиксировать момент начала и сравнивать прошедший интервал** (фрагмент `Button.c`):

```c
void Button_Run(Button_st* _button){
	uint32_t currentTime = _button->descriptor.getTick();

	debounce(_button, currentTime);
	update(_button, currentTime);
	processEvent(_button, currentTime);
}

static void debounce(Button_st* _button, uint32_t _currentTime){
	...
	else if((_currentTime - debounceDescriptor->changeTime) >= BUTTON_DEBOUNCE_TIME_MS
	...
}
```

Беззнаковая арифметика в C выполняется по модулю 2ᴺ, поэтому разность корректна и при переходе через ноль: `0x00000010 - 0xFFFFFFF0 = 0x20`, то есть фактически прошедшие 32 мс.

**Некорректно — вычислять момент окончания:**

```c
if(getTick() > changeTime + BUTTON_DEBOUNCE_TIME_MS){   // сумма может переполниться
	...
}
```

Сумма `changeTime + BUTTON_DEBOUNCE_TIME_MS` вблизи переполнения переходит через ноль, после чего условие становится истинным немедленно.

Требования:
- в объекте хранится **момент начала** интервала (`changeTime` в `Button`, `startTime` в `LED`), а не момент окончания;
- сравнение выполняется строго в форме `(currentTime - startTime) >= duration`;
- текущее время, момент начала и длительность имеют **один беззнаковый тип**, по умолчанию `uint32_t`. Для более узкого тика (например, `uint16_t`) разность явно приводится к этому типу: `(uint16_t)(currentTime - startTime)`. В противном случае integer promotion расширит операнды до `int`, и разность станет отрицательной;
- значение `getTick()` считывается **однократно** в начале `_Run` и используется до конца прохода — это обеспечивает согласованный временной срез для всех проверок;
- единица измерения указывается в имени поля или константы, если она не очевидна из контекста: `BUTTON_DEBOUNCE_TIME_MS`;
- длительность интервала не может превышать период переполнения счётчика. Длительные интервалы (часы, сутки) отсчитываются отдельным счётчиком более крупных единиц (например, секунд, инкрементируемых каждые 1000 мс).

**В unit-тестах** заглушка `getTick` инициализируется значением вблизи переполнения, например `0xFFFFFF00`, чтобы тесты покрывали переход счётчика через ноль.

### События: колбэки и context

Объект генерирует событие вызовом колбэка из своего дескриптора. Первым аргументом колбэк получает объект-источник, поэтому обработчик идентифицирует **отправителя**. Через `descriptor.context` источника обработчик получает **свой** экземпляр — объект-подписчик.

Таким образом событие связывает конкретную пару экземпляров. Это исключает глобальные указатели и диспетчеризацию вида `if(_x == instance->x)` и не ограничивает число экземпляров.

Пример: `ModuleButton` генерирует события `CLICK` / `HOLD`, приложение `Device` подписывается на них при инициализации модуля.

```c
// ModuleButton.h (фрагмент)
typedef void (*ModuleButton_EventCallback)(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event);

struct ModuleButton_Descriptor{
	Button_st*                 button;
	ModuleButton_EventCallback eventCallback;
	void*                      context;
};
```

```c
// Device.c
#include "Device.h"

static void moduleButtonEvent(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event);

void Device_Init(Device_st* _device, Device_Descriptor_st* _descriptor){
	_device->descriptor = *_descriptor;

	ModuleLED_Descriptor_st moduleLedDescriptor = {
		.led = _descriptor->led,
	};
	ModuleLED_Init(&_device->moduleLed, &moduleLedDescriptor);

	ModuleButton_Descriptor_st moduleButtonDescriptor = {
		.button        = _descriptor->button,
		.eventCallback = moduleButtonEvent,
		.context       = _device,
	};
	ModuleButton_Init(&_device->moduleButton, &moduleButtonDescriptor);
}

void Device_Run(Device_st* _device){
	ModuleButton_Run(&_device->moduleButton);
	ModuleLED_Run(&_device->moduleLed);
}

static void moduleButtonEvent(ModuleButton_st* _moduleButton, ModuleButton_Event_et _event){
	Device_st* device = (Device_st*)_moduleButton->descriptor.context;

	switch(_event){
	case MODULE_BUTTON_EVENT_CLICK:{
		ModuleLED_TurnOn(&device->moduleLed);
		break;
	}

	case MODULE_BUTTON_EVENT_HOLD:{
		ModuleLED_TurnOff(&device->moduleLed);
		break;
	}

	default: break;
	}
}
```

Подписка выполняется тем объектом, который получает источник события:
- если источник — вложенный объект (модуль внутри приложения), подписчик заполняет `eventCallback` и `context` в дескрипторе при вызове его `_Init` (`Device_Init` выше);
- если источник создан другим слоем (объект библиотеки, созданный в `Board_Init` с `eventCallback = NULL`), подписчик назначает колбэк и `context` через функцию `_SetEventCallback` в своём `_Init`: `Button_SetEventCallback(_moduleButton->descriptor.button, buttonEvent, _moduleButton);`. Для `LED` аналогично — `LED_SetIndicationIsOverCallback`.

Требования:
- обработчик события — `static` функция в `.c`-файле подписчика, сигнатура которой точно соответствует типу колбэка;
- одно событие — один подписчик. Рассылка нескольким получателям (fan-out) выполняется обработчиком подписчика;
- **weak-символы для событий запрещены**: опечатка в имени не диагностируется линковщиком, переопределение возможно только однократно на образ прошивки, семантика зависит от тулчейна.

### Обработка ошибок

> В эталонном проекте пока нет объектов с путями отказа, поэтому правила ниже приведены без примеров кода. `[Модуль]` обозначает префикс конкретной библиотеки или модуля.

Каждая библиотека и каждый модуль, операции которых могут завершиться ошибкой, определяют собственный тип кода результата `[Модуль]_Result_et`. Первая константа — `[МОДУЛЬ]_RESULT_OK` со значением `0`, далее — `_BUSY`, `_ERROR_INVALID_ARGUMENT`, `_ERROR_TIMEOUT` и т.п.

Ошибки передаются по двум каналам в зависимости от момента их обнаружения.

**1. Синхронно — через возвращаемое значение.** Функция возвращает код результата, если ошибка может быть обнаружена в момент вызова: недопустимый аргумент, объект занят, аппаратная функция вернула ошибку. Выходные данные в этом случае передаются через выходные параметры-указатели, расположенные в конце списка аргументов.

**2. Асинхронно — через колбэк ошибки.** Поскольку `_Run` неблокирующая, часть ошибок выявляется через несколько итераций цикла: истечение таймаута ответа устройства, несовпадение CRC. Такие ошибки передаются через колбэк из дескриптора с сигнатурой `void (*[Модуль]_ErrorCallback)([Модуль]_st* _obj, [Модуль]_Result_et _result)`.

Требования:
- **Функции, не имеющие путей отказа, возвращают `void`.** Возврат безусловного `_OK` ради единообразия запрещён (`LED_SetIndication`, `ModuleLED_TurnOn`).
- **Код результата обязан проверяться.** Намеренное игнорирование выражается явным приведением к `(void)`.
- **Аппаратные функции с путями отказа возвращают тип результата своей библиотеки.** `Board.c` транслирует статус HAL (`HAL_OK` / ошибка) в этот тип; `HAL_StatusTypeDef` за пределы `Board/` не распространяется.
- **Каждый слой использует собственный тип результата.** Модуль не пробрасывает тип результата библиотеки, а отображает его в свой. Это изолирует приложение от конкретной реализации (чипа) внутри модуля.
- **Аргументы, поступающие извне (индекс, размер, номер канала), валидируются**; при нарушении возвращается `_ERROR_INVALID_ARGUMENT`. Указатель на собственный экземпляр не валидируется: передача `NULL` вместо объекта — нарушение контракта (ошибка программиста), а не runtime-ситуация.
- **Библиотеки и модули не определяют политику обработки ошибок**: не входят в бесконечный цикл, не вызывают `Error_Handler()`, не выполняют сброс МК. Их ответственность — только сообщить об ошибке. Стратегию восстановления (повтор, индикация, сброс) определяет приложение.

---

## 5. Аппаратный слой: Board и main.c

### Board

На текущем этапе аппаратный слой описывается одной парой файлов:

```
Board/
├── Board.h   ← структура Board_st с объектами устройств платы и прототип Board_Init; не зависит от HAL
└── Board.c   ← аппаратные функции поверх HAL и инициализация объектов платы
```

`Board_st` содержит по одному экземпляру библиотеки на каждое физическое устройство. Поля именуются по роли устройства на плате: `buttonMain`, `ledMain`.

```c
// Board.h
#ifndef BOARD_H_
#define BOARD_H_

#include <stddef.h>
#include <stdint.h>
#include "Button.h"
#include "LED.h"

typedef struct Board Board_st;

struct Board{
	Button_st buttonMain;
	LED_st    ledMain;
};

#ifdef __cplusplus
extern "C" {
#endif

void Board_Init(Board_st* _board);

#ifdef __cplusplus
}
#endif

#endif
```

```c
// Board.c
#include "Board.h"
#include "main.h"

static Board_st* boardInstance = NULL;

static uint32_t        getTick(void);
static Button_State_et getButtonState(Button_st* _button);
static void            setLedState(LED_st* _led, LED_State_et _state);

void Board_Init(Board_st* _board){
	boardInstance = _board;

	Button_Descriptor_st buttonDescriptor = {
		.getState      = getButtonState,
		.getTick       = getTick,
		.eventCallback = NULL,
		.context       = NULL,
	};
	Button_Init(&_board->buttonMain, &buttonDescriptor);

	LED_Descriptor_st ledDescriptor = {
		.setState                 = setLedState,
		.getTick                  = getTick,
		.indicationIsOverCallback = NULL,
		.context                  = NULL,
	};
	LED_Init(&_board->ledMain, &ledDescriptor);
}

static uint32_t getTick(void){
	return HAL_GetTick();
}

static Button_State_et getButtonState(Button_st* _button){
	if(_button == &boardInstance->buttonMain){
		if(HAL_GPIO_ReadPin(BUTTON_MAIN_GPIO_Port, BUTTON_MAIN_Pin) == GPIO_PIN_SET){
			return BUTTON_STATE_PRESSED;
		}
	}

	return BUTTON_STATE_RELEASED;
}

static void setLedState(LED_st* _led, LED_State_et _state){
	if(_led == &boardInstance->ledMain){
		if(_state == LED_STATE_ON){
			HAL_GPIO_WritePin(LED_MAIN_GPIO_Port, LED_MAIN_Pin, GPIO_PIN_SET);
		}
		else{
			HAL_GPIO_WritePin(LED_MAIN_GPIO_Port, LED_MAIN_Pin, GPIO_PIN_RESET);
		}
	}
}
```

Требования:
- **`Board.h` не зависит от HAL**: подключает только заголовки библиотек. `main.h` и заголовки периферии подключаются исключительно в `Board.c`. Благодаря этому приложение и `main.c` видят только типы библиотек;
- **`Board_Init`** сохраняет указатель на экземпляр платы в `static` переменной `boardInstance`, заполняет дескрипторы аппаратными функциями и вызывает `_Init` каждого объекта платы. Колбэки событий и `context` устанавливаются в `NULL` — на события подписывается вышележащий слой (см. [События](#события-колбэки-и-context));
- **аппаратные функции — `static`**, именуются в `camelCase` без префикса (`getButtonState`, `setLedState`). Их сигнатура **точно соответствует** типу указателя на функцию из библиотеки, что позволяет присвоить её полю дескриптора без адаптеров;
- одна аппаратная функция обслуживает все объекты своего типа. Устройство определяется сравнением указателя на объект с полями `boardInstance`: `if(_led == &boardInstance->ledMain)`. Для неизвестного объекта функция ничего не делает или возвращает безопасное значение (`BUTTON_STATE_RELEASED`);
- функции HAL не передаются в дескрипторы напрямую — только через обёртки `Board.c` (`getTick` вместо `HAL_GetTick`);
- бизнес-логика запрещена — только вызовы HAL и выбор устройства. Единственное допустимое состояние — указатель `boardInstance`;
- `Board/` зависит от библиотек; обратная зависимость запрещена;
- дескрипторы периферии (`huart1`, `hspi1`) получаются из сгенерированных заголовков. В CubeMX для этого включается опция *Generate peripheral initialization as a pair of .c/.h files per peripheral*, после чего в `Board.c` подключаются `usart.h`, `spi.h` и т.д. При отключённой опции дескрипторы объявляются через `extern` в `Board.c`;
- колбэки вендорского HAL (`HAL_GPIO_EXTI_Callback` и т.п.) размещаются в `Board.c`, поскольку только он имеет доступ к объектам платы и HAL (см. [раздел 6](#6-прерывания-и-разделяемые-данные));
- файлы оформляются по общим правилам для `.h` / `.c` (разделы 3 и 4).

### main.c

`main.c` выполняет роль composition root: создаёт плату и приложение, передаёт приложению объекты платы и запускает систему. Аппаратные функции и колбэки в нём не реализуются.

Если `main.c` сгенерирован конфигуратором, пользовательский код размещается **исключительно** внутри блоков `/* USER CODE BEGIN … */ … /* USER CODE END … */`, чтобы сохраниться при перегенерации. В `USER CODE BEGIN Includes` подключаются `Board.h` и заголовок приложения:

```c
/* USER CODE BEGIN Includes */
#include "Board.h"
#include "Device.h"
/* USER CODE END Includes */
```

В `USER CODE BEGIN 0` код разделён на секции с маркерами `//-----Имя-----` (между секциями — три пустые строки):

| Секция | Содержимое |
|---|---|
| `//-----Variables-----` | Экземпляры платы и приложения с инициализатором `= {0}`, выровненные по колонкам. Порядок: плата, приложение |
| `//-----Descriptors-----` | Дескриптор приложения с назначенными инициализаторами (designated initializers): указатели на объекты платы |

```c
/* USER CODE BEGIN 0 */
//-----Variables-----
Board_st  board  = {0};
Device_st device = {0};
//-----Variables-----



//-----Descriptors-----
Device_Descriptor_st deviceDescriptor = {
	.button = &board.buttonMain,
	.led    = &board.ledMain,
};
//-----Descriptors-----
/* USER CODE END 0 */
```

В результате `main.c` показывает, какие физические устройства платы использует приложение. Связи между модулями и подписки на события находятся внутри приложения.

Последовательность в `main()`:
1. после инициализации периферии (`MX_GPIO_Init()` и т.п.) вызывается `Board_Init`, затем `_Init` приложения;
2. в суперцикле (или задаче RTOS) вызывается только `_Run` приложения.

```c
  /* USER CODE BEGIN 2 */
  Board_Init(&board);
  Device_Init(&device, &deviceDescriptor);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  Device_Run(&device);
    /* USER CODE END WHILE */
```

---

## 6. Прерывания и разделяемые данные

> В эталонном проекте прерывания пока не используются, поэтому правила ниже приведены без примеров кода.

Прерывание может вытеснить `_Run` в произвольной точке. Модификация одних данных из обоих контекстов приводит к состоянию гонки (race condition) — недетерминированному дефекту, который проявляется редко и не воспроизводится под отладчиком. Базовое правило: **ISR только регистрирует факт события, обработка выполняется в `_Run`** (deferred processing).

### Допустимые операции в ISR

- Вендорский колбэк в `Board.c` вызывает функцию библиотеки с суффиксом `FromISR` для соответствующего объекта платы: `[Модуль]_[Действие]FromISR(&boardInstance->[устройство])`. Иные вызовы из контекста прерывания запрещены.
- Функция `...FromISR` выполняет минимальное действие: инкремент счётчика, запись байта в кольцевой буфер, фиксация метки времени. Как правило, это одна-две строки.
- **Колбэки событий из контекста прерывания не вызываются.** Объект генерирует события только из `_Run`. Благодаря этому вся логика модулей и приложения исполняется в основном контексте и не требует синхронизации.
- В ISR запрещены ожидания, циклы опроса, вызовы `HAL_Delay`, `printf` и ресурсоёмких функций HAL.

### Передача данных из ISR

**Принцип единственного писателя (single writer).** Каждая разделяемая переменная модифицируется только одним контекстом — ISR либо `_Run`. При соблюдении этого принципа критическая секция не требуется.

### Требования к разделяемым полям

- Поле, модифицируемое в ISR, объявляется `volatile`. Без квалификатора компилятор вправе кэшировать значение в регистре, и `_Run` не увидит изменения.
- `volatile` **не обеспечивает атомарность**. Разрядность разделяемого поля не должна превышать разрядность шины/ядра МК: на 32-битном Cortex-M — до `uint32_t`, на 8-битном AVR — только `uint8_t`.
- Если данные невозможно разделить по принципу единственного писателя (несколько полей, изменяемых согласованно), применяется критическая секция. Она является аппаратной зависимостью и передаётся через дескриптор: поля `enterCritical` / `exitCritical`, реализации — `static` функции `enterCritical` / `exitCritical` в `Board.c`. Реализация обязана поддерживать вложенность (сохранять и восстанавливать `PRIMASK`, а не безусловно вызывать `__enable_irq()`). Внутри критической секции допускается только копирование данных.

### RTOS

Каждый объект принадлежит ровно одной задаче: `_Run` и все функции объекта вызываются только из её контекста. Межзадачный обмен выполняется средствами RTOS (очереди, task notifications). Функции RTOS также передаются через дескриптор и реализуются в `Board.c`.

---

## 7. Форматирование

- Отступ — символ табуляции.
- Открывающая фигурная скобка — на той же строке без пробела: `struct ModuleLED{`, `void ModuleLED_Run(ModuleLED_st* _moduleLed){`.
- Символ `*` указателя примыкает к типу: `ModuleLED_st* _moduleLed`.
- После ключевых слов `if`, `for`, `while`, `switch` пробел не ставится: `if(...)`.
- `else` размещается на новой строке после `}`:

  ```c
  if(_state == LED_STATE_ON){
  	HAL_GPIO_WritePin(LED_MAIN_GPIO_Port, LED_MAIN_Pin, GPIO_PIN_SET);
  }
  else{
  	HAL_GPIO_WritePin(LED_MAIN_GPIO_Port, LED_MAIN_Pin, GPIO_PIN_RESET);
  }
  ```

- В `switch` метки `case` на уровне отступа `switch`, тело каждой ветви заключено в фигурные скобки, `break;` внутри них; последней веткой идёт `default: break;`:

  ```c
  switch(_event){
  case MODULE_BUTTON_EVENT_CLICK:{
  	ModuleLED_TurnOn(&device->moduleLed);
  	break;
  }

  case MODULE_BUTTON_EVENT_HOLD:{
  	ModuleLED_TurnOff(&device->moduleLed);
  	break;
  }

  default: break;
  }
  ```

- Однородные строки (`#define`, объявления типов, поля структур, поля дескрипторов в инициализаторах) выравниваются по колонкам:

  ```c
  static uint32_t        getTick(void);
  static Button_State_et getButtonState(Button_st* _button);
  static void            setLedState(LED_st* _led, LED_State_et _state);
  ```

- Присваивание нового состояния конечного автомата (`state = ...;`) — последний оператор блока, отделённый пустой строкой.
- Секции обрамляются парными комментариями-маркерами `//-----Имя-----`.
