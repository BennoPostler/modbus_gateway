modbus_gateway – Dokumentation

    Vollständige Referenz zur Modbus TCP ↔ RTU + MQTT Gateway-Software
    Zielgruppe: Entwickler, Systemintegratoren, Techniker
    Plattform: Raspberry Pi (aarch64), Linux-kompatibel
    Autor: Benno, in Zusammenarbeit mit ChatGPT
    Version: 1.0.0
    Stand: 2025-06-26

Inhaltsverzeichnis

    Einleitung

    Funktionsüberblick

    Architekturübersicht

    Verwendete Technologien & Bibliotheken

    Verzeichnisstruktur

    Kompilierung & Build-System

    Installation & Systemintegration

    Startparameter & Aufruf

    INI-Konfiguration im Detail

    Modbus RTU Unterstützung

    Modbus TCP Anbindung

    MQTT Integration

    Modbus <-> MQTT Bridge

    Statistiken & Status

    TLS-Unterstützung für MQTT

    Logging

    Hexdump-Debugging

    Lokale Diagnosefunktionen

    Fehlerbehandlung & Reconnect-Logik

    Systemd-Serviceeinbindung

    Beispiele: MQTT-Topics

    Beispiele: Modbus TCP ↔ RTU

    Anhang: Komplette INI-Vorlage

1. Einleitung

Dieses Projekt stellt ein universelles Modbus-Gateway bereit, das:

    Modbus TCP ↔ Modbus RTU überbrückt

    MQTT-Anbindung bietet

    lokale Diagnoselogik enthält

    komplett konfigurierbar über .ini-Datei ist

    auf Systeme wie den Raspberry Pi (auch 64 Bit, RPi 5) ausgelegt ist

Es ist primär für industrielle oder smarthome-nahe Anwendungen gedacht, in denen eine zentrale Steuerung über MQTT oder TCP gewünscht ist – z. B. bei Wärmepumpen, Wechselrichtern, Sensoren etc.
2. Funktionsüberblick
Funktion	Beschreibung
Modbus TCP → RTU	Weiterleitung von Modbus TCP-Anfragen über RS485
MQTT ↔ Modbus RTU	Lesen/Schreiben einzelner Register und Coils per MQTT
TLS für MQTT	Optional verschlüsselte Kommunikation
Statistik	Periodisch oder auf Befehl per MQTT
INI-Konfiguration	Klar gegliedert, editierbar auch ohne GUI
Hexdump	Optionaler binärer Datenlog für Debugzwecke
Diagnose via Funktionscode 0x08	Echo, Fehlerzählung, Reset
Reconnect-Logik	Robuste MQTT-Wiederverbindung nach Netzverlust
Systemd-Unterstützung	Automatischer Start als Dienst
3. Architekturübersicht

+------------------+      MQTT       +------------------+
|   MQTT Broker    |<----------------|  modbus_gateway  |
| (z. B. Mosquitto) |                |                  |
+------------------+                 |                  |
                                     |                  |
            TCP 502                  |                  |
    +------------------+             |                  |
    | Modbus TCP Client |----------->|                  |
    |  (z. B. SCADA)    |            |                  |
    +------------------+             |                  |
                                     |     Linux        |
                                     |     Socket       |
                                     |     Serial       |
                                     +--------+---------+
                                              |
                                        RS485 ⬇
                                     +--------+--------+
                                     |  Modbus RTU     |
                                     |  Geräte (Slaves)|
                                     +-----------------+

4. Verwendete Technologien & Bibliotheken
Komponente	Beschreibung
libmodbus	Bibliothek für Modbus RTU und TCP (libmodbus)
Paho MQTT C++	MQTT-Client-Bibliothek (TLS-fähig) (paho-mqttpp3)
INIh	Sehr kleine INI-Parser-Bibliothek
C++17	Standard für modernen Code, Multithreading etc.
POSIX APIs	Für Netzwerk, Signale, Dateioperationen etc.
5. Verzeichnisstruktur

modbus_gateway/
├── modbus_gateway.cpp        # Hauptprogramm
├── ini.c / ini.h             # INI-Konfigurationsparser
├── modbus_gateway.service    # systemd-Dienstdatei
├── Makefile                  # Kompilierungsanweisungen
├── config.example.ini        # Beispielkonfiguration
├── modbus_gateway.md         # Diese Dokumentation
└── logs/                     # (optional) Logverzeichnis

