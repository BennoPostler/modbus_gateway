# Vorwort
🔗 [📄 English (README.en.md)](README.en.md)
The english version of the readme.md will follow shortly.  
In the meantime, use the translation function of your browser or google translate.  

Diese Software ist in einem lauffähigen Zustand.  
Ich bin aber noch am erweitern der Funktionalität.  
Wenn Interesse besteht gerne Kommentare, Verbesserungsvorschläge Melden. 



# Modbus Gateway – Dokumentation

## 1. Einführung und Überblick

Dieses Projekt stellt ein robustes und flexibles Gateway bereit, das eine Brücke zwischen Modbus TCP, Modbus RTU und MQTT schlägt. Es eignet sich ideal für die Integration klassischer Industrieanlagen (z. B. Wärmepumpen, Energiezähler, Steuerungen) in moderne IoT- oder Smart-Home-Umgebungen.

**Zentrale Eigenschaften:**

- Vermittlung zwischen **Modbus TCP ↔ RTU**
- Erweiterung um **MQTT-Kommunikation**
- Unterstützung für **Coils**, **Discrete Inputs**, **Input Registers**, **Holding Registers**
- **Mehrfachlese-Operationen** via MQTT
- **TLS-Verschlüsselung** für MQTT (optional)
- **Konfigurierbar** via `.ini`-Datei
- **Logfile-Ausgabe** und MQTT-Log
- Vollständige Unterstützung von **modbus Diagnose-Funktionen (0x08)**
- Statistik- und Diagnosedaten über MQTT

**Zielgruppen:**

- **Entwickler**, die Modbus-Systeme mit modernen IoT-Lösungen verbinden möchten
- **Systemintegratoren**, die bestehende Industrieanlagen mit MQTT-fähigen Systemen vernetzen
- **Systemadministratoren**, die eine wartbare, protokollkonforme und automatisierbare Lösung suchen

**Typische Einsatzzwecke:**

- Anbindung von Wärmepumpen oder PV-Systemen an Smart Home
- Ferndiagnose und Fernsteuerung über MQTT
- Logging und Visualisierung mit Node-RED, InfluxDB, Grafana etc.
- Ersatz oder Erweiterung bestehender Gateways mit TCP/RTU-Übersetzung

Die Anwendung läuft nativ unter **Linux (Raspberry Pi OS, Debian, Fedora)** und kann über `systemd` als Dienst gestartet werden. Ein MQTT-Broker (z. B. Mosquitto) wird für den vollen Funktionsumfang benötigt.

> Die Software ist vollständig in C++ geschrieben und nutzt moderne Bibliotheken wie libmodbus, Paho MQTT C++ und eine einfache INI-Konfigurationslogik.

2. Systemübersicht & Architektur

Das modbus_gateway ist ein modular aufgebautes Software-System für den Einsatz auf Geräten wie dem Raspberry Pi 5 (aarch64), das Modbus TCP und Modbus RTU bidirektional überbrückt, zusätzlich MQTT-Schnittstellen zur Verfügung stellt und dabei Monitoring, Logging, sowie Diagnosefunktionen nach Modbus-Norm implementiert.

2.1. Hauptfunktionen

    🔁 Modbus TCP ↔ RTU Gateway
    Weiterleitung von TCP-Anfragen über ein serielles RTU-Interface an ein oder mehrere Modbus-Slaves.

    ☁️ MQTT-Anbindung
    Direkter Zugriff auf Modbus-Register (lesen/schreiben) per MQTT Topics inkl. Mehrfachoperationen.

    📊 Statistiken & Diagnosedaten
    Veröffentlichung von Betriebsdaten über MQTT und Rückgabe über Diagnose-Subfunktionen.

    📁 INI-basierte Konfiguration
    Alle Betriebsparameter werden über eine leicht lesbare .ini-Datei konfiguriert.

    🛡️ TLS-Verschlüsselung
    MQTT-Verbindungen können optional per TLS abgesichert werden.

    📡 Watchdog- und Keepalive-Mechanismen
    Unterstützung von MQTT Keepalive, automatischer Reconnect, TCP-Zeitlimits.

2.2. Komponentenübersicht

+----------------+      Modbus TCP       +----------------+
| TCP Client(s)  | <------------------>  |   Gateway       |
|  (z. B. SCADA) |                      | (modbus_gateway)|
+----------------+                      +--------+--------+
                                                  |
                                                  | Serial Line (RTU)
                                                  |
                                          +-------+--------+
                                          | Modbus Slave(s) |
                                          +-----------------+

+------------------------+
|      MQTT Broker       |  <--- TLS / TCP --->
+------------------------+
         ▲        ▲
         |        |
    +----+    +---+----+
    | Home |  | Logging |
    |Assistant etc.     |
    +-------------------+

2.3. Architekturübersicht

Die Software ist in modularen Komponenten organisiert:
Komponente	Beschreibung
main()	Initialisiert Konfiguration, MQTT, Modbus, Server und steuert den Ablauf
init_config()	Liest INI-Datei und befüllt Config-Struktur
init_mqtt()	Erstellt MQTT-Client, setzt LWT, TLS, Subscriptions
init_modbus()	Öffnet und konfiguriert RTU-Schnittstelle
accept_clients()	Wartet auf eingehende TCP-Verbindungen (Modbus TCP Clients)
handle_client()	Leitet TCP-Anfragen an RTU weiter, verarbeitet Antworten, implementiert Diagnose
MQTTCallback	Verarbeitet eingehende MQTT-Nachrichten (lesen/schreiben von Registern & Coils)
publish_stats()	Periodisch veröffentlichte Metriken (z. B. Fehlerzähler, Uptime, Durchsatz)
shutdown_gateway()	Beendet sauber alle Threads, MQTT und RTU-Verbindung
dump_hex()	Optionales Logging von Modbus-Telegrammen (Debugging)
2.4. Datenfluss-Überblick

Modbus TCP Request
        ↓
  handle_client()
        ↓
  modbus_send_raw_request() → RTU
        ↓
  modbus_receive_confirmation()
        ↓
  build_tcp_response()
        ↓
   TCP Antwort an Client

MQTT Topic:
  - write/123  → write_register(123)
  - read/123   → read_register(123)
  - read_multi/100:3 → liest 3 Register ab Adresse 100
        ↓
  MQTTCallback::message_arrived()
        ↓
  modbus_read_*/write_*() → RTU
        ↓
  Antwort via MQTT publish()

