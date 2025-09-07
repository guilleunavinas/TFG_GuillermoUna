# Manual de funcionamiento del sistema.

Este repositorio contiene los códigos de programación desarrollados y un vídeo demostrativo del sistema en funcionamiento correspondientes al Trabajo de Fin de Grado titulado "SENSORIZACIÓN, MONITORIZACIÓN Y CONTROL REMOTO DE UNA CINTA TRANSPORTADORA" realizado por Guillermo Uña Viñas en la Escuela Superior de Ingeniería (ESI) de la Universidad de Cádiz (UCA).

El proyecto integra:
- Control local de una cinta transportadora a través de sensores y actuadores con Arduino.
- API desarrollada en Python para la comunicación entre el microcontrolador Arduino y una interfaz gráfica.
- Interfaz gráfica (dashboard) para monitorizar y controlar el sistema de forma remota, incluyendo la posibilidad de cambiar entre modo remoto y modo local.
- Configuración de acceso remoto a la API a través de CloudFlare.

**Ejecución de la API**:

  1. Cargar el código programado en Arduino en el microcontrolador, que deberá estar conectado al PC por USB.
  2. Abrir el editor de código, Visual Studio Code en este caso, y en el terminal a través de los siguientes comandos de PowerShell, lanzar la API:
      $env:ARDUINO_PORT="COM10" (Establece el puerto serie que está usando Arduino. El monitor serie de Arduino deberá estar cerrado para evitar conflictos.)
      python .\API_TFG.py (ejecuta la API)
  3. Comprobar que en el terminal aparece lo siguiente:
     API conectada a COM10 @ 9600 baudios
     * Serving Flask app 'API_TFG'
     * Debug mode: off
      WARNING: This is a development server. Do not use it in a production deployment. Use a production WSGI server instead.
     * Running on all addresses (0.0.0.0)
     * Running on http://127.0.0.1:5000
     * Running on http://192.168.1.182:5000
     Esto indicará que la API se ha ejecutado correctamente y está funcionando, para desconectarla bastará con presionar "Ctrl + C" en el terminal.

**Conexión en local con la interfaz gráfica**:

  Una vez la API este funcionando, para abrir la interfaz en local hay dos opciones:
  - Acceder directamente a la carpeta del ordenador donde está ubicado el archivo .HTML de la interfaz (en este caso, Dashboard_TFG.html) y abrirlo desde un     navegador.
  - O bien, mediante el siguiente comando ejecutado en el terminal antes de la ejecución de la API: python -m http.server 8000  y el terminal deberá devolver algo como: Serving HTTP on :: port 8000 (http://[::]:8000/) ... . Esto lo que hace es habilitar un servidor web local que accede directamente a la carpeta donde se encuentra ubicada la interfaz mediante una URL local, en este caso sería: http://localhost:8000/Dashboard_TFG.html. Ejecutando esto en el navegador, accederiamos a la interfaz gráfica.

**Conexión en remoto desde otro dispositivo con la interfaz gráfica**:

  Para acceder en remoto se usará tanto el repositorio en GitHub como Cloudflare, mediante los siguientes pasos:
  1. Subir al repositorio de GitHub el archivo .html de la interfaz gráfica.
  2. Generar una URL pública de acceso a este repositorio, para ello, dentro de GitHub acceder a Settings -> Pages y activar GitHub Pages seleccionando la rama "main" y la carpeta "root". La URL generada en este caso es la siguiente: https://guilleunavinas.github.io/TFG_Guillermo/
  3. Tener Cloudflare descargado en el ordenador y ejecutar el siguiente comando en el cmd de windows, en la carpeta donde se tenga el .exe de Cloudflare: cloudflared.exe tunnel --url http://localhost:5000. Esto creará un "tunel" entre la red de Cloudflare y el servicio que está corriendo en el ordenador en el puerto 5000, en este caso, la API desarrollada.
  4. Al ejecutar el anterior comando, Cloudflare nos devolverá en el mismo terminal una URL pública del tipo: https://xxxxx.trycloudflare.com. Esto hará que cualquier petición que llegue a esa URL, se reenviará a la API en local.
  5. Y por último, acceder a la API en remoto. Para ello, se añade como parámetro a la URL del repositorio, la URL pública que nos proporciona Cloudflare. Quedando una URL de la siguiente forma: https://guilleunavinas.github.io/TFG_Guillermo/Dashboard_TFG.html?api=https://xxxxx.trycloudflare.com. Accediendo a esta URL mediante cualquier dispositivo con acceso a Internet, se podrá manejar el sistema a través de la interfaz.

  
