#!/bin/bash

zenity --question --title "Compilador" --ok-label="Continuar" --cancel-label="Cancelar" --text="Bienvenido, esta a punto de inicar el explorador de Dispositivos ¿Qué desea hacer?"

comando=$(zenity --hide-column 2 --print-column 2 --list --column "Opción" --column "Comando oculto" "Crear Dispositivo" "dispositivo" "Eliminar Dispositivo" "eliminar" "Consultar Dispositivos" "consultar")

if [ "$comando" = "dispositivo" ];then
	host=$(zenity --entry --title="Host" --text="Ingrese el connection string del IoT Hub")
	deviceid=$(zenity --entry --title="Dispositivo" --text="Ingrese el nombre del dispositivo")
	firstkey=$(zenity --entry --title="Claves de dispositivo" --text="Ingrese la primera  clave del dispositivo(Based64 encripted)")
	secondkey=$(zenity --entry --title="Claves de dispositivo" --text="Ingrese la segunda  clave del dispositivo (Based64 encripted)")
	sed -i "/CONNECTION_STRING =/c CONNECTION_STRING = \"$host\"" Creardispositivo.py
	sed -i "/DEVICE_ID =/c DEVICE_ID = \"$deviceid\"" Creardispositivo.py 
	sed -i "s/\(primary_key = \)\(.*\)/\1\"$firstkey\"/" Creardispositivo.py
	sed -i "s/\(secondary_key = \)\(.*\)/\1\"$secondkey\"/" Creardispositivo.py
fi
