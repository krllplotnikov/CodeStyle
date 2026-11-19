# CodeStyle

Стиль кода для embedded-проектов на C/C++ (микроконтроллеры, прошивки, драйверы, библиотеки).

Главная идея простая: **код не должен знать, на каком железе он работает.** Всё, что касается конкретного микроконтроллера, живёт в одном месте — `main.c`. Остальной код получает доступ к железу только через указатели на функции. Благодаря этому:

- библиотеки и модули переносятся на другой МК без правок;
- логику можно покрыть unit-тестами на компьютере, подменив «железные» функции заглушками;
- проект легко читать: всегда понятно, где бизнес-логика, а где работа с периферией.

Живые примеры:
- библиотека — [LED](https://github.com/krllplotnikov-libraries/LED);
- проект целиком — [Terminal](https://github.com/krllplotnikov/Terminal).

---

## 1. Как устроен проект

За основу берутся файлы, которые сгенерировала IDE (например, CubeMX). Поверх них добавляются три папки, выстроенные слоями — от простого к сложному:

```
Project/
├── Libraries/      ← независимые библиотеки (LED, Button, RC522…), git submodules
├── Modules/        ← модули: кусочки бизнес-логики, собранные из библиотек
├── Applications/   ← приложение: связывает модули в готовый продукт
└── main.c          ← единственное место, где есть HAL и регистры
```

| Слой | Что делает | Знает о железе? |
|---|---|---|
| `Libraries/` | Решает одну узкую задачу (мигать светодиодом, читать кнопку). Подключается через **git submodules**, чтобы одна библиотека жила во многих проектах | Нет |
| `Modules/` | Реализует часть бизнес-логики (например, «зуммер пищит при событии») на основе библиотек | Нет |
| `Applications/` | Вся бизнес-логика продукта, объединяет модули | Нет |
| `main.c` | Пишет функции для работы с HAL и передаёт указатели на них вниз | **Да** |

### Объект и дескриптор

Каждая сущность (светодиод, кнопка, модуль) — это **структура-объект**. Чтобы объект заработал, ему при инициализации передают **дескриптор** — структуру с «внешними» данными и аппаратными функциями:

```c
LED_Descriptor_st ledDescriptor = {
	.setState = ledSetState,   // как включить/выключить светодиод
	.getTick  = HAL_GetTick,   // откуда брать время
};

LED_Init(&led, &ledDescriptor);
```

Все функции модуля первым параметром принимают указатель на свой объект: `LED_Run(&led)`. Поэтому одна библиотека спокойно обслуживает сколько угодно экземпляров.

---

## 2. Как называть вещи

Имя должно сразу говорить, *что это* и *откуда оно*. Поэтому почти везде есть префикс модуля, а у типов — суффикс.

| Что | Как | Пример |
|---|---|---|
| Файлы | `PascalCase`, аббревиатуры заглавными | `Button.h`, `LED.h` |
| Константы `#define` | `UPPER_SNAKE_CASE` с префиксом модуля | `LED_INDICATIONS_MAX_SIZE` |
| Перечисление | `Модуль_Имя` + суффикс `_et` | `LED_State_et` |
| Значения перечисления | `UPPER_SNAKE_CASE` с префиксом | `LED_STATE_OFF` |
| Структура | `Модуль_Имя` + суффикс `_st` | `LED_Descriptor_st` |
| Тип указателя на функцию | `Модуль_Имя`, без суффикса | `LED_GetTick` |
| Публичная функция | `Модуль_Действие` | `LED_SetIndication` |
| Приватная (`static`) функция | `camelCase`, без префикса | `getPlayerIdByCardUID` |
| Переменные и поля | `camelCase`, осмысленно | `indicationRepeatCounter` |
| Параметры функций | `camelCase` с `_` в начале | `_led`, `_repeatCount` |

Подчёркивание у параметров помогает внутри функции отличить «то, что пришло снаружи» от полей и локальных переменных.

```c
void LED_SetIndication(LED_st* _led, LED_Indications_et _indication, uint8_t _repeatCount);
```

**Модули и приложения:**
- модули получают префикс `Module`: папка `Modules/Buzzer/`, файлы `ModuleBuzzer.h/.c`, тип `ModuleBuzzer_st`, функции `ModuleBuzzer_SingleBeep(...)`;
- приложения называются по своему имени: `Applications/Terminal/Terminal.c`, `Terminal_st`, `Terminal_Run`.

**Функции без параметров — всегда с `(void)`.** В C пустые скобки `()` означают «аргументы неизвестны», и компилятор не проверит вызов:

```c
typedef uint32_t (*LED_GetTick)(void);
void Terminal_Reset(void);
```

---

## 3. Заголовочный файл (`.h`)

Все заголовки устроены одинаково — открываешь любой и сразу знаешь, где что искать. Порядок секций:

1. Include guard — `LED_H_`.
2. `#include` — все зависимости модуля.
3. `#define` — константы.
4. Предварительные объявления перечислений: `typedef enum LED_State LED_State_et;`
5. Предварительные объявления структур: `typedef struct LED LED_st;`
6. Типы указателей на функции.
7. Определения `enum`.
8. Определения `struct`.
9. Прототипы функций внутри `extern "C"` — чтобы библиотеку можно было подключить из C++.

Зачем предварительные объявления? Чтобы все типы были известны компилятору заранее. Например, тип колбэка может принимать `LED_st*`, хотя сама структура описана ниже.

<details>
<summary>Пример заголовка целиком</summary>

```c
#ifndef LED_H_
#define LED_H_

#include <stdio.h>
#include <stdint.h>

#define LED_INDICATIONS_MAX_SIZE       16
#define LED_INDICATION_INFINITE_REPEAT 255

typedef enum LED_Indications LED_Indications_et;
typedef enum LED_State       LED_State_et;

typedef struct LED_Indication    LED_Indication_st;
typedef struct LED_Descriptor    LED_Descriptor_st;
typedef struct LED               LED_st;

typedef void     (*LED_IndicationIsOverCallback) (LED_st* _led, LED_Indications_et _indication, uint8_t _repeatCounter);
typedef uint32_t (*LED_GetTick)                  (void);
typedef void     (*LED_SetState)                 (LED_st* _led, LED_State_et _state);

enum LED_Indications{
	LED_INDICATION_OFF,
	LED_INDICATION_ON,
	LED_INDICATION_SINGLE_BLINK,
	LED_INDICATION_DOUBLE_BLINK,
	LED_INDICATION_TRIPLE_BLINK,
	LED_INDICATION_CUSTOM
};

enum LED_State{
	LED_STATE_OFF,
	LED_STATE_ON
};

struct LED_Indication{
	uint8_t  states[LED_INDICATIONS_MAX_SIZE];
	uint32_t durations[LED_INDICATIONS_MAX_SIZE];
	uint8_t  size;
};

struct LED_Descriptor{
	LED_SetState                 setState;
	LED_GetTick                  getTick;
	LED_IndicationIsOverCallback indicationIsOverCallback;
};

struct LED{
	LED_Descriptor_st  descriptor;
	LED_Indications_et currentIndication;
	LED_Indication_st* customIndication;
	uint8_t            indicationRepeatCount;
	uint8_t            indicationRepeatCounter;
	uint8_t            indicationDescriptorCounter;
	uint32_t           currentTime;
};

#ifdef __cplusplus
extern "C" {
#endif

void LED_Init(LED_st* _led, LED_Descriptor_st* _descriptor);
void LED_Run(LED_st* _led);

#ifdef __cplusplus
}
#endif

#endif
```

</details>

---

## 4. Файл реализации (`.c`)

Порядок секций (лишние просто пропускаются):

1. `#include` **только своего** заголовка — остальные зависимости уже подключены в `.h`.
2. Данные модуля — константные таблицы, готовые наборы.
3. Указатель на экземпляр (для модулей и приложений, которым объект нужен в колбэках): `ModuleBuzzer_st* moduleBuzzerInstance = NULL;`
4. Прототипы `static` функций.
5. Публичные функции — в том же порядке, что в `.h`; первыми `_Init` и `_Run`.
6. Реализации чужих weak-колбэков, на которые подписан модуль.
7. Блок `//-----Weak-----` с пустыми реализациями собственных событий.
8. Тела `static` функций — в самом конце.

### Жизненный цикл объекта

- **`_Init(obj, descriptor)`** — копирует дескриптор в объект и явно задаёт **все** остальные поля. Никаких «и так будет ноль».
- **`_Run(obj)`** — вызывается постоянно из главного цикла или задачи RTOS. Она **никогда не блокирует**: никаких `HAL_Delay` и ожиданий. Время берётся из `getTick()`. `_Run` верхнего уровня вызывает `_Run` своих зависимостей.
- Необязательные колбэки перед вызовом проверяются на `NULL`.

### События через weak-функции

Модуль сообщает наружу о событиях через функции с атрибутом `weak`. В модуле лежит пустая реализация «по умолчанию», а приложение, которому событие интересно, просто определяет функцию с тем же именем — и линкер подставит её.

```c
#include "ModuleBuzzer.h"

ModuleBuzzer_st* moduleBuzzerInstance = NULL;

void ModuleBuzzer_Init(ModuleBuzzer_st* _moduleBuzzer, LED_st* _buzzer){
	moduleBuzzerInstance = _moduleBuzzer;
	_moduleBuzzer->buzzer = _buzzer;
}

void ModuleBuzzer_Run(ModuleBuzzer_st* _moduleBuzzer){
	LED_Run(_moduleBuzzer->buzzer);
}

void ModuleBuzzer_SingleBeep(ModuleBuzzer_st* _moduleBuzzer){
	LED_SetIndication(_moduleBuzzer->buzzer, LED_INDICATION_SINGLE_BLINK, 1);
}

//-----Weak-----
__attribute__((weak)) void ModuleBuzzer_IndicationIsOver(ModuleBuzzer_st* _moduleBuzzer, LED_Indications_et _indication, uint8_t _repeatCounter){}
//-----Weak-----
```

Подписчик проверяет, что событие пришло именно от его экземпляра:
`if(_moduleBuzzer == terminalInstance->moduleBuzzer)`.

---

## 5. `main.c` — единственное место с железом

Если `main.c` сгенерирован IDE, свой код пишется **только** внутри блоков `/* USER CODE BEGIN … */ … /* USER CODE END … */`, чтобы перегенерация ничего не стёрла.

В `USER CODE BEGIN 0` код разбит на секции с маркерами `//-----Имя-----` (между секциями — три пустые строки, внутри — подсекции по устройствам вроде `//-----Buzzer-----`):

| Секция | Что внутри |
|---|---|
| `//-----Variables-----` | Все объекты `= {0}`, выровнены по колонкам. Сначала библиотеки, потом модули, потом приложение |
| `//-----Function prototypes-----` | Прототипы аппаратных функций и колбэков |
| `//-----Descriptors-----` | Дескрипторы через designated initializers: `.getTick = HAL_GetTick,` |
| `//-----Function definitions-----` | Аппаратные функции через HAL. Если функция обслуживает несколько объектов — проверяет указатель: `if(_led == &buzzer)` |
| `//-----Callbacks-----` | Пробрасывают события библиотек в модули. **Бизнес-логики здесь нет** |

В `main()`:
1. после инициализации периферии вызываются `_Init` снизу вверх: библиотеки → модули → приложение;
2. в бесконечном цикле (или задаче RTOS) вызывается только `_Run` приложения: `Terminal_Run(&terminal);`.

---

## 6. Форматирование

- Отступы — табуляция.
- Фигурная скобка — на той же строке, без пробела: `struct LED{`, `void LED_Run(LED_st* _led){`.
- Звёздочка указателя — у типа: `LED_st* _led`.
- После `if`, `for`, `while`, `switch` пробела нет: `if(...)`.
- `else` — с новой строки после `}`:

  ```c
  if(state == LED_STATE_ON){
  	...
  }
  else{
  	...
  }
  ```

- В `switch` метки `case` на уровне `switch`, тело каждого `case` в фигурных скобках, `break;` внутри, в конце `default: break;`:

  ```c
  switch(terminalInstance->state){
  case TERMINAL_STATE_START:{
  	...
  	break;
  }

  default: break;
  }
  ```

- Однотипные строки (`#define`, объявления типов, поля структур) выравниваются по колонкам.
- Смена состояния (`terminalInstance->state = ...;`) — последней строкой блока, отделена пустой строкой.
- Секции отмечаются парными комментариями `//-----Имя-----` в начале и в конце.

---

## 7. Чек-лист для ревью

- [ ] К железу обращается только `main.c`, остальное получает его через дескриптор.
- [ ] У каждого объекта есть структура и дескриптор; первый параметр функций — указатель на объект.
- [ ] Имена соответствуют правилам: `_et` / `_st`, префикс модуля, `_` у параметров.
- [ ] Секции `.h` в правильном порядке, типы объявлены заранее, прототипы в `extern "C"`.
- [ ] Внешние библиотеки — git submodules в `Libraries/`.
- [ ] Функции без параметров объявлены с `(void)`.
- [ ] Секции `.c` в правильном порядке, `static` функции в `camelCase` и в конце файла.
- [ ] `_Init` задаёт все поля, `_Run` не блокирует, необязательные колбэки проверяются на `NULL`.
- [ ] События модулей — weak-функции в блоках `//-----Weak-----`.
- [ ] В `main.c` код только в `USER CODE`, разбит на секции, в колбэках нет бизнес-логики.
