---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

🧾 Kapitel 5: Konfigurationsdatei modbus_gateway.ini
📄 Allgemeines

Die Konfigurationsdatei steuert alle wesentlichen Komponenten des Gateways:

    Serielle Modbus RTU Verbindung

    Modbus TCP Server

    MQTT Anbindung

    TLS-Einstellungen

    Logging

    Diagnoseverhalten

Sie ist im INI-Format aufgebaut und wird beim Start geladen.
🔧 Allgemeine Struktur

[general]
log_file = /var/log/modbus_gateway.log
debug_hex_dump = false
mqtt_stats_interval = 60

[modbus]
serial_port = /dev/ttyUSB0
baudrate = 19200
parity = N
data_bits = 8
stop_bits = 1
timeout = 1000
tcp_port = 502

[mqtt]
enabled = true
broker = tcp://192.168.0.82:1883
client_id = modbus_gateway
username = user
password = pass
topic_prefix = modbus/
lwt_payload = offline
online_payload = online
qos = 1

[tls]
use_tls = false
cafile = /etc/ssl/certs/ca.crt
certfile = /etc/ssl/certs/client.crt
keyfile = /etc/ssl/private/client.key
insecure = false

🧰 [general] – Allgemeine Optionen
Schlüssel	Beschreibung
log_file	Pfad zur Logdatei
debug_hex_dump	Wenn true: Rohdaten als Hex-Dump im Log anzeigen
mqtt_stats_interval	Intervall (Sekunden) für automatische MQTT-Statistikmeldung; 0 = deaktiviert
🔌 [modbus] – Serielle RTU & TCP Einstellungen
Schlüssel	Beschreibung
serial_port	Pfad zur seriellen Schnittstelle
baudrate	Baudrate der RTU-Verbindung (z. B. 19200)
parity	Parität: N, E, O
data_bits	Anzahl Datenbits (meist 8)
stop_bits	Stopbits (1 oder 2)
timeout	Antwort-Timeout in Millisekunden
tcp_port	Port, auf dem der Modbus TCP Server lauscht
☁️ [mqtt] – MQTT Konfiguration
Schlüssel	Beschreibung
enabled	true oder false – MQTT aktivieren
broker	Adresse des MQTT-Brokers, z. B. tcp://192.168.0.82:1883
client_id	MQTT-Client-ID
username	Benutzername (optional)
password	Passwort (optional)
topic_prefix	Alle MQTT-Themen beginnen damit, z. B. modbus/
lwt_payload	Wird bei unerwarteter Trennung gesendet, z. B. offline
online_payload	Wird nach erfolgreichem Connect gesendet, z. B. online
qos	Quality of Service (0, 1 oder 2)
🔐 [tls] – TLS/SSL-Verschlüsselung für MQTT
Schlüssel	Beschreibung
use_tls	true oder false – TLS aktivieren
cafile	Pfad zur CA-Datei des Brokers
certfile	Client-Zertifikat (optional)
keyfile	Private Key für Client-Zertifikat
insecure	true = TLS-Zertifikat des Brokers wird nicht geprüft (unsicher, aber hilfreich)

    🔒 TLS kann dynamisch aktiviert werden – siehe Kapitel 11: TLS & Sicherheit.

🛠️ Validierung

Beim Start prüft init_config() alle Pfade und Werte:

    TLS-Dateien müssen lesbar sein

    Warnungen werden ins Log geschrieben

    Bei kritischen Fehlern (z. B. fehlende serielle Schnittstelle) wird das Programm beendet

✅ Beispielkonfiguration (minimal funktionsfähig)

[modbus]
serial_port = /dev/ttyUSB0
baudrate = 19200

[mqtt]
enabled = true
broker = tcp://192.168.0.82:1883
topic_prefix = modbus/

