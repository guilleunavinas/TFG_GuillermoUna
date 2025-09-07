
//ANEXO 1. CÓDIGO DE PROGRAMACIÓN.

// Conexiones del microcontrolador con el L293D, el sensor y el potenciómetro.
int pinMotor = 5; // Pin de salida para señal PWM que controla motor conectado al pin 1 (EN1) del L293D.
int pinIn1 = 8;    // Pin de salida para señal digital que pone en marcha el motor. Estará conectado al pin 2 (IN1) del L293D.
int pinIn2 = 9;    // Pin de salida para señal digital que pone en marcha el motor. Estará conectado al pin 7 (IN2) del L293D.
int pinSensor = 4; // Pin de entrada para señal del sensor infrarrojo.
int pinPotenciometro = A0; // Pin analógico de entrada para la señal del potenciómetro.
int pinPulsador = 2; // Pin de entrada para pulsador.


// Declaración e inicialización de variables.

// Variables del potenciómetro y control de motor.

int valorPot = 0;            // Valor leído del potenciómetro.
int velocidadPWM = 0;        // Velocidad aplicada al motor mediante PWM.
int pwmRemoto = 0;           // Valor de PWM recibido en modo remoto.
bool motorEncendido = false; // Estado del motor. 
bool modoRemoto = false;     // Indica si está activo el modo remoto.
bool paroSeguridad = false;  // Indica si se ha parado por seguridad.


// Variables del sensor infrarrojo y conteo de objetos.

int contadorObjetos = 0;         // Total de objetos detectados.
int estadoActualSensor = HIGH;   // Estado actual del sensor infrarrojo.
int estadoAnteriorSensor = HIGH; // Estado anterior del sensor infrarrojo.
bool objetoDetectado = false;    // Indica si se ha detectado un objeto.
unsigned long tiempoDeteccion = 0;  // Tiempo de detección de un objeto.
unsigned long pausa = 2000;         // Pausa de la cinta (ms) tras detectar objeto.
unsigned long tUltimoObjeto = 0;    // Momento de la última detección.
unsigned long tiempoSinObjeto = 15000; // Tiempo (ms) sin detección para paro por seguridad.
unsigned long reboteSensor = 30;       // Retardo para eliminar rebotes del sensor (ms).
unsigned long tUltimoCambioSensor = 0; // Momento del último cambio detectado en el sensor.


// Variables del pulsador.

int pulsadorActual = HIGH;    // Estado actual del pulsador.
int pulsadorAnterior = HIGH;  // Estado anterior del pulsador.
unsigned long rebotePulsador = 50;     // Retardo para eliminar rebotes del pulsador (ms).
unsigned long tUltCambioPulsador = 0;  // Momento del último cambio detectado en el pulsador.


// Variables para el envío de datos.

unsigned long tiempoUltimoEnvio = 0;   // Tiempo del último envío.
unsigned long intervaloEnvio = 1000;   // Intervalo de envío de los datos (ms).



void setup() {

  // Configuración de pines.
  pinMode(pinMotor, OUTPUT);
  pinMode(pinSensor, INPUT);
  pinMode(pinIn1, OUTPUT);
  pinMode(pinIn2, OUTPUT);
  pinMode(pinPulsador, INPUT_PULLUP);

  Serial.begin(9600); // Iniciar comunicación con monitor serie.
  
  digitalWrite(pinIn1, LOW); // Iniciar el motor apagado 
  digitalWrite(pinIn2, LOW);

  estadoAnteriorSensor = digitalRead(pinSensor);
  pulsadorAnterior = digitalRead(pinPulsador);
  tUltimoObjeto = millis();
  paroSeguridad = false;
}