2.5. Mehrfachschnittstellen
  
  Unterstützt mehrere parallele TCP-Clients
  
  Unterstützt gleichzeitigen MQTT-Betrieb
  
  Alle Komponenten laufen in separaten Threads, gesteuert über eine zentrale run-Flag-Atomik.

3. Konfiguration (modbus_gateway.ini)

Die gesamte Funktionsweise des Gateways wird über eine zentrale .ini-Datei gesteuert. Diese befindet sich standardmäßig unter:

/etc/modbus_gateway.ini

Sie kann jedoch beim Programmstart als Argument übergeben werden:

./modbus_gateway /pfad/zur/datei.ini

3.1. Grundstruktur

[serial]
port = /dev/ttyAMA0
baudrate = 9600
parity = N
databits = 8
stopbits = 1

[tcp]
port = 502

[modbus]
slave_id = 1

[mqtt]
broker = tcp://192.168.0.82:1883
client_id = modbus_gateway
topic_prefix = modbus/
username =
password =
keepalive = 30
lwt = offline
online = online
use_tls = 0
ca_file =
cert_file =
key_file =
insecure = 0
stats_interval = 60

[log]
file = /var/log/modbus_gateway.log
hex_dump = 0

3.2. Beschreibung aller Konfigurationsfelder
[serial]
Parameter	Bedeutung
port	Serieller Port (z. B. /dev/ttyUSB0)
baudrate	Baudrate, z. B. 9600
parity	Parität: N, E, oder O
databits	Datenbits: i. d. R. 8
stopbits	Stopbits: 1 oder 2
[tcp]
Parameter	Beschreibung
port	TCP-Port auf dem das Gateway Modbus TCP-Anfragen entgegennimmt (Default: 502)
[modbus]
Parameter	Bedeutung
slave_id	Default-Slave-ID für MQTT-Anfragen
[mqtt]
Parameter	Bedeutung
broker	MQTT-Broker URI, z. B. tcp://192.168.0.82:1883
client_id	MQTT Client-ID
topic_prefix	Präfix für alle Topics, z. B. modbus/
username	Optional: Benutzername
password	Optional: Passwort
keepalive	Keepalive in Sekunden
lwt	Payload bei Last Will (offline)
online	Payload bei erfolgreichem Connect (online)
use_tls	1 = TLS aktivieren, 0 = unverschlüsselt
ca_file	Pfad zur CA-Datei
cert_file	Pfad zum Client-Zertifikat
key_file	Pfad zum privaten Schlüssel
insecure	1 = Zertifikatsprüfung deaktivieren
stats_interval	Sekundenintervall zur MQTT-Veröffentlichung von Statistikdaten
[log]
Parameter	Bedeutung
file	Logdatei-Pfad
hex_dump	1 = Modbus-Pakete als Hexdump loggen (Debug)


4. MQTT Topics und Datenstruktur

Das Gateway verwendet ein konsistentes MQTT-Topic-Schema für alle Lese- und Schreiboperationen. Alle Topics beginnen mit dem in der INI-Datei definierten topic_prefix, z. B. modbus/.
4.1. Unterstützte MQTT-Kommandos
Operationstyp	Topic-Schema	Beschreibung
Holding Register lesen	modbus/read/<addr>	Lese Einzelregister
Holding Register schreiben	modbus/write/<addr>	Schreibe Einzelregister
Coil lesen	modbus/read_coil/<addr>	Lese Coil (einzelnes Bit)
Coil schreiben	modbus/write_coil/<addr>	Setze Coil (einzelnes Bit)
Discrete Input lesen	modbus/read_discrete/<addr>	Lese digitalen Eingang
Input Register lesen	modbus/read_input/<addr>	Lese analoges Eingangsregister
Mehrere Holding Register	modbus/read_multi/<start>:<anzahl>	Lese mehrere Holding Register
Mehrere Input Register	modbus/read_input_multi/<start>:<anzahl>	Lese mehrere Eingangsregister
Mehrere Coils	modbus/read_coil_multi/<start>:<anzahl>	Lese mehrere Coils
Mehrere Discrete Inputs	modbus/read_discrete_multi/<start>:<anzahl>	Lese mehrere digitale Eingänge
Statistik zurücksetzen	modbus/stats/reset	Setzt alle Statistikzähler zurück
Statistik abrufen	modbus/command/stats	Antwort erscheint in modbus/stats
4.2. MQTT-Beispiele
Beispiel 1: Einzelnes Register lesen

Publish:

mosquitto_pub -t modbus/read/100 -n

Antwort (Subscribe auf modbus/read/100):

431

Beispiel 2: Einzelnes Register schreiben

Publish:

mosquitto_pub -t modbus/write/100 -m "123"

Log-Ausgabe:

Modbus-Register 100 gesetzt auf 123

Beispiel 3: Mehrfachregister lesen

Publish:

mosquitto_pub -t modbus/read_multi/100:4 -n

Antwort (Subscribe auf modbus/read_multi/100:4):

431,0,65535,22

Beispiel 4: Coil setzen

mosquitto_pub -t modbus/write_coil/25 -m "1"

Beispiel 5: Discrete Input lesen

mosquitto_pub -t modbus/read_discrete/12 -n

Antwort:

0

4.3. Beispiel: Statistikabfrage

Abfrage:

mosquitto_pub -t modbus/command/stats -n

Antwort:

Statistik (on-demand): TCP=233, RTU=233, Fehler=2, Letzter RTU-Erfolg: 2025-06-26 13:42:17

4.4. Beispiel: Statistik als JSON (automatisch)

Wenn stats_interval > 0, sendet das Gateway regelmäßig:

{
  "total_tcp_requests": 233,
  "total_rtu_requests": 233,
  "total_errors": 2,
  "rtu_success": 231,
  "rtu_failures": 2,
  "dropped": 0,
  "echo_replies": 0,
  "last_success_timestamp": 1724701337
}

Topic: modbus/stats

5. Architektur und Komponenten des Programmcodes

