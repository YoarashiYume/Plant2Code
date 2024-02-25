# Plant2Code

Библиотека для преобразования plantuml кода в исполняемый С-код для проверки работоспособности устройств/агрегатов.

Процесс переноса
- [x] Считывание CSV-таблиц с сигналами
	- [x] Пропуск резервных данных
	- [x] Окончание таблиц резервным сигналом или пустой строкой
- [ ] Считывание plantuml-кода
	- [x] Допуск пустых строк после @enduml
	- [ ] Разбор plantuml на составные блоки
		- [x] Игнорирование "служебных" строк
		- [x] Блоки комментариев
		- [x] Блок вычисления
			- [x] Обработка вложенных комментариев
			- [x] Раскрытие блоков "..."
		- [ ] Блок вызова функции
			- [ ] Обработка вложенных комментариев
		- [ ] Блоки блоки ссылок
			- [ ] Блок начала алгоритма
			- [ ] Блоки окончания алгоритма
			- [ ] Блоки блок "goto"
		- [ ] Служебные блоки
			- [ ] if-else + elseif
			- [ ] switch-case
			- [ ] while
	- [ ] Считывание шапки алгоритма
- [ ] Проверка сигналов plantuml-кода
- [ ] Проверка корректности plantuml-кода
- [ ] Генератор С-кода

## Исходные данные

### CSV-таблицы

Используются для заполнения информации о сигналах.

Библиотека предусматривает использование:
- Входных сигналов.
- Выходных сигналов.
- Внутренних сигналов.
- Констант.
- Таймеров.
- Произвольных типов данных (структур).

Сигналы могут быть представлены в одной CSV-таблице (не реализовано в прошлой версии). Определение типа сигнала посходит по префиксу названия, которые задаются в при настройке считывающего класса (не реализовано в прошлой версии. Используется привязка к конкретному префиксу сигнала.). 

Колонки, содержащие необходимые атрибуты также определяются при настройке считывающего класса (не реализовано в прошлой версии. Используется привязка к конкретному названию столбца.).

Дальнейшее описание основывается на текущей реализации, при изменении/доработке дальнейший текст не действителен.

#### Возможные столбцы таблицы

- Название переменной ("Обозначение", "Обозначение состояния", "Обозначение сигнала")
- Текстовое описание ( "Наименование", "Название сигнала", "Название состояния)
    - Данные с описанием "Резерв" игнорируются 
- Тип переменной ("Тип данных")
- Значение переменной ("Значение")

#### Префиксы данных

Использующиеся префиксы:
- Входных сигналов - X.
- Выходных сигналов - Y.
- Внутренних сигналов - Z.
- Констант - C.
- Таймеров - T.
- Произвольных типов данных (струтур) - S.


#### Содержание CSV-таблицы с сигналами

- Колонка с типом данных:
    - Используются произвольно-задаваемые типы (Работа существующего варианта предусматривает типы: (U)Int8, (U)Int16, (U)Int32, (U)Int64 и float).
    - Допускает использование массивов Int8[<Размер>].
- Текстовое описание сигнала (опционально).
- Название переменной:
    - Допускает использование индекса массива Int8[0], при объявлении массива "по-кускам". При подобном объявлении массив в типе данных не указывается.
    - Допускает использование полного размера, при объявления массива за раз <Название сигнала>[0..2].При подобном объявлении массив в типе данных указывается (в данном случае <Размер> = 3).

#### Содержание CSV-таблицы со структурами

См. Содержание CSV-таблицы с сигналами.

Дополнительно допускает объявление типа структуры, при котором поля с типом данных не заполняется.

#### Содержание CSV-таблицы с константами

- Текстовое описание константы (опционально).
- Название переменной. Использование массивов не допускается.
- Значение переменной (предполагается использование численных констант. + Вычисления, которые можно провести в С).

#### Содержание CSV-таблицы с таймерами

- Текстовое описание сигнала (опционально).
- Название переменной. Использование массивов не допускается.

#### Примеры таблиц

См example/CSV/

### PlantUml

Предполагается, что plantuml написан корректно.

#### Игнорируемые строки

Игнорируются строки начинаемые с:
- !
- @

#### Комментарии plantuml

Комментарии (как однострочные, так и многострочные) сохраняются для дальнейшего переноса в C-код. (Не реализовано в старой версии).
При необходимости задаются комментарии (задаются без символов комментария), которые игнорируются. Игнорируемые комментарии
- blank

Для сопроводительных комментариев внутри блоков используется директива \$comment(). (В старой версии подобный комментарий распространяется исключительно на блоки вычисления, где комментарий выделяется директивами \$comment_start и \$comment_end)
```
!function $comment()
!return ""
!endfunction
```

#### Блоки вычисления (:])

- Строка/строки до первого вычисления считаются комментариями и НЕ выделяются директивой \$comment()
- Комментарии не должны содержать символ '='
- Допускается использование вложенных комментариев
- Каждое новое вычисление должно начинаться с переноса строки
- Внутри вычислений (после символа '=') не должно содержаться символа '='
	- Т.е вычисление L1 = (L0 = 2) + 1 должно записываться как :L0 = 2\nL1 = L0 + 1]
- Допускается использование символа "..."
	- Допускается изменение только одного параметра (оказалось невостребованным в прошлой версии)
	- Изменение допускается только для l-value (оказалось невостребованным в прошлой версии)
	- При использовании с переменными символ "..." размещается на отдельной строке (см. примеры)
	- При использовании в массиве символ "..." размещается внутри [0...2] (см. примеры)

#### Блоки функций (:\|)
- Допускается вызывать несколько функций в одном блоке (не реализовано в старой версии)
- В сопроводительных комментариях использование \$comment() опционально
- Структура вызова:
```
	:<Комментарий>"Название функции (arg1, arg2, ...) -> (return1, return2, ...)"|
```
- Комментарий - опциональная часть
- Если функция не имеет аргументов, то для вызова применяется только "Название функции"
- В аргументах не должно быть вычислений

#### Примеры таблиц

См example/plantuml/
### Генератор C-кода