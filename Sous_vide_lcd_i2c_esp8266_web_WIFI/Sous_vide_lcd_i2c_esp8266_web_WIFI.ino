/*
    desenvolvido por:
    Cesar Costa e Samuel Oliveira Costa
    09/2022


*/

//includes
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <Wire.h>  //INCLUSÃO DE BIBLIOTECA
#include <LiquidCrystal_I2C.h> //INCLUSÃO DE BIBLIOTECA


//criar instancias
LiquidCrystal_I2C lcd(0x27, 20, 4); // Default address of most PCF8574 modules, change according

//Cria uma instancia da classe ESP8266WiFiMulti, chamada 'wifiMulti'
ESP8266WiFiMulti wifiMulti;
//Cria uma instancia da classe WifiUDP para enviar e receber dados
WiFiUDP UDP;

IPAddress timeServerIP;

ESP8266WebServer server(80); //instanciar servidor no porto 80


//defines

#define aquecedor D5  

#define termometro A0
#define bt_mais D7
#define bt_menos D6
#define bt_enter D0


//Constantes e Variaveis

// Altere com as suas credenciais
const char *ssid = "ap fran";
const char *password = "franOcosta11";

//Define o servidor NTP utilizado
const char* NTPServerName = "b.ntp.br";
//Time stamp do NTP se encontra nos primeiros 48 bytes da mensagem
const int NTP_PACKET_SIZE = 48;
//Buffer para armazenar os pacotes transmitidos e recebidos
byte NTPBuffer[NTP_PACKET_SIZE];

//Requisita horario do servidor NTP a cada minuto
unsigned long intervalNTP = 60000;
unsigned long prevNTP = 0;
unsigned long lastNTPResponse = millis();
uint32_t timeUNIX = 0;
unsigned long prevActualTime = 0;

String page = "";

int medir;
int medir_anterior = 0;
int seconds = 0;
float now;
float   timeChange;
float lastTime = 0.0;
float error;
float lastErr = 0.0;
float dErr;
float errSum;
float kp = 1.0;
float ki = 0.50;
float kd = 0.0;
float Setpoint = 0.0;
float Input;
float Output;
float saida;
float kmax = 255.0;
float kmin = 0.0;
float Resistance;
float Temp;
int TempRound;
float Temp_anterior = 0.0;
float Temp_acumuladaX50 = 0.0;
float Temp_acumuladaX10 = 0.0;
int dTEmp, dSet, dHora, dMinuto;

unsigned long segundos;
unsigned long minutos;
unsigned int horas;
unsigned int minuto;
unsigned int hora;
int dias;


//****************************************//

int PID() {
  now = millis();
  timeChange =  ((unsigned long)now - lastTime) / 1000;   //((long)now - lastTime) / 1000;

  error = Setpoint - Input;
  if (error > kmax)error = kmax;
  if (error < kmin - kmax)error = kmin - kmax;

  errSum += (error * timeChange);
  if (errSum > kmax)errSum = kmax;
  if (errSum < kmin - kmax)errSum = kmin - kmax;

  dErr = (error - lastErr) / timeChange;
  if (dErr > kmax)dErr = kmax;
  if (dErr < kmin - kmax)dErr = kmin - kmax;

  Output = (kp * error) + (ki * errSum) + (kd * dErr);
  //  Output=Output*-1;
  if (Output > kmax)Output = kmax;
  if (Output < kmin)Output = kmin;

  lastErr = error;
  lastTime = now;
  return Output;
}

//****************************************//

void medir_temp() {



  medir = analogRead(termometro);
  medir = (medir + medir_anterior) / 2;
  medir_anterior = medir;


  Resistance = ((10240000.0 / medir) - 10000);
  /******************************************************************/
  /* Utilizes the Steinhart-Hart Thermistor Equation:        */
  /*    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^3}   */
  /*    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08  */
  /******************************************************************/

  Temp = log(Resistance);
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // Convert Kelvin to Celsius
  Temp = (Temp + Temp_anterior) / 2;
  Temp_anterior = Temp;

}

//****************************************//

