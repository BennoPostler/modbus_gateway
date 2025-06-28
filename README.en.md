Foreword

🔗 [📄 Deutsch (README.md)](README.md)

This software is in an operation.
But I am still expanding the functionality.
If there is interest if there is pleased to comments, suggestions for improvement reports.
Modbus Gateway – Documentation
1. Introduction and overview

This project provides a robust and flexible gateway that bridges the gap between Modbus TCP, Modbus RTU and MQTT. It is ideally suited for the integration of classic industrial plants (e.g. B. Heat pumps, energy meters, controls) in modern IoT or smart home environments.

Central features:

    Mediation between Modbus TCP - RTU
    Extension to MQTT communication
    Support for coils, discrete inputs, input registers, holding registers
    Multiple reading operations via MQTT
    TLS encryption for MQTT (optional)
    Configurable via .ini-File
    Logfile output and MQTT log
    Full support of modbus diagnostic functions (0x08)
    Statistics and diagnostic data via MQTT

Target groups:

    Developers who want to connect Modbus systems with modern IoT solutions
    System integrators who network existing industrial plants with MQTT-enabled systems
    System administrators looking for a tactile, protocol-compliant and automated solution

Typical applications:

    Connection of heat pumps or PV systems to Smart Home
    Remote diagnosis and remote control via MQTT
    Logging and visualization with Node-RED, InfluxDB, Grafana etc.
    Replacement or extension of existing gateways with TCP/RTU translation

The application runs natively on Linux (Raspberry Pi OS, Debian, Fedora) and can systemdstarted as a service. An MQTT broker (e.g. B. Mosquitto) is required for the full range of functions.

    The software is written entirely in C++ and uses modern libraries such as libmodbus, Paho MQTT C++ and a simple INI configuration logic.

    System overview & architecture

The modbus-gateway is a modular software system for use on devices such as the Raspberry Pi 5 (aarch64), which bridges Modbus TCP and Modbus RTU bidirectionally, provides MQTT interfaces, and implements monitoring, logging and diagnostic functions according to Modbus standard.

2.1. Main functions

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

2.2. Component overview

+-------------+ Modbus TCP +-------------------------------+ | TCP Client(s) | > > Ã------------> | Gateway | | (e.g. B. SCADA) | | (modbus-gateway)| +----------------+ +-----+--------------------+ | | | Serial Line (RTU) | | +----------------+ | Modbus Slave(s) | +-----------------+

+---------------------------+ | MQTT Broker | >--- TLS / TCP +---------------------------+ - | | | +----+ +---++ | Home | | Logging | Logging | |Assistant etc. | | +-------------------+

2.3. Architectural overview

The software is organized into modular components: Component description main() Initialized configuration, MQTT, Modbus, server and controls the process init-config() Reads INI file and fills config structure init-mqtt() Creates MQTT client, sets LWT, TLS, Subscriptions init-modbus() Opens and configures RTU interface accept-clients() Wait for incoming TCP connections (Modbus TCP clients) handle-client() Forwards TCP requests to RTU, processes answers, implements diagnosis MQTTCallback Processes incoming MQTT messages (read/write of registers & coils) publish-stats() Periodically published metrics (e.g. B. Error counter, uptime, throughput) shutdown-gateway() Stops clean all threads, MQTT and RTU connection dump-hex() Optional logging of Modbus telegrams (debugging) 2.4. Data flow overview

Modbus TCP Request handle-client() modbus-send-raw-request() Ã RTU modbus-receive-confirmation() build-tcp-response() TCP response to client

MQTT Topic:

    write/123 - write-register(123)
    read/123 Ã readÃ register(123)
    read-multi/100:3 reads 3 registers from address 100 MQTTCallback::message-arrived() modbus-read- /write-() - RTU Answer via MQTT publish()

2.5. Multiple interfaces

Supports multiple parallel TCP clients

Supports simultaneous MQTT operation

All components run in separate threads, controlled by a central run anti-flush antimic.

    Configuration (modbus-gateway.ini)

The entire functionality of the gateway is controlled by a central .ini file. This is located by default under:

/etc/modbus-gateway.ini

However, it can be passed as an argument when starting the program:

./modbus-gateway /path/to/file.ini

3.1. Basic structure

[serial] /dev/ttyAMA0 baudrate 9600 parity Ã N databits Ã 8 stopbits 1

[tcp] port Ã 502

[modbus] slave id 1

[mqtt] broker Ã tcp://192.168.0.82:1883 client id - modbus-gateway topic-prefix - modbus/ username password keepal 30 lwt offline online online use-tls - 0 ca file cert file key file insecure Ã 0 stats-interval 60

[log] file Ã /var/log/modbusÃ3gateway.log hex dump 0