6. Kompilierung & Build-System
Voraussetzungen

    Betriebssystem: Linux (z. B. Raspberry Pi OS, Fedora, Debian)

    Compiler: g++ (C++17-kompatibel)

    Abhängigkeiten:

Paketname (Debian)	Zweck
libmodbus-dev	Modbus RTU/TCP
libpaho-mqttpp3-dev	MQTT C++-Client
libssl-dev	TLS-Unterstützung für MQTT

# Debian / Ubuntu:
sudo apt install g++ make libmodbus-dev libpaho-mqttpp3-dev libssl-dev

Kompilieren

make

Erzeugt:

./modbus_gateway

Mit TLS-Option:

make TLS=1

Clean

make clean

7. Installation & Systemintegration
Schritt-für-Schritt (manuell):

sudo cp modbus_gateway /usr/local/bin/
sudo cp modbus_gateway.service /etc/systemd/system/
sudo mkdir -p /etc/modbus_gateway/
sudo cp config.example.ini /etc/modbus_gateway/modbus_gateway.ini
sudo systemctl daemon-reexec
sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway

Systemd-Service

modbus_gateway.service:

[Unit]
Description=Modbus TCP-RTU-MQTT Gateway
After=network.target

[Service]
ExecStart=/usr/local/bin/modbus_gateway /etc/modbus_gateway/modbus_gateway.ini
User=modbus
Restart=on-failure
RestartSec=3
LimitNOFILE=4096

[Install]
WantedBy=multi-user.target

8. Startparameter & Aufruf

./modbus_gateway [config.ini] [Optionen]

Optionen
Parameter	Beschreibung
--help, -h	Hilfe anzeigen
--version	Version anzeigen
--dry-run	Nur Konfiguration prüfen, keine Verbindung aufbauen
--validate-only	Alias für --dry-run
Beispiele

./modbus_gateway /etc/modbus_gateway/modbus_gateway.ini
./modbus_gateway /etc/modbus_gateway/modbus_gateway.ini --dry-run

9. INI-Konfiguration im Detail

Beispieldatei: /etc/modbus_gateway/modbus_gateway.ini

Sektion [modbus]
Schlüssel	Beschreibung
serial_port	Pfad zur seriellen Schnittstelle
baudrate	Baudrate (z. B. 19200, 9600)
parity	Parität: none, even, odd
data_bits	Datenbits (i. d. R. 8)
stop_bits	Stoppbits (1 oder 2)

Sektion [tcp]
Schlüssel	Beschreibung
port		TCP-Port, auf dem gelauscht wird

