---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

# Modbus Gateway – Vollständige Dokumentation

## Inhaltsverzeichnis

1. Einleitung  
2. Systemüberblick  
3. Architektur  
4. Installationsanleitung  
5. Konfigurationsdatei  
6. Kompilierung & Makefile  
7. Starten des Gateways  
8. MQTT Integration  
9. Modbus TCP ↔ RTU  
10. Diagnose & Statistik  
11. TLS & Sicherheit  
12. Logging  
13. Fehlerbehandlung  
14. systemd Integration  
15. Deployment  
16. Beispiele  
17. Troubleshooting  
18. Erweiterungen  
19. Codeübersicht  
20. Quellcode-Erklärung  
21. Verwendete Bibliotheken  
22. Lizenz  
23. Anhang  

---

🧩 Kapitel 1: Einleitung
Ziel des Projekts

Das Projekt modbus_gateway wurde entwickelt, um eine verlässliche, flexible und leicht erweiterbare Schnittstelle zwischen Modbus TCP-Clients und Modbus RTU-Slaves bereitzustellen. Zusätzlich bietet es eine vollständige MQTT-Integration, um Zustände, Statistiken und Steuerbefehle in moderne IoT-Systeme wie Node-RED, Home Assistant oder Industrie-Leitsysteme einzubinden.
Motivation

Viele bestehende Gateways bieten entweder nur die reine TCP↔RTU-Funktionalität oder sind schwer erweiterbar. Ziel war es, ein System zu schaffen, das:
Merkmal	Zielsetzung
⚙️ Plattform	Betrieb auf Raspberry Pi 5 (64-Bit), Linux-basiert
🔄 Protokolle	Modbus TCP ↔ RTU (RS485) + MQTT-Unterstützung
📡 Kommunikation	Unterstützung für verschlüsselte MQTT-Verbindungen (TLS optional)
📊 Diagnose & Statistik	Umfangreiche Statistiken lokal und via MQTT
🔧 Konfiguration	Konfiguration über .ini-Datei ohne Neukompilierung
🚀 Deployment	Integration über systemd + automatischer Start
📚 Dokumentation	Vollständige Entwickler- und Integratoren-Doku in Markdown
Typische Anwendungsfälle
Anwendungsfall	Beschreibung
🏭 Industrieanlagen	Bestehende Modbus-RTU-Hardware via Ethernet verfügbar machen
🏠 Smart Home & Gebäudeautomation	PV-Anlagen, Wärmepumpen, Zähler in MQTT-Systeme integrieren
🧪 Forschung / Prototyping	Logging, Analyse, Simulation von Modbus-Daten über MQTT
🧩 Edge-Devices	Lokales Vorverarbeiten von Modbus-Daten auf Raspberry Pi mit MQTT-Export
Warum Open Source?

Das Projekt wurde modular und quelloffen entwickelt, um:

    🔄 Anpassungen für neue Geräte zu ermöglichen (z. B. weitere Modbus-Funktioncodes, Registertypen)

    💬 Community-Feedback & Beiträge zu fördern

    🔍 Audits und Sicherheitsprüfungen zu erleichtern

    🧩 Integration in bestehende Systeme ohne Lizenzprobleme zu erlauben

Überblick: Funktionsweise

           +------------------+       TCP      +------------------+
           |  Modbus TCP      | <------------> |  Gateway         |
           |  Client (z. B. SCADA) |          |                  |
           +------------------+               |                  |
                                              |    ┌──────────┐  |
                                              |    │ MQTT     │<─┐
                                              |    └──────────┘  │
                                              |                  │
                                              |    ┌──────────┐  │
                                              |    │ RTU      │<─┘
                                              |    │ (RS485)  │────→ Modbus-RTU-Slave
                                              |    └──────────┘
                                              +------------------+

Softwarekomponenten
Komponente	Beschreibung
modbus_gateway.cpp	Hauptprogramm, verarbeitet TCP ↔ RTU und MQTT
config.h/.cpp	Konfigurationsverwaltung über .ini-Datei
ini.c/.h	Lightweight INI-Parser
Makefile	Kompilieren & Deployment
modbus_gateway.service	systemd-Service zur automatischen Ausführung beim Systemstart
modbus_gateway.md	Diese Dokumentation (für GitHub und lokale Hilfe)



🧭 Kapitel 2: Systemüberblick
Übersicht der Komponenten

Das modbus_gateway besteht aus mehreren logisch getrennten Komponenten, die klar definierten Aufgaben folgen:

                 +--------------------------+
                 |   Modbus TCP Clients     |
                 |   (SCADA, HMI, etc.)     |
                 +-----------+--------------+
                             |
                             | Modbus TCP (Port 502/konfigurierbar)
                             |
                 +-----------v--------------+
                 |   Gateway (TCP-Server)   |
                 |                          |
                 |  +--------------------+  |
                 |  |  TCP ↔ RTU Engine  |  |
                 |  +--------------------+  |
                 |  |     MQTT Client    |  |
                 |  +--------------------+  |
                 |  |  Statistik & Logik |  |
                 |  +--------------------+  |
                 |  |    Konfiguration   |  |
                 +-----------+--------------+
                             |
                             | Modbus RTU (RS485)
                             |
                 +-----------v--------------+
                 |      RTU Slave(s)        |
                 |  (Zähler, Aktoren, etc.) |
                 +--------------------------+

Datenfluss im Betrieb

    Modbus TCP Anfrage:

        TCP-Client (z. B. SCADA) sendet eine Anfrage an das Gateway.

        Diese wird mit MBAP-Header (7 Byte) empfangen und analysiert.

    Übersetzung & Weiterleitung:

        Der MBAP-Header wird entfernt.

        Die PDU wird um die RTU-Adresse ergänzt.

        Der komplette RTU-Frame wird per RS485 versendet.

    Antwort vom RTU-Slave:

        Die Antwort wird empfangen, CRC geprüft und validiert.

        Anschließend wird der MBAP-Header rekonstruiert.

        Die Antwort wird an den TCP-Client zurückgesendet.

    MQTT-Verarbeitung (optional):

        Während oder nach Verarbeitung können:

            Statistiken publiziert werden.

            Lese-/Schreibbefehle über MQTT verarbeitet werden.

            Logging-Ereignisse publiziert werden.

MQTT Integration – Übersicht

         +---------------------+
         |     MQTT Broker     |
         +----------+----------+
                    ^
                    | subscribe/publish
         +----------v----------+
         |      Gateway        |
         |                    |
         |  - modbus/read/#   |  ← Anfrage von MQTT-Client
         |  - modbus/write/#  |  ← Schreibauftrag
         |  - modbus/stats    |  → Statistik regelmäßig
         |  - modbus/log      |  → Log-Ereignisse
         +--------------------+

    🔧 Der Broker, die Topics und Sicherheitsoptionen (TLS, Auth) werden in der .ini-Datei konfiguriert.

Systemrollen und Protokolle
Rolle	Protokoll	Richtung	Beschreibung
🖥️ Modbus TCP Client	Modbus TCP	IN/OUT	Sendet/empfängt Anfragen
📡 MQTT Broker	MQTT 3.1.1	IN/OUT	Kommuniziert mit Gateway
⚙️ RTU-Gerät (Slave)	Modbus RTU	OUT	Antwortet auf RTU-Anfragen
📟 Gateway (dieses Projekt)	TCP, RTU, MQTT	beides	Vermittelt, übersetzt und loggt
Unterstützte Plattformen
Plattform	Status	Bemerkung
✅ Raspberry Pi 5	🟢 Empfohlen	64-Bit Raspberry Pi OS, getestet mit systemd
✅ Raspberry Pi 4	🟢 Getestet	Funktioniert bei angepasstem sysroot
✅ Linux x86_64	🟢 OK	Entwicklung unter Fedora & Ubuntu getestet
⛔ Windows	🔴 Nein	Derzeit nicht unterstützt
Besonderheiten

    🧵 Multi-Threading:

        Jeder TCP-Client wird in einem eigenen Thread verarbeitet.

        MQTT läuft unabhängig im Hintergrund.

    📈 Statistik:

        Echtzeitdaten werden gesammelt und über MQTT veröffentlicht.

        Optional über Modbus-Diagnose abrufbar (z. B. Echo, Zählerrückgabe).

    🔒 TLS-Unterstützung:

        MQTT-Kommunikation kann verschlüsselt erfolgen.

        Zertifikatspflicht oder insecure-Modus möglich.



🧱 Kapitel 3: Architektur
Ziel der Architektur

