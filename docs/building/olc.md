# OLC

Поваренная книга, или справочник по OLC _\(оригинал этой книги находится в мире в библиотеке на Улице Вязов\)_

 Старинная ужасно потрепаная рукопись. По всей видимости, ее не раз варили. Чернила растеклись, так что явно половина рецептов утрачена. На обложке красуется надпись большими зелеными буквами O L C. Чуть ниже кривым почерком 'здесь был Филдс'. В конце книги есть еще пору свободных страниц. Вероятно для того, чтобы всегда было можно ее дополнить.

 Если есть свободное время и нечего больше читать, эта книга поможет тебе узнать кое-что новое о мире кулинарии. Начинай прямо с введения \(см. [intro](olc.md#XreFintro)\).

 [Введение](olc.md#Введение)

 [Команда vnumck.](olc.md#Команда%20vnumck.)

 [Команда trap.](olc.md#Команда%20trap.)

 [Редактор строк.](olc.md#Редактор%20строк.)

 [Команда security.](olc.md#Команда%20security.)

 [Команда olcvnum.](olc.md#Команда%20olcvnum.)

 [Команда edit room. \(redit\)](olc.md#Команда%20edit%20room.%20%28redit%29)

 [Команда resets.](olc.md#Команда%20resets.)

 [Команда olchelp.](olc.md#Команда%20olchelp.)

 [Команда edit object. \(oedit\)](olc.md#Команда%20edit%20object.)

 [Команда edit mobile. \(medit\)](olc.md#Команда%20edit%20mobile.)

 [Команда ed.](olc.md#Команда%20ed.)

 [Команды east, north, south, west, up, down.](olc.md#Команды%20east,%20north,%20south,%20west)

 [Команда eexit.](olc.md#Команда%20eexit.)

 [Команда edit.](olc.md#Команда%20edit.)

 [Команда asave.](olc.md#Команда%20asave.)

 [Команда edit area. \(aedit\)](olc.md#Команда%20edit%20area.%20%28aedit%29)

 [Команда alist.](olc.md#Команда%20alist.)

 Введение

 Плагин, реализующий набор команд, предназначеных для редактирования арий.

 В различных состояниях набор команд отличается.

 Обычно командный интерпретатор находится в начальном состоянии. В этом состоянии доступны все стандартные команды.

 OLC реализует следующие команды доступные в начальном состоянии: 

 edit \(см. [edit](olc.md#XreFedit)\)

 resets \(см. [resets](olc.md#XreFresets)\)

 alist \(см. [alist](olc.md#XreFalist)\)

 security \(см. [security](olc.md#XreFsecurity)\)

 olcvnum \(см. [olcvnum](olc.md#XreFolcvnum)\)

 vnumck \(см. [vnumck](olc.md#XreFvnumck)\)

 olchelp \(см. [olchelp](olc.md#XreFolchelp)\)

 asave \(см. [asave](olc.md#XreFasave)\)

 При переходе из одного состояния в другое, предыдущее запоминается. В каждом из состояний предусмотрена возможность вернуться к предыдущему. К примеру, не завершив редактирования комнаты, можно начать редактировать объект. После завершения завершения редактирования объекта, продолжится редактирование комнаты.

 Команда vnumck.

 Синтаксис: 

 vnumck

 Проверить диапазон выделеных/используемых vnumов.

 Формат вывода: 

 `<min_vnum>-<max_vnum>: gap (<gap_size>)` свободный промежуток между эриями

 `<min_vnum>-<max_vnum>: <area_credits>: used: <used_min>-<used_max>` выделеный блок vnumов с `<min_vnum>` по `<max_vnum>`, из которых

 OLC устроено так, что нарушить границы выделения не удастся. Данная команда предназначена для проверки целостности диапазонов в случае, если эрия добавляется ввиде файла.

 Команда trap.

 Комада более не поддерживается. Все ловушки описываются с помощью [Fenia](about:blank).

 Редактор строк.

 Рекдактор строк похож на юниксовую команду ed. Хелп [тут](https://github.com/dreamland-mud/dreamland_code/tree/52bbc48f8ac46a4f8213ed1920a0bfd9f331730a/docs/ed.html).

 Команда security.

 Синтаксис: 

 security &lt;player&gt; &lt;число&gt;

 Устанавливает уровень привелегии чара в `<число>`. Чару необходимо иметь уровень привелегий больше 100, чтоб иметь право устанавливать привелегии другим.

 Команда olcvnum.

 Синтаксис: 

 olcvnum &lt;player&gt; show

 olcvnum &lt;player&gt; set &lt;min\_vnum&gt; &lt;max\_vnum&gt; &lt;security level&gt;

 olcvnum &lt;player&gt; del &lt;min\_vnum&gt; &lt;max\_vnum&gt;

 Устанавливает диапазоны внумов, которые может редактировать игрок. Для этих внумов ему будут разрешены medit, oedit, redit, asave area. Команда доступна чарам с уровнем привилегий больше 100. NB: поле `<security level>` обязательно, но пока нигде не используется.

 Команда edit room. \(redit\)

 Синтаксис: 

 edit room

 edit room &lt;vnum&gt;

 edit room create &lt;vnum&gt;

 edit room reset

 edit room show &lt;vnum&gt;

 Команда для редактирования комнат.

|  `edit room` |  переводит редактор в состояние редактирования комнаты в которой находится чар. |
| :--- | :--- |


|  `edit room <vnum>` |  переводит редактор в состояние редактирования комнаты с номером `<vnum>`. |
| :--- | :--- |


|  `edit room create <vnum>` |  создает новую комнату с номером `<vnum>` и переводит редактор в состояние редактирования новосозданной комнаты. |
| :--- | :--- |


|  `edit room create next` |  создает новую комнату со следующим свободным внумом |
| :--- | :--- |


|  `edit room reset` |  обновляет редактируемую комнату. |
| :--- | :--- |


|  `edit room show <vnum>` |  показывает информацию о комнате `<vnum>` |
| :--- | :--- |


 Следующие команды доступны только из состояния редактирования комнаты:

|  |  синоним edit room create `<vnum>` |
| :--- | :--- |


|  |  синоним edit room create next |
| :--- | :--- |


|  |  устанавливает описание, переводит редактор в состояние редактирования строки \(см. [string](olc.md#XreFstring)\) |
| :--- | :--- |


|  |  см. [extradescr](olc.md#XreFextradescr) |
| :--- | :--- |


|  |  форматировать описание |
| :--- | :--- |


|  |  установить имя комнаты |
| :--- | :--- |


|  |  показать инф-ю о редактируемой комнате |
| :--- | :--- |


|  |  синоним show |
| :--- | :--- |


|  |  скорость восстанавления hp. 100 - нормальная скорость |
| :--- | :--- |


|  |  скорость восстанавления mana. 100 - нормальная скорость |
| :--- | :--- |


|  |  см. [exits](olc.md#XreFexits) |
| :--- | :--- |


|  |  установить владельца |
| :--- | :--- |


|  |  установить клан |
| :--- | :--- |


|  |  установить vnum хранилища животных \(если не следующий\) |
| :--- | :--- |


|  |  см. [eexits](olc.md#XreFeexits) |
| :--- | :--- |


|  |  см. [traps](olc.md#XreFtraps) |
| :--- | :--- |


|  |  добавить мультирум. чары с левелом ниже `<level>` будут попадать в комнату `<vnum>` |
| :--- | :--- |


|  |  удалить мультирум. |
| :--- | :--- |


|  |  список мобов в этой эрии \(all - всех, имя - с заданым именем\) |
| :--- | :--- |


|  |  список объектов в этой эрии \( `all` - всех, `тип` - с заданым типом \(см. [olchelp](olc.md#XreFolchelp)\), `имя` - с заданым именем\) |
| :--- | :--- |


|  |  список комнат в этой эрии |
| :--- | :--- |


|  |  см. [olchelp](olc.md#XreFolchelp) |
| :--- | :--- |


|  |  см. [olchelp](olc.md#XreFolchelp) |
| :--- | :--- |


|  |  добавить ресет моба в редактируемой комнате. `<vnum>` - номер моба, `<mr>` - максимум мобов в комнате, `<mw>` - максимум мобов в мире. \(\*\) |
| :--- | :--- |


|  |  добавить ресет объекта в редактируемой комнате. `<vnum>` - номер объекта. объект будет ресетиться на полу. \(\*\) |
| :--- | :--- |


|  |  добавить ресет объекта в редактируемой комнате. `<vnum>` - номер объекта. `<строка>` - имя объекта, в котором должен ресетиться добавляемы. \(\*\) |
| :--- | :--- |


|  |  добавить ресет объекта в редактируемой комнате. объект с номером `<vnum>` будет надет на моба с именем `<строка>` на wear location `<wear-loc>`. \(\*\) \(см. [olchelp](olc.md#XreFolchelp)\) |
| :--- | :--- |


 Команды отмеченые как \(\*\) детально не отлаживались т.к. существует аналог с более широкой функциональностью \(см. [resets](olc.md#XreFresets)\).

 Команда resets.

 Синтаксис: 

 resets

 resets &lt;index&gt; obj &lt;vnum&gt; &lt;wear-loc&gt;

 resets &lt;index&gt; obj &lt;vnum&gt; inside &lt;vnum&gt; \[limit\] \[count\]

 resets &lt;index&gt; obj &lt;vnum&gt; room

 resets &lt;index&gt; mob &lt;vnum&gt; \[max\_world\] \[max\_room\]

 resets &lt;index&gt; delete

 Команда редактирования списка ресетов комнаты. Порядок ресетов имеет значение: если один объект должен попасть в другой, или в инвентарь к мобу, то моб, или контейнер должен быть описан первым. Порядок определяется индексом &lt;index&gt;.

 resets &lt;index&gt; obj &lt;vnum&gt; &lt;wear-loc&gt; добавить объект &lt;vnum&gt; в список ресетов инвентаря последнего моба. &lt;wear-loc&gt; определяе куда должен быть одет объект.

 resets &lt;index&gt; obj &lt;vnum&gt; inside &lt;vnum2&gt; \[limit\] \[count\] добавить объект &lt;vnum&gt; в список ресетов внутри объекта &lt;vnum2&gt;. максимальное число объектов в мире - \[limit\]. в контейнере - \[count\].

 resets &lt;index&gt; obj &lt;vnum&gt; room добавить объект &lt;vnum&gt; в список ресетов комнаты \(на полу\).

 resets &lt;index&gt; mob &lt;vnum&gt; \[max\_world\] \[max\_room\] добавить моб &lt;vnum&gt; в список ресетов комнаты. число мобов в мире - \[max\_world\]. в комнате - \[max\_room\]

 resets &lt;index&gt; delete удалить ресет с индексом &lt;index&gt;.

 Команда olchelp.

 Синтаксис: 

 olchelp

 olchelp &lt;таблица&gt; \[...\]

 Команда для посмотреть возможные значения флагов и перечислимых типов \(размер, аффект, пол, спелы, и т.п.\)

 olchelp без параметров показывает список названий таблиц и их описание.

 olchelp &lt;таблица&gt; \[...\] показывает возможные значения элементов таблицы. некоторые таблицы предполагают возможность ограничить вывод дополнительными ограничениями \(olchelp spells defend\).

 Команда edit object. \(oedit\)

 Синтаксис: 

 edit object &lt;vnum&gt;

 edit object create &lt;vnum&gt;

 edit object show &lt;vnum&gt;

 edit object load &lt;vnum&gt;

 Команда для редактирования объектов.

 edit object &lt;vnum&gt; редактировать объект с номером &lt;vnum&gt;. Переводит редактор в состояние редактирования объекта.

 edit object create &lt;vnum&gt; создать нового моба с номером &lt;vnum&gt;. Переводит редактор в состояние редактирования объекта.

 edit object show &lt;vnum&gt; показать информацию об объекте с номером &lt;vnum&gt;.

 edit object load &lt;vnum&gt; загрузить объект с номером &lt;vnum&gt;. Объект попадет к вам в руки, а если у него нет WEAR\_TAKE, то в комнату.

 В режиме редактирования объекта доступны следующие команды: 

 show показать информацию о редактируемом объекте.

 '' \(пустая строка\) синоним show

 create &lt;vnum&gt; синоним edit object create &lt;vnum&gt;

 addaffect &lt;apply&gt; &lt;число&gt; \[&lt;affwhere&gt; &lt;флаги&gt;\] добавить аффект. см. [olchelp](olc.md#XreFolchelp)

 delaffect &lt;число&gt; удалить аффект с индексом &lt;число&gt;

 ed ... см. [extradescr](olc.md#XreFextradescr)

 cost &lt;число&gt; цена

 long &lt;строка&gt; длинное описание \(look в комнате\)

 name &lt;строка&gt; на что откликается \(take &lt;строка\)

 short &lt;строка&gt; короткое описание \(Ты вворужешься &lt;строка&gt;\)

 v0, v1, v2, v3, v4 значениея, зависящие от типа объекта \(\*\)

 weight &lt;число&gt; вес

 extra &lt;extra&gt; экстра-флаги объекта \(см. [olchelp](olc.md#XreFolchelp)\)

 wear &lt;wear-loc&gt; куда одевается \(см. [olchelp](olc.md#XreFolchelp)\)

 type &lt;type&gt; тип объекта

 material &lt;строка&gt; из чего сделано

 level &lt;число&gt; уровень

 limit &lt;число&gt; лимит \(-1 не лимитный шмот\)

 condition &lt;число&gt; состояние. 100 - превосходное. 0 - ужасное

 list в каких комнатах ресетится

 \(\*\): Для каждого типа объекта. Для того, чтоб узнать какие из v\[0-4\] что означают, можно установить необходимый тип и сделать show.

 Команда edit mobile. \(medit\)

 Синтаксис: 

 edit mobile &lt;vnum&gt;

 edit mobile create &lt;vnum&gt;

 edit mobile show &lt;vnum&gt;

 edit mobile load &lt;vnum&gt;

 Команда для редактирования мобов.

 edit mobile &lt;vnum&gt; редактировать моба с номером &lt;vnum&gt;. Переводит редактор в состояние редактирования мобов.

 edit mobile create &lt;vnum&gt; создать нового моба с номером &lt;vnum&gt;. Переводит редактор в состояние редактирования мобов.

 edit mobile show &lt;vnum&gt; показать информацию о мобе с номером &lt;vnum&gt;.

 edit mobile load &lt;vnum&gt; загрузить монстра с номером &lt;vnum&gt;. Монстр появится рядом с вами.

 В режиме редактирования моба доступны следующие команды: 

 alignment &lt;число&gt; характер

 create &lt;vnum&gt; синоним edit mobile create &lt;vnum&gt;

 desc дескр. переводит в состояние редакторирования строки \(см. [string](olc.md#XreFstring)\)

 level &lt;число&gt; левел

 long &lt;строка&gt; длинное описание \(look в комнате\)

 name &lt;строка&gt; имя \(на что отзывается, kill &lt;строка&gt;\)

 shop hours &lt;число&gt; &lt;число&gt; время открытия/закрытия

 shop profit &lt;число&gt; &lt;число&gt; процент с продажи/покупки

 shop type &lt;число&gt; &lt;строка&gt; что покупаем \(&lt;число&gt; от 0 до 4, &lt;строка&gt; - item type\)

 shop delete &lt;число&gt; удалить что было объявлено в type. \(&lt;число&gt; от 0 до 4\)

 short &lt;строка&gt; короткое описание. \(&lt;строка&gt; ушел на север.\)

 show показать текущую инф-ю о мобе.

 '' \(пустая строка\) синоним show

 spec &lt;строка&gt; установить спец. процедуру. \(см. [olchelp](olc.md#XreFolchelp)\)

 sex &lt;sex&gt; пол \(см. [olchelp](olc.md#XreFolchelp)\)

 act &lt;act&gt; экт \(см. [olchelp](olc.md#XreFolchelp)\)

 affect &lt;affect&gt; аффекты \(см. [olchelp](olc.md#XreFolchelp)\)

 detection &lt;detection&gt; детекты \(см. [olchelp](olc.md#XreFolchelp)\)

 armor \[ac-pierce \[ac-bash \[ac-slash \[ac-exotic\]\]\]\] защита

 form &lt;form&gt; форма \(см. [olchelp](olc.md#XreFolchelp)\)

 part &lt;part&gt; части тела \(см. [olchelp](olc.md#XreFolchelp)\)

 imm &lt;immune&gt; иммунности \(см. [olchelp](olc.md#XreFolchelp)\)

 res &lt;resist&gt; стойкости \(см. [olchelp](olc.md#XreFolchelp)\)

 vuln &lt;vuln&gt; уязвимости \(см. [olchelp](olc.md#XreFolchelp)\)

 material &lt;строка&gt; материал \(см. [olchelp](olc.md#XreFolchelp)\)

 off &lt;offensive&gt; атакующие способности \(см. [olchelp](olc.md#XreFolchelp)\)

 size &lt;size&gt; размер \(см. [olchelp](olc.md#XreFolchelp)\)

 hitdice &lt;число&gt;d&lt;число&gt;+&lt;число&gt; hp

 manadice &lt;число&gt;d&lt;число&gt;+&lt;число&gt; mana

 damdice &lt;число&gt;d&lt;число&gt;+&lt;число&gt; damage

 race &lt;race&gt; расса \(см. [olchelp](olc.md#XreFolchelp)\)

 position start &lt;position&gt; стартовая позиция \(см. [olchelp](olc.md#XreFolchelp)\)

 position default &lt;position&gt; нормальная позиция \(см. [olchelp](olc.md#XreFolchelp)\)

 wealth &lt;число&gt; богатство

 group &lt;число&gt; группа

 practicer &lt;group&gt; может практиковать группу \(см. [olchelp](olc.md#XreFolchelp)\)

 hitroll &lt;число&gt; хитролл

 damtype &lt;damtype&gt; тип атаки \(см. [olchelp](olc.md#XreFolchelp)\)

 list список комнат где ресетится.

 Команда ed.

 Синтаксис: 

 ed add &lt;строка&gt;

 ed edit &lt;строка&gt;

 ed delete &lt;строка&gt;

 ed format &lt;строка&gt;

 Команда для редактирования дополнительных описаний комнат/объектов. Достопна исключительно из состояний редактирования комнат и объектов.

 ed add &lt;строка&gt; добавить описание с ключом &lt;строка&gt;. переводит редактор в состояниередактирования строки \(см. [string](olc.md#XreFstring)\)

 ed edit &lt;строка&gt; изменить описание с ключом &lt;строка&gt;. переводит редактор в состояниередактирования строки \(см. [string](olc.md#XreFstring)\)

 ed delete &lt;строка&gt; удаляет описание с ключом &lt;строка&gt;.

 ed format &lt;строка&gt; форматирует описание с ключом &lt;строка&gt;.

 Команды east, north, south, west, up, down.

 dir ::= &lt;east\|north\|south\|west\|up\|down&gt;

 Синтаксис: 

 &lt;dir&gt;

 &lt;dir&gt; &lt;exit\_flags&gt;

 &lt;dir&gt; ?

 &lt;dir&gt; delete

 &lt;dir&gt; link &lt;vnum&gt;

 &lt;dir&gt; dig &lt;vnum&gt;

 &lt;dir&gt; dig next

 &lt;dir&gt; room &lt;vnum&gt;

 &lt;dir&gt; key &lt;vnum&gt;

 &lt;dir&gt; name &lt;строка&gt;

 &lt;dir&gt; descr

 Команды предназначеные для редактирования стандартных выходов из комнаты. Доступны только из состояния редактирования комнаты. Исключение составляет первый вариант команды, который дублируется в стандартном наборе команд.

 &lt;dir&gt; переместить чара в направлении &lt;dir&gt;. Эта команда не изменяет состояние редактора. Вместо этого редактирование переходит к комнате в которую попал чар.

 &lt;dir&gt; &lt;exit\_flags&gt; изменить флаги выхода \(см. [olchelp](olc.md#XreFolchelp)\)

 &lt;dir&gt; ? синоним olchelp exit

 &lt;dir&gt; delete удалить двусторонний проход. если проход односторонний, или выход с другой стороны ведет в другую комнату, удаляется только односотронний проход.

 &lt;dir&gt; link &lt;vnum&gt; создать двусторонний проход с существующей комнатой &lt;vnum&gt;.

 &lt;dir&gt; dig &lt;vnum&gt; создать двусторонний проход в новую комнату. созданая комната будет иметь номер &lt;vnum&gt;.

 &lt;dir&gt; dig next создать двусторонний проход в новую комнату. созданая комната будет иметь следующий свободный номер.

 &lt;dir&gt; room &lt;vnum&gt; создать односторонний проход в существующую комнату &lt;vnum&gt;.

 &lt;dir&gt; key &lt;vnum&gt; установить ключ \(для двери\)

 &lt;dir&gt; name &lt;строка&gt; установить имя выхода \(для open door, lock door\)

 &lt;dir&gt; descr установить описание выхода \(для look north\). переводит редактор в состояние редактирования строки. \(см. [string](olc.md#XreFstring)\)

 Команда eexit.

 Синтаксис: 

 eexit &lt;keyword&gt; delete

 eexit &lt;keyword&gt; add

 eexit &lt;keyword&gt;

 Команда для редактирования экстравыходов. Доступна исключительно из состояния редактирования комнаты.

 eexit &lt;keyword&gt; delete удаляет экстравыход &lt;keyword&gt;.

 eexit &lt;keyword&gt; add добавляет экстравыход &lt;keyword&gt;. Эта команда переводит редактор в состояние редактирования экстравыхода.

 eexit &lt;keyword&gt; редактирует экстравыход &lt;keyword&gt;. Эта команда переводит редактор в состояние редактирования экстравыхода.

 Если название экстравыхода \(&lt;keyword&gt;\) должно состоять из нескольких слов, его необходимо брать в кавычки.

 В состоянии редактирования экстравыходов доступны следующие команды: 

 show показать текущую инф-ю о редактируемом экстравыходе.

 '' \(пустая строка\) синоним show

 desc редактировать описание. \(для look keyword\). переводит редактор в состояние редактирования строки. \(см. [string](olc.md#XreFstring)\)

 rdesc редактировать описание комнаты. \(добавляется к описанию комнаты при look, если чар может видеть этот экстравыход\). переводит редактор в состояние редактирования строки. \(см. [string](olc.md#XreFstring)\)

 name &lt;строка&gt; изменить ключевое слово.

 key &lt;vnum&gt; установить ключ.

 target &lt;vnum&gt; установить комнату назначения.

 from &lt;число1&gt; &lt;число2&gt; &lt;строка&gt; сообщение тем, кто остался в комнате, из которой ушел чар. \(\*\)

 to &lt;число1&gt; &lt;число2&gt; &lt;строка&gt; сообщение тем, кто находился в комнате, в которую пришел чар. \(\*\)

 &lt;eexit\_flags&gt; см. [olchelp](olc.md#XreFolchelp)

 &lt;size&gt; установить максимальный размер чара, способного пройти в этот экстравыход. \(см. [olchelp](olc.md#XreFolchelp)\)

 \(\*\): Сообщение для from формируется таким образом:

```
 <ru> <rt> <short>. 
```

 Сообщение для to формируется таким образом:

```
 <rp> <rt> <short>. 
```

 здесь: 

 &lt;ru&gt; задается &lt;числом1&gt; из команды from,

 &lt;rp&gt; - &lt;числом1&gt; из команды to,

 &lt;rt&gt; - &lt;числом2&gt; одинакого для from и to,

 &lt;short&gt; - &lt;строка&gt; одинакого для from и to.

|  &lt;ru&gt; |  &lt;rp&gt; |  &lt;rt&gt; |
| :--- | :--- | :--- |


|  0 |  ушел |  0 |  пришел |  0 |  в |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  1 |  взобрался |  1 |  забрался |  1 |  на |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  2 |  запрыгнул |  2 |  запрыгнул |  2 |  сквозь |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  3 |  бросился |  3 |  упал |  3 |  между |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  4 |  нырнул |  4 |  донырнул |  4 |  над |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  5 |  уплыл |  5 |  приплыл |  5 |  через |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  6 |  всплыл |  6 |  всплыл |  6 |  под |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  7 |  протиснулся |  7 |  протиснулся |  7 |  с |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  8 |  улетел |  8 |  прилетел |  8 |  из |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  9 |  спрыгнул |  9 |  спрыгнул |  9 |  со |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  10 |  слез |  10 |  слез |  10 |  из под |
| :--- | :--- | :--- | :--- | :--- | :--- |


|  11 |  спустился |  11 |  спустился |  11 |  по |
| :--- | :--- | :--- | :--- | :--- | :--- |


 Бред какой-то. Это вместо того, чтоб хранить просто строку act\_p. \= Может рыжая переделает када-то.

 Команда edit.

 Синтаксис: 

 edit area ... \(aedit ...\)

 edit room ... \(redit ...\)

 edit mobile ... \(medit ...\)

 edit object ... \(oedit ...\)

 Предназначена для смены состояния редактора. \(переход к редактированию чего либо\).

 Для краткости, команда edit имеет ряд синонимов \(aedit, redit, medit, oedit\).

 В каждом из состояний будут доступны следующие команды: 

 version показать версию OLC

 commands показать список команд доступных в этом состоянии \(кроме обычных команд\)

 ? синоним команды olchelp \(см. [olchelp](olc.md#XreFolchelp)\)

 done вернуться в предыдущее состояние.

 Для подробностей см. [area](olc.md#XreFarea), [room](olc.md#XreFroom), [mobile](olc.md#XreFmobile), [object](olc.md#XreFobject).

 Команда asave.

 Синтаксис: 

 asave &lt;vnum&gt;

 asave list

 asave area

 asave changed

 asave world

 Команда для сохранения эрий на диск.

 asave &lt;vnum&gt; сохранить одну эрию с номером &lt;vnum&gt;. номер можно узнать из alist

 asave list сохранить только список эрий

 asave area сохранить эрию в которой находится чар

 asave changed сохранить измененные эрии

 asave world сохранить все эрии

 Команда edit area. \(aedit\)

 Синтаксис: 

 edit area

 edit area &lt;номер&gt;

 edit area create

 edit area без аргументов переводит редактор в состояние редактирования эрии в которой находится чар.

 edit area &lt;номер&gt; переводит редактор в состояние редактирования эрии с номером &lt;номер&gt;. Номер существующий эрии можно узнать с помощью команд alist. \(см. [alist](olc.md#XreFalist)\)

 edit area create создает новую эрию со следующим свободным номером и переводит редактор в состояние редактирования новосозданной эрии.

 Следующие команды доступны только из состояния редактирования эрии: 

 age &lt;число&gt; установить возраст эрии.

 builder &lt;чар1 чар2 ...&gt; установить список редакторов. \(\*\)

 create синоним edit area create.

 filename &lt;строка&gt; установить имя файла эрии.

 name &lt;строка&gt; установить имя эрии.

 reset обновить эрию \(repop\).

 security &lt;число&gt; установить необходимый уровень приврлегий. \(\*\)

 show показать текущую ин-ю о редактируемой эрии.

 '' \(пустая строка\) синоним show.

 vnum &lt;число&gt; &lt;число&gt; установить диапазон vnum'ов.

 lvnum &lt;число&gt; установить нижнюю границу диапазона vnum'ов.

 uvnum &lt;число&gt; установить верхнюю границу диапазона vnum'ов.

 levels &lt;число&gt; &lt;число&gt; установить рекомендуемый диапазон левелов.

 credits &lt;строка&gt; установить строку описания.

 resetmsg &lt;строка&gt; установить reset message.

 flag &lt;area\_flags&gt; установить флаги \(см. [olchelp](olc.md#XreFolchelp)\)

 \(\*\): не поддерживается из-за совместимости со старым форматом эрий.

 Команда alist.

 Синтаксис: 

 alist

 Команда для посмотреть номера эрий. \(см. [area](olc.md#XreFarea)\)