3.2. Description of all configuration fields [serial] Parameters meaning port serial port (e.g. /dev/ttyUSB0) baudrate baud rate, e.g. B. 9600 parity parity: N, E, or O databits data bits: i. d. R. 8th stopbits Stopbits: 1 or 2 [tcp] Parameters Description port TCP port on which the gateway accepts Modbus TCP requests (default: 502) [modbus] Parameters meaning slave id Default-Slave-ID for MQTT requests [mqtt] Parameters meaning broker MQTT broker URI, e.g. B. tcp://192.168.0.82:1883 client id MQTT Client-ID topic prefix prefix for all topics, e.g. modbus/ username Optional: Username password Optional: Password keepalive keepalive in seconds lwt Payload at Last Will (offline) online Payload at successful Connect (online) activated by useÃ tls 1 - TLS, 0 - unencrypted ca file path to CA file cert file path to the client certificate key file path to private key insecure 1 - Deactivate certificate check stats-interval second interval for MQTT publication of statistical data [log] Parameters meaning file Log File Path hex dump 1 - Choose Modbus packets as hex dump (debug)

    MQTT Topics and Data Structure

The gateway uses a consistent MQTT topic for all read and write operations. All topics begin with the topic prefix defined in the INI file, e.g. modbus/. 4.1. Supported MQTT commands Operation type Topic schema Description Holding Register Read modbus/read/Read individual register Holding Register writing modbus/write/ single registers Read the coil modbus/read-coil/read coil (single bit) Coil writing modbus/write-coil/ Setce Coil (single bit) Discrete Input Read modbus/read discrete/read digital input Input tab Read modbus/read-input/Read analog input register Several Holding Register modbus/read-multi/: Read Multiple Holding Registers Multiple Input Register modbus/read-input-multi/: Read Multiple Input Registers Multiple coils modbus/read-coil-multi/: Read multiple coils Multiple Discrete Inputs modbus/read-discrete-multi/: Read multiple digital inputs Move statistics modbus/stats/reset Resets all statistics counters Get statistics modbus/command/stats Answer appears in modbus/stats 4.2. MQTT examples Example 1: Read single register

Publish:

mosquitto-pub -t modbus/read/100 -n

Answer (Subscribe on modbus/read/100):

431

Example 2: Write a single register

Publish:

mosquitto-pub -t modbus/write/100 -m "123"

Log output:

Modbus register 100 set to 123

Example 3: Read multiple registers

Publish:

mosquitto-pub -t modbus/read-multi/100:4 -n

Answer (Subscribe on modbus/read-multi/100:4):

431,0,65535,22

Example 4: Putting Coil

mosquitto-pub -t modbus/write-coil/25 -m "1"

Example 5: Read Discrete Input

mosquitto-pub -t modbus/read-discrete/12 -n

Answer:

0 0

4.3. Example: statistics query

Query:

mosquitto-pub -t modbus/command/stats -n

Answer:

On-demand statistics: TCP-233, RTU-233, BugÃ2, Last RTU success: 2025-06-26 13:42:17

4.4. Example: Statistics as JSON (automatic)

If stats-interval > 0, the gateway sends regularly:

"totalÃ3tcpÃ3requests": 233, "totalÃ3rtuÃ3requests": 233, "total'errors": 2, "rtu'success": 231, "rtuÃ3failures": 2, "dropped": 0, Echoesplies: 0, "last-success-timestamp": 1724701337

Topic: modbus/stats

    Program code architecture and components

The project follows a structured and modularized architecture, in which the three main interfaces (Modbus TCP, Modbus RTU, MQTT) are clearly separated from each other, but are nevertheless narrowly integrated. 5.1. Main components Component description main() entry point, initialization of configuration, logging, MQTT, Modbus, TCP server config-handler() parser for the INI configuration file init-mqtt() Connection and subscription to MQTT MQTTCallback Processing incoming MQTT commands init-modbus() Setup of the Modbus RTU connection (TTY, Parity, Baudrate etc.) start-tcp-server() Opens TCP socket and waits for incoming connections accept-clients() Accepts new clients and starts threads for processing handle-client() Processing of individual Modbus TCP requests, forwarding to RTU, response publish-stats() Periodic publication of statistical data via MQTT reconnect-mqtt() Logic to restore an interrupted MQTT connection dump-hex() Optional hex dump for debug purposes 5.2. Data flow overview

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

Modbus-Write Modbus-Read (via RTU) (via RTU) | | | +-----------------------------+ | | v Optional: Answer via MQTT

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

5.3. Thread model

Main Thread: Initialisierung, Logging, Signalverarbeitung, Start der Komponenten

MQTT-Thread: Intern durch paho-mqtt verwaltet

publish_stats(): Separater Hintergrundthread zur periodischen Statistikausgabe

Client-Threads: Jeder TCP-Client wird in einem eigenen Thread verarbeitet (Modbus TCP → RTU Weiterleitung)

5.4. Accordance with the protocol