Die Architektur des modbus_gateway wurde modular und erweiterbar gestaltet. Sie erfüllt industrielle Anforderungen wie:

    Robuste Protokollverarbeitung (TCP/RTU/MQTT)

    Trennung von Transport und Logik

    Skalierbarkeit durch Threading

    Konfigurierbarkeit zur Laufzeit (via INI und MQTT)

Hauptmodule und Zuständigkeiten
Modul	Datei	Aufgabe
🔁 main()	modbus_gateway.cpp	Einstiegspunkt, Initialisierung, Steuerung
⚙️ init_config()	modbus_gateway.cpp	Laden & Prüfen der INI-Konfiguration
🔌 init_modbus()	modbus_gateway.cpp	Initialisierung des RTU-Modbus-Contexts
🌐 start_tcp_server()	modbus_gateway.cpp	Öffnen des TCP-Ports und Entgegennahme neuer Clients
📦 handle_client()	modbus_gateway.cpp	Übersetzen & Weiterleiten von Modbus TCP <-> RTU
☁️ init_mqtt()	modbus_gateway.cpp	Aufbau der MQTT-Verbindung inkl. TLS & Authentifizierung
🧠 MQTTCallback	mqtt_handler.cpp	Empfang von MQTT-Kommandos (read/write/stats)
🧾 log()	log.cpp	Log-Einträge mit Zeitstempel & optional MQTT
📈 publish_stats()	stats.cpp	Versenden von Statistiken über MQTT
Lebenszyklus des Programms

+---------------------+
|       Start         |
+---------------------+
          |
          v
+-----------------------------+
| Initialisierung (main)      |
| - Konfiguration laden       |
| - MQTT-Setup (optional TLS)|
| - RTU-Gerät vorbereiten     |
| - TCP-Server starten        |
+-----------------------------+
          |
          v
+-----------------------------+
|  Client-Loop (accept)       |
|  - Jeder TCP-Client = Thread|
|  - handle_client()          |
+-----------------------------+
          |
          v
+-----------------------------+
|  Gateway läuft dauerhaft    |
|  - MQTT-Abfragen empfangen  |
|  - Statistik regelmäßig senden|
|  - Logging und Fehlerbehandlung |
+-----------------------------+
          |
          v
+-----------------------------+
|      Beenden (Signal)       |
|  - Alle Sockets schließen   |
|  - Modbus freigeben         |
|  - MQTT disconnect & LWT    |
+-----------------------------+

Architekturprinzipien

    Thread-per-Connection: Jeder TCP-Client läuft in eigenem Thread → keine Blockade.

    Sperrenfreies MQTT: MQTT läuft in eigenem Kontext durch paho::async_client.

    Konfigurierbarkeit:

        INI-Datei mit allen Parametern

        Topics, Pfade und Ports lassen sich extern einstellen

    Erweiterbarkeit:

        Zentrale Dispatch-Struktur für MQTT-Kommandos

        Modbus-Diagnosefunktionen modular

Zeitverhalten
Bereich	Frequenz / Trigger	Thread
MQTT-Statistiken	alle mqttStatsInterval Sek.	stats_thread
TCP-Anfragen	bei Anfrage	je Client-Thread
RTU-Kommunikation	pro Anfrage	je Client-Thread
MQTT-Kommandos	bei Nachricht	paho::client
Modbus TCP zu RTU Ablauf

[Modbus TCP Client]
    |
    | TCP Anfrage mit MBAP
    v
[handle_client()]
    |
    | Entferne MBAP, extrahiere Unit-ID + PDU
    |
    | -> Sende RTU-Anfrage (libmodbus)
    | <- Empfange Antwort
    |
    | MBAP rekonstruieren + senden an TCP
    v
[Modbus TCP Client]

MQTT Verarbeitung – Architektur

[MQTT Broker]
     |
     | (modbus/read/1/40001)
     v
[MQTTCallback::message_arrived()]
     |
     | -> Leseauftrag per modbus_read_registers()
     | <- Antwort als JSON oder Fehlercode
     |
     | publish(modbus/response/1/40001)

Vorteile dieser Architektur

    ✅ Stabiler Betrieb: Fehlerhafte Clients crashen das System nicht.

    ✅ Echtzeitfähig genug: Verarbeitung innerhalb Millisekunden möglich.

    ✅ Erweiterbar: MQTT-Logik, neue Diagnosen oder CoAP, REST später ergänzbar.

    ✅ Plattformunabhängig: Cross-compilable für ARM & x86.


⚙️ Kapitel 4: Installationsanleitung
📦 Voraussetzungen

Bevor das Gateway kompiliert oder genutzt werden kann, müssen folgende Pakete und Tools auf dem Entwicklungsrechner installiert sein:
🔧 Abhängigkeiten unter Fedora / Red Hat

sudo dnf install git cmake gcc-c++ make \
    libmodbus-devel openssl-devel \
    paho-mqtt-c-devel paho-mqtt-cpp-devel

🔧 Abhängigkeiten unter Debian / Raspberry Pi OS

sudo apt install git cmake g++ make \
    libmodbus-dev libssl-dev \
    libpaho-mqtt-dev libpaho-mqttpp-dev

    📌 Alternativ wird ein statisches Build mitgeliefert – siehe Kapitel 15: Deployment.

🧱 Struktur des Projektverzeichnisses

modbus_gateway/
├── include/                  # Header-Dateien (z. B. log.h, config.h)
├── src/                      # Implementierung (.cpp-Dateien)
├── third_party/              # Eingebettete externe Bibliotheken (MQTT)
├── build/                    # Build-Ausgabe (nicht eingecheckt)
├── modbus_gateway.ini        # Konfigurationsdatei
├── Makefile                  # Build-Skript
├── modbus_gateway.service    # systemd-Unit (für Raspberry Pi)
└── modbus_gateway.md         # Diese Dokumentation

🏗️ Kompilieren
Einfacher Build mit dem Makefile:

make

Das erzeugt:

    modbus_gateway: Die ausführbare Datei im Projektverzeichnis

    Optional: statisch gelinkte Variante modbus_gateway_static

    💡 Für ARM (z. B. Raspberry Pi) Cross-Kompilierung siehe Kapitel 6.

🧪 Testlauf ohne Verbindung (--dry-run)

./modbus_gateway modbus_gateway.ini --dry-run

Dadurch wird:

    die INI-Konfiguration vollständig geprüft

    TLS-Zertifikate verifiziert (falls TLS aktiviert)

    keine Verbindung zu MQTT oder Modbus aufgebaut

Ausgabe-Beispiel:

Validierung erfolgreich: Konfiguration wurde geladen.
MQTT-Server: mqtt://192.168.0.82:1883
Modbus-Gerät: /dev/ttyUSB0 @ 19200
TCP-Port: 502

📁 Konfiguration (INI-Datei)

Die Datei modbus_gateway.ini muss entweder:

    als erstes Argument übergeben werden

    standardmäßig unter /etc/modbus_gateway.ini liegen

Beispiel:

./modbus_gateway /etc/modbus_gateway.ini

🧯 Beenden des Gateways

Das Gateway reagiert auf:

    SIGINT (Ctrl+C im Terminal)

    SIGTERM (z. B. durch systemctl stop modbus_gateway)

Dadurch werden:

    alle offenen TCP-Verbindungen geschlossen

    MQTT clean disconnect + LWT-Reset gesendet

    Statistik- und Client-Threads ordentlich beendet

    das Logfile sauber abgeschlossen

🚀 Schnellstart
1. Konfiguration anpassen

cp modbus_gateway.ini.example modbus_gateway.ini
nano modbus_gateway.ini

2. Kompilieren

make

3. Starten

./modbus_gateway modbus_gateway.ini


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


🛠 Kapitel 6: Kompilierung & Makefile
🔧 Zielsystem

Das Projekt ist ausgelegt für den Einsatz auf einem Raspberry Pi 5 (aarch64) mit einem modernen Linux-System (z. B. Raspberry Pi OS 64 Bit).
Getestete Umgebungen
Plattform	Status
Raspberry Pi OS 64 bit	✅ stabil
Fedora (Cross-Build)	✅ unterstützt
Debian x86_64 (Build)	✅ mit Cross
🔨 Projektstruktur (relevant für Build)

modbus_gateway/
├── src/                   # C++ Quellcode (.cpp)
├── include/               # Headerdateien (.h)
├── third_party/
│   ├── paho.mqtt.cpp/     # MQTT C++-Bibliothek (mit eingebetteter C-Bibliothek)
│   └── paho.mqtt.c/
├── systemd/               # systemd Service-Dateien
├── tools/                 # Hilfsskripte, Logrotate
├── modbus_gateway.ini     # Konfigurationsdatei (Beispiel)
├── Makefile               # Build-System
└── README.md              # Dokumentation

