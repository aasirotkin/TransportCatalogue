### Транспортный справочник.

Программа поддерживает:
- графический вывод маршрутов в формате svg;
- поиск оптимальных по времени маршрутов;
- вычисление времени маршрутов;
- сериализацию и десериализацию транспортного каталога;
- обработку запросов в формате JSON.

---

### Установка Protobuf

Для того, чтобы собрать проект требуется установить Google Protocol Buffers (Protobuf).

Для этого необходимо:
- Скачать Protobuf можно с [репозитория на GitHub](https://github.com/protocolbuffers/protobuf/releases), Нужно выбрать архив protobuf-cpp с исходным кодом последней версии и распаковать его на своём компьютере в папку <путь к protobuf>;
- Создать папки build-debug и build-release для сборки двух конфигураций Protobuf. Если вы используете Visual Studio, будет достаточно одной папки build;
- Создать папку, в которой будет размещён пакет Protobuf. Будем называть её ***/path/to/protobuf/package***;

Если Вы работаете **НЕ** в Visual Studio, то в папке build_debug необходимо выполнить следующие команды:
1. cmake <путь к protobuf>/cmake с дополнительными параметрами:
    - -DCMAKE_BUILD_TYPE=Debug (чтобы задать тип сборки)
    - -Dprotobuf_BUILD_TESTS=OFF (чтобы не тратить время на сборку тестов)
    - -DCMAKE_INSTALL_PREFIX=***/path/to/protobuf/package*** (чтобы сообщить, где нужно будет создать пакет Protobuf)
2. cmake --build .
3. cmake --install .

Для конфигурации Release необходимо проделать теже самые шаги, только в папке build-release и с параметром -DCMAKE_BUILD_TYPE=Release.

Если Вы рабоатете **В** Visual Studio, то в папке build необходимо выполнить следующие команды:
1. cmake <путь к protobuf>/cmake с дополнительными параметрами:
    - -Dprotobuf_BUILD_TESTS=OFF (чтобы не тратить время на сборку тестов)
    - -DCMAKE_INSTALL_PREFIX=***/path/to/protobuf/package*** (чтобы сообщить, где нужно будет создать пакет Protobuf)
    - -Dprotobuf_MSVC_STATIC_RUNTIME=OFF (для правильного выбора Runtime-библиотек)
2. cmake --build . --config Debug
3. cmake --install . --config Debug

Для конфигурации Release необходимо проделать теже самые шаги, только в пунктах 2 и 3 с ключевым словом Release.

После выполненных комманд CMake скопирует все необходимые файлы в заранее подготовленное место ***/path/to/protobuf/package***.

Далее, при попытке запуска проекта, cmake не сможет найти Protobuf, для того, чтобы ему помочь, нужно указать где находится ***/path/to/protobuf/package*** задав переменную -DCMAKE_PREFIX_PATH=***/path/to/protobuf/package***.
