#include <LiquidCrystal.h>
#include <WiFi.h> 
#include <HTTPClient.h>

// Pinos do LCD
LiquidCrystal lcd(2, 0, 17, 5, 18, 19);

// Pinos dos componentes
const int led_amarelo = 32;
const int led_verde = 33;
const int buzzer = 22;
const int trigPin = 15;
const int echoPin = 21;
long tempo;
float distancia;
int porc = 0;

// Variáveis para o LCD
const int screenWidth = 16;
const int screenHeight = 2;

// Configurações do WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações do TagoIO
const char* tagoToken = "3b478277-a9bf-489a-8507-b94736b12ad5";
const char* tagoEndpoint = "https://api.tago.io/data";

void setup(){
  // Configurações dos pinos
  pinMode(led_amarelo, OUTPUT);
  pinMode(led_verde, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Inicialização do Serial
  Serial.begin(9600);

  // Inicialização do LCD
  lcd.begin(screenWidth, screenHeight);

  // Conexão WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  lcd.clear();
  medir();
  porcentagem();
  lcd.setCursor(4, 0); // coluna, linha
  lcd.print(distancia);
  lcd.print("cm");
  lcd.setCursor(4, 1); // coluna, linha
  lcd.print(porc);
  lcd.print("%");
  delay(100);

  if (porc >= 99) {
    alarme();
    digitalWrite(led_amarelo, HIGH);
    digitalWrite(led_verde, LOW);
  } else {
    digitalWrite(led_amarelo, LOW);
    digitalWrite(led_verde, HIGH);
  }

  // Envia as informações para o TagoIO
  sendToTago(distancia, porc);
}

void alarme() {
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);
}

void porcentagem() {
  porc = map(distancia, 5, 90, 100, 0); // converter de cm a %
}

void medir() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  tempo = pulseIn(echoPin, HIGH); // us=microsegundos
  distancia = float(tempo * 0.0343) / 2;
  delay(10);
}

void sendToTago(float distancia, int porcentagem) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(tagoEndpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Device-Token", tagoToken);

    // Dados a serem enviados para o TagoIO
    String jsonData = "[{\"variable\": \"distancia\", \"value\": " + String(distancia) + "}, {\"variable\": \"porcentagem\", \"value\": " + String(porcentagem) + "}]";
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