📦 Abhängigkeiten

Die wichtigsten Abhängigkeiten sind lokal eingebettet:
Bibliothek	Quelle	Status
libmodbus	Systembibliothek	⬅ erforderlich
Paho MQTT C++	third_party/paho.mqtt.cpp	✅ integriert
Paho MQTT C	third_party/paho.mqtt.c	✅ integriert
Paketinstallation (Build-Host)
Fedora:

sudo dnf install libmodbus-devel gcc-c++ make openssl-devel

Debian/Raspberry Pi OS:

sudo apt install libmodbus-dev build-essential libssl-dev

🧪 Cross-Compile für Raspberry Pi 5

Optional kann auf einem x86_64-System für ARM64 (aarch64) kompiliert werden.
Toolchain-Voraussetzung

sudo dnf install gcc-aarch64-linux-gnu

Beispielkonfiguration siehe Datei: toolchain-aarch64.cmake
🧰 Makefile – Übersicht

Das Projekt wird über ein zentrales Makefile verwaltet.

make             # Standard-Build für native Plattform
make clean       # Bereinigt alle erzeugten Dateien
make install     # Installation ins Zielsystem (z. B. /usr/local/bin)
make tarball     # Erzeugt ein .tar.gz Release-Paket

Für Cross-Compilation:

make CROSS=1     # Build für Raspberry Pi (aarch64)

📂 Ausgaben
Ziel	Beschreibung
bin/modbus_gateway	Finales Binary
build/	Objektdateien + Abhängigkeiten
modbus_gateway.tar.gz	Optionales Release-Archiv
🔍 Beispiel: Kompilieren auf dem Pi

git clone https://github.com/deinuser/modbus_gateway.git
cd modbus_gateway
make

🔍 Beispiel: Kompilieren auf x86 für Raspberry Pi

make CROSS=1

→ Vorausgesetzt: Cross-Toolchain ist in $PATH, z. B. aarch64-linux-gnu-g++.


🚀 Kapitel 7: Starten des Gateways
🖥️ Voraussetzungen

Stelle sicher, dass folgende Komponenten auf deinem Raspberry Pi oder Zielsystem vorhanden sind:

    Modbus RTU Gerät via /dev/ttyAMA0, /dev/ttyUSB0, etc.

    MQTT-Broker (lokal oder extern, z. B. Mosquitto)

    Konfigurationsdatei modbus_gateway.ini mit gültigen Parametern

🔧 Start über Terminal

./modbus_gateway /etc/modbus_gateway.ini

Optional:

./modbus_gateway /etc/modbus_gateway.ini --dry-run

📋 Aufrufparameter
Parameter	Beschreibung
[config.ini]	Pfad zur Konfigurationsdatei (Standard: /etc/modbus_gateway.ini)
--dry-run	Validiert nur die Konfiguration, startet keine Netzwerkverbindungen
--validate-only	Synonym für --dry-run
--version	Gibt die aktuelle Versionsnummer aus
--help / -h	Zeigt Hilfetext und verfügbare Optionen an
🧪 Beispielaufrufe

Standardbetrieb mit Konfig-Datei:

./modbus_gateway /etc/modbus_gateway.ini

Nur Konfiguration testen:

./modbus_gateway /etc/modbus_gateway.ini --dry-run

Versionsinfo anzeigen:

./modbus_gateway --version

🏁 Startmeldung (Beispielausgabe)

Starte Gateway...
→ TLS ist aktiviert → MQTT-Verbindung erfolgt verschlüsselt.
→ MQTT-Broker: mqtts://192.168.0.82
→ Modbus RTU: /dev/ttyAMA0 @ 19200 baud
→ TCP-Server aktiv auf Port 502

🧼 Beenden des Gateways

Beende das Programm mit CTRL+C oder sende ein SIGINT/SIGTERM Signal. Der Shutdown erfolgt sauber:

    MQTT-Statusmeldung wird als LWT gesendet

    Modbus & Sockets werden geschlossen

    Alle Threads (Clients, Statistik) werden synchronisiert

🧩 Nächste Schritte

→ Wenn du systemweite Integration planst (z. B. Autostart beim Boot), lies weiter in:

➡️ Kapitel 14: systemd-Integration


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


🔄 Kapitel 9: Modbus TCP ↔ RTU Kommunikation
📦 Protokollübersicht

Das Gateway verbindet zwei Protokollwelten:
Protokoll	Typ	Transport	Anwendung
Modbus TCP	Binär	TCP/IP (Port 502)	Von SCADA, SPS, Visualisierungen
Modbus RTU	Binär (ASCII optional)	RS485 / Serial	Kommunikation mit realen Slaves

Die zentrale Aufgabe: Weiterleitung von TCP-Anfragen an RTU-Geräte und Rückleitung der Antwort – vollständig protokollkonform.
📥 TCP-Eingang

Der TCP-Server horcht standardmäßig auf Port 502, kann aber per config.ini konfiguriert werden:

tcpPort = 502

Jeder neue Client wird in einem separaten Thread (std::thread) verarbeitet.
📤 RTU-Ausgang

Die RTU-Kommunikation basiert auf der libmodbus-Bibliothek und nutzt ein konfiguriertes Gerät, z. B.:

serialPort = /dev/ttyAMA0
baudrate = 19200
parity = N
databits = 8
stopbits = 1

Alle RTU-Zugriffe werden synchron (blocking) verarbeitet.
🔄 Ablauf der Anfrageverarbeitung

    Empfang der TCP-Anfrage

        Auslesen von MBAP-Header (Transaction-ID etc.)

    Extraktion der PDU (Protocol Data Unit)

        Funktionscode + Payload

    Zusammenbau der RTU-Anfrage

        Unit-ID wird vorangestellt

    Versand über Modbus RTU

        Mittels modbus_send_raw_request()

    Empfang der RTU-Antwort

        Über modbus_receive_confirmation()

    Rekonstruktion der MBAP-TCP-Antwort

        Wiederherstellung der Transaction-ID

    Antwort an TCP-Client

        Per send() an den verbundenen Socket

📋 Unterstützte Funktionscodes
FC	Beschreibung	Unterstützt
0x01	Read Coils	✅
0x02	Read Discrete Inputs	✅
0x03	Read Holding Registers	✅
0x04	Read Input Registers	✅
0x05	Write Single Coil	✅
0x06	Write Single Holding Register	✅
0x0F	Write Multiple Coils	✅
0x10	Write Multiple Holding Registers	✅
0x08	Diagnostics (lokal + transparent)	✅
🔎 Diagnosefunktionen

Das Gateway erkennt einige Modbus-Diagnosefunktionen (FC 0x08) lokal:
Subfunktion	Beschreibung	Gateway-Reaktion
0x0000	Echo Test (Loopback)	✅ Lokale Antwort
0x000A	Clear Counters	✅ Setzt Statistik zurück
0x000B	Return Bus Message Count	✅ Liefert TCP-Zähler
0x000C	Return Bus Communication Error Count	✅ Liefert Fehleranzahl
andere	Weiterleitung an RTU	🔁
🔧 Debug-Hex-Dump

Wenn debugHexDump = true in der .ini gesetzt ist, werden alle übertragenen Daten im Log als Hex-Dump ausgegeben:

debugHexDump = true

Beispielausgabe:

→ RTU TX:  01 03 00 64 00 01 85 DB
← RTU RX:  01 03 02 00 7B 79 84

🚨 Fehlerbehandlung

    Bei Fehlern von libmodbus erfolgt Logging + Fehlerzählerinkrement

    Verbindungsfehler am TCP werden protokolliert und führen zum Thread-Abbruch

⚙️ Multi-Threaded Verarbeitung

    Jeder TCP-Client hat einen eigenen Thread

    Gleichzeitige Verarbeitung mehrerer Anfragen möglich

    Zugriff auf modbus_t* für RTU erfolgt serialisiert


📊 Kapitel 10: Diagnose & Statistik
🧭 Ziel der Statistikfunktionen

Das Gateway protokolliert und veröffentlicht auf Wunsch intern wichtige Betriebskennzahlen. Diese dienen:

    der Laufzeitdiagnose,

    dem Debugging,

    der externen Visualisierung (z. B. via MQTT + Grafana).

📈 Verfügbare Zähler
Bezeichner	Bedeutung
total_tcp_requests	Anzahl aller Modbus TCP-Anfragen
successful_rtu_requests	Erfolgreich ausgeführte RTU-Anfragen
failed_rtu_requests	Fehlgeschlagene RTU-Kommunikationen
total_dropped	Abgelehnte/verlorene Verbindungen
total_echo	Lokal beantwortete Echo-Funktionsaufrufe
🧮 Datenstruktur im Code