Das Projekt folgt einer strukturierten und modularisierten Architektur, bei der die drei Hauptschnittstellen (Modbus TCP, Modbus RTU, MQTT) klar voneinander getrennt, aber dennoch eng integriert sind.
5.1. Hauptkomponenten
Komponente	Beschreibung
main()	Einstiegspunkt, Initialisierung von Konfiguration, Logging, MQTT, Modbus, TCP-Server
config_handler()	Parser für die INI-Konfigurationsdatei
init_mqtt()	Verbindungsaufbau und Subscription zu MQTT
MQTTCallback	Verarbeitung eingehender MQTT-Kommandos
init_modbus()	Setup der Modbus-RTU-Verbindung (TTY, Parity, Baudrate etc.)
start_tcp_server()	Öffnet TCP-Socket und wartet auf eingehende Verbindungen
accept_clients()	Akzeptiert neue Clients und startet Threads zur Verarbeitung
handle_client()	Verarbeitung einzelner Modbus-TCP-Anfragen, Weiterleitung an RTU, Rückantwort
publish_stats()	Periodische Publikation von Statistikdaten via MQTT
reconnect_mqtt()	Logik zum Wiederherstellen einer unterbrochenen MQTT-Verbindung
dump_hex()	Optionaler Hexdump für Debugzwecke
5.2. Datenflussübersicht

        +----------------+
        |    MQTT-Client |
        +----------------+
                |
                v
          [MQTTCallback]
                |
    +-----------+------------+
    |                        |
    v                        v
Modbus-Write          Modbus-Read
  (via RTU)              (via RTU)
    |                        |
    +-----------+------------+
                |
                v
      Optional: Antwort über MQTT

--------------------------------------------------

        +--------------------+
        |   TCP-Client (Modbus) |
        +--------------------+
                |
                v
       [handle_client()]
                |
                v
      Anfrage → RTU via libmodbus
                |
                v
      Antwort ← RTU (PDU)
                |
                v
       → TCP-Antwort mit MBAP-Header

5.3. Thread-Modell

    Main Thread: Initialisierung, Logging, Signalverarbeitung, Start der Komponenten

    MQTT-Thread: Intern durch paho-mqtt verwaltet

    publish_stats(): Separater Hintergrundthread zur periodischen Statistikausgabe

    Client-Threads: Jeder TCP-Client wird in einem eigenen Thread verarbeitet (Modbus TCP → RTU Weiterleitung)

5.4. Protokollkonformität

Die TCP-Antworten werden mit originaler Transaction-ID und rekonstruierter MBAP-Headerstruktur zurückgegeben. Auch lokale Diagnosefunktionen (z. B. Echo-Test 0x08 00 00) werden erkannt und direkt beantwortet.
5.5. Signalbehandlung

signal(SIGINT, signal_handler);
signal(SIGTERM, signal_handler);

Ermöglicht sauberen Shutdown bei CTRL+C oder systemd-Stop.

6. INI-Konfiguration – Aufbau und Optionen

Die Konfiguration des Gateways erfolgt über eine klassische INI-Datei, typischerweise unter dem Pfad:

/etc/modbus_gateway.ini

Diese Datei wird beim Programmstart geparst und beeinflusst alle Schnittstellen (Modbus, MQTT, Logging etc.). Alternativ kann der Pfad übergeben werden:

./modbus_gateway /pfad/zur/konfiguration.ini

6.1. Sektionen und Schlüssel
[serial] – Modbus-RTU-Schnittstelle
Schlüssel	Typ	Beschreibung	Beispiel
port	String	Serielles Interface	/dev/ttyAMA0
baudrate	int	Baudrate	9600
parity	char	Parität: N, E, O	N
databits	int	Datenbits (meist 8)	8
stopbits	int	Stoppbits (1 oder 2)	1
[tcp] – Modbus-TCP-Server
Schlüssel	Typ	Beschreibung	Beispiel
port	int	TCP-Port, auf dem der Server lauscht	502
[modbus] – Modbus-spezifische Optionen
Schlüssel	Typ	Beschreibung	Beispiel
slave_id	int	Standard-Slave-ID (wird bei TCP-Anfragen überschrieben)	1
[mqtt] – MQTT-Verbindung
Schlüssel	Typ	Beschreibung	Beispiel
broker	String	MQTT-Broker URI	tcp://192.168.0.82:1883
client_id	String	Client-ID	modbus_gateway
username	String	Optionaler Username für Authentifizierung	mqttuser
password	String	Passwort für obigen Benutzer	secrets123
keepalive	int	KeepAlive-Intervall in Sekunden	30
topic_prefix	String	Präfix aller Topics	modbus/
lwt	String	Last Will Payload (offline-Meldung)	offline
online	String	Payload beim Start (z. B. online)	online
use_tls	bool	TLS-Verbindung aktivieren	1
ca_file	String	CA-Zertifikatspfad	/etc/ssl/certs/ca.crt
cert_file	String	Client-Zertifikat (optional)	/etc/modbus/cert.pem
key_file	String	Privater Schlüssel zum Client-Zertifikat	/etc/modbus/key.pem
insecure	bool	Zertifikatsprüfung deaktivieren (unsicher)	1
stats_interval	int	Intervall für Statistikveröffentlichung in Sekunden	60
[log] – Logging
Schlüssel	Typ	Beschreibung	Beispiel
file	String	Logdatei	/var/log/modbus_gateway.log
hex_dump	bool	Hexdump aktivieren (1=ja, 0=nein)	1
6.2. Beispielkonfiguration

[serial]
port = /dev/ttyAMA0
baudrate = 9600
parity = N
databits = 8
stopbits = 1

[tcp]
port = 502

[modbus]
slave_id = 1

[mqtt]
broker = tcp://192.168.0.82:1883
client_id = modbus_gateway
username = mqttuser
password = secret
topic_prefix = modbus/
keepalive = 30
lwt = offline
online = online
use_tls = 0
stats_interval = 60

[log]
file = /var/log/modbus_gateway.log
hex_dump = 1

7. MQTT-Integration – Topics, Payloads und Beispiele

Das Gateway nutzt MQTT für bidirektionale Kommunikation: Steuerung, Statusmeldungen, Statistiken und Fehlerberichte. Die MQTT-Funktionalität ist vollständig optional und kann per Konfiguration deaktiviert werden.
7.1. Topic-Übersicht
Topic	Typ	Beschreibung
modbus/status	LWT	Verbindungsstatus des Gateways (online / offline)
modbus/stats	Status	Statistiken als JSON (optional zyklisch)
modbus/log	Log	Lognachrichten aus dem Gateway
modbus/read/<type>/<address>	Befehl	Lesezugriff auf Register
modbus/write/<type>/<address>	Befehl	Schreibzugriff auf Register
modbus/command/stats	Befehl	Löst sofortige Statistikmeldung aus
7.2. Registertypen
Typ (im Topic)	Modbus-Typ	Bedeutung
coil	Coils	0xxxx: Digitalausgang (1 Bit)
discrete	Discrete Inputs	1xxxx: Digitaleingang (1 Bit, read-only)
input	Input Register	3xxxx: Analogeingang (16 Bit, read-only)
holding	Holding Register	4xxxx: Analoge Ausgangswerte (16 Bit)
7.3. Leseoperation via MQTT

