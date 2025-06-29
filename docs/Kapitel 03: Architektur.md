---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

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

