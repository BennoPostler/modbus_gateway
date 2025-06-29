---
title: "Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi"
layout: default
---

🧪 Kapitel 16: Beispiele – Modbus TCP, MQTT, YAML-Konfiguration

In diesem Kapitel findest du eine Vielzahl von praktischen Beispielen, die dir als Referenz und Testhilfe dienen – ideal für Entwickler, Integratoren und zur Fehlersuche im Betrieb.
⚡ 16.1 Beispiel: Modbus TCP mit mbpoll

Zum direkten Test des TCP-RTU-Gateways kannst du mbpoll verwenden.
📥 Lese Holding Register 40001 (Adresse 0)

mbpoll -m tcp -a 1 -r 0 -c 2 192.168.0.50 -p 502

    -m tcp: TCP-Modus

    -a 1: Unit ID (Slave-Adresse)

    -r 0: Startadresse

    -c 2: Anzahl Register

    -p 502: TCP-Port

🧠 16.2 Beispiel: Lokale Diagnosefunktion (Echo)

Das Gateway antwortet direkt auf diese Diagnosefunktion ohne Weiterleitung:

mbpoll -m tcp -a 1 -r 0 -t 8:0 192.168.0.50

    -t 8:0: Function Code 8 (Diagnose), Subfunction 0x0000 (Loopback/Echo)

Erwartete Antwort: Echo der gesendeten Datenbytes.
🛰 16.3 Beispiel: MQTT – Einfache Abfrage per mosquitto_pub
📤 Holding Register lesen

mosquitto_pub -h 192.168.0.82 -t "modbus/request/holding" -m '{
  "unit": 1,
  "address": 0,
  "count": 2
}'

→ Antwort (Topic: modbus/response/holding/1/0):

{
  "unit": 1,
  "address": 0,
  "values": [123, 456]
}

🧪 Statistiken abrufen

mosquitto_pub -h 192.168.0.82 -t "modbus/command/stats" -m ""

→ Antwort (Topic: modbus/stats):

{
  "tcp_requests": 172,
  "rtu_success": 165,
  "rtu_failures": 7,
  "dropped": 0,
  "echo_replies": 3
}

📚 16.4 Beispiel: modbus_gateway.ini

[network]
tcp_port = 502
log_file = /var/log/modbus_gateway.log

[serial]
device = /dev/ttyUSB0
baudrate = 19200
parity = N
data_bits = 8
stop_bits = 1

[mqtt]
enabled = true
broker = 192.168.0.82
client_id = modbus-gateway
topic_prefix = modbus/
qos = 1
retain = false

[tls]
use_tls = false

[advanced]
mqtt_stats_interval = 60
debug_hex_dump = true

💡 Hinweise

    Die MQTT-Kommandos unterstützen optional ein Feld "id" für Referenzen

    Lokale Diagnosefunktionen können direkt im Gateway bearbeitet werden

    YAML-Kompatibilität ist über Konvertierung möglich (z. B. mit yq)