Diese Zähler sind als std::atomic<uint64_t> deklariert, um Thread-Sicherheit bei gleichzeitiger Verarbeitung zu gewährleisten:

std::atomic<uint64_t> total_tcp_requests{0};
std::atomic<uint64_t> successful_rtu_requests{0};
std::atomic<uint64_t> failed_rtu_requests{0};
std::atomic<uint64_t> total_dropped{0};
std::atomic<uint64_t> total_echo{0};

🔁 Statistik-Thread

Ein Hintergrund-Thread wird bei Programmstart aktiviert, wenn in der Konfiguration gesetzt:

mqttStatsInterval = 60

Dieser Thread publiziert regelmäßig die aktuellen Statistikdaten via MQTT (siehe unten).

Beispielcode:

if (config.mqttStatsInterval > 0)
{
    stats_thread = std::thread([] {
        while (run)
        {
            std::this_thread::sleep_for(std::chrono::seconds(config.mqttStatsInterval));
            if (mqttClient && mqttClient->is_connected())
                publish_stats();
        }
    });
}

📤 MQTT Statistikmeldung

Die Statistiken werden zyklisch über das MQTT-Topic modbus/stats (konfigurierbar) im JSON-Format gesendet:

Beispielausgabe:

{
  "tcp_requests": 2035,
  "rtu_success": 1992,
  "rtu_failures": 9,
  "dropped": 3,
  "echo_replies": 31
}

Topic-Beispiel:

mqttTopicPrefix = modbus/

Ergibt z. B.: modbus/stats
📮 Statistik auf Anfrage senden

Optional kann das Gateway auf das Topic modbus/command/stats hören (falls aktiviert), um auf Befehl die aktuellen Statistiken zu veröffentlichen.

Das funktioniert durch zusätzliche Subscription + Topic-Abfrage in der Callback-Funktion.
♻️ Zähler zurücksetzen

Über Diagnosefunktion 0x08/0x000A (Clear Counters) lassen sich Zähler auf 0 zurücksetzen – lokal beantwortet ohne RTU.

Beispiel:

Client sendet Diagnose-Anfrage (0x08 00 0A 00 00)
→ Zähler im Gateway werden genullt
← Antwort erfolgt sofort vom Gateway

🧪 Beispiel: MQTT + Grafana

Die Werte können per MQTT-Bridge (z. B. Node-RED, Telegraf) in eine Zeitreihendatenbank überführt und dort visualisiert werden.
Mögliche Tools:

    InfluxDB

    Grafana

    Node-RED

🛠️ Konfigurationsoptionen

mqttStatsInterval = 30        ; Sekunden – 0 = deaktiviert
mqttTopicPrefix = modbus/

🧼 Beenden und Cleanup

Der Statistik-Thread wird im shutdown_gateway() über join() beendet, damit keine Zombie-Threads entstehen.


🔐 Kapitel 11: TLS & Sicherheit
🧭 Ziel der TLS-Unterstützung

Das Gateway kann – optional – verschlüsselte TLS-Verbindungen zu einem MQTT-Broker aufbauen. Dies erhöht:

    Vertraulichkeit (Inhalte werden verschlüsselt)

    Integrität (Abwehr von Man-in-the-Middle-Angriffen)

    Authentizität (Client und/oder Broker-Zertifikate)

🔧 Aktivierung von TLS

Die TLS-Funktionalität wird über die Konfigurationsdatei aktiviert:

mqttUseTLS     = true
mqttCAFile     = /etc/ssl/certs/ca-certificates.crt
mqttCertFile   = /etc/modbus_gateway/client.crt
mqttKeyFile    = /etc/modbus_gateway/client.key
mqttInsecure   = false

Erklärung der Parameter:
Name	Beschreibung
mqttUseTLS	TLS aktivieren (true/false)
mqttCAFile	CA-Zertifikat zur Broker-Verifizierung
mqttCertFile	Optionales Client-Zertifikat (für gegenseitige Authentifizierung)
mqttKeyFile	Schlüssel zum Zertifikat
mqttInsecure	Deaktiviert CA-Prüfung (unsicher, aber hilfreich bei Tests)
🔐 TLS-Handshake-Verhalten

    Bei aktiviertem TLS wird intern der mqtt::ssl_options Block gesetzt und dem mqtt::connect_options hinzugefügt.

    Die Kommunikation wird dann automatisch über SSL abgewickelt.

mqtt::ssl_options sslopts;
sslopts.set_trust_store(config.mqttCAFile);
sslopts.set_key_store(config.mqttCertFile);
sslopts.set_private_key(config.mqttKeyFile);
sslopts.set_enable_server_cert_auth(!config.mqttInsecure);

connOpts.set_ssl(sslopts);

🛠️ Fehlersuche bei TLS-Problemen

Typische Ursachen für Verbindungsfehler:

    CA-Datei fehlt oder ist nicht lesbar:
    → Prüfe Pfad und Berechtigungen.

    Client-Zertifikat + Key fehlen bei Auth-Zwang:
    → MQTT-Broker verlangt Client-Zertifikat.

    Zertifikatsprüfung schlägt fehl:
    → Setze testweise mqttInsecure = true, um das zu verifizieren.

📄 Zertifikate erzeugen (Beispiel)
🧪 Selbstsigniertes Zertifikat für Testbetrieb:

openssl req -x509 -newkey rsa:2048 -nodes -keyout client.key -out client.crt -days 365

Dann in modbus_gateway.ini eintragen:

mqttCertFile = /pfad/zum/client.crt
mqttKeyFile  = /pfad/zum/client.key
mqttInsecure = true

🧼 Fallback zu unverschlüsselt

Wenn mqttUseTLS = false gesetzt ist, wird keine TLS-Verschlüsselung aktiviert. Die Verbindung läuft dann über Standard-Port 1883 oder benutzerdefiniert.
🧮 Integration in init_mqtt()

Die TLS-Parameter werden nur gesetzt, wenn mqttUseTLS == true. Andernfalls erfolgt der Verbindungsaufbau ohne Verschlüsselung.

Fehler beim Aufbau werden geloggt und führen zum Abbruch des Startvorgangs.
🔒 Empfehlung

In produktiven Umgebungen sollte TLS immer aktiviert sein. Besonders bei:

    Cloud-Deployments

    öffentlich erreichbaren MQTT-Brokern

    integritätskritischen Automatisierungslösungen


🪵 Kapitel 12: Logging
📋 Ziel des Loggings

Das Gateway führt ein internes Logbuch, um alle relevanten Ereignisse, Fehler und Statusmeldungen systematisch zu protokollieren. Dies erleichtert:

    Debugging und Fehlersuche

    Analyse von Verbindungsproblemen

    Nachvollziehbarkeit von Modbus-/MQTT-Abläufen

🧩 Implementierung
🔧 Zentrale Logfunktion

Die Datei modbus_gateway.cpp enthält eine zentrale log()-Funktion:

void log(const std::string &msg)
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%F %T") << " - " << msg;

    std::string fullMsg = ss.str();
    std::cout << fullMsg << std::endl;
    if (logFile.is_open())
        logFile << fullMsg << std::endl;
}

⏺ Format der Ausgabe

YYYY-MM-DD HH:MM:SS - <Nachricht>

Beispiel:

2025-06-26 19:45:12 - Modbus-TCP-Anfrage empfangen, Länge: 12

🧾 Logdatei

Die Logdatei wird automatisch beim Start des Gateways geöffnet:

logFile.open(config.logFilePath, std::ios::app);

🗂 Konfigurierbarer Pfad

logFilePath = /var/log/modbus_gateway.log

Wird diese Datei nicht explizit gesetzt, schreibt das Gateway nur in die Konsole.
🧮 Log-Level

Das aktuelle Logging ist informativ und unstrukturiert. Es gibt keine formellen Log-Level wie DEBUG / INFO / WARN / ERROR, aber es könnten folgende Strategien manuell übernommen werden:
Nachrichtentyp	Beispiel
🔹 Info	„Starte Gateway...“
⚠️ Warnung	„Kein CA-Zertifikat angegeben“
❌ Fehler	„Fehler bei MQTT-Verbindung: ...“
✅ Erfolg	„MQTT erfolgreich verbunden.“

Eine Erweiterung um formelle Log-Level (z. B. per Enum) wäre optional sinnvoll.
🧪 Beispielhafte Logzeilen

