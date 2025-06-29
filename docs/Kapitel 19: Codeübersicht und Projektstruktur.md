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

