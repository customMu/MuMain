# Работа удалённо (без ПК) — подсказка

Файл одинаковый по смыслу во всех трёх репозиториях проекта; в конце — раздел про этот репозиторий.
Главная подробная памятка (`CLAUDE.md` в папке `mu\` на ПК) **в git не лежит** — здесь её выжимка для работы
с телефона/ноутбука, в облачной сессии Claude Code или на GitHub.

## Проект целиком

MU Online (Season 6) со своим балансом: 50 ресетов, мобы 1–400, ранги вещей, свои плагины дропа, сайт с вики.

| Репозиторий | Что | Ветка | Upstream (только fetch, никогда push) |
|---|---|---|---|
| [customMu/OpenMU](https://github.com/customMu/OpenMU) | сервер, C#/.NET 10 | `master` | MUnique/OpenMU `master` |
| [customMu/MuMain](https://github.com/customMu/MuMain) | клиент, C++ (SDL3) + C# сетевой слой | `main` | sven-n/MuMain `main` |
| [customMu/MU-Portal](https://github.com/customMu/MU-Portal) | сайт «Tales of Kundun», ASP.NET Razor Pages | `main` | — |

Папка на ПК (`Desktop\mu\`):

```
mu\
├── Client\customclient\MuMain\   клиент (репо MuMain)
├── Server\OpenMU-master\          сервер (репо OpenMU)
├── Server\*.sql                    SQL-правки живых данных (НЕ в git)
├── Portal\                         сайт (репо MU-Portal)
├── tools\balance\                 Python-генераторы баланса и SQL (НЕ в git): mob_layout.py, weapon_ranks.py,
│                                   item_drop_sql.py, skill_books.py, boss_drop_sql.py, spots_sql.py, wiki_items.py …
├── backup\                         дампы БД перед каждой правкой (НЕ в git)
├── mu-wiki.md                      целевой дизайн для игроков (русский исходник)
├── mu-server-concept.md            решения и статусы
├── mu-gm-commands.md               GM-команды
└── CLAUDE.md                       полная памятка по сессиям (НЕ в git)
```

## Чего удалённо НЕТ

- **Живой БД.** PostgreSQL в Docker на ПК (контейнер `database`, база `openmu`). Все рейты, шансы дропа, уровни/HP мобов,
  защита вещей, **настройки плагинов** живут в БД, а не в коде. Сид-инициализаторы (`src/Persistence/Initialization`) и
  значения по умолчанию в `*Configuration.cs` плагинов действуют только для новой БД / сброшенного конфига.
  ⇒ Правка дефолта в коде **не меняет игру**: нужен SQL на ПК (`config."PlugInConfiguration"."CustomConfiguration"`,
  JSON, коллекции в формате `{"$id": "", "$values": [...]}`; сервер держит конфиг в памяти — применять при остановленном сервере).
- **Генераторов `tools\balance` и SQL-файлов** — их запускают на ПК.
- **Сборки клиента** (Windows, MSVC, vcpkg) и игровых данных клиента (`Data\`).
- **Секретов**: API-ключ AdminPanel и пароли — только на ПК (`appsettings.Secrets.local.json` и т. п.). В git их не писать —
  репозитории публичные.

Поэтому удалённо: правим код и документацию, а в описании коммита/PR пишем, **что сделать на ПК**
(какой SQL, какой генератор запустить, что пересобрать и перезапустить).

## Правила проекта (коротко)

- **Уровни мобов — только по нашей лестнице 1–400** (с картой и ступенью ресетов: «Balrog — 130, Lost Tower 7, 5 ресетов»),
  не оригинальные уровни Webzen. Мобы выше 255 не попадают в родные группы дропа (уровень хранится в `byte`).
- **Любое видимое игроку изменение** (рейты, шансы, формулы, цены, ивенты, ресеты) — сразу в вики сайта:
  `Portal/src/MuPortal/Wiki/NN-slug.md`, **на английском**, числа в английском формате (`0.4%`, `1,500`).
- **Игровые названия не переводятся** нигде (карты, мобы, NPC, ивенты, предметы, скиллы, классы, статы): Lorencia, Kundun,
  Jewel of Bless, Blood Castle, Master Level …
- **Протокол клиент ↔ сервер**: меняя пакет, править **оба репо одновременно** (свои пакеты — группа `FB`: 03/0F/10/11 режим
  дропа пати, 06 споты миникарты и др.).
- **Копии данных в клиенте** — синхронизировать с сервером/БД: `GameLogic/Items/ArmorRanks.h`, `WeaponRanks.h`,
  `ItemResetRequirements.h`, `RepairPrice.h`, `PotionCooldown.h`, `SetGuard.*`, `GameLogic/Character/ResetBoost.h`,
  `GameLogic/Travel/TravelRequirements.h`, `GameLogic/Quests/ClassChangeQuests.h`, `GameLogic/MuHelper/HelperZenFee.h`;
  `GameLogic/Monsters/MonsterLevels.h` генерируется из БД (`tools\balance\monster_levels_client.py`).
- **Сайт повторяет логику плагинов дропа** (`Portal/src/MuPortal/Services/GameDatabaseBuilder.cs` — страницы `/Database`
  строятся из живой БД): меняя расчёт шанса в плагине сервера, поправить и его.
- Код — по стилю окружающего (StyleCop на сервере, `docs/CODING_RULES.md` в клиенте), комментарии и строки — английский,
  строки игроку — только `en` ресурсы (`PlugInResources`, `PlayerMessage`, `Game.en.resx`), другие локали не трогать.
- Коммит и пуш — по команде владельца; `upstream` — только `fetch`.

## Вернулся к ПК — чек-лист

1. `git pull` во всех трёх репо.
2. Выполнить SQL / генераторы, описанные в коммитах (сначала бэкап БД: `pg_dump` в `mu\backup\`).
3. Сервер: остановить → пересобрать в VS (`src\MUnique.OpenMU.sln`) → запустить; новые Data updates — AdminPanel → Updates.
4. Клиент: закрыть игру → VS, `x64 Release`, Ctrl+Shift+B (полный ребилд — только если начались краши `c0000374`).
5. Сайт: перезапустить (`dotnet run --urls http://localhost:5080` в `Portal\src\MuPortal`).
6. Дописать итог в `mu\CLAUDE.md` (раздел сессий).

## Этот репозиторий — клиент (MuMain)

- Сборка только на Windows: CMake-пресет `windows-x64` + vcpkg + Ninja Multi-Config, VS открывать как папку.
  Удалённо собрать нельзя — патчи идут «не собрано», собирают на ПК.
- Конвенции — `AGENTS.md` → `docs/CODING_RULES.md`.
- Наши правки в основном в `src/source/GameLogic/` (ранги вещей, ресеты, Set Guard, ремонт, откат банок, уровни мобов,
  квесты смены класса, варпы по ресетам, MU Helper со статистикой) и UI `src/source/UI/NewUI/` (окно C, M, T, P, миникарта со спотами).
- Строки — `src/Localization/Game.en.resx` (генерируются в `I18N::Game::…` при сборке); остальные локали не трогать.
- Сетевой слой — `ClientLibrary/` (C#): новые пакеты клиент → сервер — `ConnectionManager.ClientToServerFunctions.cs`;
  разбор входящих — `src/source/Network/Server/WSclient.cpp`. Пакет менять вместе с сервером.
- После правки заголовков краши `c0000374` лечатся полным ребилдом (Delete Cache and Reconfigure → Rebuild All).
