

Para la implementación de contiki en la plaqueta de desarrollo Zolertia Zoul, lo primero que se debe realizar es la copia del repositorio del contiki desde el github del fabricante. Para esto se deben ejecutar los siguientes códigos:

    git clone --recursive https://github.com/alignan/contiki
    sudo apt-get install python-serial python-pip
    pip install intelhex
    pip install python-magic

Tras esto se deben seguir los pasos que se especifican en el libro Iot in five Days, especificamente los de las páginas 43 y 44.

Se recomienda que en la opcion en la que se instala el openjdk se instale la ultima versión de este.

Sin embargo si se llega a presentar el problema de que no se detecte el comando: 

    arm-gcc-none-eabi-gcc --version


Se recomienda seguir los siguientes pasos extraidos de http://gnuarmeclipse.github.io/toolchain/install/

    sudo apt-get -y install lib32ncurses5 ia32-libs

Descargar el archivo gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2  de la siguiente dirección:

https://launchpad.net/gcc-arm-embedded/+download 

Finalmente, ejecutar los siguientes comandos:

    cd /usr/local
    sudo tar xjf ~/Downloads/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2
    /usr/local/gcc-arm-none-eabi-4_8-2014q1/bin/arm-none-eabi-gcc --version


Ahora para probar el ejemplo que viene incluido en el contiki se sigue los pasos enunciados en las páginas 48 para compilar el ejemplo. Si lo que se quiere es flashear el programa en la plaqueta se utiliza el siguiente comando: 

    make "nombre-ejemplo"
    make clean && make "nombre-ejemplo".upload

En caso de que se presente un error en la ejecución de este último comando puede que se tengan problemas de permisos, por lo que se debe ejecutar el siguiente comando:

    sudo chmod 666 /dev/"puerto-al-que-se-conecta-la-plaqueta"

Tras ejectuar este último comando se vuelve a probar el anterior comando y ya debería funcionar.