2025-06-26 18:17:43 - Starte Gateway...
2025-06-26 18:17:43 - TLS ist aktiviert → MQTT-Verbindung erfolgt verschlüsselt.
2025-06-26 18:17:44 - Neuer Client verbunden
2025-06-26 18:17:45 - → Modbus Diagnose-Funktion erkannt (Subfunction: 0x0000)
2025-06-26 18:17:45 - → Lokale Antwort auf Echo-Test (FC 0x08, Sub 0x0000)
2025-06-26 18:17:47 - Fehler beim Senden an RTU: Connection timed out

🔧 Optionale Erweiterungen
Vorschlag	Beschreibung
🔹 log_debug(), log_error()	Unterscheidung von Logtypen
🔹 Rotation via logrotate	Vermeidung von überlaufenden Logdateien
🔹 Remote-Logging via MQTT/HTTP	Übertragung der Logs an zentralen Server
🔹 JSON-Format	Strukturierte Logdaten für maschinelle Auswertung
🧩 Integration in Systemd

Falls systemd verwendet wird, kann das Logging auch an journalctl übergeben werden – z. B. durch:

logFilePath = /dev/stdout

Dann stehen alle Logs zentral per journalctl -u modbus_gateway.service zur Verfügung.


❗ Kapitel 13: Fehlerbehandlung
🧩 Ziel der Fehlerbehandlung

Ein robustes Gateway muss fehlerhafte Zustände erkennen, protokollieren und – sofern möglich – automatisch beheben oder den Betrieb sicher beenden.
🔧 Kategorien von Fehlern
Kategorie	Beispiele	Behandlungsstrategie
🔌 Verbindungsfehler	MQTT-Server nicht erreichbar, RTU getrennt	Wiederverbindungsversuche
⚙️ Konfigurationsfehler	Fehlende/ungültige INI-Werte, nicht lesbare Dateien	Frühzeitiger Abbruch mit Log
🧱 Protokollfehler	Ungültige MBAP-/RTU-Rahmen, unzulässige Funktionen	Abbruch einzelner Anfrage
💥 Laufzeitfehler	Exceptions, System-Signale	sauberes shutdown_gateway()
🪓 MQTT-Verbindungsfehler
Fehlerfall:

MQTT error [-6]: Invalid (NULL) parameter

Behandlung:

    Try/Catch in init_mqtt()

    Logging der Fehlermeldung

    Rückgabe false → beendet main()

Beispiel:

catch (const mqtt::exception &e)
{
    log("Fehler bei MQTT-Verbindung: " + std::string(e.what()));
    return false;
}

🧱 Fehler bei Modbus-Übertragung
a) Fehler beim Senden an RTU

int sent = modbus_send_raw_request(rtu_ctx, full_request, full_len);
if (sent == -1)
{
    log("Fehler beim Senden an RTU: " + std::string(modbus_strerror(errno)));
    break;
}

b) Fehler bei Empfang

int res_len = modbus_receive_confirmation(rtu_ctx, response);
if (res_len == -1)
{
    failed_rtu_requests++;
    log("Fehler beim Empfangen von RTU: " + std::string(modbus_strerror(errno)));
    break;
}

📥 Fehlerhafte Konfiguration

In init_config():

if (ini_parse(config_path, config_handler, &config) < 0)
{
    log("Konnte Konfiguration nicht laden, nutze Defaults.");
    return false;
}

Erweiterung für TLS-Pfade:

if (!config.mqttCAFile.empty() && access(config.mqttCAFile.c_str(), R_OK) != 0)
    log("⚠️  Fehler: CA-Datei nicht lesbar: " + config.mqttCAFile);

🔄 Automatischer Reconnect bei MQTT

Der MQTT Callback behandelt:

    Verbindungsabbruch (connection_lost)

    Reconnect (ohne Parallelität durch Flag)

Beispiel:

void mqtt_callback::connection_lost(const std::string &cause)
{
    if (!reconnecting)
    {
        reconnecting = true;
        log("MQTT-Verbindung verloren: " + cause);
        std::thread([]()
        {
            while (run && !mqttClient->is_connected())
            {
                try { mqttClient->reconnect()->wait(); }
                catch (...) { std::this_thread::sleep_for(2s); }
            }
            reconnecting = false;
        }).detach();
    }
}

🔔 Behandlung von Signalen

In main():

signal(SIGINT, signal_handler);
signal(SIGTERM, signal_handler);

In signal_handler():

void signal_handler(int)
{
    run = false;
    log("Beende Gateway durch Signal");
}

→ Führt zum sauberen Beenden über shutdown_gateway()
🧪 Diagnose durch Statistik

Fehler werden zählbar gemacht:

failed_rtu_requests++;
total_dropped++;

Diese Werte werden regelmäßig als JSON via MQTT veröffentlicht:

{
  "tcp_requests": 122,
  "rtu_success": 118,
  "rtu_failures": 4,
  "dropped": 2,
  "echo_replies": 3
}

💡 Erweiterungsvorschläge
Vorschlag	Nutzen
📈 Fehlerrate pro Minute	Früherkennung von Störungen
📉 Reset bei zu vielen Fehlern	Auto-Recovery / Watchdog
🧩 Fehlercodes als MQTT-Topic	z. B. modbus_gateway/error mit Ursache & Zeitstempel


14. Systemd Integration
14.1 Service-Datei (/etc/systemd/system/modbus_gateway.service)

[Unit]
Description=Modbus TCP <-> RTU + MQTT Gateway
After=network.target

[Service]
ExecStart=/usr/local/bin/modbus_gateway /etc/modbus_gateway.ini
Restart=always
User=modbus
Group=modbus

[Install]
WantedBy=multi-user.target

🧩 Kapitel 14: Systemd Integration

Dieses Kapitel beschreibt die Integration des Modbus-Gateways als systemd-Service unter Linux – insbesondere für Raspberry Pi OS (Debian-basiert), aber auch allgemein unter allen Systemd-fähigen Distributionen wie Fedora, Ubuntu oder Arch.
✅ Ziel

Das Modbus-Gateway soll:

    automatisch beim Systemstart starten

    bei einem Absturz automatisch neu starten

    unter einem eigenen Benutzer mit minimalen Rechten laufen

    Logs gezielt in systemd-journal oder separate Dateien schreiben

    sich sauber beenden lassen, z. B. per systemctl stop modbus_gateway

📁 Service-Datei: /etc/systemd/system/modbus_gateway.service

[Unit]
Description=Modbus TCP <-> RTU + MQTT Gateway
After=network.target

[Service]
Type=simple
User=modbusgw
Group=modbusgw
ExecStart=/usr/local/bin/modbus_gateway /etc/modbus_gateway.ini
WorkingDirectory=/usr/local/bin
Restart=on-failure
RestartSec=5
StandardOutput=journal
StandardError=journal

# Ressourcenlimits (optional)
LimitCORE=infinity
LimitNOFILE=4096

[Install]
WantedBy=multi-user.target

👤 Benutzer & Berechtigungen

Aus Sicherheitsgründen sollte das Gateway nicht als root laufen. Lege einen dedizierten Benutzer an:

sudo useradd --system --no-create-home --shell /usr/sbin/nologin modbusgw

Stelle sicher, dass:

    /usr/local/bin/modbus_gateway vom Benutzer modbusgw ausführbar ist

    /etc/modbus_gateway.ini lesbar ist

    ggf. die Modbus-Seriellschnittstelle (z. B. /dev/ttyUSB0) lesbar ist:

sudo usermod -aG dialout modbusgw
sudo chmod 660 /dev/ttyUSB0

🛠️ Installation & Aktivierung

    Installiere das Binary:

sudo cp modbus_gateway /usr/local/bin/
sudo chmod 755 /usr/local/bin/modbus_gateway

    Kopiere die .ini-Datei:

sudo cp modbus_gateway.ini /etc/

    Erstelle die service-Datei:

sudo nano /etc/systemd/system/modbus_gateway.service

    Aktivieren und starten:

sudo systemctl daemon-reexec       # Nur wenn systemd frisch konfiguriert
sudo systemctl daemon-reload       # Dienstdefinitionen neu einlesen
sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway

    Status prüfen:

sudo systemctl status modbus_gateway
journalctl -u modbus_gateway -f

♻️ Automatischer Neustart bei Fehlern

Dank Restart=on-failure wird der Dienst bei Abstürzen automatisch neu gestartet. Du kannst dieses Verhalten anpassen:
Option	Beschreibung
Restart=always	Neustart bei jedem Exit-Code
Restart=on-abort	Nur bei Signals wie SIGABRT
RestartSec=5	5 Sekunden Pause vor Neustart
📃 Log-Ausgabe

Die Anwendung schreibt standardmäßig zusätzlich in logFilePath (aus der .ini), aber systemd protokolliert auch:

journalctl -u modbus_gateway.service

Wenn du willst, kannst du den Logausgabe-Modus umstellen:

StandardOutput=file:/var/log/modbus_gateway.log
StandardError=inherit

Dann ggf. Logrotate einrichten (→ Kapitel 15).
15. Deployment
15.1 Verzeichnisse
Zweck	Pfad
Konfig	/etc/modbus_gateway.ini
Logs	/var/log/modbus_gateway.log
Service	/etc/systemd/system/...

🚀 Kapitel 15: Deployment

In diesem Kapitel beschreiben wir den empfohlenen Ablauf für die Bereitstellung (Deployment) des Gateways auf einem Zielsystem – typischerweise einem Raspberry Pi 5 mit Raspberry Pi OS (64-bit).
📦 Ziel des Deployments

Das Deployment-Paket soll:

    das Kompilat für aarch64 enthalten

    die Konfigurationsdatei (modbus_gateway.ini) beilegen

    die systemd-Service-Datei bereitstellen

    optional: ein Installationsskript mit Setup-Schritten

📁 Empfohlene Struktur des Release-Verzeichnisses

modbus_gateway_release/
├── modbus_gateway              # ⚙️ Kompilierte Binärdatei (aarch64)
├── modbus_gateway.ini          # 🛠 Konfigurationsdatei
├── modbus_gateway.service      # 🧩 systemd-Service
├── install.sh                  # 🧰 Installationsskript
└── README.md                   # 📘 Dokumentation

🧰 Beispiel: Installationsskript install.sh

#!/bin/bash

echo "📦 Installiere Modbus Gateway..."

# Benutzer anlegen, falls nicht vorhanden
if ! id "modbusgw" &>/dev/null; then
    echo "👤 Erstelle Benutzer 'modbusgw'"
    sudo useradd --system --no-create-home --shell /usr/sbin/nologin modbusgw
fi

# Dateien kopieren
echo "📁 Kopiere Dateien..."
sudo cp modbus_gateway /usr/local/bin/
sudo cp modbus_gateway.ini /etc/
sudo cp modbus_gateway.service /etc/systemd/system/

# Berechtigungen setzen
sudo chown root:root /usr/local/bin/modbus_gateway
sudo chmod 755 /usr/local/bin/modbus_gateway

sudo chmod 644 /etc/modbus_gateway.ini
sudo chmod 644 /etc/systemd/system/modbus_gateway.service

# Benutzer zur dialout-Gruppe hinzufügen (für /dev/ttyUSB0 Zugriff)
sudo usermod -aG dialout modbusgw

# Dienst aktivieren und starten
sudo systemctl daemon-reload
sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway

echo "✅ Modbus Gateway wurde installiert und gestartet."

📦 Paket-Erstellung mit tar.gz

Auf deinem Buildsystem (z. B. Fedora, Cross-Compiled):

mkdir modbus_gateway_release
cp modbus_gateway modbus_gateway_release/
cp modbus_gateway.ini modbus_gateway_release/
cp modbus_gateway.service modbus_gateway_release/
cp install.sh modbus_gateway_release/
cp README.md modbus_gateway_release/

tar -czvf modbus_gateway.tar.gz modbus_gateway_release/

→ Dieses Archiv lässt sich direkt auf dem Pi entpacken und ausführen.
🧪 Testlauf auf Zielsystem

scp modbus_gateway.tar.gz pi@raspberrypi:/tmp/
ssh pi@raspberrypi

cd /tmp
tar -xzf modbus_gateway.tar.gz
cd modbus_gateway_release
chmod +x install.sh
./install.sh

Danach prüfen:

sudo systemctl status modbus_gateway

🔒 Sicherheitshinweise

    Gib nur Lesezugriff auf modbus_gateway.ini

    Nutze User=modbusgw in der Service-Datei

    Verwende TLS für MQTT, wo möglich

    Nutze Firewalls für TCP-Port 502, falls öffentlich erreichbar


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

🧯 Kapitel 17: Troubleshooting & Fehlersuche

Dieses Kapitel hilft dir bei der systematischen Diagnose, wenn etwas nicht wie erwartet funktioniert. Es enthält typische Fehlersymptome, mögliche Ursachen und empfohlene Lösungswege.
🔌 17.1 Keine Verbindung zum Modbus-RTU-Gerät

Symptome:

    Es erscheinen regelmäßig Fehler beim Senden an RTU oder Fehler beim Empfangen von RTU

    Alle Anfragen schlagen fehl

Ursachen & Lösungen:
Ursache	Lösung
Falscher serieller Port	Prüfen: device = /dev/ttyUSB0 oder z. B. /dev/ttyAMA0
Baudrate/Parität falsch	Anpassen auf Gerät (z. B. 19200 bps, 8N1)
Kein Abschlusswiderstand	Bei RS485 prüfen
Busspannung fehlt	GND-/RS485-Leitungen checken
Kein oder falscher Slave	unit-ID überprüfen
🌐 17.2 Kein Zugriff über Modbus TCP

Symptome:

    Verbindung via mbpoll oder SCADA schlägt fehl

    Port 502 nicht erreichbar

Prüfpunkte:

    Läuft das Gateway? → ps aux | grep modbus_gateway

    Lauscht es auf dem TCP-Port? → netstat -an | grep 502

    Firewall geöffnet? → sudo ufw allow 502/tcp

    Wird start_tcp_server() erfolgreich ausgeführt?

📡 17.3 MQTT-Verbindung schlägt fehl

Logmeldungen:

    Fehler bei MQTT-Verbindung: MQTT error [-6]: Invalid (NULL) parameter

    connect error: Unknown host oder TLS-Fehler

Maßnahmen:

    Broker-Adresse korrekt? → broker = 192.168.0.82

    Port geöffnet? (meist 1883 / 8883 für TLS)

    TLS aktiv, aber ohne gültige Zertifikate? → mqttUseTLS = false testweise setzen

    Broker online? → mosquitto_sub -h ... oder telnet 192.168.0.82 1883

🧪 17.4 Diagnosefunktion funktioniert nicht

Symptom: mbpoll mit Function Code 08 liefert keine Antwort.

Lösungen:

    Gateway unterstützt nur bestimmte Subfunktionen:

        0x0000 (Echo-Test)

        0x000A (Zähler zurücksetzen)

        0x000B (Bus-Message-Count)

        0x000C (Fehlerzähler)

    Für nicht unterstützte wird korrekt keine lokale Antwort gegeben → Anfrage geht an RTU

📉 17.5 Statistik wird nicht gesendet

Symptom: MQTT-Topic modbus/stats bleibt leer.

Checkliste:
Prüfen	Korrektur
mqttStatsInterval > 0 gesetzt?	Im INI setzen, z. B. mqttStatsInterval = 60
mqttClient->is_connected()?	MQTT läuft und verbunden
Wird publish_stats() aufgerufen?	Ja – über Thread in main()
Debuglog aktiviert?	debug_hex_dump = true liefert mehr Details
🔁 17.6 Gateway hängt oder beendet sich

Symptome:

    Keine Reaktion mehr

    Crash bei vielen gleichzeitigen Verbindungen

Mögliche Ursachen:

    Stacküberlauf in Threads → zu kleine Puffer

    Ungültige RTU-Antworten → besser mit dump_hex() debuggen

    race conditions durch fehlende std::atomic beim Statistikzähler

    TLS: Nicht-blockierende Fehler bei Verbindungsversuch


🚀 Kapitel 18: Erweiterungsmöglichkeiten & Roadmap

Dieses Kapitel beleuchtet denkbare künftige Erweiterungen deines Gateways sowie Optionen zur Modularisierung, Optimierung und Integration in größere Systeme. Es dient gleichzeitig als Ideensammlung für spätere Versionen.
🔄 18.1 Modbus-TCP → RTU-Multicast

Ziel:
Anfragen von einem TCP-Client simultan an mehrere RTU-Slaves senden.

Beispiel:
Broadcast-Read von Holding Register 0 an alle Slaves 1–5.

Mögliche Umsetzung:

    INI-Einstellung multicast_ids = 1,2,3,4,5

    Anfrage duplizieren und einzeln pro ID senden

    Rückmeldungen sammeln (oder nur erste liefern)

🔐 18.2 Authentifizierung für MQTT

Ziel:
Sicherer Betrieb in öffentlichen oder Cloud-Netzwerken.

Varianten:

    Username/Passwort (via mqttUser / mqttPassword)

    TLS-Clientzertifikate (bereits vorbereitet)

    ACL-Mechanismen über externe Broker-Konfiguration

📈 18.3 Web-Dashboard

Ziel:
Lokale oder remote Visualisierung der:

    Verbindungszustände

    Statistikdaten

    Live-Werte aus Modbus-Devices