void setup()
{





  //Pagina HTML
  page = "<html><head><title>Controle Sinaleiro</title></head>"//barra de titulo
         "<body>    <h1>      <em><strong>Sinaleiro programavel</strong></em></h1>    <p>"
         "<a href=\"LEDOn\"><button style=\"background: #069cc2; border-radius: 6px; padding: 15px; cursor: pointer; color: #fff; border: none; font-size: 26px;\">Ligar o Led Azul </button></a>"  // botão extra
         "<p>   <strong><a href=\"LEDOn\"><button>ON</button></a></strong></p>  " // botão pequeno
         "<p> <p> <p> <p>          <p>      "
         "<strong>&nbsp;<a href=\"LEDOff\"><button>OFF</button></a></strong></p>  </body></html>"

         "<p> <p> <p> <p>          "

         "<a href=\"LEDOff\"><button style=\"background: #111213; border-radius: 6px; padding: 15px; cursor: pointer; color: #fff; border: none; font-size: 26px;\">Desligar o Led Azul </button></a>"  // botão extra

         ;  //segundo botão


  //DEsligar PINs
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  delay(1000);
  Serial.begin(9600);
  WiFi.begin(ssid, password); //Iniciar ligação à rede Wi-Fi
  Serial.println("");

  // Aguardar por ligação
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Ligado a: ");
  Serial.println(ssid);
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/html", page);
  });
  server.on("/LEDOn", []() {
    Setpoint++;
    
    server.send(200, "text/html", page);
    digitalWrite(LED_BUILTIN, LOW);
    

  });
  server.on("/LEDOff", []() {
    Setpoint--;
  
    server.send(200, "text/html", page);
    digitalWrite(LED_BUILTIN, HIGH);
  });
  server.on("/ATHora", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
  });
  server.begin();
  Serial.println("Web server Inicializado!");


  ///hora UDP

  startUDP();
  if (!WiFi.hostByName(NTPServerName, timeServerIP))
  {
    //Obtem o endereco IP do servidor NTP
    Serial.println("DNS lookup failed. Rebooting.");
    Serial.flush();
    ESP.reset();
  }
  Serial.print("IP do servidor NTP:t");
  Serial.println(timeServerIP);
  Serial.println("rnEnviando requisicao NTP...");
  sendNTPpacket(timeServerIP);



  /// end hora UPD


  // lcd

  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  Serial.println("Setpoint , temperatura , Output");

  pinMode(aquecedor, OUTPUT);
  pinMode(termometro, INPUT);
  pinMode(bt_mais, INPUT_PULLUP);
  pinMode(bt_menos, INPUT_PULLUP);



  lcd.setCursor(0, 0);
  lcd.print("Temp. Cuba: ");
  lcd.setCursor(0, 1);
  lcd.print("Set Point: ");
  lcd.setCursor(0, 2);
  lcd.print("tempo ligado");
  lcd.setCursor(17, 2);
  lcd.print(":");
  lcd.setCursor(0, 3);
  lcd.print("HORA UPD");
  lcd.setCursor(17, 3);
  lcd.print(":");

for(int i=0 ; i<50;i++){
  medir_temp();
  delay(1);
}
  int temperatura = Temp;
  Setpoint = temperatura;

}

