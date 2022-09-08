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
f_help(){      # Напоминалка по командам  ЭТА
grep '^##' $0
cat <<EOL
#  ##################################################################################
   arcconf GETCONFIG ${CN} LD
   arcconf GETCONFIG ${CN} PD
   arcconf RESCAN    ${CN}
   arcconf GETCONFIG ${CN} LD
   arcconf GETCONFIG ${CN} PD
   arcconf GETCONFIG 1 PD | egrep "Device #|State\>|Reported Location|Reported Channel|S.M.A.R.T. warnings"
   arcconf IDENTIFY ${CN} DEVICE ${EX} ${PD}
   PD=4 ; smartctl --test=long  -d aacraid,0,0,${PD} /dev/sd${HD}
   PD=4 ; smartctl -l selftest  -d aacraid,0,0,${PD} /dev/sd${HD}
   arcconf CREATE ${CN} LOGICALDRIVE MAX 10 0 2 0 3 0 4 0 5 NOPROMPT
   arcconf CREATE ${CN} LOGICALDRIVE MAX 10 0 2 0 3 0 4 0 5 NOPROMPT
   arcconf SETNAME 1 LOGICALDRIVE 0 RD10
   arcconf CREATE ${CN} LOGICALDRIVE RVOLUME 0   ???
   arcconf SETALARM   ${CN} silence
   arcconf DELETE     ${CN} LOGICALDRIVE ${LD}
   arcconf SETSTATE   ${CN} DEVICE ${EX} ${PD} DDD
   arcconf SETSTATE   ${CN} DEVICE ${EX} ${PD} RDY
   arcconf TASK START ${CN} DEVICE ${EX} ${PD} INITIALIZE
   arcconf CREATE     ${CN} LOGICALDRIVE MAX   0 ${EX} ${PD} NOPROMPT
   arcconf SETNAME    ${CN} LOGICALDRIVE ${LD} BOOT
   arcconf SETBOOT    ${CN} LOGICALDRIVE ${LD} TYPE Primary [nologs]
   arcconf SETSTATSDATACOLLECTION ${CN} enable
# для нового диска
  arcconf GETLOGS ${CN} DEVICE clear
  arcconf GETLOGS ${CN} DEAD   clear
  arcconf GETLOGS ${CN} EVENT  clear
# после dd smart dd
  arcconf GETLOGS ${CN} DEVICE tabular
  arcconf GETLOGS ${CN} DEAD   tabular
  arcconf GETLOGS ${CN} EVENT  tabular
# ###################################################################################
EOL
[ _"f_" != _"${HLP}" ] && grep ^f_ $0 | sort
}
# ###################################################################################
PRG=arcconf
#PRG=echo
CN=${CN:-1} ; LD=${LD:-0} ; EX=${EX:-0} ; PD=${PD:-4} ; NAME=${NAME:-BOOT}
EP="${EX} 2 ${EX} 3 ${EX} 4 ${EX} 5"
EP="${EX} ${PD}"
export     FN=${1:-help}      #function
   f_${FN} $*