Beispiel: Ein Holding-Register 40010 lesen.

Topic:

modbus/read/holding/10

Antwort:

modbus/response/holding/10
Payload: 4711

7.4. Schreiboperation via MQTT

Beispiel: Schreibe Wert 1234 in Holding-Register 40020.

Topic:

modbus/write/holding/20

Payload:

1234

Antwort:

modbus/response/holding/20
Payload: OK

7.5. Mehrfachoperationen (Batch)

Mehrere Register können in einem JSON-Array geschrieben oder gelesen werden.
Mehrere Holding Register schreiben:

Topic:

modbus/write/holding

Payload:

{
  "10": 111,
  "11": 222,
  "12": 333
}

Mehrere Coils lesen:

Topic:

modbus/read/coil

Payload:

[1, 2, 3, 4]

Antwort (z. B. in modbus/response/coil)

{
  "1": 1,
  "2": 0,
  "3": 1,
  "4": 1
}

7.6. Statistik anfordern

Topic:

modbus/command/stats

Payload: (beliebig oder leer)

Antwort (sofortige Statusmeldung an modbus/stats):

{
  "tcp_requests": 234,
  "rtu_success": 231,
  "rtu_failures": 3,
  "dropped": 0,
  "echo_replies": 7
}

7.7. Logging

Topic:

modbus/log

Beispielhafte Payload:

[2025-06-26 19:33:00] → RTU TX: 01 03 00 00 00 02 65 CB

Aktivierung über:

[log]
hex_dump = 1

8. Modbus TCP ↔ RTU – Protokollfluss und Umsetzung

Das Gateway fungiert als Protokollübersetzer: Es empfängt Modbus TCP-Anfragen von Clients, wandelt sie in Modbus RTU um, leitet sie via RS485 weiter und antwortet mit korrekt konstruierten Modbus TCP-Paketen.
8.1. Ablauf einer TCP-Anfrage

    TCP-Anfrage empfangen

        Das Gateway lauscht auf dem konfigurierten Port (Default: 502).

        modbus_receive() extrahiert MBAP-Header und PDU.

    MBAP-Header analysieren

        Transaction-ID, Protocol-ID, Length, Unit-ID werden ausgelesen.

    PDU analysieren

        Der Modbus-Funktionscode bestimmt die Art der Anfrage (z. B. 0x03 = Holding Register lesen).

        Bei Sonderfunktionen wie 0x08 (Diagnose) erfolgt ggf. lokale Beantwortung.

    RTU-Anfrage vorbereiten

        Die RTU-Nachricht wird durch Voranstellen der Unit-ID an die PDU erzeugt.

        Mit modbus_send_raw_request() über serielle Schnittstelle versendet.

    RTU-Antwort empfangen

        modbus_receive_confirmation() liest Antwort von Slave.

        Validierung und Fehlerbehandlung erfolgen.

    TCP-Antwort erzeugen

        Neuer MBAP-Header mit Original Transaction-ID + Length wird erzeugt.

        Die PDU der RTU-Antwort (ohne Slave-ID) wird angefügt.

    TCP-Antwort senden

        Antwort wird via send() an Client zurückgegeben.

8.2. Lokale Diagnose-Funktionen

Der Funktionscode 0x08 (Diagnose) wird teilweise intern im Gateway bearbeitet, um schnellere Antworten zu ermöglichen:
Subfunktion	Beschreibung	Gateway-Verhalten
0x0000	Loopback-Test (Echo)	Antwort lokal erzeugt
0x000A	Statistikzähler zurücksetzen	Zähler auf 0 setzen
0x000B	TCP-Anfragen-Zähler zurückgeben	Antwort lokal erzeugt
0x000C	Fehlerzähler zurückgeben	Antwort lokal erzeugt
sonstige	Nicht erkannt	Anfrage an RTU weitergeleitet
8.3. Hex-Debug-Log

Die Funktion dump_hex() erzeugt ein Log in hexadezimaler Form:

dump_hex("→ RTU TX: ", data, len);

Beispielausgabe (in modbus/log):

→ RTU TX: 01 03 00 00 00 02 65 CB
← RTU RX: 01 03 04 00 0A 00 0B 75 D3

8.4. Verbindungsparameter

In der INI-Datei konfigurierbar:

[serial]
port = /dev/ttyAMA0
baudrate = 9600
parity = N
databits = 8
stopbits = 1

[modbus]
slave_id = 1

8.5. RTU-Zeitverhalten

modbus_set_response_timeout(ctx, 1, 0);  // 1 Sekunde Timeout

Zuverlässige Antwortzeiten hängen stark vom RTU-Slave ab. Typische Roundtrip-Zeiten: 5 – 50 ms.

9. Systemintegration: systemd, Autostart & Logging

Damit das Gateway beim Systemstart automatisch ausgeführt wird und sauber verwaltet werden kann, ist eine Integration mit systemd vorgesehen.
9.1. systemd-Service-Datei

Pfad: /etc/systemd/system/modbus_gateway.service

[Unit]
Description=Modbus TCP to RTU + MQTT Gateway
After=network.target

[Service]
ExecStart=/usr/local/bin/modbus_gateway /etc/modbus_gateway.ini
Restart=on-failure
User=modbus
Group=modbus
StandardOutput=append:/var/log/modbus_gateway.log
StandardError=append:/var/log/modbus_gateway.err
AmbientCapabilities=CAP_NET_BIND_SERVICE
LimitNOFILE=65536

[Install]
WantedBy=multi-user.target

9.2. Benutzer & Rechte

# Benutzer anlegen (falls noch nicht vorhanden)
sudo useradd -r -s /usr/sbin/nologin modbus

# Verzeichnisse anlegen
sudo mkdir -p /var/log
sudo touch /var/log/modbus_gateway.log
sudo chown modbus:modbus /var/log/modbus_gateway.log

9.3. Aktivieren & Starten

sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway

Statusabfrage:

sudo systemctl status modbus_gateway

Loganzeige (Live):

journalctl -u modbus_gateway -f

9.4. Logrotate-Unterstützung

Pfad: /etc/logrotate.d/modbus_gateway

/var/log/modbus_gateway.log {
    rotate 7
    daily
    compress
    delaycompress
    missingok
    notifempty
    copytruncate
}

Aktivierung (manuell testen):

sudo logrotate -f /etc/logrotate.d/modbus_gateway

