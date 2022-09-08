Интерактивное меню для ленивых;)
совместно с bash инструмент механизации повторяющихся работ

Вызов:
Menu Unix (mu)
********************************************************
*ВАЗ ПАП БРАС   24.10.91  Булов B.Г.(@bvg) version 2.0 *
********************************************************
*  Программа работы с меню в UNIX ( mu )               *
*  Вызов:  mu [-m меню] [-d каталог] [-p file] [-h]    *
*  где:    -m меню    = файл с меню       ( .root.u )  *
*          -d каталог = перейти в каталог ( . )        *
*          -p file    = протол в файл     ( prot.u  )  *
*          -h         = справка эта                    *
********************************************************

Пример:  файла с меню .rootV1.u

{OLD:VER
#
# АО РТИ им. Академика А.Л. Минца 2022/09/09  Булов B.Г.(V.Bulov@gmail.ru) version 1.0
#
предыдущее меню:<PRED
}
{OLD
@0 0
#   ******   ***   *****   *    *           *******  *******   *****   *******
#   *     *   *   *     *  *   *               *     *        *     *     *
#   *     *   *   *        *  *                *     *        *           *
#   *     *   *    *****   ***     *******     *     *****     *****      *
#   *     *   *         *  *  *                *     *              *     *
#   *     *   *   *     *  *   *               *     *        *     *     *
#   ******   ***   *****   *    *              *     *******   *****      *
Логи      :<log
Контролер :<sctl
Диск      :<diskDD
Версия    :<VER
Help      :export HLP="f_" ; arcconf.sh
установки :<-set
}
{OLD:log
@0 0
#                      ***   ***   ***** *   *
#                     *  *  *   *  *   * *  **
#                    *   *  *   *  *     * * *
#                    *   *  *   *  *     **  *
#                    *   *   ***   *     *   *
Очистить        .:arcconf.sh LOG_CLEAR
Посмотреть       : arcconf.sh LOG_SHOW | less
предыдущее меню:<PRED
}
{OLD:sctl
@0 0
#
#                       Работа с arcconf
#
предыдущее меню:<PRED
Диски контролера            .: arcconf.sh  GET_PD | less
Логические диски контролера  :arcconf.sh  GET_LD
Статус с контролера          :arcconf.sh  GETSTATUS
Текущее состояние -a         :(export KEY=a; arcconf.sh TEST_SHOW | tee disk.txt )
Текущеe состояние -x         :(export KEY=x; arcconf.sh TEST_SHOW | tee disk.txt )
Посмотреть re                    : re disk.txt
Посмотреть TC отдельные поля     :arcconf.sh GET_PD_S
Сохранить состоятие {SN}.txt     :arcconf.sh SAVE
Внести даные  {SN}.txt           : re Itog.csv $(ls -rt | tail -n1)
Выбрать и сохранить поля в отчете:есho "в разработке"
установки :<-set
}
{OLD:diskDD
@0 0
#
#                       Чтение, запись smart test дискa
#
Сохранить состоятие {SN}.txt  (0):arcconf.sh SAVE
Включить сохранение логов -S  (1):arcconf.sh SET_LOG
Пишем на диск                 (2):arcconf.sh DDW
Запустить тест длинный        (3):(export TEST=long;  arcconf.sh TEST)
Запустить тест корокий           :(export TEST=short; arcconf.sh TEST)
Ждем окончания теста          (4):arcconf.sh WATCH
Читаем с диска                (5):arcconf.sh DDR
ddrescue                         :ddrescue --ask --verbose --binary-prefixes --idirect --force /dev/sd${HD} /dev/null disk.map
Сохранить состоятие {SN}.txt  (0):arcconf.sh SAVE
Остановить диск               (6):arcconf.sh DDD
Контролер :<sctl
установки :<-set
Предыдущее меню:<PRED
}
{OLD:set
@50 4
#
#  Текущие установки
#
Предыдущее меню:<PRED
Диск        [   ]:$PD
Диск /dev/sd[   ]:$HD
Контролер   [   ]:$CN
ЛогичДиск   [   ]:$LD
Экспандер   [   ]:$EX
Название[       ]:$NAME
}
Пример:  файла arcconf.sh
#!/bin/bash -f
############################################################################
##  Работа с контролером Adaptec ASR7805 по тестированию дисков
##  совместно с утилитой mu
##  arcconf.sh VBulov@tri-mints.ru 2022/09/08 v1
############################################################################
f_GETSTATUS(){ # Статус с контролера
               ${PRG} GETSTATUS  ${CN}                                     ;}