void loop() {

  // --- 1. Leer comandos desde API ---
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando == "1") {
      if (modoRemoto) {
        motorEncendido = true;
        tUltimoObjeto = millis();
        paroSeguridad = false;     // rearme del paro de seguridad
      }
      
    } else if (comando == "0") {
      if (modoRemoto) {
      motorEncendido = false;
      digitalWrite(pinIn1, LOW);
      digitalWrite(pinIn2, LOW);
      analogWrite(pinMotor, 0);
      }

    } else if (comando == "reset") {
      // Reinicia el contador de objetos
      contadorObjetos = 0;

    } else if (comando == "manual") {
      // Vuelve a modo MANUAL (potenciómetro físico)
      modoRemoto = false;
      

    } else if (comando.startsWith("r")) {
      // "rXXX" -> activar REMOTO y fijar PWM a XXX
      int nuevoPWM = comando.substring(1).toInt();
      pwmRemoto = constrain(nuevoPWM, 0, 255);
      modoRemoto = true;
    }
  }


  // --- 2. Lectura del potenciómetro (para modo manual) ---
  valorPot = analogRead(pinPotenciometro);
  int pwmManual = map(valorPot, 0, 1023, 0, 255);

  // --- 3. Detección de objetos. ---
  int lectura = digitalRead(pinSensor);

  if (lectura != estadoAnteriorSensor) {
    if (millis() - tUltimoCambioSensor >= reboteSensor) {
    estadoActualSensor = lectura;
    tUltimoCambioSensor = millis();
    }
  } else {
  estadoActualSensor = lectura;
  }

  if (!objetoDetectado                
    && motorEncendido                
    && estadoAnteriorSensor == HIGH 
    && estadoActualSensor == LOW) { 

  objetoDetectado = true;
  contadorObjetos++;
  motorEncendido = false;
  
  digitalWrite(pinIn1, LOW);
  digitalWrite(pinIn2, LOW);
  analogWrite(pinMotor, 0);
  tiempoDeteccion = millis();
  }

  // --- 4. Reanudar motor después de pausa ---
  if (objetoDetectado && millis() - tiempoDeteccion >= pausa) {
    motorEncendido = true;
    objetoDetectado = false;
    tUltimoObjeto = millis();
  }
  estadoAnteriorSensor = estadoActualSensor;


  // --- 5. Lectura de pulsador ---
  int lecturapulsador = digitalRead(pinPulsador);

  if (lecturapulsador != pulsadorAnterior) {
    if (millis() - tUltCambioPulsador >= rebotePulsador) {
    pulsadorActual = lecturapulsador;
    tUltCambioPulsador = millis();

      if (pulsadorAnterior == HIGH && pulsadorActual == LOW) {
        if (!modoRemoto) {
        // En MANUAL: funciona como conmutador del motor
        motorEncendido = !motorEncendido;
          if (motorEncendido) {
            tUltimoObjeto = millis();
            paroSeguridad = false;
          }


        } else {
        // En REMOTO: parada de emergencia
          motorEncendido = false;
          digitalWrite(pinIn1, LOW);
          digitalWrite(pinIn2, LOW);
          analogWrite(pinMotor, 0);

          } 
      }
    }
  } else {
  pulsadorActual = lecturapulsador;
  }

  pulsadorAnterior = pulsadorActual;


  // --- 6. Control PWM del motor. ---
  if (motorEncendido) {
    digitalWrite(pinIn1, HIGH);
    digitalWrite(pinIn2, LOW);
    // Selección de fuente PWM según el modo
    velocidadPWM = modoRemoto ? pwmRemoto : pwmManual;
    analogWrite(pinMotor, velocidadPWM);
  } else {
    digitalWrite(pinIn1, LOW);
    digitalWrite(pinIn2, LOW);
    analogWrite(pinMotor, 0);
  }


  // --- 7. Paro de seguridad de 15 segundos. ---
  if (motorEncendido && !objetoDetectado) {
  if (millis() - tUltimoObjeto >= tiempoSinObjeto) {
    motorEncendido = false;
    paroSeguridad = true; 
    // Paramos motor:
    digitalWrite(pinIn1, LOW);
    digitalWrite(pinIn2, LOW);
    analogWrite(pinMotor, 0);
  }
}

  // --- 8. Envío periódico de estado por puerto serial. ---
  if (millis() - tiempoUltimoEnvio >= intervaloEnvio) {
    Serial.print("OBJ:");
    Serial.print(contadorObjetos);
    Serial.print(", POT:");
    Serial.print(valorPot);
    Serial.print(", PWM:");
    Serial.print(velocidadPWM);
    Serial.print(", MODO:");
    Serial.println(modoRemoto ? "REMOTO" : "MANUAL");

    tiempoUltimoEnvio = millis();
  }

  delay(50);
}