The TCP responses are returned with original temperature ID and reconstructed MBAP header structure. Also local diagnostic functions (e.g. B. Echo test 0x08 00 00) are detected and answered directly. 5.5. Signal treatment

signal(SIGINT, signal-handler); signal(SIGTERM, signal-handler);

Allows clean shutdown at CTRL+C or systemd-stop.

    INI configuration – structure and options

The configuration of the gateway is done via a classic INI file, typically under the path:

/etc/modbus-gateway.ini

This file is parsed at the program start and affects all interfaces (Modbus, MQTT, Logging, etc.). Alternatively, the path can be passed:

./modbus-gateway /path/configuration.ini

6.1. Sections and keys [serial] – Modbus RTU interface Key type Description Example port String Serial Interface /dev/ttyAMA0 baudrate int Baudrate 9600 parity char Parity: N, E, O N databits int Datenbits (mostly 8) 8 stopbits int Stoppbits (1 or 2) 1 [tcp] – Modbus TCP Server Key type Description Example port int TCP port where the server is listening to 502 [modbus] – Modbus-specific options Key type Description Example slave-id int Standard-Slave-ID (overwritten for TCP requests) 1 [mqtt] – MQTT connection Key type Description Example broker String MQTT brokers URI tcp://192.168.0.82:1883 client-id String Client-ID modbus-gateway username String Optional username for authentication mqttuser password String Password for the above user secrets123 keepalive int KeepAlive-Interval in seconds 30 topic-prefix String prefix of all Topics modbus/ lwt String Last Will Payload (offline notification) offline offline online string payload at startup (e.g. B. online) online activate use-tls bool TLS connection 1 ca-file String CA certificate path /etc/ssl/certs/ca.crt cert file String Client certificate (optional) /etc/modbus/cert.pem key file String Private key to the client certificate /etc/modbus/key.pem insecure bool Deactivate certificate check (uncertain) 1 stats-interval int Statistics publication in seconds 60 [log] – Logging Key type Description Example file String log file /var/log/modbus-gateway.log enable hex-dump bool Hexdump (1-yYes, 0-no) 1 6.2. Example configuration

[serial] /dev/ttyAMA0 baudrate 9600 parity Ã N databits Ã 8 stopbits 1

[tcp] port Ã 502

[modbus] slave id 1

[mqtt] broker Ã tcp://192.168.0.82:1883 client id - modbus-gateway username Ã mqttuser password - secret topic-prefix - modbus/ keepal 30 lwt offline online online use-tls - 0 stats-interval 60

[log] file Ã /var/log/modbusÃ3gateway.log hex dump 1

    MQTT integration – Topics, payloads and examples

The gateway uses MQTT for bi-directional communication: control, status messages, statistics and error reports. The MQTT functionality is completely optional and can be deactivated via configuration. 7.1. Topic overview Topic type Description modbus/status LWT gateway connection status (online / offline) modbus/stats Status Statistics as JSON (optional cyclic) modbus/log Log log messages from the gateway modbus/read//
Command Read access to register modbus/write// Command Write access to Register modbus/command/stats command Unlock immediate statistics report 7.2. Register Types Type (in Topic) Modbus type meaning coil Coils 0xxxx: Digital output (1 bit) discrete Discrete Inputs 1xxxx: Digital input (1 bit, read-only) input input tab 3xxxx: Analog input (16 bit, read-only) holding holding register 4xxxx: Analogue source values (16 bit) 7.3. Reading operation via MQTT

Example: Read a holding register 40010.

Topic:

modbus/read/holding/10

Answer:

modbus/response/holding/10 Payload: 4711

7.4. Writing operation via MQTT

Example: Write value 1234 in holding register 40020.

Topic:

modbus/write/holding/20

Payload:

1234

Answer:

modbus/response/holding/20 Payload: OK

7.5. Multiple operations (batch)

Multiple tabs can be written or read in a JSON array. Several Holding Register writing:

Topic:

modbus/write/holding

Payload:

"10": 111, "11": 222, "12": 333

Several coils read:

Topic:

modbus/read/coil

Payload:

[1, 2, 3, 4]

Answer (e.g. in modbus/response/coil)

"1": 1, "2": 0, "3": 1, "4": 1

7.6. Request statistics

Topic:

modbus/command/stats

Payload: (any or empty)

Answer (so-called status message to modbus/stats):

"tcp'requests": 234, "rtu'success": 231, "rtuÃ3failures": 3, "dropped": 0, Echoeplese": 7

7.7. Logging

Topic:

modbus/log

Sample Payload:

[2025-06-26 19:33:00] Ã RTU TX: 01 03 00 00 00 02 65 CB

Activation via:

[log] hex dump 1

    Modbus TCP - RTU – Protocol flow and implementation

The gateway acts as a protocol translator: It receives Modbus TCP requests from clients, converts it to Modbus RTU, redirects it via RS485 and responds with the Modbus TCP packets constructed correctly. 8.1. Procedure of a TCP request

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