f_RESCAN(){    # Сканировать переподключенные диски
               ${PRG} RESCAN     ${CN}                                     ;}
f_GET_LD(){    # Статус логических дисков
               ${PRG} GETCONFIG  ${CN} LD                                  ;}
f_GET_PD(){    # Статус физических дисков
               ${PRG} GETCONFIG  ${CN} PD                                  ;}
f_GET_PD_S(){  # Статус физических дисков коротко
               ${PRG} GETCONFIG  ${CN} PD| egrep "Device #|State\>|Reported Location|Reported Channel|S.M.A.R.T. warnings"  ;}
f_DDD(){       # Остановить диск
               ${PRG} SETSTATE   ${CN} DEVICE ${EX} ${PD} DDD              ;}     # stop disk
f_RDY(){       # Запустить пустить диск
               ${PRG} SETSTATE   ${CN} DEVICE ${EX} ${PD} RDY              ;}
f_INI(){       # Инициализировать диск
               ${PRG} TASK START ${CN} DEVICE ${EX} ${PD} INITIALIZE       ;}
f_IDENTIFY(){  # Обозначить диск ?
               ${PRG} IDENTIFY   ${CN} DEVICE ${EX} ${PD}                  ;}
f_ALARM(){     # Отключить звук на контролере
               ${PRG} SETALARM   ${CN} silence                             ;}
f_DELETE(){    # Удалить логический диск
               ${PRG} DELETE     ${CN} LOGICALDRIVE ${LD}                  ;}
f_SETNAME(){   # Поименовать логический диск
               ${PRG} SETNAME    ${CN} LOGICALDRIVE ${LD} ${NAME}          ;}
f_LOG_CLEAR(){ # для нового диска очистить логи контролера
               ${PRG} GETLOGS ${CN} DEVICE clear
               ${PRG} GETLOGS ${CN} DEAD   clear
               ${PRG} GETLOGS ${CN} EVENT  clear
}
f_LOG_SHOW(){  # после dd smart dd посмотреть  логи контролера
               ${PRG} GETLOGS ${CN} DEVICE tabular
               ${PRG} GETLOGS ${CN} DEAD   tabular
               ${PRG} GETLOGS ${CN} EVENT  tabular
}
f_TEST(){      # Запустить тест на контролере диска
               smartctl --test=${TEST:-short} -d aacraid,0,0,${PD} /dev/sd${HD} ;}
f_TEST_SHOW(){ # Состояние диска с контролера диска
               smartctl -${KEY:-a}            -d aacraid,0,0,${PD} /dev/sd${HD} ;}
f_SET_LOG(){   # Включить сохранение логов на контролере диска
               smartctl -S on -s on           -d aacraid,0,0,${PD} /dev/sd${HD} ;}
f_SAVE(){      # Сохранить состояние с контролера диска в файл серийныйНомер.txt
               export KEY=a ; export ExR=return ; f_TEST_SHOW > /tmp/arcconf$$.txt
               File=$(grep 'Serial number' < /tmp/arcconf$$.txt | cut -d: -f2| tr -d ' ')
               cat  /tmp/arcconf$$.txt >> ${File}.txt
               echo "===${File}===$(date +%Y/%m/%d-%H:%M)===" >> ${File}.txt
               rm    /tmp/arcconf$$.txt
               grep "=${File}=" ${File}.txt
}
f_WATCH(){     # Ожидаем окончания прохождения теста контролера дисков
               watch -n 10 "smartctl -a  -d aacraid,0,0,${PD} /dev/sd${HD} | grep Self-test" | tee /dev/tty1  ;}
f_DDR(){       # Читаем с диска
               dd if=/dev/sd${HD} of=/dev/null bs=1M   status=progress 2>&1 | tee /dev/tty1  ;}
f_DDW(){       # Пишем на диск НУЛИ
               dd if=/dev/zero of=/dev/sd${HD} bs=1M   status=progress 2>&1 | tee /dev/tty1  ;}
#  ##################################################################################
PRG=arcconf
#PRG=echo
CN=${CN:-1} ; LD=${LD:-0} ; EX=${EX:-0} ; PD=${PD:-4} ; NAME=${NAME:-BOOT}
EP="${EX} 2 ${EX} 3 ${EX} 4 ${EX} 5"
EP="${EX} ${PD}"
export     FN=${1:-help}      #function
   f_${FN} $*