9.5. Binary & Pfade

Standardmäßig wird die Anwendung kompiliert nach:

/usr/local/bin/modbus_gateway

Konfiguration:

/etc/modbus_gateway.ini

10. Kompilierung, Deployment und Makefile

Die Anwendung wird mit g++ unter Linux kompiliert. Für den Einsatz auf einem Raspberry Pi (z. B. Pi 5 mit Raspberry Pi OS 64 Bit) kann nativ oder per Cross-Compile gebaut werden.
10.1. Makefile für native Kompilierung (z. B. auf dem Raspberry Pi)

Pfad: Makefile

TARGET = modbus_gateway
SRC = modbus_gateway.cpp ini.c
CXXFLAGS = -Wall -O2 -std=c++17 -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lmodbus -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/$(TARGET)
	install -m 644 modbus_gateway.service /etc/systemd/system/modbus_gateway.service

clean:
	rm -f $(TARGET)

Build & Install:

make
sudo make install

10.2. Cross-Kompilierung für Raspberry Pi OS (aarch64)

toolchain-aarch64.cmake (für CMake-Nutzung – optional):

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

SET(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

SET(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)

Alternativ, für manuelle Cross-Compile:

aarch64-linux-gnu-g++ -std=c++17 -O2 \
  -I/usr/local/include -L/usr/local/lib \
  -o modbus_gateway modbus_gateway.cpp ini.c \
  -lmodbus -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread

10.3. Verzeichnisstruktur (empfohlen)

modbus_gateway/
├── Makefile
├── modbus_gateway.cpp
├── ini.c
├── ini.h
├── modbus_gateway.service
├── modbus_gateway.ini.example
├── README.md

10.4. Release-Archiv (optional)

Erstellen eines Archivs:

tar czf modbus_gateway.tar.gz modbus_gateway/

Bereitstellen auf GitHub oder lokal per scp auf den Ziel-Raspberry Pi kopieren:

scp modbus_gateway.tar.gz pi@raspberrypi:/home/pi/

11. Beispiele für MQTT und Modbus TCP Integration

Dieses Kapitel zeigt praxisnahe Beispiele zur Nutzung des Gateways mit MQTT und Modbus TCP. Zielgruppe sind Integratoren, die das Gateway z. B. mit Node-RED, Home Assistant oder eigenen Automatisierungen einsetzen.
11.1. MQTT Beispiele
📥 Schreiben eines Holding Registers

mosquitto_pub -t modbus/write/100 -m 42

➡️ Setzt das Modbus-Holding-Register 100 auf den Wert 42.
📤 Lesen eines Holding Registers

mosquitto_pub -t modbus/read/100 -n

📬 Das Gateway antwortet mit:

Topic: modbus/read/100
Payload: 42

📥 Schreiben eines Coils (Schaltbefehl)

mosquitto_pub -t modbus/write_coil/12 -m 1

➡️ Schaltet Coil 12 auf ON.
📤 Lesen eines Coils

mosquitto_pub -t modbus/read_coil/12 -n

📬 Antwort:

Topic: modbus/read_coil/12
Payload: 1

🧪 Mehrfachlesen von Holding- oder Input-Registries

mosquitto_pub -t modbus/read_multi/200:4 -n

➡️ Liest 4 Werte ab Adresse 200.

Antwort:

Topic: modbus/read_multi/200:4
Payload: 122,456,789,0

🛠 Statistik manuell abrufen

mosquitto_pub -t modbus/command/stats -n

Antwort:

Topic: modbus/stats
Payload: Statistik (on-demand): TCP=1234, RTU=1229, Fehler=5, Letzter RTU-Erfolg: 2025-06-26 12:12:59

🔁 Statistik automatisch alle X Sekunden

Wenn in der INI definiert:

[mqtt]
stats_interval = 60

Dann sendet das Gateway regelmäßig an:

Topic: modbus/stats
Payload: {"total_tcp_requests":1234, "total_rtu_requests":1229, ...}

🔄 Zurücksetzen der Statistikzähler

mosquitto_pub -t modbus/stats/reset -n

11.2. Modbus TCP Beispiele

Verbindung z. B. mit mbpoll herstellen:

mbpoll -m tcp -a 1 -t 3 -r 100 -c 2 127.0.0.1

Erklärung:

    -m tcp: TCP-Modus

    -a 1: Slave-ID

    -t 3: Holding Register

    -r 100: Startadresse 100

    -c 2: 2 Register lesen

    127.0.0.1: Gateway-Adresse

Antwort (wenn erfolgreich):

[1]: 42
[2]: 81

11.3. Node-RED Integration (Beispiel)

Subscribe Node:

    Topic: modbus/read/100

    Empfängt: Registerwert

Inject Node:

    Topic: modbus/write/100

    Payload: 123

➡️ Damit kann Node-RED Modbus-Werte setzen und auslesen – ideal für Automatisierung.

12. Fehlerbehandlung und Debugging

Diese Sektion beschreibt, wie Fehler im Gateway erkannt, protokolliert und im Idealfall behoben werden können. Sie hilft sowohl Entwicklern als auch Integratoren, Probleme schneller zu analysieren.
12.1. Fehlerquellen im Überblick
Fehlerquelle	Typische Ursache	Lösungsvorschlag
Modbus RTU Timeout	Kein Gerät antwortet auf Anfrage	Baudrate, Parity, Slave-ID prüfen
MQTT Fehler	Broker nicht erreichbar oder Zertifikatfehler	MQTT-Konfiguration prüfen
TCP Fehler	TCP-Client unterbricht Verbindung	Normal bei Verbindungsende, ggf. loggen
INI-Fehler	Syntaxfehler oder Pfad nicht vorhanden	Pfad & Format prüfen, /etc/modbus_gateway.ini
Berechtigungen	Zugriff auf /dev/ttyAMA0 oder Log-Datei verweigert	Benutzer zur dialout-Gruppe hinzufügen
12.2. Logging

Alle Ereignisse werden sowohl…

    lokal in die Logdatei (/var/log/modbus_gateway.log) geschrieben

    optional zusätzlich via MQTT an den Topic modbus/log gesendet

Beispieleintrag:

2025-06-26 20:31:17 - Fehler beim Lesen von Modbus-Register 100: Input/output error

💡 Die Logdatei ist das erste Mittel zur Fehleranalyse.
12.3. Hex-Dump Debugging

Aktivierung per INI-Datei:

[log]
hex_dump = 1

Dann erscheinen z. B. solche Zeilen im Log/MQTT:

→ RTU TX: 01 03 00 64 00 01 85 D4
← RTU RX: 01 03 02 00 2A B8 44

Diese geben Aufschluss über den exakten Modbus-Datenverkehr.
12.4. Diagnostik via Modbus Diagnosefunktion (0x08)

Der TCP-Client kann spezielle Subfunktionen senden:
Subfunktion	Bedeutung	Reaktion des Gateways
0x0000	Echo-Test	Antwortet direkt lokal (Loopback)
0x000A	Statistikzähler zurücksetzen	Zähler wie total_tcp_requests auf 0 setzen
0x000B	Anzahl TCP-Requests zurückgeben	2-Byte Antwort mit aktuellem Zählerstand
0x000C	Anzahl Fehler zurückgeben	2-Byte Antwort mit Fehlerzähler

Diese sind ideal für externe Überwachungssoftware.
12.5. MQTT-Statistik und Diagnosedaten

Alle Zählerwerte sind im Topic modbus/stats enthalten, z. B.:

{
  "total_tcp_requests": 156,
  "total_rtu_requests": 154,
  "total_errors": 2,
  "rtu_success": 154,
  "rtu_failures": 0,
  "dropped": 0,
  "echo_replies": 1,
  "last_success_timestamp": 1719414809
}

Erlaubt grafische Auswertungen (z. B. in Grafana oder Node-RED Dashboard).
12.6. Erhöhte Fehlertoleranz

Das Gateway bricht TCP-Verbindungen nicht sofort bei jedem Fehler ab, sondern erlaubt z. B.:

    mehrere fehlerhafte RTU-Kommunikationen nacheinander

    leer empfangene TCP-Pakete → werden ignoriert

    Verbindungsabbrüche → sauberer Cleanup im shutdown_gateway()


13. Deployment und Systemintegration

In diesem Kapitel wird beschrieben, wie das Modbus-Gateway auf einem Zielsystem (z. B. Raspberry Pi) installiert, automatisch gestartet und verwaltet wird. Dies umfasst sowohl manuelles Deployment als auch systemd-Integration.
13.1. Voraussetzungen

    Betriebssystem: Empfohlen: Raspberry Pi OS 64 Bit (Debian-basiert)

    Benötigte Pakete:

sudo apt install libmodbus-dev libpaho-mqttpp3-dev build-essential

Serielle Berechtigung: Der Benutzer muss Zugriff auf /dev/ttyAMA0 haben:

    sudo usermod -aG dialout $USER

13.2. Kompilierung

Im Projektverzeichnis:

make

Erzeugt:

    die ausführbare Datei modbus_gateway

Alternativ manuell:

g++ -o modbus_gateway modbus_gateway.cpp ini.c -lmodbus -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread

13.3. systemd-Service-Datei
Datei: /etc/systemd/system/modbus_gateway.service

[Unit]
Description=Modbus TCP↔RTU + MQTT Gateway
After=network.target

[Service]
ExecStart=/home/benno/modbus_gateway/modbus_gateway /etc/modbus_gateway.ini
Restart=always
User=benno
Group=benno
WorkingDirectory=/home/benno/modbus_gateway
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target

13.4. Aktivieren des Dienstes

sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway

13.5. Status prüfen

systemctl status modbus_gateway
journalctl -u modbus_gateway -f

13.6. Deployment-Ordnerstruktur

Empfohlen:

/home/benno/modbus_gateway/
├── modbus_gateway            # Executable
├── modbus_gateway.cpp        # Quellcode
├── ini.c / ini.h             # INI-Parser
├── Makefile
├── README.md
├── systemd/
│   └── modbus_gateway.service
└── config/
    └── modbus_gateway.ini

13.7. Tipps

    Logs gehen standardmäßig nach /var/log/modbus_gateway.log

    Die Logdatei kann via logrotate rotiert werden

    MQTT-Fehler und Wiederverbindungen werden automatisch gehandhabt

14. INI-Konfigurationsformat und Parameterreferenz

Die zentrale Konfiguration des Gateways erfolgt über eine INI-Datei. Diese ermöglicht es, alle relevanten Parameter strukturiert zu setzen – von seriellen Einstellungen bis hin zu MQTT und Logging.
14.1. Speicherort

Standardpfad:

/etc/modbus_gateway.ini

Beim Start kann alternativ eine Konfigurationsdatei übergeben werden:

./modbus_gateway /pfad/zur/datei.ini

14.2. Struktur der INI-Datei

[serial]
port = /dev/ttyAMA0
baudrate = 9600
parity = N
databits = 8
stopbits = 1

[tcp]
port = 502

[modbus]
slave_id = 1

[mqtt]
broker = tcp://localhost:1883
client_id = modbus_gateway
topic_prefix = modbus/
username =
password =
keepalive = 30
lwt = offline
online = online
use_tls = 0
ca_file =
cert_file =
key_file =
insecure = 0
stats_interval = 60

[log]
file = /var/log/modbus_gateway.log
hex_dump = 0

14.3. Beschreibung der Parameter
[serial]
Parameter	Typ	Beschreibung
port	String	Serielle Schnittstelle
baudrate	Zahl	Baudrate (z. B. 9600, 19200, …)
parity	Zeichen	Parität: N, E oder O
databits	Zahl	Datenbits (typisch: 8)
stopbits	Zahl	Stoppbits (typisch: 1)
[tcp]
Parameter	Typ	Beschreibung
port	Zahl	TCP-Port für Modbus-TCP-Server
[modbus]
Parameter	Typ	Beschreibung
slave_id	Zahl	Default Slave-ID (nur für MQTT-Zugriffe)
[mqtt]
Parameter	Typ	Beschreibung
broker	String	MQTT-Broker URI (z. B. tcp://localhost:1883)
client_id	String	Client-ID des MQTT-Clients
topic_prefix	String	Präfix für alle MQTT-Topics (z. B. modbus/)
username	String	MQTT-Benutzername
password	String	MQTT-Passwort
keepalive	Zahl	Keepalive in Sekunden
lwt	String	Last-Will-Payload (z. B. „offline“)
online	String	Online-Payload beim Start (z. B. „online“)
use_tls	0/1	TLS aktivieren (1 = ja)
ca_file	String	Pfad zur CA-Datei
cert_file	String	Client-Zertifikatdatei
key_file	String	Client-Schlüsseldatei
insecure	0/1	TLS-Zertifikatprüfung deaktivieren (unsicher!)
stats_interval	Zahl	Intervall in Sekunden zur Veröffentlichung
[log]
Parameter	Typ	Beschreibung
file	String	Pfad zur Log-Datei
hex_dump	0/1	Hexdump für Modbus-Rohdaten aktivieren (Debug)

15. Beispiele für MQTT-Kommunikation

In diesem Kapitel werden typische MQTT-Kommandos und Nachrichtenformate beschrieben, um über MQTT auf Modbus-Geräte zuzugreifen oder Statusinformationen zu erhalten.
15.1. Schreibzugriff: Holding Register oder Coils

Das Gateway verarbeitet MQTT-Nachrichten unterhalb des Topics:

modbus/write/<type>/<slave_id>/<register>

<type>
Typ	Bedeutung
coil	Schreibzugriff auf Coil (FC 5, 15)
holding	Schreibzugriff auf Holding Register (FC 6, 16)
Beispiel 1: Coil setzen (Slave 1, Coil 100 auf true setzen)

Topic:

modbus/write/coil/1/100

Payload (JSON):

true

Beispiel 2: Holding Register schreiben (Slave 3, Register 40001 = 1234)

Topic:

modbus/write/holding/3/0

Payload:

1234

📌 Hinweis: Holding Register Adressen beginnen bei 0!
15.2. Lesezugriff: per MQTT-Request

MQTT unterstützt lesende Anfragen durch Veröffentlichung unter:

modbus/read/<type>/<slave_id>/<register>[,<anzahl>]

Der Gateway antwortet auf:

modbus/response/<type>/<slave_id>/<register>

Beispiel 3: Holding Register 0 (40001) von Slave 3 lesen

Topic (Anfrage):

modbus/read/holding/3/0

Antwort:

modbus/response/holding/3/0

Payload:

[1234]

Beispiel 4: Input Register 30010 bis 30014 von Slave 2 lesen

Topic (Anfrage):

modbus/read/input/2/9,5

Antwort:

modbus/response/input/2/9

Payload:

[318, 319, 320, 321, 322]

15.3. Diagnosedaten via MQTT erhalten

Wird in der INI-Datei mqttStatsInterval > 0 gesetzt, sendet das Gateway periodisch Statistiken:

Topic:

modbus/stats

Payload:

{
  "tcp_requests": 123,
  "rtu_success": 120,
  "rtu_failures": 3,
  "dropped": 0,
  "echo_replies": 5
}

15.4. Statistik manuell per Kommando anfordern

Topic:

modbus/command/stats

Payload: (optional, z. B. "now")
Antwort erfolgt erneut auf modbus/stats.

16. Beispiele für Modbus TCP-Kommunikation

Dieses Kapitel zeigt, wie sich ein externer Modbus TCP-Client mit dem Gateway verbinden kann, um über die TCP-Schnittstelle auf die dahinterliegenden RTU-Geräte zuzugreifen. Das Gateway fungiert als Proxy zwischen TCP und serieller RTU-Verbindung.
16.1. Verbindungsaufbau

Der TCP-Server lauscht standardmäßig auf Port 502, sofern nicht anders in der modbus_gateway.ini angegeben.

Ein typischer Modbus-TCP-Client verbindet sich wie folgt:

    Host: IP-Adresse des Raspberry Pi

    Port: 502

    Slave ID: 1…247 (je nach Zielgerät)

    Funktion: Standard-Modbus-Funktionen (FC1, FC2, FC3, FC4, FC5, FC6, FC15, FC16)

16.2. Beispiel mit mbpoll (Modbus-TCP)

Installiere mbpoll (z. B. unter Linux via Paketmanager) und führe folgendes aus:
Lesen eines Holding Registers (Register 0) von Slave 1:

mbpoll -m tcp -a 1 -r 0 -c 1 -t 4:int -1 192.168.0.100

Erklärung:

    -m tcp → Modbus TCP

    -a 1 → Slave-Adresse 1

    -r 0 → Registeradresse 0

    -c 1 → 1 Register lesen

    -t 4:int → Format: 16-bit Holding Register

    -1 → Port 502

    192.168.0.100 → Gateway-IP

16.3. Schreiben in ein Holding Register

mbpoll -m tcp -a 1 -r 0 -t 4:int -1 192.168.0.100 --write 1234

Schreibt den Wert 1234 in Register 0 von Slave 1.
16.4. Weitere Clients

Auch andere Clients wie:

    QModMaster (Windows/Qt)

    Node-RED mit Modbus Nodes

    Python (pymodbus)

    CODESYS oder TIA Portal

können ohne spezielle Anpassungen direkt mit dem Gateway arbeiten, solange sie Modbus TCP unterstützen.
17. Logging und Diagnose
17.1. Datei-Logging

Alle Ereignisse, Fehler, Statuswechsel werden in die Datei geloggt:

/var/log/modbus_gateway.log

Konfigurierbar via:

[log]
file = /var/log/modbus_gateway.log

17.2. MQTT-Logging

Jede Logzeile wird zusätzlich auf dem Topic:

modbus/log

veröffentlicht – vorausgesetzt, MQTT ist verbunden.
17.3. Hex-Dump aktivieren

Zur Analyse der RTU-Kommunikation kann ein Hexdump aktiviert werden:

[log]
hex_dump = 1

Beispielausgabe:

→ RTU TX: 01 03 00 6B 00 03 76 87
← RTU RX: 01 03 06 02 2B 00 00 00 64 39 80

18. systemd-Integration, Autostart und Benutzerverwaltung

Um das Gateway automatisch beim Systemstart zu laden und im Hintergrund als Dienst zu betreiben, kann eine systemd-Serviceeinheit verwendet werden.
18.1. Beispiel modbus_gateway.service

Speichern unter:
/etc/systemd/system/modbus_gateway.service

[Unit]
Description=Modbus TCP <-> RTU + MQTT Gateway
After=network.target

[Service]
ExecStart=/usr/local/bin/modbus_gateway /etc/modbus_gateway.ini
Restart=always
RestartSec=5
User=modbus
Group=modbus
StandardOutput=syslog
StandardError=syslog
SyslogIdentifier=modbus_gateway

[Install]
WantedBy=multi-user.target

18.2. Benutzer modbus anlegen (empfohlen)

Damit das Gateway nicht als root laufen muss:

sudo useradd --system --no-create-home --group modbus modbus
sudo chown modbus:modbus /var/log/modbus_gateway.log

Optional:
Modbus-Benutzer Zugriff auf /dev/ttyAMA0 erlauben:

sudo usermod -aG dialout modbus

18.3. Dienst aktivieren und starten

sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable modbus_gateway.service
sudo systemctl start modbus_gateway.service

18.4. Status prüfen

sudo systemctl status modbus_gateway

18.5. Neustart/Stop

sudo systemctl restart modbus_gateway
sudo systemctl stop modbus_gateway

19. Sicherheit und TLS
19.1. TLS für MQTT aktivieren

Die Verbindung zum MQTT-Broker kann TLS-verschlüsselt erfolgen. Dazu folgende Parameter in der INI-Datei setzen:

[mqtt]
use_tls = 1
ca_file = /etc/ssl/certs/ca.crt
cert_file = /etc/ssl/certs/client.crt
key_file = /etc/ssl/private/client.key
insecure = 0

    use_tls = 1 aktiviert TLS

    insecure = 1 deaktiviert die Serverzertifikatsprüfung (nicht empfohlen!)

19.2. MQTT über TLS testen

Falls dein Broker z. B. unter mqtts://broker.local:8883 erreichbar ist:

broker = ssl://broker.local:8883

Achte darauf, dass der Port und das Protokoll (ssl:// statt tcp://) korrekt sind.
20. Statistiken und Überwachung
20.1. Automatische Statistik (periodisch)

Alle mqttStatsInterval Sekunden wird auf folgendes Topic veröffentlicht:

modbus/stats

Beispielinhalt:

{
  "total_tcp_requests": 1432,
  "total_rtu_requests": 1410,
  "total_errors": 22,
  "rtu_success": 1405,
  "rtu_failures": 5,
  "dropped": 2,
  "echo_replies": 18,
  "last_success_timestamp": 1719438511
}

20.2. Statistik sofort anfordern

MQTT senden an:

modbus/command/stats

Antwort erfolgt auf:

modbus/stats

20.3. Statistik zurücksetzen

Sende ein leeres MQTT-Message an:

modbus/stats/reset

21. Fehlerbehandlung und Wiederverbindung
21.1. Modbus RTU Fehlerbehandlung

Alle Fehler beim Senden oder Empfangen auf der seriellen Schnittstelle werden:

    mit modbus_strerror(errno) ins Log geschrieben,

    im MQTT-Log publiziert (sofern Verbindung besteht),

    und statistisch gezählt (total_errors, rtu_failures).

Beispiel-Logeintrag bei Kommunikationsfehler:

2025-06-26 10:15:27 - Fehler beim Empfangen von RTU: Connection timed out

21.2. MQTT-Verbindungsüberwachung

Die MQTT-Verbindung wird automatisch überwacht:

    Bei Verbindungsverlust wird ein Hintergrundthread gestartet, der periodisch einen Reconnect versucht.

    Während des Reconnects wird kein weiterer Reconnect-Thread gestartet.

    Erfolgreicher Reconnect führt zum Neuaufbau aller Subscriptions und Publikation des Online-Status.

21.3. Abfangen von Signalen

Die folgenden Signale werden behandelt:

    SIGINT (z. B. Ctrl+C)

    SIGTERM (z. B. via kill)

Diese führen zu einem sauberen Shutdown:

    Alle Client-Threads werden beendet.

    MQTT wird abgemeldet und der Offline-Status gesendet.

    Modbus wird sauber geschlossen.

    Log-Datei wird beendet.

21.4. TCP-Zeitüberschreitung

Jeder verbundene Client bekommt ein Timeout:

timeout.tv_sec = 60;
setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

Verbindungen ohne Aktivität werden damit nach 60 Sekunden automatisch getrennt.
22. Logging und Diagnose
22.1. Log-Datei

Standard: /var/log/modbus_gateway.log
Pfad konfigurierbar via INI:

[log]
file = /var/log/modbus_gateway.log

Format:

YYYY-MM-DD HH:MM:SS - <Nachricht>

22.2. MQTT-Log

Falls MQTT verbunden ist, werden alle Log-Meldungen ebenfalls an folgendes Topic gesendet:

modbus/log

Dies ermöglicht z. B. eine Anbindung an:

Node-RED (Debug)
InfluxDB (mit Log-Level-Analyse)
Prometheus (via Exporter)

22.3. Hex-Dumps

Wenn aktiviert:

[log]
hex_dump = 1

werden Modbus-Anfragen und -Antworten im Log hexadezimal ausgegeben.

Beispiel:

→ RTU TX: 01 03 00 00 00 02 C4 0B
← RTU RX: 01 03 04 00 0A 00 14 FA 33

23. Projektstruktur und Dateien
23.1. Wichtige Quelldateien
Datei	Beschreibung
modbus_gateway.cpp	Hauptanwendung mit Modbus-/MQTT-Logik
ini.c/h	INI-Dateiparser
Makefile	Kompilation und Installation
modbus_gateway.service	systemd-Dienstdefinition
23.2. Konfigurationsdatei

Beispiel:

/etc/modbus_gateway.ini

Struktur siehe Kapitel 3.3 INI-Datei Aufbau.
23.3. Kompilierung

make

Installiert nach /usr/local/bin/modbus_gateway
23.4. Logs

/var/log/modbus_gateway.log

23.5. Systemdienst

/etc/systemd/system/modbus_gateway.service

23.6. MQTT Topics Übersicht (kompakt)
Zweck	Topic	Beispiel
Status	modbus/status	online / offline
Logging	modbus/log	Logeinträge
Statistik	modbus/stats	JSON mit Zählerwerten
Statistik abrufen	modbus/command/stats	leer
Statistik zurücksetzen	modbus/stats/reset	leer
Holding Register lesen	modbus/read/1234	Antwort: 42
Holding Register schreiben	modbus/write/1234	Nachricht: 42
Coil lesen	modbus/read_coil/1234	Antwort: 1
Coil schreiben	modbus/write_coil/1234	Nachricht: 0
Input Register lesen	modbus/read_input/1234	Antwort: 1042
Discrete Input lesen	modbus/read_discrete/1234	Antwort: 0
Mehrfachregister lesen	modbus/read_multi/100:4	Antwort: 123,456,789,1011
Mehrfach-Coils lesen	modbus/read_coil_multi/0:8	Antwort: 1,1,0,0,1,0,1,1
23.7. Lizenzen / Abhängigkeiten

    libmodbus – https://libmodbus.org/

    Eclipse Paho MQTT C++ – https://www.eclipse.org/paho/

    inih (INI-Parser) – https://github.com/benhoyt/inih

Alle Drittkomponenten sind unter liberalen Open-Source-Lizenzen verfügbar.