Umsetzung:

    JSON-Daten über MQTT

    Node-RED Dashboard (ready-to-use Flow beilegen)

    Optional: Integrierter Micro-Webserver

🔌 18.4 Konfigurations-Reload zur Laufzeit

Motivation:
Derzeit ist ein Neustart des Gateways nötig bei Änderungen der INI-Datei.

Vorschläge:

    MQTT-Befehl modbus/command/reload

    Oder SIGHUP-Signal abfangen

    Nur ausgewählte Sektionen neu laden (z. B. Logging-Level)

🧠 18.5 Künstliche Intelligenz für Regelstrategien

Beispiel-Anwendung:
PV-gesteuerte Wärmepumpe mit dynamischer Anpassung des Modbus-Schaltverhaltens.

Möglichkeiten:

    Reinforcement Learning (Q-Learning)

    Node-RED mit Logikblöcken

    Decision Trees für Regelzeitpunkte (z. B. 15 min Moving Window)

📜 18.6 Erweiterte Diagnose-Subfunktionen (FC 0x08)

Noch fehlende Subfunktionen:
Subfunktion	Beschreibung
0x0001	Restart Communication Option
0x0004	Force Listen Only Mode
0x000E	Clear Counters and Diagnostic Register
0x000F	Return Bus Exception Error Count

Hinweis:
Nicht alle sind sinnvoll für Gateways – aber implementierbar.
🧪 18.7 Test- und Simulationsmodus

Funktion:
Ein Mock-Modbus-RTU-Endgerät simuliert intern Werte.

Vorteile:

    Betrieb auch ohne reale RS485-Hardware

    Integrationstest im CI/CD

Umsetzung:

    --simulate Flag → alle Modbus-PDU-Funktionen intern behandeln

    Responses synthetisch generieren

🗺 18.8 Alternative Backend-Protokolle

Zukunftsoptionen:

    BACnet (Building Automation)

    OPC-UA (Industrial IoT)

    REST API für JSON-Zugriffe

    gRPC für performante Clients

✅ Fazit

Das Gateway ist bereits jetzt stabil und robust. Es eignet sich ideal als IoT-Bridge, kann aber durch zukünftige Erweiterungen erheblich an Funktionsumfang und Flexibilität gewinnen.


🧩 Kapitel 19: Codeübersicht & Projektstruktur

Dieses Kapitel bietet eine strukturierte Übersicht über alle relevanten Quellcodedateien, deren Zuständigkeiten und das Zusammenspiel der einzelnen Module im Projekt. Es richtet sich sowohl an Entwickler (zur Erweiterung oder Wartung) als auch an Systemintegratoren (zur Anpassung und Integration).
🗂️ 19.1 Projektverzeichnis

Die Projektstruktur sieht typischerweise wie folgt aus:

modbus_gateway/
├── include/
│   ├── ini.h                   # INI-Parser Header
│   ├── mqtt_client.h          # MQTT-Client Funktionen (optional)
│   └── config_structs.h       # Strukturdefinitionen
├── src/
│   ├── modbus_gateway.cpp     # Hauptprogramm
│   ├── ini.c                  # INI-Parser Implementierung
│   ├── mqtt_client.cpp        # MQTT-Client Implementierung
│   └── utils.cpp              # Hilfsfunktionen (z.B. Logging, Hexdump)
├── third_party/
│   └── paho.mqtt.*            # Eingebettete MQTT-Bibliotheken (C/C++)
├── modbus_gateway.ini         # Beispielkonfigurationsdatei
├── Makefile                   # Kompilierungsregelwerk
├── modbus_gateway.service     # systemd-Unit
└── README.md                  # Diese Dokumentation

⚙️ 19.2 Hauptbestandteile
Datei	Aufgabe
modbus_gateway.cpp	Haupt-Logik für TCP-Server, Modbus-Weiterleitung, MQTT-Handling
ini.c/h	Leichtgewichtiger INI-Parser ohne Abhängigkeiten
mqtt_client.cpp/h	Abstrahierung für MQTT-Verbindung, Topics, Callback-Registrierung
utils.cpp	Hexdump, Logging, Signal-Handler, Statistik-Funktionen
Makefile	Plattformübergreifende Cross-Compile-Optionen (auch für Raspberry Pi)
modbus_gateway.service	Automatischer Start über systemd inkl. Restart-Strategie
modbus_gateway.ini	Konfigurationsdatei mit allen Betriebsparametern
🔄 19.3 Ablaufdiagramm (vereinfachte Hauptlogik)

           +-----------------------+
           |      Programmstart    |
           +----------+------------+
                      |
                      v
            +-------------------+
            |  Konfiguration    |
            |  aus INI laden    |
            +--------+----------+
                     |
        +------------v-------------+
        | Initialisiere:           |
        | MQTT, Modbus RTU, TCP    |
        +------------+-------------+
                     |
              +------v------+           +------------------+
              | TCP-Server  | <-------- | Clients verbinden |
              +-------------+           +------------------+
                     |
            +--------v---------+
            |  TCP → MBAP      |
            |  Modbus PDU → RTU|
            +--------+---------+
                     |
            +--------v---------+
            | Sende an RTU     |
            | Empfange Antwort |
            +--------+---------+
                     |
            +--------v---------+
            | Antwort an TCP   |
            +------------------+

🧠 19.4 Multithreading

Das Gateway verwendet mehrere Threads:

    Main-Thread: Initialisierung, Logging, Konfiguration

    Client-Threads: Jeder TCP-Client erhält einen dedizierten Thread (handle_client)

    Statistik-Thread: Sendet zyklisch MQTT-Statusdaten

    (optional): Watchdog, Healthchecks, MQTT-Reconnect

📊 19.5 Globale Variablen

Die meisten globalen Zustände (z. B. run, config, mqttClient) sind entweder:

    std::atomic<> für Zähler

    Smart Pointer (std::unique_ptr / std::shared_ptr)

    Konfigurationsstruktur mit Werten aus der .ini

Beispiel (vereinfacht):

std::atomic<uint64_t> total_tcp_requests{0};
extern Config config;
std::unique_ptr<mqtt::async_client> mqttClient;

🔧 19.6 Compilerkompatibilität

    Getestet mit:

        g++ (native und aarch64-linux-gnu-g++)

        clang++

    Unterstützte Standards:

        C++17

        C17 (für Hilfsbibliotheken)

Das Makefile und die c_cpp_properties.json sind für Cross-Kompilation vorbereitet.
💡 19.7 Besonderheiten

    MBAP-Header-Rekonstruktion bei TCP-RTU-Gateway mit voller Transaction-ID-Unterstützung

    Diagnose-Funktion 0x08 (lokale Subfunktionen wie Echo, Zählerabruf, Reset)

    MQTT-Befehle wie modbus/command/stats

    Hexdump für Debug bei Bedarf aktivierbar

    TLS-Unterstützung über Paho MQTT möglich

    Dry-Run-Modus zur reinen Konfigurationsvalidierung ohne Verbindungsaufbau


🧬 Kapitel 20: Quellcode-Erklärung – modbus_gateway.cpp

Dieses Kapitel enthält eine ausführliche Erklärung des zentralen Quellcodes. Ziel ist es, alle relevanten Logikabschnitte nachvollziehbar zu beschreiben, damit Entwickler sich schnell einarbeiten und das Verhalten anpassen können.
🧾 20.1 Includes & globale Initialisierung

#include <modbus/modbus.h>
#include <mqtt/async_client.h>
#include <fstream>
#include <thread>
#include <atomic>
#include <csignal>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "ini.h"
#include "config_structs.h"

    Modbus: libmodbus für TCP/RTU-Kommunikation

    MQTT: Eclipse Paho MQTT C++ Client

    Threading: std::thread, std::atomic

    INI: Eigenes INI-Parsing-Modul

    Konfiguration: Struktur zur zentralen Konfigurationshaltung

Globale Variablen wie z. B. std::atomic<uint64_t> total_tcp_requests; werden deklariert, um Statistik zu führen.
🔔 20.2 Signalbehandlung

void signal_handler(int signal)
{
    run = false;
}

    Abfangen von SIGINT und SIGTERM

    Beendet alle Hauptschleifen (u. a. Statistik-Thread, Client-Threads)

📦 20.3 main() – Programmstart

int main(int argc, char *argv[])

Hauptaufgaben:

    Parameter analysieren:

        --help, --version, --dry-run

    Initialisierung:

        init_config()

        init_mqtt()

        init_modbus()

        start_tcp_server()

    Statistik-Thread starten

    Client-Anfragen akzeptieren

    Cleanup mit shutdown_gateway()