void loop()
{



server.handleClient();

  /// hora CERTA
  unsigned long currentMillis = millis();
  if (currentMillis - prevNTP > intervalNTP)
  {
    //Verificar se passou um minuto da ultima requisicao
    prevNTP = currentMillis;
    Serial.println("rnEnviando requisicao NTP ...");
    sendNTPpacket(timeServerIP);
  }
  uint32_t time = getTime();
  if (time)
  {
    timeUNIX = time - 10800;
    Serial.print("Resposta NTP:t");
    Serial.println(timeUNIX);
    lastNTPResponse = currentMillis;
  }
  else if ((currentMillis - lastNTPResponse) > 3600000) {
    Serial.println("Mais de 1 hora desde a ultima resposta NTP. Reiniciando.");
    Serial.flush();
    ESP.reset();
  }
  uint32_t actualTime = timeUNIX + (currentMillis - lastNTPResponse) / 1000;
  if (actualTime != prevActualTime && timeUNIX != 0)
  {
    //Verifica se passou um segundo desde a ultima impressao de valores no serial monitor
    prevActualTime = actualTime;
    //Serial.printf("rUTC time:t%d:%d:%d   ", getHours(actualTime), getMinutes(actualTime), getSeconds(actualTime));
    //Serial.println();
  }


  
//Pagina HTML
  page = "<html><head><title>Controle Temperatura Sous Vide</title></head>"//barra de titulo
         "<body>    <h1>      <em><strong> Controle Temperatura Sous Vide </strong></em></h1>    <p>"


         "<a href=\"LEDOn\"><button style=\"background: #069cc2; border-radius: 6px; padding: 15px; cursor: pointer; color: #fff; border: none; font-size: 26px;\">Subir Setpoint Led On </button></a>"  // botão extra


         "<a href=\"LEDOff\"><button style=\"background: #111213; border-radius: 6px; padding: 15px; cursor: pointer; color: #fff; border: none; font-size: 26px;\">Descer Setpoint Led Off</button></a>"  // botão extra

         "<p><a href=\"ATHora\"><button style=\"background: #111213; border-radius: 6px; padding: 15px; cursor: pointer; color: #fff; border: none; font-size: 26px;\"> Atualizar Temperatura e  piscar led </button></a>"  // botão extra
         "<p><body>    <h1>      <em><strong>Data da Leitura:  "
         ;  //segundo botão


  page = page + (String(getHours(actualTime)));
  page = page + ":";
  page = page + (String(getMinutes(actualTime)));
  page = page + ":";
  page = page + (String(getSeconds(actualTime)));
  page = page+ "<br>Tempo Ligado: ";
  page = page + (hora);
  page = page + ":";
   page = page + (minuto);

  page = page + " </strong></em></h1>    <p>";

  page = page + "<hr/><p><body>    <h1>      <em><strong>Ultima Leitura:  ";
  page = page + (String (Temp));
  page = page + "`C <br> Setpoint ajustado em: ";
  page = page + (String (Setpoint));

  page = page + "  <img  alt=\"alternative text for the liked image\""
         "src=\"http://zequielmarmores.com.br/wp-content/uploads/2015/01/negroMarquinaEspanhol.jpg\""
         "width=\"5\"     height=\"133\"    border=\"5\"    style=\"border:1px solid #337AB7\" />";


  page = page + "<img  alt=\"alternative text for the liked image\""
         "src=\"http://zequielmarmores.com.br/wp-content/uploads/2015/01/rojoAlicante.jpg\""
         "width=\"75\"    height=\"";
  page = page + (String(Temp + 1));
  page = page + "\"    border=\"5\"    style=\"border:1px solid #337AB7\" /><img  alt=\"alternative text for the liked image\""
         "src=\"http://zequielmarmores.com.br/wp-content/uploads/2015/01/qsAzulEstrelar.jpg\""
         "width=\"75\"    height=\"";
  page = page + (String(Setpoint + 1));



  page = page + "\"    border=\"5\"    style=\"border:1px solid #337AB7\" /><img  alt=\"alternative text for the liked image\""
         "src=\"http://zequielmarmores.com.br/wp-content/uploads/2015/01/negroMarquinaEspanhol.jpg\""
         "width=\"5\"     height=\"133\"    border=\"5\"    style=\"border:1px solid #337AB7\" />"
         "<hr/><img alt=\"html image example\" src=\"https://www.vectorlogo.zone/logos/arduino/arduino-official.svg\"  />";



  page = page + " </strong></em></h1>    <p>";


  
  //    //Mostra o horario atualizado
  //       Serial.println("");
  //        Serial.print( String(getHours(actualTime)));
  //         Serial.print( ":");
  //          Serial.print(String(getMinutes(actualTime)));
  //         Serial.print( ":");
  //       Serial.print( String(getSeconds(actualTime)));



  /// em hora certa







  //programa sous vide



  segundos = millis() / 1000;
  minutos = segundos / 60;
  horas = minutos / 60;
  minuto = minutos % 60;
  hora = horas % 24;
  dias = hora / 24;

  if (!digitalRead(bt_mais)) {
    Setpoint++;
    delay(1);
  }
  if (!digitalRead(bt_menos)) {
    Setpoint--;
    delay(1);
  }

  medir_temp();

  for (int j = 0 ; j < 9; j++) {
    for (int i = 0; i < 49; i++) {

      Temp_acumuladaX50 = Temp_acumuladaX50 + Temp;
      medir_temp();
      delayMicroseconds(800);
    }
    delayMicroseconds(500);
    Temp = Temp_acumuladaX50 / 50.0;
    TempRound = Temp * 10;
    Temp = TempRound / 10.0;

    Temp_acumuladaX50 = Temp_acumuladaX50 / 50.0;
    Temp_acumuladaX10 = Temp_acumuladaX10 + Temp;
  }
  Temp = Temp_acumuladaX10 / 10.0;
  TempRound = Temp * 10;
  Temp = TempRound / 10.0;

  Temp_acumuladaX10 = Temp_acumuladaX10 / 10;


  Input = Temp;
  saida = PID();
  analogWrite(aquecedor, saida);

  //lcd.clear();
  lcd.setCursor(15, 0);
  if (Temp < 0 && dTEmp != -1) {
    lcd.print("     ");
    lcd.setCursor(15, 0);
  }  else if (Temp == 0 && dTEmp != 0) {
    lcd.print("     ");
    lcd.setCursor(16, 0);
  } else if (Temp < 10 && dTEmp != 10) {
    lcd.print("     ");
    lcd.setCursor(16, 0);
  } else if (Temp < 100 && dTEmp != 100) {
    lcd.print("     ");
    lcd.setCursor(15, 0);
  }
  lcd.print(Temp);

  lcd.setCursor(15, 1);
  if (Setpoint < 10 && dSet != 10) {
    lcd.print("    ");
    lcd.setCursor(16, 1);
  } else if (Setpoint < 100 && dSet != 100) {
    lcd.print("    ");
    lcd.setCursor(15, 1);
  }
  lcd.print(Setpoint);

  lcd.setCursor(15, 2);
  if (hora < 10)  lcd.print("0");
  lcd.print(hora);
  lcd.setCursor(18, 2);
  if (minuto < 10)  lcd.print("0");
  lcd.print(minuto);

  lcd.setCursor(0, 3);
  lcd.printf("Hora UPD: %d:%d:%d  ", getHours(actualTime), getMinutes(actualTime), getSeconds(actualTime));



  Serial.print(Setpoint);
  Serial.print(" , ");
  Serial.print(Temp);
  Serial.print(" , ");
  Serial.println(map(Output, 0, 255, 0, 100));





  delay(1);


}
