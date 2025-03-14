# Веб-сервер ESP8266 & P10 PAnel

## Описание
Этот проект реализует подключение и взаимодействие через веб-сервер ESP8266 и LED панели P10 4*3

## Возможности
- Обработка веб-запросов
- Изменение текста на матрице
- Реализовано 4 режима работы матрицы
- Вывод инвормации в JSON-формате 

## Используемые библиотеки
- ESP8266WiFi
- Ticker
- DMD2
- ESPAsyncWebServer
- ESPAsyncTCP
- ArduinoJSON

## Установка
Клонируйте репозиторий:
```sh 
git clone https://github.com/maksb2bb/LCD_P10
```

Установите недостающие библиотеки и загрузите код на ESP8266

## Путь и тело запроса

**GET /api/led**

Возвращает текущее состояние панели 

**POST /api/text**

Принимает текст отображающийся на панели
Пример запроса:
```json
{
    "text":"Hello \nfrom \nGitTea"
}
```
**POST /api/led**

Изменяет состояние панели (Включение/выключение) и изменяет режим работы панели
Пример запроса:
Запрос на включение/выключение:
```json
{
    "panel": "on"
}
```
```json
{
    "panel": "off"
}
```
Запрос на изменение режима: (state : 1-4)
```json
{
    "state": "1"
}
```
Запросы на включение и изменение режима могут приниматься вместе:
```json
{
    "panel": "on", 
    "state": "3"
}
```
## Изменение рисунка
Изображение должно быть размером 128*64 в ч/б виде

```sh 
cd data/ && python3 main.py --input [Путь к изображению]
```
После выполнения скрипта он созраст в директории data файл output.h скомпилируйте код и загрузите в контроллер
