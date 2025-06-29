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

