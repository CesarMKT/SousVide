
#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 20, 4); // Default address of most PCF8574 modules, change according

#define aquecedor 11
#define termometro 14
#define potenciometro 15

int medir;
int medir_anterior = 0;
int seconds = 0;
unsigned long now;
float timeChange;
float lastTime = 0;
float error;
float lastErr = 0;
float dErr;
float errSum;
float kp = 1;
float ki = 1;
float kd = 0;
float Setpoint = 0;
float Input;
float Output;
float saida;
float kmax = 255;
float kmin = 0;
float Resistance;
float Temp;
int TempRound;
float Temp_anterior = 0;
float Temp_acumuladaX50 = 0;
float Temp_acumuladaX10 = 0;
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
  timeChange =  ((long)now - lastTime) / 1000;

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
  lcd.begin(); // If you are using more I2C devices using the Wire library use lcd.begin(false)
  // this stop the library(LCD_I2C) from calling Wire.begin()
  lcd.backlight();
  Serial.begin(9600);
  Serial.println("Setpoint , temperatura , Output");
  pinMode(13, OUTPUT);
  pinMode(aquecedor, OUTPUT);
  pinMode(potenciometro, INPUT);
  pinMode(termometro, INPUT);
  digitalWrite(13, 1);

  lcd.setCursor(0, 0);
  lcd.print("Temp. Cuba: ");
  lcd.setCursor(0, 1);
  lcd.print("Set Point: ");
  lcd.setCursor(0, 2);
  lcd.print("tempo ligado");
  lcd.setCursor(17, 2);
  lcd.print(":");


}

void loop()
{
  segundos = millis() / 1000;
  minutos = segundos / 60;
  horas = minutos / 60;
  minuto = minutos % 60;
  hora = horas % 24;
  dias = hora / 24;

  Setpoint = analogRead(potenciometro);
  Setpoint = map(Setpoint, 0, 1023, 0, 90);
  medir_temp();

  for (int j = 0 ; j < 9; j++) {
    for (int i = 0; i < 49; i++) {

      Temp_acumuladaX50 = Temp_acumuladaX50 + Temp;
      medir_temp();
      delayMicroseconds(800);
    }
    delayMicroseconds(500);
      Temp = Temp_acumuladaX50 / 50.0;
      TempRound=Temp*10;
      Temp=TempRound/10.0;

  Temp_acumuladaX50 = Temp_acumuladaX50 / 50.0;
  Temp_acumuladaX10 = Temp_acumuladaX10+Temp;
  }
  Temp = Temp_acumuladaX10 / 10.0;
        TempRound=Temp*10;
      Temp=TempRound/10.0;

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



  Serial.print(Setpoint);
  Serial.print(" , ");
  Serial.print(Temp);
  Serial.print(" , ");
  Serial.println(map(Output, 0, 255, 0, 100));





  delay(1);


}
