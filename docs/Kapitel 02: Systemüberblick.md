---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

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

