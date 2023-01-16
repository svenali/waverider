# waverider
 DAB+/Internet Radio Server.
 
 Для других языков, нажмите ссылку: [Английский](README.md), [Немецкий](README.de.md).

 Waverider является ПО для радиоприема в диапазоне DAB+ (Digital Audio Broadcasting) и через интернет. Waverider хочет убрать все отрицательные своиства других приборов. К тому относится даже записание полученной программой (на пример музыки), как в тогдашних времменён, когда все люди записали их музыку на кассете. Это ПО в сравнении [Welle.io](https://www.welle.io/) по другому работает, предоставляет потребителем WebGUI, т. е. управление **waverider** будет через браузер возможно и потребител может управлять севером в роде пульта дистанционного управления.    
 Музыка будет возпроизводена через стереоаппаратуру или, если ПО (сервер) **waverider** будет управлен браузерером, через колонки компьютера. 
 Кроме того радио программа, не важно ли интернет или DAB+, может записана в разных форматах
 Zusätzlich kann das Radioprogramm, egal ob Internetsender oder DAB+, in vielen gängigen Formaten (AAC, MP3, FLAC, WAV, etc.). Я использую для ПО **waverider** Raspberry PI с расширенией HiFiBerry. Там я установил Headless System (Raspbian).   

## Условия

### Аппаратное обеспечение

 Эти здесь насчитанные части соответствуют только личный совет:

 - [Raspian PI Version 4](https://www.berrybase.de/raspberry-pi-4)
 - [HifiBerry dac+](https://www.reichelt.de/raspberry-pi-shield-hifiberry-digi-pro-rpi-hb-digi-pro-p191035.html?PROVID=2788&gclid=Cj0KCQiAiJSeBhCCARIsAHnAzT__QmJPWgV-ErtblZ-7ycyYZwIkmJqCKKA4leR8-YvK2ETBWSpr_3AaAh9kEALw_wcB)
 - [HiFiBerry+ Metall Gehäuse](https://www.amazon.de/HiFiBerry-Digi-Metall-Raspberry-schwarz-Black/dp/B08YDNJVRL/ref=sr_1_2?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&crid=2NMDXL3KY4TLW&keywords=HIFI+berry%2B+metall+case&qid=1673864322&sprefix=hifi+berry%2B+metall+case%2Caps%2C84&sr=8-2).
 - [DVB-T/DAB+ USB Stick](https://www.amazon.de/DollaTek-Digitale-Fernsehtuner-Empf%C3%A4nger-Unterst%C3%BCtzung/dp/B07DJT5NHD/ref=sr_1_5?keywords=dvb-t+stick+usb&qid=1673864429&sprefix=dvb-t+st%2Caps%2C87&sr=8-5)

 На самом деле **waverider** адресовано всем, исползующим операционную систему Linux или Unix или Mac на своём устройстве. USB прибор для DAB+ не обязано. USB прибор может и установлено на другом устройстве и может исползовано через другой компьютер. Это имеет смысл, если например  радиоприем на другом месте лучше. 

### ПО

 Вам необходимо установить вот эти дополнительные ПО и библиотеки для **waverider**:

 - Для компиляции [cmake](https://cmake.org/)
 - Для WebGUI [Wt](https://www.webtoolkit.eu/wt)
 - MP3 библиотека [libmpg123](https://www.mpg123.de/)
 - Для всех услуг (больше Codecs, etc.) [FFMPEG](https://ffmpeg.org/download.html)
 - [boost] (https://www.boost.org/)
 - [rtlsdr] (https://github.com/osmocom/rtl-sdr)
 - Для радиоприема DAB+ [libfaad](https://wiki.videolan.org/FAAD2_and_FAAC/)
 - Для радиоприеме DAB+ Radio [FFTW](https://www.fftw.org/)
 - Для выход музыки через стереоаппаратуру: [Alsa](https://www.alsa-project.org/wiki/Main_Page)

## Установление

### Установить условия

 С самом сначала установить все библиотеки. На устройстве Mac [Homebrew](https://brew.sh/) будет советован. 
 
 На Raspian или Debian Linux установление следующих ПО достаточно:

 ```
 sudo apt-get install alsa-utils
 sudo apt-get install cmake
 sudo apt-get install ffmpeg
 sudo apt-get install libboost1.74-all-dev
 sudo apt-get install libfaad2
 sudo apt-get install libfaad-dev
 sudo apt-get install libfftw3-3
 sudo apt-get install libfftw3-dev
 sudo apt-get install libssl-dev
 ```

 На debian GUI wt доступен и может установлен вот так: 

 ```
 sudo apt-get install witty witty-dev witty-doc witty-dbg
 ```

 На raspbian к сожалению это не так, вам надо установить **wt** ручной.

 ```
 wget https://github.com/emweb/wt/archive/4.8.3.tgz
 ```

 Распакуйте файл, и затем:
 ```
 cmake .
 make
 make install
 ```

 В случае указания ошибки, добавите следующие строчку в файл CMakeList.txt:
 ```
 set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic") 
 ```
 и повторйте компиляция ещё раз.

 Для компиляции **waverider** на MacOS, эта строчка написана в терминал может стать необходимо:

 ```
 export LIBRARY_PATH="/usr/local/opt/librtlsdr/lib:/usr/local/opt/mpg123/lib:/usr/local/opt/faad2/lib:/usr/local/opt/fftw/lib:/usr/local/lib"
 ```

### ПО Waverider установить

 ```
 wget https://github.com/svenali/waverider/archive/wr-0.9.tar.bz2
 cd wr-0.9
 mkdir build
 cd build
 cmake ../
 make
 ```

## Быстрый вход

### waverider стартовать

 Стартуйте **waverider** так:

 ```
 ./waverider --approot=../approot --docroot=../docroot --http-listen 0.0.0.0:9090
 ```
 Писать скрипт будет советовано (например wr.sh):

 ```
 #!/bin/bash
 ./waverider --approot=../approot --docroot=../docroot --http-listen 0.0.0.0:9090 &
 ```

### waverider управлять
 
 Сейчас пишйте в браузере в адресную строчку адрес прибора, где **waverider** было статовано.Например:

 ```
 http://pi4:9090
 ```

 Затем сайт должен быть на экране видно (см. в следующую картинку).

 <img src="screenshots/Ansicht.png" width="640px" height="auto">

 На верху, на правой стороне пожалуйста, нажмите на ссылку настроек и исправите ваши настроика так, как вам необходимо:

 <img src="screenshots/Settings.png" width="400px" height="auto">

 На самом деле исправление записанной папки дотаточно. Кроме того здесь можно настроить Codecs для записания музыки. На сличае интернет радиоприема советовано остаться Codec станции, чтобы сэкономить хранилище. Если FFMPEG не установлено, только записание в формате WAV доступно. Для тех, кто исползует USB прибор в другом устройстве, могут здесь указать адрес другого устройства.

 Затем снова нажать на кнопку настроек, чтобы закрыли настроики. Сейчас через кнопку SCAN, все радио станций в эфире вашего региона могут быть наидены. Интернет станции могут скачаны, если вы меняете режим в онлайн (нажмите на земля с наушниками наверху-слево и затем на Scan).

 После всех Scans, все станции будут выбраны в меню на левой стороне.

 Повеселиться!

## Лизенц

 GPL 3.0

## Благодарность

 Я благодарен разработникам [welle.io](https://www.welle.io/) за их работу.

## Помощь

 Обратитесь к мне на случае проблем или Feedback: svenali [at] gmx [dot] de.