8.2. Local diagnostic functions

The function code 0x08 (diagnostic) is partially edited internally in the gateway to enable faster answers: Sub function Description Gateway behavior 0x0000 Loopback test (echo) reply generated locally 0x000A Statistics counter Reset meter to 0 0x000B TCP request counter return generated locally 0x000C Error counter return response generated locally other not detected request to RTU forwarded 8.3. Hex Devug Log

The function dump-hex() creates a log in hexadecimal form:

dump-hex("Ã RTU TX: ", data, len);

Example edition (in modbus/log):

RTU TX: 01 03 00 00 00 02 65 CB RTU RX: 01 03 04 00 0A 00 0B 75 D3

8.4. Connection parameters

Configurable in the INI file:

[serial] /dev/ttyAMA0 baudrate 9600 parity Ã N databits Ã 8 stopbits 1

[modbus] slave id 1

8.5. RTU time behavior

modbus-set-response-timeout(ctx, 1, 0); // 1 second timeout

Reliable response times depend heavily on the RTU slave. Typical roundtrip times: 5 – 50 ms.

    System integration: systemd, autostart & logging

Integration with systemd is provided so that the gateway is automatically executed at system startup and can be managed properly. 9.1. systemd service file

Path: /etc/systemd/system/modbus-gateway.service

[Unit] Description-Modbus TCP to RTU + MQTT Gateway After-network.target

[Service] ExecStart/usr/local/bin/modbus-gateway /etc/modbus-gateway.ini Restart-on-failure User-modbus Group-modbus StandardOutput-append:/var/log/modbus-gateway.log StandardError-append:/var/log/modbus-gateway.err AmbientCapabilities-CAP-NET-BIND-SERVICE LimitNOFILE-65536

[Install] WantedBy-multi-user.target

9.2. Users & rights
Creating users (if not yet available)

sudo useradd -r -s /usr/sbin/nologin modbus
Create directories

sudo mkdir -p /var/log sudo touch /var/log/modbus-gateway.log sudo chown modbus:modbus /var/log/modbus-gateway.log

9.3. Activate & start

sudo systemctl daemon-reexec sudo systemctl daemon-reload sudo systemctl enable modbus-gateway sudo systemctl start modbus-gateway

Status query:

sudo systemctl status modbus-gateway

Log display (live):

journalctl -u modbus-gateway -f

9.4. Logrotate support

Path: /etc/logrotate.d/modbus-gateway

/var/log/modbus-gateway.log rotate 7 daily compress delaycompress missingok notifempty copytruncate

Activation (manually test):

sudo logrotate -f /etc/logrotate.d/modbus-gateway

9.5. Binary & trails

By default, the application is compiled according to:

/usr/local/bin/modbus-gateway

Configuration:

/etc/modbus-gateway.ini

    Compilation, Deployment and Makefile

The application is compiled with g++ under Linux. For use on a Raspberry Pi (e.g. B. Pi 5 with Raspberry Pi OS 64 bit) can be built natively or by cross-compile. 10.1. Makefile for native compilation (e.g. on the Raspberry Pi)

Path: Makefile

TARGET - modbus-gateway SRC - modbus-gateway.cpp ini.c CXXFLAGS -Wall -O2 -stdÃ3c+17 -I/usr/local/include LDFLAGS - L/usr/local/lib -lmodbus -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread

. PHONY: all clean install

all: (TARGET)

( T A R G E T ) : (SRC) ( C X X ) (CXXFLAGS) -o @ (LDFLAGS)

install: ( T A R G E T ) i n s t a l l − m 755 (TARGET) /usr/local/bin/(TARGET) install -m 644 modbus-gateway.service /etc/systemd/system/modbus-gateway.service

clean: rm -f (TARGET)

Build & Install:

make sudo make install

10.2. Cross Compilation for Raspberry Pi OS (aarch64)

toolchain-aarch64.cmake (for CMake use – optional):

SET(CMAKE SYSTEM-NAME Linux) SET(CMAKE SYSTEM-PROCESSOR aarch64)

SET(CMAKE-COMPILER aarch64-linux-gnu-gcc) SET(CMAKE-CXX-COMPILER aarch64-linux-gnu-g++)

SET(CMAKE-FIND-ROOT-PATH /usr/aarch64-linux-gnu)

Alternatively, for manual Cross-Compile:

aarch64-linux-gnu-g++ -std-c++17 -O2
-I/usr/local/include -L/usr/local/lib
-o modbus-gateway modbus-gateway.cpp ini.c
-lmodbus -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread

10.3. Directory structure (recommended)

modbus gateway/ Makefile modbus-gateway.cpp ini.c ini.h modbus gateway.service modbus gateway.ini.example README.md

10.4. Release archive (optional)

Creating an archive:

tar czf modbus-gateway.tar.gz modbus-gateway/

Deploy to GitHub or locally per scp to the target rack brawler pin:

scp modbus.tar.tar.gz piÃ3raspberrypi:/home/pi/

    Examples of MQTT and Modbus TCP Integration

This chapter shows practical examples of using the gateway with MQTT and Modbus TCP. The target group are integrators, which the gateway e.g. with Node-RED, Home Assistant or your own automations. 11.1. MQTT examples Writing a holding register

mosquitto-pub -t modbus/write/100 -m 42

Sets the Modbus Holding register 100 to the value 42. Read a Holding Register

mosquitto-pub -t modbus/read/100 -n

The gateway responds with:

Topic: modbus/read/100 Payload: 42

Writing a coil (switching command)

mosquitto-pub -t modbus/write-coil/12 -m 1

Switches Coil 12 to ON. Reading a Coil

mosquitto-pub -t modbus/read-coil/12 -n

Answer:

Topic: modbus/read-coil/12 Payload: 1

Multiple reading of holding or input registries

mosquitto-pub -t modbus/read-multi/200:4 -n

Read 4 values from address 200.

Answer:

Topic: modbus/read-multi/200:4 Payload: 122,456,789.0

Manually retrieve statistics

mosquitto-pub -t modbus/command/stats -n

Answer:

Topic: modbus/stats Payload: statistics (on-demand): TCP-1234, RTU-1229, error 5, last RTU success: 2025-06-26 12:12:59

Statistics automatically all X seconds

If defined in the INI:

[mqtt] stats-interval 60

Then the gateway sends to:

Topic: modbus/stats Payload: Ã...totalÃ3tcpÃ3requests":1234, "totalÃ3rtuÃ3requests":1229, ...

Reset the statistics counter

mosquitto-pub -t modbus/stats/reset -n

11.2. Modbus TCP examples

Connection e.g. B. using mbpoll:

mbpoll -m tcp -a 1 -t 3 -r 100 -c 2 127.0.0.1

Explanation:

-m tcp: TCP-Modus

-a 1: Slave-ID

-t 3: Holding Register

-r 100: Startadresse 100

-c 2: 2 Register lesen

127.0.0.1: Gateway-Adresse

Answer (if successful):

11.3. Node-RED Integration (example)

Subscribe Node:

Topic: modbus/read/100

Empfängt: Registerwert

Inject Node:

Topic: modbus/write/100

Payload: 123

This allows Node-RED to set and read Modbus values – ideal for automation.

    Error handling and debugging

This section describes how errors in the gateway can be detected, logged and ideally fixed. It helps both developers and integrators to analyze problems faster. 12.1. Overview of sources of error Error source Typical cause solution proposal Modbus RTU Timeout No device responds on request Baud rate, Parity, Slave ID Check MQTT Error Broker not reachable or certificate error MQTT configuration TCP error TCP client interrupts connection Normal at end of connection, if necessary log INI error Syntax error or path not present Path & Check format, /etc/modbus-gateway.ini Permissions Access to /dev/ttyAMA0 or log file denies user to add dialout group 12.2. Logging

All events are...

lokal in die Logdatei (/var/log/modbus_gateway.log) geschrieben

optional zusätzlich via MQTT an den Topic modbus/log gesendet

Example entry:

2025-06-26 20:31:17 - Error reading Modbus register 100: Input/output error

The log file is the first means for error analysis. 12.3. Hex dump Debugging

Activation via INI file:

[log] hex dump 1

Then e.g. such lines in the log/MQTT:

RTU TX: 01 03 00 64 00 01 85 D4 RTU RX: 01 03 02 00 2A B8 44

These provide information about the exact Modbus data traffic. 12.4. Diagnosis via Modbus diagnostic function (0x08)

The TCP client can send special sub-functions: Sub function meaning reaction of the gateway 0x0000 Echo Test Answers directly locally (loopback) 0x000A Reset statistics counters to set counters to 0 totalÃ3tcpÃ3requests 0x000B Returns Number of TCP Requests 2-byte response with current counter reading 0x000C Return number of errors 2-byte response with error counter

These are ideal for external monitoring software. 12.5. MQTT statistics and diagnostic data

All counter values are included in the Topic modbus/stats, e.g. B.:

"totalÃ3tcpÃ3requests": 156, "totalÃ3rtuÃ3requests": 154, "total'errors": 2, "rtu'success": 154, "rtuÃ3failures": 0, "dropped": 0, Echoeplese": 1, "last-success-timestamp": 1719414809

Allows graphical evaluations (e.g. in Grafana or Node-RED Dashboard). 12.6. Increased error tolerance

The gateway does not immediately repel TCP connections for every error, but allows e.g. B.:

mehrere fehlerhafte RTU-Kommunikationen nacheinander

leer empfangene TCP-Pakete → werden ignoriert