Sektion [mqtt]
Schlüssel	Beschreibung
broker	Adresse des Brokers (mqtt://... oder ssl://...)
client_id	MQTT-Client-ID
username	Benutzername (optional)
password	Passwort (optional)
topic_prefix	Basis-Topic, z. B. modbus/
will_payload	LWT-Nachricht (z. B. "offline")
online_payload	Online-Nachricht (z. B. "online")

TLS (optional)
Schlüssel	Beschreibung

use_tls	true oder false
cafile	CA-Zertifikat
certfile	Client-Zertifikat
keyfile	Private Key
insecure	Deaktiviert TLS-Zertifikatsprüfung
Statistik & Debug
Schlüssel	Beschreibung
stats_interval	Sekundentakt für MQTT-Statistiken
logfile	Pfad zur Logdatei
debug_hex_dump	true → Hex-Ausgabe der Rohdaten


10. Modbus RTU Unterstützung

Das Gateway verwendet die libmodbus für die serielle Kommunikation über Modbus RTU.
Einstellungen (aus INI)

[serial]
port = /dev/ttyAMA0
baudrate = 9600
parity = N
databits = 8
stopbits = 1

    Der Modbus-RTU-Slave kann flexibel zur Laufzeit gesetzt werden.

    Response-Timeouts sind fest auf 1 Sekunde (MODBUS_RTU_TIMEOUT_SEC) gesetzt.

RTU-Antwort

Der Modbus-RTU-Response wird vollständig eingelesen, optional im Hex-Dump ausgegeben und als Modbus-TCP-Antwort rekonstruiert:

uint8_t response[MODBUS_RTU_MAX_ADU_LENGTH];
int res_len = modbus_receive_confirmation(rtu_ctx, response);

Wenn debugHexDump = true, wird ein Hexdump erzeugt:

← RTU RX: 01 03 04 00 2A 00 07 F9 B6

11. Modbus TCP Anbindung

Das Gateway öffnet einen TCP-Listener auf Port 502 (konfigurierbar):

[tcp]
port = 502

Funktionsweise:

    Empfängt Modbus-TCP-Frames

    Extrahiert den MBAP-Header (Transaction ID, Unit ID, ...)

    Sendet Anfrage als RTU-Frame über die serielle Schnittstelle

    Wartet auf RTU-Antwort

    Rekonstruiert TCP-Antwortpaket

Besonderheiten:

    TCP-Verbindungen erhalten einen Timeout von 60 s

    Unterstützt auch lokale Verarbeitung von Diagnose-Funktion 08 (z. B. Echo-Test)

12. MQTT Integration

Verwendete Bibliothek: Eclipse Paho MQTT C++

Verbindung zu MQTT-Broker via tcp://, ssl:// oder mqtts://.
Topics (Basis: modbus/)
Kategorie	Beispiel		Beschreibung

Status		modbus/status		"online" / "offline" (LWT)
Logging		modbus/log		Textnachrichten
Statistik	modbus/stats		JSON mit Zählern
Reset-Stats	modbus/stats/reset	Befehl zum Zurücksetzen der Statistik
Einzelregister	modbus/read/123		Lese Modbus-Holding Register 123
Schreiben	modbus/write/123	Setze Register 123
Coils lesen	modbus/read_coil/10	Lies Coil 10
Coils schreiben	modbus/write_coil/10	Setze Coil 10
Discrete Inputs	modbus/read_discrete/20	Lies digitalen Eingang
Input Registers	modbus/read_input/5	Lies Input Register

13. Modbus <-> MQTT Bridge

Das Herzstück des Gateways ist die Brücke zwischen MQTT und Modbus:
Lesen via MQTT
Holding Register

Topic: modbus/read/100
→ Antwort: modbus/read/100 → 456

Mehrfachlesen (Register)

Topic: modbus/read_multi/200:4
→ Antwort: modbus/read_multi/200:4 → 123,456,789,12

Discrete Inputs

Topic: modbus/read_discrete/10
→ Antwort: 1

Schreiben via MQTT
Holding Register

Topic: modbus/write/123
Payload: 42
→ Setzt Register 123 auf 42

Coil

Topic: modbus/write_coil/1
Payload: 1
→ Setzt Coil 1 auf HIGH

On-Demand Statistik

Topic: modbus/command/stats
→ Antwort: modbus/stats → JSON oder Text


14. MQTT Topics – vollständige Referenz

Die folgenden Topics sind abonniert und verarbeitet:
14.1 Lesezugriffe
Topic	Beschreibung		Antwort-Topic (falls vorhanden)
modbus/read/<addr>		Liest 1 Holding Register	modbus/read/<addr>
modbus/read_input/<addr>	Liest 1 Input Register	modbus/read_input/<addr>
modbus/read_coil/<addr>		Liest 1 Coil	modbus/read_coil/<addr>
modbus/read_discrete/<addr>	Liest 1 Discrete Input	modbus/read_discrete/<addr>
modbus/read_multi/<addr>:<n>	Liest mehrere Holding Register	modbus/read_multi/<addr>:<n>
modbus/read_input_multi/<a>:<n>	Liest mehrere Input Register	modbus/read_input_multi/...
modbus/read_coil_multi/<a>:<n>	Liest mehrere Coils	modbus/read_coil_multi/...
modbus/read_discrete_multi/...	Liest mehrere Discrete Inputs	modbus/read_discrete_multi/...
14.2 Schreibzugriffe
Topic				Beschreibung		Payload
modbus/write/<addr>		Setzt Holding Register	Zahl
modbus/write_coil/<addr>	Setzt Coil (0 oder 1)	0/1
14.3 Steuerung
Topic	Beschreibung
modbus/stats/reset		Setzt alle Statistik-Zähler auf 0
modbus/command/stats		Sendet aktuelle Statistik als Text
14.4 Status / System
Topic				Beschreibung
modbus/status			"online" beim Start, "offline" als LWT
modbus/log			Gateway-Protokollnachrichten
modbus/stats			JSON mit laufenden Metriken
15. Logging und Diagnose

Das Logging ist doppelt implementiert:

    Dateiloggung über config.logFilePath (/var/log/modbus_gateway.log)

    MQTT-Logging über modbus/log (nur bei aktivem MQTT)

Beispiel:

2025-06-26 12:42:31 - Fehler beim Lesen von Register 40001: Connection timeout

16. Statistiksystem

Das Statistiksystem läuft im Hintergrund:
Aktivierung:

[mqtt]
stats_interval = 60

Metriken (JSON):

{
  "total_tcp_requests": 232,
  "total_rtu_requests": 228,
  "total_errors": 2,
  "rtu_success": 226,
  "rtu_failures": 2,
  "dropped": 0,
  "echo_replies": 1,
  "last_success_timestamp": 1719419331
}

Interaktiv zurücksetzen:

→ Topic: modbus/stats/reset

17. Erweiterte Diagnosefunktionen

Das Gateway unterstützt Modbus-Funktion 08 (Diagnose) lokal:
Subfunktion	Bedeutung	Verhalten des Gateways
0x0000	Echo-Test	Lokale Rückgabe der Anfrage
0x000A	Statistik zurücksetzen	Zähler auf 0 setzen
0x000B	Anzahl TCP-Anfragen ausgeben	TCP-Zähler als Antwort liefern
0x000C	Anzahl Fehler zurückgeben	Fehlerzähler als Antwort liefern

Antwort wird korrekt als TCP-Response zurückgegeben, inklusive MBAP.
18. Sicherheit: TLS, Benutzername, Passwort

Die MQTT-Verbindung kann verschlüsselt werden.
TLS aktivieren:

[mqtt]
use_tls = 1
ca_file = /etc/ssl/certs/ca.crt
cert_file = /etc/ssl/certs/client.crt
key_file = /etc/ssl/private/client.key
insecure = 0  ; Prüfung aktiv

    ⚠️ Falls insecure = 1, wird die Serverzertifikatsprüfung deaktiviert.

Benutzername/Passwort:

username = gatewayuser
password = geheim123


9. Codeanalyse – Übersicht & Architektur
19.1 Zentrale Komponenten
Komponente	Beschreibung
main()	Initialisiert alle Module, startet Gateway
accept_clients()	TCP-Server, akzeptiert neue Verbindungen
handle_client()	verarbeitet Modbus-TCP → Modbus-RTU
MQTTCallback	empfängt MQTT-Nachrichten & verarbeitet sie
publish_stats()	Hintergrundthread zur Statistikausgabe
config_handler()	Lädt .ini-Konfiguration
19.2 Wichtige Threads

    Client-Thread pro TCP-Verbindung

    Statistik-Thread für regelmäßige Metrik-Publikation

    MQTT-Reconnect-Thread im Fehlerfall

20. Buildsystem – Makefile

Ein einfaches Makefile:

CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17
LDFLAGS = -lmodbus -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread

SRC = modbus_gateway.cpp ini.c
BIN = modbus_gateway

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BIN)

