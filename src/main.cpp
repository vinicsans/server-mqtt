#include <Arduino.h>
#include "internet.h" // Certifique-se que este arquivo existe na pasta
#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// --- Configurações de Hardware ---
const int LED_PIN = 2; // LED Interno da ESP32 (Geralmente GPIO 2)

// --- Configurações MQTT ---
const char *mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char *mqtt_client_id = "senai134_esp_full_vinicius"; 
const char *mqtt_topic_sub = "senai134/devgoogle/sub"; 
const char *mqtt_topic_pub = "senai134/devgoogle/pub";

WiFiClient espClient;
PubSubClient client(espClient);

void conectaMqtt();
void retornoMqtt(char *, byte *, unsigned int);

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(100); 

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  conectaWiFi(); 

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(retornoMqtt); 
}

void loop()
{
  checkWiFi(); 

  if (!client.connected())
  {
    conectaMqtt();
  }

  client.loop(); 

  if (Serial.available() > 0)
  {
    String textoDigitado = Serial.readStringUntil('\n');
    textoDigitado.trim();

    if (textoDigitado.length() > 0)
    {
      JsonDocument doc;
      doc["disp"] = "ESP32_Vinicius";
      doc["msg"] = textoDigitado;
      doc["time"] = millis();
      
      String stringJson;
      serializeJson(doc, stringJson);

      client.publish(mqtt_topic_pub, stringJson.c_str());
      
      Serial.print(">> Enviado: ");
      Serial.println(stringJson);
    }
  }
}

void conectaMqtt()
{
  while (!client.connected())
  {
    Serial.print("Conectando ao MQTT...");
    if (client.connect(mqtt_client_id))
    {
      Serial.println("Conectado!");
      client.subscribe(mqtt_topic_sub);
      Serial.println("Assinado em: " + String(mqtt_topic_sub));
    }
    else
    {
      Serial.print("Falha: ");
      Serial.print(client.state());
      Serial.println(" Tentando em 5s...");
      delay(5000);
    }
  }
}

void retornoMqtt(char *topic, byte *payload, unsigned int length)
{
  Serial.println("\n<< Mensagem Recebida!");

  String mensagemRecebida = "";
  for (int i = 0; i < length; i++)
  {
    mensagemRecebida += (char)payload[i];
  }
  Serial.println("JSON: " + mensagemRecebida);

  JsonDocument doc;
  DeserializationError erro = deserializeJson(doc, mensagemRecebida);

  if (erro)
  {
    Serial.print("Erro no JSON: ");
    Serial.println(erro.c_str());
    return;
  }

  String msg = doc["msg"].as<String>(); 
  const char* disp = doc["disp"];

  Serial.printf("Remetente: %s | Comando: %s\n", disp, msg.c_str());

  if (msg.equalsIgnoreCase("on")) 
  {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Status: LED LIGADO");
  }
  else if (msg.equalsIgnoreCase("off"))
  {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Status: LED DESLIGADO");
  }
  
  Serial.println("---------------------------");
}