Verbindungsabbrüche → sauberer Cleanup im shutdown_gateway()

    Deployment and system integration

This chapter describes how the Modbus gateway on a target system (e.g. B. Raspberry Pi) is installed, automatically started and managed. This includes both manual deployment and systemd integration. 13.1. Prerequisites

Betriebssystem: Empfohlen: Raspberry Pi OS 64 Bit (Debian-basiert)

Benötigte Pakete:

sudo apt install libmodbus-dev libpaho-mqttpp3-dev build-essential

Serial Permission: The user must have access to /dev/ttyAMA0:

sudo usermod -aG dialout $USER

13.2. Compilation

In the project directory:

make

Created:

die ausführbare Datei modbus_gateway

Alternatively manually:

g++ -o modbus-gateway modbus-gateway.cpp ini.c -lmodbus -lpaho-mqttpp3 -lpaho-mqtt3s -lpthread

13.3. systemd service file File: /etc/systemd/system/modbus-gateway.service

[Unit] Description-Modbus TCP-RTU + MQTT Gateway After-network.target

[Service] ExecStart/home/benno/modbus-gateway/modbus-gateway /etc/modbus-gateway.ini Restart-always Userbenno Groupbenno WorkingDirectory/home/benno/modbus-gateway StandardOutput journal StandardError journal

[Install] WantedBy-multi-user.target

13.4. Activating the service

sudo systemctl daemon-reexec sudo systemctl daemon-reload sudo systemctl enable modbus-gateway sudo systemctl start modbus-gateway

13.5. Checking status

systemctl status modbus-gateway journalctl -u modbus-gateway -f

13.6. Deployment folder structure

Recommended:

/home/benno/modbus gateway/ - modbus-gateway - Executable - modbus-gateway.cpp - source code ini.c / ini.h - INI-Parser Makefile README.md systemd/ modbus-gateway.service config/ modbus gateway.ini

13.7. Tips

Logs gehen standardmäßig nach /var/log/modbus_gateway.log

Die Logdatei kann via logrotate rotiert werden

MQTT-Fehler und Wiederverbindungen werden automatisch gehandhabt

    INI configuration format and parameter reference

The central configuration of the gateway is done via an INI file. This makes it possible to set all relevant parameters in a structured manner – from serial settings to MQTT and logging. 14.1. Storage location

Standard path:

/etc/modbus-gateway.ini

Alternatively, a configuration file can be passed at startup:

./modbus-gateway /path/to/file.ini

14.2. Structure of the INI file

[serial] /dev/ttyAMA0 baudrate 9600 parity Ã N databits Ã 8 stopbits 1

[tcp] port Ã 502

[modbus] slave id 1

[mqtt] broker Ã tcp://localhost:1883 client id - modbus-gateway topic-prefix - modbus/ username password keepal 30 lwt offline online online use-tls - 0 ca file cert file key file insecure Ã 0 stats-interval 60

[log] file Ã /var/log/modbusÃ3gateway.log hex dump 0

