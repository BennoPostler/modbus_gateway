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

