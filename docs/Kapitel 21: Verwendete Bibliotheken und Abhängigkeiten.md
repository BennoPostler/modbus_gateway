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

