---
title: "Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi"
layout: default
---

📎 Kapitel 23: Anhang

Dieses Kapitel enthält ergänzende Informationen, Referenzlisten, Tools sowie wichtige Tabellen, die für Entwicklung, Integration und Fehlersuche hilfreich sind.
🧾 23.1 Übersicht der MQTT-Topics
Topic	Beschreibung	Modus
modbus/request/read/holding	Leseanfrage an Holding Register	➜ Subscribe
modbus/request/read/input	Leseanfrage an Input Register	➜ Subscribe
modbus/request/read/coils	Leseanfrage an Coils	➜ Subscribe
modbus/request/read/discrete	Leseanfrage an Discrete Inputs	➜ Subscribe
modbus/request/write/holding	Schreibanfrage an Holding Register	➜ Subscribe
modbus/request/write/coils	Schreibanfrage an Coils	➜ Subscribe
modbus/response/...	Antwort auf alle obigen Operationen	➜ Publish
modbus/command/stats	Trigger zum manuellen Versand der Statistiken	➜ Subscribe
modbus/stats	Automatisch erzeugte Statistikdaten	➜ Publish
modbus/log	Laufzeit-Lognachrichten (optional)	➜ Publish
modbus/status	Lebenszeichen (Online-/Offline-Anzeige)	➜ LWT / Publish
🛠️ 23.2 Konfigurationsoptionen der modbus_gateway.ini
Option	Typ	Beschreibung
mqtt_broker	String	IP oder Hostname des MQTT-Brokers
mqtt_port	Integer	TCP-Port, üblicherweise 1883 (oder 8883 für TLS)
mqtt_client_id	String	Eindeutiger Name des MQTT-Clients
mqtt_topic_prefix	String	Basis-Prefix für Topics
mqtt_stats_interval	Integer	Sekundenintervall für automatische Statistik
mqtt_use_tls	Bool	TLS-Verbindung verwenden
mqtt_ca_file	Pfad	Pfad zur CA-Datei
mqtt_cert_file	Pfad	Pfad zum Client-Zertifikat
mqtt_key_file	Pfad	Pfad zum Private Key
mqtt_insecure	Bool	TLS-Verifikation deaktivieren
serial_port	String	z. B. /dev/ttyAMA0
baudrate	Integer	Modbus-Baudrate
parity	String	N, E oder O
data_bits	Integer	Üblicherweise 8
stop_bits	Integer	1 oder 2
tcp_port	Integer	Port für den Modbus-TCP-Listener
log_file_path	Pfad	Pfad für Logfile
debug_hex_dump	Bool	Rohdaten in Hex anzeigen
reset_pin	Integer	GPIO-Pin für Reset-Mechanismus (optional)
🧪 23.3 Modbus-Diagnosefunktionen (lokal beantwortet)
Function Code	Subfunktion	Beschreibung	Lokal beantwortet?
0x08	0x0000	Echo-Test	✅ ja
0x08	0x000A	Zähler zurücksetzen	✅ ja
0x08	0x000B	Rückgabe: TCP-Zähler	✅ ja
0x08	0x000C	Rückgabe: Fehlerzähler	✅ ja
0x08	(alle anderen)	Weiterleitung an RTU	✅ sofern erlaubt
📜 23.4 Build & Deployment Tools

Empfohlene Befehle:

make clean
make all
sudo make install

Systemd-Benutzung:

sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway
sudo journalctl -u modbus_gateway -f

🧮 23.5 Beispiel: Statistik-MQTT-Payload

{
  "tcp_requests": 12345,
  "rtu_success": 12000,
  "rtu_failures": 345,
  "dropped": 2,
  "echo_replies": 17
}

🧠 23.6 FAQ

F: Was passiert, wenn MQTT ausfällt?
A: Das Gateway funktioniert als TCP↔RTU-Proxy weiter. MQTT wird beim Reconnect reaktiviert.

F: Kann ich nur Modbus TCP verwenden?
A: Ja, MQTT ist optional.

F: Kann ich mehrere Clients gleichzeitig verbinden?
A: Ja, durch Multi-Threading (accept_clients()).

F: Wie viele Register kann ich in einer Anfrage lesen?
A: Bis zu 125 Holding/Input Register, 2000 Coils (laut Modbus-Spezifikation).