Build via:

make

21. Systemstart – systemd-Service

Erstelle /etc/systemd/system/modbus_gateway.service:

[Unit]
Description=Modbus TCP↔RTU + MQTT Gateway
After=network.target

[Service]
ExecStart=/usr/local/bin/modbus_gateway /etc/modbus_gateway.ini
WorkingDirectory=/etc
User=modbus
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target

Dann aktivieren:

sudo systemctl daemon-reexec
sudo systemctl enable --now modbus_gateway.service

22. Erweiterbarkeit & zukünftige Module
Geplante Features

Modbus-RTU → TCP Proxy (Rückrichtung)

Priorisierung von MQTT-Befehlen

REST-API statt nur MQTT

    Webinterface zur Konfiguration

Erweiterungsideen
Ziel	Idee
Visualisierung	Node-RED Flow basierend auf Metriken
Zeitsteuerung	CRON- oder Zeitlogik über MQTT/REST
PV-Steuerung	Steuerlogik bei Energieüberschuss (via MQTT)
Sicherheitszonen	MQTT-Schreibschutz durch Tokenmechanismus
23. Changelog (Auszug)
Datum	Änderung
2025-06-24	Erste vollständige Version mit TLS, Statistik, MQTT & RTU
2025-06-25	Coils, Discrete Inputs, Mehrfachlese-Befehle
2025-06-26	systemd-Support, Hex-Dump-Debug, vollständig rekonstruierter MBAP Header