Wenn --dry-run gesetzt ist, werden nur Konfigurationswerte geprüft und die Software beendet sich danach automatisch.
🌐 20.4 TCP-Verarbeitung: accept_clients()

void accept_clients()

    Lauscht auf eingehende TCP-Verbindungen (Standard: Port 5020)

    Für jeden Client wird std::thread mit handle_client() gestartet

    Jeder Thread führt TCP→RTU→TCP-Übersetzung durch

🔁 20.5 Modbus-Verarbeitung: handle_client()

Der wichtigste Teil der Anwendung.
Ablauf:

    TCP-Query via modbus_receive()

    MBAP-Header analysieren (transaction_id, unit_id)

    Bei Funktionscode 0x08:

        Subfunktion 0x0000 (Echo): lokal beantworten

        Subfunktion 0x000B (Anfragezähler): lokal simulieren

    PDU extrahieren, RTU-Request zusammensetzen

    Senden via modbus_send_raw_request()

    Antwort empfangen via modbus_receive_confirmation()

    MBAP-Header rekonstruieren für TCP

    Antwort an Client senden

Besonderheiten:

    Fehlererkennung mit Logging

    Debug-Hexdump bei Bedarf aktivierbar

    Zähler für erfolgreiche/fehlgeschlagene Modbus-Antworten

📊 20.6 Statistik-Thread

std::thread stats_thread;

Startet im Hintergrund, wenn mqttStatsInterval > 0 gesetzt ist.
Sendet regelmäßig:

{
  "tcp_requests": 42,
  "rtu_success": 40,
  "rtu_failures": 2,
  "dropped": 0,
  "echo_replies": 1
}

→ Ziel-Topic: modbus/stats
📤 20.7 publish_stats() (on-demand per MQTT)

Diese Funktion wird aufgerufen, wenn über das Topic modbus/command/stats eine Anforderung kommt.

void publish_stats()

    Baut ein JSON mit aktuellen Werten

    Sendet via mqttClient->publish(...)

📑 20.8 Shutdown & Cleanup

void shutdown_gateway()

Führt aus:

    Alle client_threads.join()

    Statistik-Thread join()

    MQTT clean disconnect

    Modbus-RTU-Verbindung schließen

    Socket schließen

    Logdatei abschließen

📦 20.9 Optional: TLS (MQTT)

Wenn mqttUseTLS=true gesetzt ist, werden folgende TLS-Felder genutzt:

    mqttCAFile

    mqttCertFile

    mqttKeyFile

    mqttInsecure (Zertifikatsprüfung deaktivieren)

Wird direkt in init_mqtt() ausgewertet und an die mqtt::ssl_options übergeben.
🧪 20.10 Dry-Run & Konfiguration

bool dry_run = false;

    Parameter --dry-run oder --validate-only

    Lädt die Konfiguration, prüft Datei-Zugriffe und TLS-Dateien

    Beendet sich ohne Modbus-/MQTT-/TCP-Verbindung


🧩 Kapitel 21: Verwendete Bibliotheken und Abhängigkeiten

In diesem Kapitel werden sämtliche externen und internen Bibliotheken aufgelistet, die im Projekt modbus_gateway verwendet werden – inklusive Zweck, Lizenz, Integration und Besonderheiten.
📚 21.1 Externe Bibliotheken
🔌 libmodbus

    Zweck: Kommunikation über Modbus TCP und RTU (seriell)

    Version: ≥ 3.1.5 empfohlen

    Lizenz: LGPLv2.1

    Link: https://libmodbus.org/

    Nutzung:

        modbus_new_tcp(), modbus_new_rtu()

        modbus_receive(), modbus_send_raw_request(), modbus_receive_confirmation()

    Besonderheiten:

        Ermöglicht direkten Zugriff auf ADU (Application Data Unit)

        Sehr stabil in industriellen Umgebungen

        Unterstützt Timeouts, Slave-ID-Management

☁️ Eclipse Paho MQTT C++ & C

    Zweck: Kommunikation mit einem MQTT-Broker

    Version: C++ (1.2+), C (1.3+)

    Lizenz: EPL/EDL (Apache-kompatibel)

    Link: https://www.eclipse.org/paho/

    Nutzung:

        mqtt::async_client für nicht-blockierende Kommunikation

        mqtt::connect_options, mqtt::ssl_options

        publish(), subscribe(), disconnect()

    TLS-Unterstützung:

        CA, Client-Cert, Private Key

        Optional ohne Zertifikatsprüfung (mqttInsecure=true)

🔧 21.2 Interne Module
📄 ini.h / ini.c

    Zweck: INI-Dateien einlesen und Konfigurationswerte zuordnen

    Typ: Minimalistische INI-Parserbibliothek

    Lizenz: zlib/libpng (frei verwendbar)

    Integration: Direkt in Projekt eingebettet

🗂️ config_structs.h

    Zweck: Definition der Config-Struktur

    Inhalte:

        TCP-Port, MQTT-Broker, TLS-Pfade

        Verhalten bei Debug, Hexdump, Statistik etc.

    Wird beim Parsen aus der INI-Datei befüllt

🧪 21.3 Entwicklungsumgebung
🛠 Compiler

    Cross-Compiler: aarch64-linux-gnu-g++

    Target: Raspberry Pi 5 / Raspberry Pi OS (aarch64)

    Standard: c++17

💻 Build-System

    Makefile: Unterstützt statisches und dynamisches Linken

    Toolchain: Optional über toolchain-aarch64.cmake für CMake-Projekte

    Systemd-Service: Start beim Booten mit Restart-Option

📦 21.4 Abhängigkeiten für den Build
Komponente	Paketname (Debian)	Paketname (Fedora)
libmodbus	libmodbus-dev	libmodbus-devel
Eclipse Paho C	libpaho-mqtt-dev	paho-mqtt-c-devel
Eclipse Paho C++	libpaho-mqttpp-dev	paho-mqtt-cpp-devel
pthreads (Thread)	Teil von libc	Teil von glibc
🔒 21.5 TLS-Zertifikate (optional)

Wenn MQTT über TLS genutzt wird:

    mqttUseTLS=true

    Benötigt ggf.:

        mqttCAFile=/etc/ssl/certs/ca-certificates.crt

        mqttCertFile=/etc/ssl/certs/client.pem

        mqttKeyFile=/etc/ssl/private/client.key

Falls keine Prüfung gewünscht:

    mqttInsecure=true (nicht empfohlen für produktive Systeme!)


🏷️ Kapitel 22: Lizenz

Dieses Projekt folgt dem Prinzip der Offenheit, Transparenz und freien Weiterverwendung, orientiert an etablierten Open-Source-Praktiken. In diesem Kapitel findest du alle Lizenzinformationen für das Gateway selbst sowie alle verwendeten Bibliotheken.
📘 22.1 Projektlizenz (modbus_gateway)

Das Projekt modbus_gateway wird unter der folgenden Lizenz veröffentlicht:

MIT License

Copyright (c) 2025 Benno + OpenAI

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

🧩 22.2 Drittabhängigkeiten und Lizenzen
Bibliothek	Lizenztyp	Kompatibilität	Bemerkung
libmodbus	LGPLv2.1	✅ Ja	Dynamisch oder als Systempaket linkbar
Paho MQTT C	EPL/EDL	✅ Ja	Apache-kompatibel, permissiv
Paho MQTT C++	EPL/EDL	✅ Ja	Für C++ Wrapper um C-Bibliothek
ini.h/ini.c	zlib/libpng	✅ Ja	Kompakt, MIT-kompatibel
pthread / POSIX	libc/GPL	✅ Ja	Systembibliotheken, keine Lizenzbindung
🔄 22.3 Lizenzhinweise im Code

Alle Module, die Drittcode enthalten, beinhalten eine kurze Lizenzangabe am Anfang der Datei, z. B.:

// ini.h - Simple INI Parser
// License: zlib/libpng

Für eingebettete Bibliotheken wird empfohlen, auch die zugehörige LICENSE-Datei im Projekt mitzuführen.
📄 22.4 Lizenzkompatibilität und Weitergabe
Du darfst:

    das Projekt frei weitergeben und anpassen

    es in kommerziellen oder privaten Szenarien nutzen

    Binary-Distributionen anbieten

Du solltest:

    Lizenz- und Copyright-Hinweis beibehalten

    bei Forks oder Erweiterungen ebenfalls MIT-kompatibel lizenzieren

🧠 22.5 Lizenzwahl – warum MIT?

Die MIT-Lizenz wurde gewählt, weil sie:

    extrem verbreitet ist (GitHub, Open Source Foundation)

    mit praktisch allen Open-Source-Lizenzen kompatibel ist

    maximale Freiheit für Entwickler und Integratoren bietet


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

