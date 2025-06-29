---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

📡 Kapitel 8: MQTT Integration
🌐 Überblick

Das Gateway integriert MQTT als zweite Kommunikationsschicht. Dadurch können:

    Modbus-Werte über MQTT abgerufen und geschrieben werden

    Diagnose- und Statistikdaten periodisch publiziert werden

    Kommandos wie Neustart, Konfig-Abfrage oder Statistik-Trigger ausgelöst werden

🔧 MQTT-Basisparameter

Alle MQTT-Einstellungen werden in der modbus_gateway.ini gesetzt (siehe Kapitel 5):
Parameter	Bedeutung
mqttBroker	Adresse des MQTT-Brokers (mqtt:// oder mqtts://)
mqttClientID	ID des Clients (z. B. modbus-gateway-pi)
mqttTopicPrefix	Prefix für Topics (z. B. modbus/)
mqttUser, mqttPass	Authentifizierung (optional)
mqttLwtPayload	"Last Will"-Nachricht (z. B. offline)
mqttStatsInterval	Intervall für Statistikmeldungen (Sekunden)
mqttUseTLS	TLS-Verschlüsselung (true/false)
mqttCAFile, mqttCertFile, mqttKeyFile	Pfade zu TLS-Zertifikaten
mqttInsecure	TLS-Zertifikatsprüfung abschalten (unsicher)
🔄 Unterstützte Topics
📥 Eingehende Topics (subscribe)
Topic	Typ	Beschreibung
modbus/coil/write	JSON	Setze Coil (digitales Ausgangsbit)
modbus/holding/write	JSON	Schreibe Holding Register (16-Bit)
modbus/command/stats	String	Löst sofortigen Statistik-Report aus
modbus/command/restart	String	Führt einen Soft-Neustart durch (falls aktiviert)
Beispiel: Schreiben eines Coils

{
  "slave": 1,
  "address": 10,
  "value": true
}

→ wird an modbus/coil/write gesendet
Beispiel: Schreiben eines Holding-Registers

{
  "slave": 2,
  "address": 100,
  "value": 12345
}

→ wird an modbus/holding/write gesendet
📤 Ausgehende Topics (publish)
Topic	Inhalt
modbus/stats	JSON mit periodischer Statistik
modbus/log	Lognachrichten
modbus/status	online oder offline (LWT/Startup)
modbus/error	Letzter aufgetretener Fehler
Beispiel: modbus/stats

{
  "tcp_requests": 212,
  "rtu_success": 208,
  "rtu_failures": 4,
  "dropped": 0,
  "echo_replies": 5
}

🔒 TLS-gesicherte MQTT-Verbindung

Falls mqttUseTLS = true gesetzt ist:

    Verbindung erfolgt via mqtts://

    Zertifikate (CA, Client, Key) müssen vorhanden und lesbar sein

    Optional kann mit mqttInsecure = true die Prüfung deaktiviert werden (nicht empfohlen)

🧪 Manuelle Tests
Mit mosquitto_pub

mosquitto_pub -t modbus/command/stats -m "trigger" -h 192.168.0.82

Mit mosquitto_sub

mosquitto_sub -t "modbus/#" -v -h 192.168.0.82

📋 Hinweise

    Alle Schreiboperationen erfolgen asynchron – Rückmeldung über Erfolg per log oder error

    Statistiken sind vollständig dokumentiert in Kapitel 10