14.3. Description of the parameters [serial] Parameters Type Description port String Serial interface baudrate number Building rate (e.g. B. 9600, 19200, ...) parity character parity: N, E or O databits Number data bits (typical: 8) stopbits Number stop bits (typical: 1) [tcp] Parameters Type Description port Number TCP port for Modbus TCP servers [modbus] Parameters Type Description slave id Number Default Slave-ID (only for MQTT access) [mqtt] Parameters Type Description broker String MQTT broker URI (e.g. tcp://localhost:1883) client-id String Client ID of the MQTT client topic prefix string prefix for all MQTT topics (e.g. modbus/) username String MQTT username password String MQTT password keepal number Keepalive in seconds lwt String Last-Will-Payload (e.g. "offline") online string online payload at start (e.g. B. "online") activate use-tls 0/1 TLS (1 yes) ca file string path to the CA file cert file String Client certificate file key file String client key file insecure 0/1 TLS certificate check (unsafe!) stats-interval number interval in seconds for publication [log] Parameters Type Description file String Path to Log File hex dump 0/1 Hexdump for Modbus raw data (Debug)

    Examples of MQTT communication

This chapter describes typical MQTT commands and message formats to access Modbus devices or get status information through MQTT. 15.1. Write access: Holding Register or Coils

The gateway processes MQTT messages below the topic:

modbus/write//'slave'id>/
Type meaning coil write access to coil (FC 5, 15) holding write access to Holding Register (FC 6, 16) Example 1: Setting Coil (Slave 1, Coil 100 on true)

Topic:

modbus/write/coil/1/100

Payload (JSON):

true

Example 2: Write Holding Register (Slave 3, Register 40001 - 1234)

Topic:

modbus/write/holding/3/0

Payload:

1234

Note: Holding Register Addresses start at 0! 15.2. Read access: via MQTT-Request

MQTT supports reading requests by publishing:

modbus/read//-slave-id>/[,]

The gateway responds to:

modbus/response//'slave'id>/

Example 3: Holding Register 0 (40001) of Slave 3 Read

Topic (request):

modbus/read/holding/3/0

Answer:

modbus/response/holding/3/0

Payload:

[1234]

Example 4: Read Input Register 30010 to 30014 of Slave 2

Topic (request):

modbus/read/input/2/9.5

Answer:

modbus/response/input/2/9

Payload:

[318, 319, 320, 321, 322]

15.3. Get diagnostic data via MQTT

If set in the . . mqttStatsInterval > 0 file, the gateway periodically sends statistics:

Topic:

modbus/stats

Payload:

"tcp'requests": 123, "rtuÃ3success": 120, "rtuÃ3failures": 3, "dropped": 0, "echoreplies": 5

15.4. Request statistics manually by command

Topic:

modbus/command/stats

Payload: (optional, e.g. B. "now") Answer is done again on modbus/stats.

    Examples of Modbus TCP communication

This chapter shows how an external Modbus TCP client can connect to the gateway to access the RTU devices behind it via the TCP interface. The gateway acts as a proxy between TCP and serial RTU connection. 16.1. Connection setup

The TCP server listens to port 50 by default, unless otherwise specified in the modbus-gateway.ini.

A typical Modbus TCP client connects as follows:

Host: IP-Adresse des Raspberry Pi

Port: 502

Slave ID: 1…247 (je nach Zielgerät)

Funktion: Standard-Modbus-Funktionen (FC1, FC2, FC3, FC4, FC5, FC6, FC15, FC16)

16.2. Example with mbpoll (Modbus-TCP)

Install mbpoll (z. under Linux via package manager) and do the following: Reading a Holding Register (Register 0) by Slave 1:

mbpoll -m tcp -a 1 -r 0 -c 1 -t 4:int -1 192.168.0.100

Explanation:

-m tcp → Modbus TCP

-a 1 → Slave-Adresse 1

-r 0 → Registeradresse 0

-c 1 → 1 Register lesen

-t 4:int → Format: 16-bit Holding Register

-1 → Port 502

192.168.0.100 → Gateway-IP

16.3. Writing to a Holding Register

mbpoll -m tcp -a 1 -r 0 -t 4:int -1 192.168.0.100 --write 1234

Writes the value 1234 in register 0 of Slave 1. 16.4. Other clients

Also other clients like:

QModMaster (Windows/Qt)

Node-RED mit Modbus Nodes

Python (pymodbus)

CODESYS oder TIA Portal

can work directly with the gateway without special adjustments, as long as they support Modbus TCP. 17. Logging and Diagnosis 17.1. File logging

All events, errors, status changes are logged into the file:

/var/log/modbus-gateway.log

Configurable via:

[log] file Ã /var/log/modbusÃ3gateway.log

17.2. MQTT logging

Each log line will be additionally on the topic:

modbus/log

published – provided that MQTT is connected. 17.3. Activate Hex-Dump

To analyze RTU communication, a hex dump can be activated:

[log] hex dump 1

Example edition:

RTU TX: 01 03 00 6B 00 03 76 87 RTU RX: 01 03 06 02 2B 00 00 64 39 80

    systemd integration, autostart and user management

To automatically load the gateway at system startup and operate it as a service in the background, a systemd service unit can be used. 18.1. Example modbus-gateway.service

Save as: /etc/systemd/system/modbus gateway.service

[Unit] Description-Modbus TCP -> RTU + MQTT Gateway After-network.target

[Service] ExecStart/usr/local/bin/modbus-gateway /etc/modbus-gateway.ini Restart-always RestartSec-5 User-modbus Group-modbus StandardOutput-syslog StandardError syslog SyslogIdentifier-modbus-gateway

[Install] WantedBy-multi-user.target

18.2. User modbus attach (recommended)

So that the gateway does not have to run as root:

sudo useradd --system --no-create-home --group modbus modbus sudo chown modbus:modbus /var/log/modbus-gateway.log

Optional: Allow Modbus users to access /dev/ttyAMA0:

sudo usermod -aG dialout modbus

18.3. Activate and start service

sudo systemctl daemon-reexec sudo systemctl daemon-reload sudo systemctl enable modbus-gateway.service sudo systemctl start modbus-gateway.service

18.4. Checking status

sudo systemctl status modbus-gateway

18.5. Restart/Stop

sudo systemctl restart modbus-gateway sudo systemctl stop modbus-gateway

    Security and TLS 19.1. Activate TLS for MQTT

The connection to the MQTT broker can be TLS-encrypted. Set the following parameters in the INI file:

[mqtt] use-tls - 1 ca-file /etc/ssl/certs/ca.crt cert file /etc/ssl/certs/client.crt key file /etc/ssl/private/client.key insecure Ã 0

use_tls = 1 aktiviert TLS

insecure = 1 deaktiviert die Serverzertifikatsprüfung (nicht empfohlen!)

19.2. Testing MQTT over TLS

If your broker e.g. mqtts://broker.local:8883 is available:

broker Ã ssl://broker.local:8883

Make sure the port and the protocol (ssl:// instead of tcp://) are correct. 20. Statistics and monitoring 20.1. Automatic statistics (periodic)

All mqttStatsInterval seconds will be published on the following topic:

modbus/stats

Example content:

"totalÃ3tcpÃ3requests": 1432, "totalÃ3rtuÃ3requests": 1410, "total-errors": 22, "rtuÃ3success": 1405, "rtuÃ3failures": 5, "dropped": 2, "echoreplies": 18, "last-success-timestamp": 1719438511

20.2. Request statistics immediately

MQTT send to:

modbus/command/stats

Answer is done to:

modbus/stats

20.3. Reset statistics

Send an empty MQTT message to:

modbus/stats/reset

    Error handling and reconnection 21.1. Modbus RTU error handling

All errors when sending or receiving on the serial port are:

mit modbus_strerror(errno) ins Log geschrieben,

im MQTT-Log publiziert (sofern Verbindung besteht),

und statistisch gezählt (total_errors, rtu_failures).

Example log entry for communication errors:

2025-06-26 10:15:27 - Error receiving RTU: Connection timed out

21.2. MQTT connection monitoring

The MQTT connection is automatically monitored:

Bei Verbindungsverlust wird ein Hintergrundthread gestartet, der periodisch einen Reconnect versucht.

Während des Reconnects wird kein weiterer Reconnect-Thread gestartet.

Erfolgreicher Reconnect führt zum Neuaufbau aller Subscriptions und Publikation des Online-Status.

21.3. Intercepting signals

The following signals are handled:

SIGINT (z. B. Ctrl+C)

SIGTERM (z. B. via kill)

These lead to a clean shutdown:

Alle Client-Threads werden beendet.

MQTT wird abgemeldet und der Offline-Status gesendet.

Modbus wird sauber geschlossen.

Log-Datei wird beendet.

21.4. TCP contours

Each connected client gets a timeout:

timeout.tv sec 60; setsockopt(client-so-sock, SOL-SOCKET, SO-RCVTIMEO, &timeout, sizeof(timeout)); setsockopt(client-sock, SOL-SOCKET, SO-SNDTIMEO, &timeout, sizeof(timeout));

Connections without activity are therefore automatically disconnected after 60 seconds. 22. Logging and Diagnosis 22.1. Log file

Standard: /var/log/modbus-gateway.log Path configurable via INI:

[log] file Ã /var/log/modbusÃ3gateway.log

Format:

YYYY-MM-DD HH:MM:SS -

22.2. MQTT log

If MQTT is connected, all log messages are also sent to the following topic:

modbus/log

This allows e.g. a connection to:

Node-RED (Debug) InfluxDB (with log level analysis) Prometheus (via Exporter)

22.3. Hex dumps

If enabled:

[log] hex dump 1

Modbus requests and responses are output in the log hexadecimal.

Example:

RTU TX: 01 03 00 00 00 02 C4 0B RTU RX: 01 03 04 00 0A 00 14 FA 33

    Project structure and files 23.1. Important source files File Description modbus-gateway.cpp Main application with Modbus/MQTT logic ini.c/h INI-fileparser Makefile compilation and installation modbus-gateway.service systemd service definition 23.2. Configuration file

Example:

/etc/modbus-gateway.ini

Structure see chapter 3.3 INI-file structure. 23.3. Compilation

make

Installed by /usr/local/bin/modbus-gateway 23.4. Logs

/var/log/modbus-gateway.log

23.5. System service

/etc/systemd/system/modbus gateway.service

23.6. MQTT Topics Overview (compact) Purpose Topic Example Status modbus/status online / offline Logging modbus/log log entries modbus/stats statistics JSON with counter values Get statistics modbus/command/stats empty Reset statistics modbus/stats/reset empty Holding Read register modbus/read/1234 Answer: 42 Holding Register writing modbus/write/1234 Message: 42 Read the coil modbus/read-coil/1234 Answer: 1 Coil write modbus/write-coil/1234 Message: 0 Input Register Read modbus/read-input/1234 Answer: 1042 Discrete Input Read modbus/read-discrete/1234 Answer: 0 Multiple registers read modbus/read-multi/100:4 Answer: 123,456,789,1011 Multiple coils read modbus/read-coil-multi/0:8 Answer: 1.1,0,0,1.0,1,1 23.7. Licenses / dependencies

libmodbus – https://libmodbus.org/

Eclipse Paho MQTT C++ – https://www.eclipse.org/paho/

inih (INI-Parser) – https://github.com/benhoyt/inih

All third-party components are available under liberal open source licenses.
