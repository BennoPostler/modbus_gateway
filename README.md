📘 Abschnitt 1: Projektübersicht
Modbus TCP/RTU + MQTT Gateway

Dieses Projekt implementiert ein robustes Gateway, das Modbus TCP-Anfragen entgegennimmt und über eine serielle Modbus RTU-Verbindung weiterleitet. Zusätzlich bietet es eine vollständige MQTT-Schnittstelle zur Kommunikation mit Modbus-RTU-Geräten, inklusive Unterstützung für:

    Holding Register (Lesen/Schreiben)

    Input Register (Lesen)

    Coils (Lesen/Schreiben)

    Discrete Inputs (Lesen)

    Lesen einzelner oder mehrerer Werte über MQTT

    Volle MQTT-Logging-Integration

Das Gateway eignet sich ideal als Bindeglied zwischen industriellen Modbus-Geräten, Heimautomatisierungssystemen (z. B. Node-RED oder Home Assistant) und übergeordneten Steuerungen.
🚀 Funktionen

    Modbus-TCP-zu-RTU-Brücke
    Weiterleitung von Modbus-TCP-Anfragen an einen RTU-Slave über eine serielle Schnittstelle.

    MQTT-Schnittstelle
    Lese- und Schreibzugriff auf Modbus-Geräte via MQTT Topics.

    Mehrfache Registerabfragen
    Lesung mehrerer Modbus-Register oder Bits in einem Schritt über MQTT.

    Rekonfiguration per INI-Datei
    Alle Parameter (seriell, MQTT, Modbus) sind über /etc/modbus_gateway.ini konfigurierbar.

    Stabiles Logging
    Ausgabe in eine Logdatei und optional via MQTT-Topic.

📡 Abschnitt 2: MQTT-Schnittstelle & Topic-Referenz

Das Gateway nutzt MQTT als zweite Hauptschnittstelle neben Modbus-TCP. Über definierte Topic-Strukturen können Modbus-Daten gelesen oder geschrieben werden – sowohl einzeln als auch blockweise.
🔧 Allgemeine Struktur

Alle MQTT-Topics sind relativ zum Prefix modbus/ (Standard, konfigurierbar über mqtt.topic_prefix in der INI-Datei).

[mqtt]
topic_prefix = modbus/

📥 Schreiben
🧾 Holding Register (16-bit)

Topic:

modbus/write/<Adresse>

Payload:
Ganzzahl (dezimal), z. B. 1234

Beispiel:

modbus/write/100 → 1234

🔘 Coils (1-bit)

Topic:

modbus/write_coil/<Adresse>

Payload:
0 oder 1

Beispiel:

modbus/write_coil/10 → 1

📤 Lesen (einzelne Werte)
📖 Holding Register

Topic (Subscribe):

modbus/read/<Adresse>

Antwort-Topic (Publish):
Selbes Topic mit dem gelesenen Wert als Payload.
📖 Input Register

modbus/read_input/<Adresse>

📖 Coil

modbus/read_coil/<Adresse>

📖 Discrete Input

modbus/read_discrete/<Adresse>

📤 Lesen (mehrere Werte)

Der Doppelpunkt : trennt Startadresse und Anzahl der Werte.
📚 Holding Register

modbus/read_multi/<Startadresse>:<Anzahl>

Beispiel:

modbus/read_multi/100:3 → "111,222,333"

📚 Input Register

modbus/read_input_multi/<Startadresse>:<Anzahl>

📚 Coils

modbus/read_coil_multi/<Startadresse>:<Anzahl>

📚 Discrete Inputs

modbus/read_discrete_multi/<Startadresse>:<Anzahl>

🪵 Logging (nur Ausgabe)

Topic:

modbus/log

Beispiel-Inhalt:

2025-06-24 20:03:17 - Modbus-Register 100 gesetzt auf 1234

🧩 Abschnitt 3: Aufbau der Softwarekomponenten und Modulstruktur

Das Projekt ist vollständig in einer einzigen Datei (modbus_gateway.cpp) implementiert, mit klar getrennten Funktionsbereichen. Alle zentralen Funktionen sind modular aufgebaut, sodass sie leicht erweiterbar oder in eigene Module auslagerbar wären.
🔄 Hauptkomponenten
Komponente	Zweck
main()	Einstiegspunkt, Initialisierung und Start der Haupt-Threads
init_config()	Einlesen der .ini-Konfigurationsdatei mittels ini.h
init_mqtt()	Aufbau der MQTT-Verbindung, Abonnieren von Topics
reconnect_mqtt()	Wiederverbindungs-Logik bei MQTT-Verlust
init_modbus()	Initialisierung der seriellen Modbus-RTU-Verbindung
start_tcp_server()	Startet TCP-Server auf Port 502
accept_clients()	Nimmt Modbus-TCP-Clients entgegen und startet Threads
handle_client()	Übersetzt Modbus-TCP-Anfragen zu Modbus-RTU
log()	Lognachrichten schreiben in Datei und (wenn verbunden) per MQTT
shutdown_gateway()	Aufräumlogik bei Programmende (SIGINT, SIGTERM)
🧠 Klassenübersicht
Config

struct Config { ... };

    Enthält alle konfigurierbaren Parameter (MQTT, Modbus, Logging usw.)

    Wird durch init_config() aus INI-Datei befüllt

    Zentraler Punkt zur Parametrisierung

MQTTCallback

class MQTTCallback : public virtual mqtt::callback

    Implementiert connection_lost() und message_arrived()

    Verarbeitet MQTT-Nachrichten:

        schreibt/liest Register, Coils, Inputs

        unterstützt Einzel- & Mehrfachlese (mit : Syntax)

    Triggert automatische Reconnect-Logik im Fehlerfall

🔁 Thread-Modell
Thread/Funktion	Aufgabe
main()	Startpunkt, initialisiert alles
accept_clients()	Lauscht auf TCP-Port und startet …
handle_client()	… je einen neuen Thread pro Client
reconnect_mqtt()	ggf. als separater Thread im Fehlerfall
modbus_receive()	blockierend, verarbeitet Modbus-TCP
⚙️ Externe Bibliotheken
Bibliothek	Verwendung
libmodbus	Kommunikation via Modbus RTU/TCP
paho.mqtt.cpp	MQTT-Client (async)
ini.h (benhoyt/inih)	Konfigurationsdatei (INI-Parser)
<thread>, <mutex>	Multithreading, Thread-Sicherheit
<atomic>	Threadsichere Flags, z. B. run, mqttConnecting


🛠️ Abschnitt 4: Konfigurationsdatei (modbus_gateway.ini)

Das Gateway wird vollständig über eine .ini-Datei konfiguriert, die beim Start angegeben wird oder standardmäßig unter /etc/modbus_gateway.ini gesucht wird.
🔑 Aufbau der .ini-Datei

Die Datei ist in folgende Sektionen unterteilt:
📦 [serial]

Konfiguration der seriellen RTU-Verbindung zum Modbus-Slave (z. B. Wärmepumpe):
Schlüssel	Beschreibung	Beispiel
port	Serieller Port	/dev/ttyAMA0
baudrate	Baudrate	9600
parity	Parität (N/E/O)	N
databits	Anzahl Datenbits	8
stopbits	Anzahl Stopbits	1
🌐 [tcp]

Modbus-TCP-Server, der auf eingehende Verbindungen wartet:
Schlüssel	Beschreibung	Beispiel
port	TCP-Port (Standard: 502)	502
☁️ [mqtt]

MQTT-Kommunikation (z. B. mit Home Assistant oder Node-RED):
Schlüssel	Beschreibung	Beispiel
broker	Adresse des MQTT-Brokers	tcp://192.168.0.82:1883
client_id	MQTT-Client-ID	modbus_gateway
topic_prefix	Präfix für alle Topics	modbus/
username	(Optional) Benutzername	mqttuser
password	(Optional) Passwort	mqttpass
keepalive	Keep-Alive in Sekunden	30
lwt	Last Will Payload bei Verbindungsverlust	offline
online	Payload nach erfolgreicher Verbindung	online
📜 [log]

Logging-Konfiguration:
Schlüssel	Beschreibung	Beispiel
file	Pfad zur Logdatei	/var/log/modbus_gateway.log
🧱 [modbus]

Modbus-spezifische Parameter:
Schlüssel	Beschreibung	Beispiel
slave_id	Modbus-Slave-Adresse (Unit ID)	1
🧪 Beispielkonfiguration (modbus_gateway.ini)

[serial]
port = /dev/ttyAMA0
baudrate = 9600
parity = N
databits = 8
stopbits = 1

[tcp]
port = 502

[mqtt]
broker = tcp://192.168.0.82:1883
client_id = modbus_gateway
topic_prefix = modbus/
username = mqttuser
password = mqttpass
keepalive = 30
lwt = offline
online = online

[log]
file = /var/log/modbus_gateway.log

[modbus]
slave_id = 1

📡 Abschnitt 5: Unterstützte MQTT-Topics & Payloads

Das Modbus-Gateway verarbeitet MQTT-Nachrichten zur Steuerung und Abfrage von Modbus-Geräten. Dabei kommen verschiedene Topics und Payload-Formate zum Einsatz.
🔃 Übersicht der MQTT-Themenstruktur
Aktion	Topic-Format (Suffix)	Beschreibung
📥 Register schreiben	write/<address>	Holding Register (einzeln) schreiben
📥 Coil schreiben	write_coil/<address>	Coil (einzeln) setzen/clearen
📤 Holding lesen	read/<address>	Holding Register (einzeln) lesen
📤 Input Register lesen	read_input/<address>	Input Register (einzeln) lesen
📤 Discrete Input lesen	read_discrete/<address>	Digitaler Eingang (einzeln) lesen
📤 Coil lesen	read_coil/<address>	Coil (einzeln) lesen
📤 Holding mehrfach	read_multi/<start>:<count>	Mehrere Holding Register lesen
📤 Input Register mehrfach	read_input_multi/<start>:<count>	Mehrere Input Register lesen
📤 Coils mehrfach	read_coil_multi/<start>:<count>	Mehrere Coils lesen
📤 Discrete Inputs mehrfach	read_discrete_multi/<start>:<count>	Mehrere Discrete Inputs lesen
📋 Logging	log	Automatisches Logging über MQTT
📶 Status	status	Online-/Offline-Zustand des Gateways
🧾 Beispiele für MQTT-Payloads
🔧 Schreiben

Topic:     modbus/write/100
Payload:   42
⇒ Schreibt 42 in das Holding Register 100

Topic:     modbus/write_coil/200
Payload:   1
⇒ Setzt Coil 200 auf TRUE

🔍 Einfaches Lesen

Topic:     modbus/read/100
⇒ Gateway liest Register 100 und veröffentlicht:
modbus/read/100 → 42

Topic:     modbus/read_input/10
⇒ modbus/read_input/10 → 305

🔍 Mehrfachlesen

Topic:     modbus/read_multi/100:3
⇒ Antwort: modbus/read_multi/100:3 → 10,20,30

Topic:     modbus/read_input_multi/10:2
⇒ Antwort: modbus/read_input_multi/10:2 → 123,456

Topic:     modbus/read_coil_multi/5:4
⇒ Antwort: modbus/read_coil_multi/5:4 → 1,0,1,1

⚠️ Validierungsregeln

    Adressen müssen gültige Integer ≥ 0 sein.

    count beim Mehrfachlesen: 1–125 (Modbus-Grenze).

    Wenn das Topic kein : enthält, wird eine Fehlermeldung erzeugt.

    Bei MQTT-Verbindungsproblemen wird geloggt, aber nicht abgestürzt.

🔁 Abschnitt 6: Interner MQTT-Reconnect und Fehlerbehandlung

Das Gateway besitzt eine robuste Reconnect-Logik für MQTT, um eine möglichst durchgehende Verbindung zum Broker sicherzustellen. Zudem wird jeder relevante Fehler (MQTT oder Modbus) intern geloggt und optional über MQTT publiziert.
🔌 MQTT-Verbindungsaufbau

Beim Start wird die Funktion init_mqtt() aufgerufen:

bool init_mqtt();

    Erstellt den MQTT-Client (mqtt::async_client)

    Setzt Callback-Handler (mqttCb)

    Verbindet zum Broker mit gesetzten Optionen:

        keepalive, clean_session, will, Benutzername/Passwort

    Veröffentlicht Online-Status (modbus/status → online)

    Abonniert alle relevanten Topics:

        write/#, read/#, read_coil/#, usw.

🔄 Automatischer Reconnect

Im Falle eines Verbindungsverlusts wird ein Reconnect initiiert:

void connection_lost(const std::string& cause) override;

    Loggt den Verbindungsverlust

    Startet einen neuen Thread mit reconnect_mqtt()

    Verhindert Mehrfach-Reconnects durch std::atomic<bool> reconnecting

    Verwendet die gleichen Verbindungsparameter wie init_mqtt()

    Nach erfolgreicher Verbindung:

        Status → online

        Re-subscriben aller Topics

        Setzen des Callbacks erneut erforderlich

🔒 Schutz vor parallelen Verbindungsversuchen

std::atomic<bool> mqttConnecting = false;

if (mqttConnecting.exchange(true)) return;

    Verhindert doppelte gleichzeitige Verbindungsversuche

    Sicher bei gleichzeitigen Disconnects durch andere Threads

🪵 Logging

Alle Fehler oder Verbindungsprobleme werden zentral geloggt:

void log(const std::string& msg);

    Schreibt die Nachricht ins Logfile (/var/log/modbus_gateway.log)

    Optionales MQTT-Logging auf modbus/log

    Zeitstempel im Format: YYYY-MM-DD HH:MM:SS

🔁 Ablauf bei MQTT-Ausfall

    connection_lost() wird durch Paho-MQTT aufgerufen

    Neuer Thread startet reconnect_mqtt()

    Alle Subscriptions werden erneut eingerichtet

    Status-Topic aktualisiert sich auf „online“

    Falls Reconnect fehlschlägt, wird nach 5 Sekunden erneut versucht

✅ Der Reconnect-Mechanismus ist vollautomatisch und stabilisiert das System auch bei temporären MQTT-Ausfällen, ohne manuelles Eingreifen.

🛠️ Abschnitt 7: Modbus-TCP zu Modbus-RTU Weiterleitung (TCP-Proxy)

Das Gateway fungiert als Modbus-TCP zu Modbus-RTU Proxy: Es nimmt TCP-Verbindungen entgegen, dekodiert die Anfrage, leitet sie per Modbus-RTU weiter und sendet die Antwort zurück.
🔧 Aufbau der Weiterleitung

Sobald ein TCP-Client sich verbindet, wird ein neuer Thread erzeugt:

void accept_clients();  // Wartet auf neue Verbindungen

→ ruft handle_client(client_sock, rtu_ctx) auf
🧵 Funktionsweise von handle_client

void handle_client(int client_sock, modbus_t* rtu_ctx);

Ablauf im Thread:

    Initialisierung

        Ein Modbus-TCP-Context (modbus_new_tcp()) wird genutzt, um die TCP-Daten zu dekodieren.

        Socket des Clients wird diesem Kontext zugewiesen: modbus_set_socket(tcp_ctx, client_sock)

    Empfang der Anfrage

int query_len = modbus_receive(tcp_ctx, query);

    Die MBAP-Header werden durch modbus_receive dekodiert.

    query enthält anschließend das Modbus-PDU.

Weiterleitung an RTU

modbus_send_raw_request(rtu_ctx, query, query_len);

    Die Anfrage wird 1:1 per Modbus-RTU über die serielle Schnittstelle gesendet.

Empfang der RTU-Antwort

int res_len = modbus_receive_confirmation(rtu_ctx, response);

Rücksendung an den TCP-Client

    send(client_sock, response, res_len, 0);

    Fehlerbehandlung

        Bei Verbindungsfehlern oder Zeitüberschreitungen wird der Thread beendet.

📌 Besonderheiten

    TCP-Client bleibt während der Transaktion blockierend verbunden.

    Keine parallelen Anfragen pro Client, aber parallele Clients werden durch std::thread unterstützt.

    Modbus-RTU ist sequenziell, aber durch das Kontextkonzept sauber isolierbar.

📁 Beispiel: Modbus TCP zu RTU
Beispiel-Anfrage (Client via TCP):

    Host sendet Read Holding Register an Adresse 40001.

Intern:

    MBAP-Header entfernt → RTU-Request generiert

    Request per RS485 (z. B. an NIBE-Wärmepumpe) gesendet

    Antwort per RS485 empfangen

    Antwort wird als TCP zurück an den ursprünglichen Client gesendet

✅ Ergebnis: Das System funktioniert als echter Modbus-Gateway – transparent für den Client.


🔄 Abschnitt 8: MQTT-Schnittstelle zum Lesen & Schreiben von Modbus-Daten

Das Gateway bietet eine flexible MQTT-API, über die Modbus-Register (Holding, Input), Coils und Discrete Inputs gelesen und geschrieben werden können. Alle Aktionen erfolgen bidirektional (MQTT → RTU & RTU → MQTT).
🧠 Allgemeines Prinzip

    Lesen (read) → per MQTT-Topic triggern → Antwort auf gleichem Topic

    Schreiben (write) → per MQTT-Topic + Payload → Aktion auf RTU-Bus

Beispiel:

modbus/read/5         → Antwort: 4711
modbus/write/10       → Payload: 23
modbus/read_multi/0:3 → Antwort: 12,13,14

📚 Unterstützte Registertypen
Typ	Modbus-Funktion	MQTT-Topic Prefix
Holding Register	0x03 Read / 0x06 Write	read/ / write/
Input Register	0x04	read_input/
Coil (Bit)	0x01 / 0x05	read_coil/ / write_coil/
Discrete Input	0x02	read_discrete/
Mehrfach-Lesen (bulk)	0x01–0x04	*_multi/ (z. B. read_multi/0:5)
📤 Beispiel: Lesen über MQTT
Einzelnes Register:

🔁 Senden → Topic: modbus/read/10
➡️ Antwort → modbus/read/10 mit Payload: 1234

Mehrere Register:

🔁 Senden → Topic: modbus/read_multi/10:3
➡️ Antwort → modbus/read_multi/10:3 mit Payload: 123,456,789

    Die Antwort erfolgt auf exakt demselben Topic – kompatibel mit MQTT-Subskriptionen.

✍️ Beispiel: Schreiben über MQTT
Holding Register:

🔁 Topic: modbus/write/11
📦 Payload: 42

Coil (Bit):

🔁 Topic: modbus/write_coil/4
📦 Payload: 1

🧩 Verarbeitung in MQTTCallback::message_arrived()

    Prefix-Erkennung mit rfind() (z. B. read/, write/, read_input/)

    Adresse extrahieren (std::stoi aus Topic)

    Payload interpretieren (bei write)

    Lesen / Schreiben über libmodbus

    Antwort als Publish senden

🧪 Gültigkeitsprüfungen

    Bei Mehrfachlese:

        Syntax: addr:count

        Prüfung auf addr >= 0 && count > 0 && count <= 125

    Bei Fehlern:

        Logging über log() mit Fehlermeldung (z. B. modbus_strerror(errno))

🧠 Architekturhinweis

    Die MQTT-Schnittstelle läuft unabhängig vom TCP-Gateway (zwei Wege zum Zugriff).

    Die Verarbeitung ist asynchron (libpaho MQTT + message_arrived Callback).

✅ Ergebnis: Volle MQTT-Integration für Automatisierung via Home Assistant, Node-RED etc.


📦 Abschnitt 9: Unterstützung für Mehrfach-Lesen über MQTT (*_multi/ Topics)

Neben dem Lesen einzelner Modbus-Register bietet das Gateway auch die Möglichkeit, mehrere Werte auf einmal zu lesen – ideal für:

    Effizientere Kommunikation

    Reduzierte MQTT-Last

    Kompatibilität mit Visualisierungstools

🔧 Funktionsweise

Die Mehrfachabfrage erfolgt über spezielle MQTT-Topics im Format:

modbus/<typ>_multi/<startadresse>:<anzahl>

Beispiel:

modbus/read_multi/100:4 → liest Holding Register 100 bis 103

🧪 Gültigkeitsprüfung

Vor dem Ausführen der Anfrage wird geprüft:

    : muss enthalten sein

    Startadresse und Anzahl müssen gültige Ganzzahlen sein

    addr ≥ 0, count > 0, count ≤ 125

Ungültige Anfragen führen zu einem Logeintrag, aber keinem Absturz.
💬 Antwort

Die Antwort wird auf demselben Topic gesendet – mit CSV-Payload:

Topic: modbus/read_multi/100:4
Payload: 111,112,113,114

🧱 Unterstützte Mehrfach-Typen
Modbus-Typ	MQTT-Topic-Prefix	libmodbus-Funktion
Holding Register	read_multi/	modbus_read_registers()
Input Register	read_input_multi/	modbus_read_input_registers()
Coils	read_coil_multi/	modbus_read_bits()
Discrete Inputs	read_discrete_multi/	modbus_read_input_bits()
🧠 Implementierung (Code-Auszug)

In MQTTCallback::message_arrived() definiert:

auto handle_read_multi = [&](const std::string& prefix, auto read_fn) {
    if (topic.rfind(prefix, 0) == 0) {
        std::string addr_str = topic.substr(prefix.length());
        size_t colon = addr_str.find(':');
        if (colon == std::string::npos) {
            log("Fehlerhaftes Topic (kein ':' gefunden) bei Mehrfachlese-Anfrage: " + topic);
            return;
        }

        int addr = std::stoi(addr_str.substr(0, colon));
        int count = std::stoi(addr_str.substr(colon + 1));
        if (addr < 0 || count <= 0 || count > 125) {
            log("Ungültige Adress-/Anzahlparameter: addr=" + std::to_string(addr) + ", count=" + std::to_string(count));
            return;
        }

        std::vector<uint16_t> values(count);
        int rc = read_fn(rtu_ctx, addr, count, values.data());
        if (rc == -1) {
            log("Fehler beim Lesen von " + topic + ": " + modbus_strerror(errno));
        } else {
            std::ostringstream oss;
            for (int i = 0; i < count; ++i) {
                if (i > 0) oss << ",";
                oss << values[i];
            }
            mqttClient->publish(topic, oss.str().c_str(), MQTT_QOS, false);
        }
    }
};

Verwendung:

handle_read_multi(config.mqttTopicPrefix + "read_multi/", modbus_read_registers);
handle_read_multi(config.mqttTopicPrefix + "read_input_multi/", modbus_read_input_registers);
handle_read_multi(config.mqttTopicPrefix + "read_coil_multi/", modbus_read_bits);
handle_read_multi(config.mqttTopicPrefix + "read_discrete_multi/", modbus_read_input_bits);

✅ Ergebnis: MQTT-basierte Bulk-Abfragen für alle wichtigen Modbus-Typen – effizient, stabil und flexibel.


🧵 Abschnitt 10: Modbus-TCP zu RTU Gateway (TCP-Forwarding)

Das Gateway übernimmt die Rolle eines Brückenservers, der eingehende Modbus-TCP-Anfragen 1:1 an ein RTU-Zielgerät weiterleitet und die Antwort zurückgibt.
🔁 Ablaufdiagramm

Client (Modbus TCP)
    |
    | Modbus TCP (Port 502)
    v
Modbus Gateway (Raspberry Pi)
    |
    | Weiterleitung als Modbus RTU (über RS485)
    v
Zielgerät (z. B. NIBE, Wechselrichter)

⚙️ Ablauf im Code

    TCP-Server starten:

start_tcp_server();

→ Socket an Port 502 wird geöffnet (AF_INET, SOCK_STREAM)

    Clients akzeptieren:

accept_clients();

→ jeder neue Client wird in eigenem Thread mit handle_client(...) verarbeitet

    Verbindung pro Client behandeln:

void handle_client(int client_sock, modbus_t* rtu_ctx);

    Modbus-TCP-Anfrage empfangen (modbus_receive)

    Anfrage als Raw-Request an RTU senden (modbus_send_raw_request)

    Antwort vom RTU empfangen (modbus_receive_confirmation)

    Antwort an TCP-Client senden (send())

    Verbindung schließen bei Fehler oder Disconnect

📜 Beispiel: Weiterleitung

int query_len = modbus_receive(tcp_ctx, query);
int sent = modbus_send_raw_request(rtu_ctx, query, query_len);
int res_len = modbus_receive_confirmation(rtu_ctx, response);
send(client_sock, response, res_len, 0);

🧵 Threadsicherheit

Jeder TCP-Client wird in einem separaten Thread behandelt:

client_threads.emplace_back(handle_client, client_sock, rtu_ctx);

🧠 Hinweis:
rtu_ctx wird gemeinsam genutzt, ist aber bei libmodbus nicht thread-safe → hier könnte ein Mutex ergänzt werden, wenn paralleler Zugriff auf RTU-Verbindung nötig ist.
🧹 Aufräumen bei Shutdown

for (auto& t : client_threads) if (t.joinable()) t.join();

🚧 Hinweise für Produktion
Punkt	Empfehlung
RTU-Mutex (thread-safe)	optional – wenn paralleler Zugriff erforderlich
Timeout	bereits gesetzt (modbus_set_response_timeout())
Max. Clients	aktuell unbeschränkt, evtl. TCP_BACKLOG anpassen
Port 502	ggf. sudo nötig oder in iptables umleiten

✅ Ergebnis: Das System kann Modbus-TCP-Anfragen vollständig als RTU umsetzen – ideal für klassische HMI/SCADA-Systeme.


🧾 Abschnitt 11: Konfigurationsdatei (modbus_gateway.ini)

Das System erlaubt eine vollständige Konfiguration über eine externe INI-Datei, was Anpassungen ohne Neukompilieren ermöglicht.
📁 Pfad und Aufruf

Standardpfad:

/etc/modbus_gateway.ini

Überschreibbar via Startparameter:

./modbus_gateway /pfad/zur/config.ini

🧩 INI-Parsing mit ini.h

Verwendete Bibliothek: inih
Einfache Callback-basierte INI-Verarbeitung.

Parser-Funktion:

int config_handler(void* user, const char* section, const char* name, const char* value);

→ Wird für jedes [section] key=value Triplet aufgerufen.

Zuweisung über:

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

📚 Konfigurierbare Parameter
Sektion	Parameter	Beschreibung
[serial]	port	Serielle Schnittstelle (z. B. /dev/ttyAMA0)
	baudrate	Baudrate für RS485-Verbindung
	parity	N, E, O (Keine, Gerade, Ungerade)
	databits	Meist 8
	stopbits	Meist 1
[modbus]	slave_id	Modbus-Adresse des RTU-Slave
[tcp]	port	TCP-Port für Modbus-TCP (Standard: 502)
[mqtt]	broker	MQTT-Broker-Adresse (z. B. tcp://192.168.0.82:1883)
	client_id	Client-ID im Broker
	topic_prefix	Prefix für alle MQTT-Themen (z. B. modbus/)
	username	(optional) Benutzername für MQTT
	password	(optional) Passwort
	keepalive	Keepalive-Intervall in Sekunden
	lwt	Payload bei Verbindungsverlust
	online	Payload nach erfolgreicher Verbindung
[log]	file	Pfad zur Log-Datei (z. B. /var/log/modbus.log)
⚙️ Initialisierung im Code

bool init_config(int argc, char* argv[], bool& dry_run) {
    const char* config_path = "/etc/modbus_gateway.ini";
    if (argc > 1 && std::string(argv[1]) != "--dry-run") config_path = argv[1];
    if (ini_parse(config_path, config_handler, &config) < 0) {
        log("Konnte Konfiguration nicht laden, nutze Defaults.");
        return false;
    }
    return true;
}

Optionaler Modus --dry-run unterdrückt MQTT & Modbus Start.
💡 Vorteile

✅ Keine Neukompilierung bei Anpassungen
✅ Trennung von Code und Systemparametern
✅ Erweiterbar um weitere Parameter bei Bedarf


⚠️ Abschnitt 12: Signalverarbeitung & sicherer Shutdown

Dieser Abschnitt behandelt die Absicherung gegen abrupte Programmabbrüche (z. B. durch Strg+C, kill, systemd) und sorgt für:

    geordnetes Herunterfahren

    Schließen aller Sockets, Verbindungen & Threads

    Vermeidung von Datei-/Modbus-Korruption

🚦 Signalbehandlung (SIGINT, SIGTERM)

Im main():

signal(SIGINT, signal_handler);
signal(SIGTERM, signal_handler);

Registriert das benutzerdefinierte Callback:

void signal_handler(int) {
    run = false;
    if (server_sock != -1)
        shutdown(server_sock, SHUT_RDWR);
    log("Beende Gateway durch Signal...");
}

Wirkung:

    run wird auf false gesetzt → alle Schleifen (z. B. while (run)) beenden sich kontrolliert.

    server_sock wird aktiv per shutdown() geschlossen → unterbricht ggf. accept().

🛑 Geordneter Shutdown: shutdown_gateway()

Wird ganz am Ende aufgerufen, z. B. nach Beendigung von accept_clients() oder in --dry-run.

void shutdown_gateway() {
    for (auto& t : client_threads)
        if (t.joinable()) t.join();  // Verbindungen sauber beenden

    if (mqttClient && mqttClient->is_connected()) {
        mqttClient->publish(config.mqttTopicPrefix + "status", config.mqttLwtPayload.c_str(), MQTT_QOS, true);
        mqttClient->disconnect()->wait();
    }

    if (rtu_ctx) {
        modbus_close(rtu_ctx);
        modbus_free(rtu_ctx);
    }

    if (server_sock != -1)
        close(server_sock);

    log("Gateway beendet");
    logFile.close();
}

💡Was wird sichergestellt?
Ressource	Aktion bei Shutdown
Modbus-RTU Kontext	modbus_close() + modbus_free()
MQTT Verbindung	disconnect() + LWT-Publish
TCP-Clients	Alle Threads werden gejoined
Log-Datei	Wird ordnungsgemäß geschlossen
🔄 Wiederverwendbarkeit

Diese Mechanik schützt auch bei Konfigurationsfehlern, MQTT-Fehlschlägen oder ungültiger Modbus-Verbindung.


📄 Abschnitt 13: MQTT-Integration – Subscriptions & Payload-Formate

Die MQTT-Anbindung ist ein zentrales Feature des Gateways, um Modbus-Geräte flexibel über ein Netzwerkprotokoll zu steuern und zu überwachen.
📥 Unterstützte MQTT-Topics (Eingehend)

Das Gateway abonniert folgende MQTT-Topics und reagiert darauf:
Topic-Pattern	Beschreibung	Beispiel
modbus/write/<addr>	Schreibe Holding Register	modbus/write/123
modbus/write_coil/<addr>	Schreibe Coil (0/1)	modbus/write_coil/5
modbus/read/<addr>	Lese Holding Register (1 Wort)	modbus/read/10
modbus/read_coil/<addr>	Lese Coil (1 Bit)	modbus/read_coil/7
modbus/read_discrete/<addr>	Lese Discrete Input	modbus/read_discrete/14
modbus/read_input/<addr>	Lese Input Register	modbus/read_input/300
modbus/read_multi/<start>:<anzahl>	Lese mehrere Holding Register	modbus/read_multi/100:4
modbus/read_input_multi/<start>:<anzahl>	Lese mehrere Input Register	modbus/read_input_multi/200:6
modbus/read_coil_multi/<start>:<anzahl>	Lese mehrere Coils	modbus/read_coil_multi/0:8
modbus/read_discrete_multi/<start>:<anzahl>	Lese mehrere Discrete Inputs	modbus/read_discrete_multi/20:3

    Alle Adressen sind dezimal. Beim multi-Lesen trennt ein : die Startadresse und Anzahl.

🧾 Payload-Formate
1. write-Topics

Payload: eine Ganzzahl (z. B. 123 oder 0/1)

modbus/write/42
Payload: 5678
→ schreibt Holding Register 42 mit 5678

2. read-Topics

Payload: leer
→ Antwort erfolgt als MQTT-Publish mit gleichem Topic.

modbus/read/10
→ Antwort: "4321" (Holding Register 10)

modbus/read_input_multi/300:3
→ Antwort: "10,20,30"

📤 Ausgehende MQTT-Publishs (Antworten / Logs)
Topic	Inhalt
modbus/log	Lognachrichten (Info, Fehler etc.)
modbus/status	"online" / "offline"
alle eingehenden read-Topics	Modbus-Werte (Antworten)

Beispiel:

Topic: modbus/read/42
Payload: 1234

Topic: modbus/read_coil_multi/0:4
Payload: 1,0,0,1

📌 Besonderheiten

    Alle MQTT-Topics sind konfigurierbar via mqtt.topic_prefix

    Subscriptions werden nach jedem Reconnect erneut gesetzt

    Last Will Topic (status) meldet "offline" bei Verbindungsverlust


📄 Abschnitt 14: Modbus-TCP → Modbus-RTU Proxy-Funktion

Dieses Gateway agiert als Brücke zwischen einem Modbus-TCP-Master (z. B. SCADA, HMI oder SPS) und einem Modbus-RTU-Slave-Gerät (z. B. Wärmepumpe, Sensor etc.).
🔁 Ablauf: TCP-zu-RTU-Weiterleitung

    Modbus-TCP-Client verbindet sich mit dem Gateway auf Port 502 (oder konfigurierbar).

    Das Gateway empfängt ein vollständiges Modbus-TCP-Request-Paket.

    Es extrahiert den PDU-Teil (Protokolldaten) aus dem TCP-Frame.

    Die PDU wird eins-zu-eins per Modbus-RTU über die serielle Schnittstelle weitergeleitet.

    Die Antwort vom RTU-Slave wird empfangen und verpackt in ein Modbus-TCP-Frame.

    Die Antwort wird an den TCP-Client zurückgesendet.

⚙️ Technische Umsetzung

modbus_t* tcp_ctx = modbus_new_tcp(NULL, 0);
modbus_set_socket(tcp_ctx, client_sock);

int query_len = modbus_receive(tcp_ctx, query);
int sent = modbus_send_raw_request(rtu_ctx, query, query_len);
int res_len = modbus_receive_confirmation(rtu_ctx, response);

send(client_sock, response, res_len, 0);

    Dabei wird nur die minimale Puffer- und Umsetzungslogik verwendet – der Fokus liegt auf schneller Durchleitung.

🧵 Parallele TCP-Clients

    Jeder TCP-Client wird in einem eigenen Thread verarbeitet.

    Alle Verbindungen teilen sich die eine serielle RTU-Leitung (Synchronisation erfolgt durch libmodbus).

    Anzahl gleichzeitiger Clients begrenzt durch Systemressourcen und die Einstellung TCP_BACKLOG.

🧪 Beispiel: Modbus TCP Client (z. B. modpoll)

modpoll -m tcp -t 4 -r 100 -c 2 192.168.1.50

→ liest 2 Holding Register ab Adresse 100 vom RTU-Slave hinter dem Gateway
📌 Hinweise

    Es wird kein Mapping oder Konvertierung vorgenommen – die PDU bleibt unverändert.

    Der RTU-Slave ist über die in modbus_slave_id konfigurierte ID erreichbar.

    Alle Fehler (Sende-/Empfangsfehler) werden per Logfile und MQTT-Log gemeldet.


🧠 Abschnitt 15: Fehlerbehandlung und Logging-Strategie

Das Gateway besitzt eine durchgängige Fehlerbehandlung, gekoppelt mit einem Dual-Logging-System:
→ Dateilog + MQTT-Logmeldung
🛠 Fehlerbehandlung im Code
Allgemeines Prinzip

    Jeder relevante Rückgabewert (z. B. bei modbus_*(), socket(), connect()) wird geprüft.

    Im Fehlerfall erfolgt:

        🧾 Ein Eintrag im Logfile (zeitgestempelt)

        📡 Optional ein MQTT-Log über modbus/log

🔐 Beispiele
Modbus RTU initialisieren

if (!rtu_ctx) {
    log("Fehler beim Öffnen der seriellen Schnittstelle: " + std::string(modbus_strerror(errno)));
}

MQTT reconnect fehlgeschlagen

log("MQTT-Reconnect fehlgeschlagen: " + std::string(e.what()));

MQTT-Parsing-Fehler

log("Fehlerhaftes Topic (kein ':' gefunden) bei Mehrfachlese-Anfrage: " + topic);

🧾 Logging in Datei

    Standardpfad: /var/log/modbus_gateway.log

    Konfigurierbar über INI: [log] file = /pfad/zur/logdatei.log

    Log-Einträge enthalten Zeitstempel:

2025-06-24 13:42:17 - MQTT-Verbindung verloren: ...
2025-06-24 13:42:22 - MQTT-Reconnect erfolgreich

📡 Logging via MQTT

    Wenn verbunden, werden Logmeldungen zusätzlich an den Topic modbus/log gesendet.

    Beispiel für externe Überwachung per Node-RED:

    Topic: modbus/log
    Payload: Fehler beim Lesen Input Register 3012: Input/output error

🧠 Vorteile

    Doppelte Absicherung bei Fehlersuche

    Ermöglicht Integration in Monitoring-Systeme (Node-RED, Grafana, etc.)

    Zentrale Informationsquelle für Debugging


⚙️ Abschnitt 16: Konfiguration über INI-Datei

Das Gateway liest seine Startparameter aus einer einfachen .ini-Konfigurationsdatei. Dies ermöglicht eine flexible und benutzerfreundliche Anpassung des Systems – ohne Neukompilierung.
📄 Aufbau der Datei

Die Standard-Konfigurationsdatei liegt unter:

/etc/modbus_gateway.ini

Sie ist nach Sektionen gegliedert ([section]) und enthält einfache Schlüssel-Wert-Paare (key = value).
🔧 Unterstützte Konfigurationsoptionen
[serial] – Modbus RTU Einstellungen
Schlüssel	Beschreibung	Beispiel
port	Serielle Schnittstelle	/dev/ttyAMA0
baudrate	Baudrate	9600
parity	Parität (N, E, O)	N
databits	Anzahl Datenbits	8
stopbits	Anzahl Stopbits	1
[tcp] – Modbus TCP Server
Schlüssel	Beschreibung	Beispiel
port	Port für TCP Server	502
[modbus] – Allgemeines Modbus
Schlüssel	Beschreibung	Beispiel
slave_id	Standard-Slave-ID für RTU	1
[mqtt] – MQTT-Einstellungen
Schlüssel	Beschreibung	Beispiel
broker	MQTT-Broker-Adresse	tcp://192.168.0.82:1883
client_id	MQTT-Client-ID	modbus_gateway
topic_prefix	Präfix für Topics	modbus/
username	(Optional) MQTT-Username	user1
password	(Optional) MQTT-Passwort	geheim
keepalive	Keep-Alive-Zeit in Sekunden	30
lwt	Payload bei Verbindungsverlust	offline
online	Payload nach erfolgreicher Verbindung	online
[log] – Logging
Schlüssel	Beschreibung	Beispiel
file	Pfad zur Logdatei	/var/log/modbus_gateway.log
🔍 Validierung

Die Datei wird beim Start durch ini_parse() geladen. Ungültige oder fehlende Felder führen nicht zum Abbruch, sondern es werden Defaults verwendet. (Log-Eintrag wird erzeugt.)
🧪 Spezial-Optionen beim Start

Beim Start kann eine alternative INI-Datei übergeben werden:

./modbus_gateway /pfad/zur/anderen.ini

Oder für Testzwecke:

./modbus_gateway --dry-run

📡 Abschnitt 17: MQTT Topics & Topic-Struktur

Das Gateway verwendet MQTT zur Kommunikation mit anderen Systemen (z. B. Node-RED, Home Assistant, Grafana). Die Struktur ist bewusst einfach und lesbar gehalten, sodass sie auch mit einfachen MQTT-Tools (z. B. mosquitto_pub) genutzt werden kann.
🔁 MQTT-Basisstruktur

Alle Topics beginnen mit einem konfigurierbaren Präfix (Standard: modbus/) – definiert über:

[mqtt]
topic_prefix = modbus/

📝 Übersicht der verfügbaren Topics
Topic	Aktion	Beschreibung
modbus/status	(intern)	Verbindungsstatus (online/offline)
modbus/log	(intern)	Laufende Log-Meldungen
modbus/write/1234	Publish	Schreibe Holding Register 1234 mit Payload (int)
modbus/read/1234	Publish	Lese Holding Register 1234 → Antwort auf gleichem Topic
modbus/write_coil/45	Publish	Schreibe Coil 45 (0 oder 1)
modbus/read_coil/45	Publish	Lese Coil 45 → Antwort auf gleichem Topic
modbus/read_input/200	Publish	Lese Input Register 200 → Antwort auf gleichem Topic
modbus/read_discrete/12	Publish	Lese Discrete Input 12 → Antwort auf gleichem Topic
modbus/read_multi/100:4	Publish	Lese Holding Register 100–103, CSV-Antwort auf gleichem Topic
modbus/read_input_multi/300:2	Publish	Lese Input Register 300–301, CSV-Antwort
modbus/read_coil_multi/20:8	Publish	Lese Coils 20–27, CSV-Antwort
modbus/read_discrete_multi/5:5	Publish	Lese Discrete Inputs 5–9, CSV-Antwort
🧪 Beispiel: Schreiben eines Registerwerts

mosquitto_pub -t modbus/write/100 -m 42

→ schreibt den Wert 42 in Register 100.
🧪 Beispiel: Lesen mehrerer Register

mosquitto_pub -t modbus/read_multi/200:3 -n

→ liest Register 200, 201, 202
← Antwort als: 23,0,98 auf Topic modbus/read_multi/200:3
🧪 Beispiel: Lesen eines Coils

mosquitto_pub -t modbus/read_coil/15 -n

← Antwort: 0 oder 1
❗ Hinweis zur Topic-Form

    Lesebefehle (read*, read_multi*) werden nur durch Veröffentlichung auf dem Topic ausgelöst.

    Antwort erfolgt auf demselben Topic als Payload.

    Schreibbefehle (write*) erwarten einen gültigen Integer- oder Bool-Payload.


🧠 Abschnitt 18: MQTT Reconnect-Logik & Stabilität

Damit das Gateway auch bei Netzwerkproblemen oder MQTT-Server-Neustarts zuverlässig arbeitet, wurde eine robuste Reconnect-Strategie umgesetzt.
💡 Grundidee

    Der MQTT-Client wird bei Verbindungsverlust automatisch neu verbunden.

    Dabei werden Subscriptions und Callbacks erneut gesetzt.

    Parallele Reconnects werden verhindert.

🔧 Wichtige Komponenten
🔁 Reconnect-Thread

void reconnect_mqtt() {
    if (mqttConnecting.exchange(true)) return;  // 🔒 verhindert Doppelstart

    while (run) {
        try {
            // Verbindung mit Optionen (inkl. LWT etc.)
            mqttClient->connect(connOpts)->wait();
            log("MQTT-Reconnect erfolgreich");

            // Callback und Topics erneut setzen
            mqttClient->set_callback(mqttCb);
            mqttClient->publish(config.mqttTopicPrefix + "status", config.mqttOnlinePayload, MQTT_QOS, true);
            mqttClient->subscribe(config.mqttTopicPrefix + "write/#", MQTT_QOS)->wait();
            mqttClient->subscribe(config.mqttTopicPrefix + "read/#", MQTT_QOS)->wait();

            break;
        } catch (...) {
            log("MQTT-Reconnect fehlgeschlagen, erneuter Versuch in 5s...");
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    mqttConnecting = false;
}

📉 Automatischer Reconnect bei Verbindungsverlust

Die MQTTCallback-Klasse reagiert auf connection_lost():

void connection_lost(const std::string& cause) override {
    log("MQTT-Verbindung verloren: " + cause);
    static std::atomic<bool> reconnecting = false;
    if (!reconnecting.exchange(true)) {
        std::thread([] {
            reconnect_mqtt();
            reconnecting = false;
        }).detach();
    }
}

➡️ Wichtig: std::atomic<bool> verhindert parallele Threads.
✅ Vorteile der Lösung

    Keine doppelten Verbindungsversuche

    Subscriptions und Callback werden garantiert wiederhergestellt

    Funktioniert stabil bei:

        Neustart des MQTT-Brokers

        Netzwerkunterbrechung

        Stromausfall auf Broker-Seite

🧪 Testen

    Broker neu starten → Verbindung wird automatisch wiederhergestellt.

    Netzwerk trennen und reconnecten → MQTT wird neu verbunden.

    mosquitto_sub auf modbus/status → "offline" → "online"


🗂️ Abschnitt 19: Logging (Dateiausgabe & MQTT)

Das Gateway führt detailliertes Logging durch – sowohl lokal in eine Datei als auch optional über MQTT, damit der Systemstatus jederzeit überwachbar ist.
📝 Zentrale Logfunktion

void log(const std::string& msg) {
    std::lock_guard<std::mutex> lock(logMutex);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    logFile << std::put_time(std::localtime(&now), "%F %T") << " - " << msg << std::endl;

    if (mqttClient && mqttClient->is_connected()) {
        mqttClient->publish(config.mqttTopicPrefix + "log", msg.c_str(), MQTT_QOS, false);
    }
}

Merkmale:

    Thread-sicher durch std::mutex

    Zeitstempel im Format YYYY-MM-DD HH:MM:SS

    Ausgabe:

        📁 Log-Datei (Pfad per INI konfigurierbar)

        📡 MQTT-Topic: modbus/log

📂 Konfigurierbarer Pfad

[log]
file = /var/log/modbus_gateway.log

Wird über config_handler() eingelesen und bei Start geöffnet:

logFile.open(config.logFilePath, std::ios::app);

🚨 Verwendung im Code

    Startup: log("Starte Gateway...");

    Fehlerzustände: z. B. bei Verbindungsabbrüchen, Lesefehlern

    Aktivitäten: z. B. wenn ein Client verbunden oder getrennt wird, MQTT-Reconnects usw.

    Modbus-Aktionen: Lese-/Schreibvorgänge mit Adresse und Ergebnis

🔍 Beispielausgabe

2025-06-24 14:03:25 - Starte Gateway...
2025-06-24 14:03:25 - Modbus RTU initialisiert
2025-06-24 14:04:10 - Modbus-Register 300 gesetzt auf 42
2025-06-24 14:04:11 - MQTT-Verbindung verloren: Connection reset
2025-06-24 14:04:16 - MQTT-Reconnect erfolgreich

📡 MQTT-Log-Nutzung

Mit Tools wie mosquitto_sub kannst du das Gateway remote überwachen:

mosquitto_sub -t modbus/log -h 192.168.0.82

In Node-RED kannst du mqtt in → debug oder text area verwenden, um Logmeldungen anzuzeigen.
✅ Vorteile

    Zentrale Fehlerdiagnose – auch bei entfernten Systemen

    Reproduzierbarkeit von Problemen

    Keine zusätzlichen Abhängigkeiten notwendig


📦 Abschnitt 20: Konfigurationsdatei (modbus_gateway.ini)

Die Konfigurationsdatei erlaubt es, das Verhalten des Gateways zur Laufzeit flexibel anzupassen – ohne Neukompilierung. Sie basiert auf dem .ini-Format und wird beim Start des Programms gelesen.
📁 Pfad und Verarbeitung

Standardpfad:

const char* config_path = "/etc/modbus_gateway.ini";

Optional kann ein Pfad per Kommandozeilenargument übergeben werden. Zusätzlich gibt es einen --dry-run-Modus zur Validierung.
🧠 Parsing

Wird mit Hilfe von inih verarbeitet:

if (ini_parse(config_path, config_handler, &config) < 0) {
    log("Konnte Konfiguration nicht laden, nutze Defaults.");
    return false;
}

Der config_handler() setzt dabei die Felder der struct Config.
🧾 Unterstützte Konfigurationsoptionen

Hier ein Beispiel für eine vollständige Datei:

[serial]
port = /dev/ttyAMA0
baudrate = 9600
parity = N
databits = 8
stopbits = 1

[tcp]
port = 502

[mqtt]
broker = tcp://192.168.0.82:1883
client_id = modbus_gateway
topic_prefix = modbus/
username = myuser
password = mypass
keepalive = 30
lwt = offline
online = online

[log]
file = /var/log/modbus_gateway.log

[modbus]
slave_id = 1

🧩 Details zu jedem Abschnitt
🔌 [serial]

    Konfiguriert die Modbus-RTU Schnittstelle.

    Unterstützt alle gängigen Parameter: Baudrate, Parität, Daten-/Stoppbits.

🌐 [tcp]

    Konfiguriert den TCP-Port für Modbus-TCP zu RTU Forwarding.

📡 [mqtt]

    Verbindungsdaten zum MQTT-Broker

    topic_prefix steuert alle MQTT-Themen (z. B. modbus/write/123)

    LWT und Online-Payloads helfen bei Statusüberwachung

📋 [log]

    Legt den Pfad für die Logdatei fest (z. B. /var/log/modbus_gateway.log)

🔁 [modbus]

    Gibt den Slave-ID des angesprochenen Modbus-Ziels an

🔄 Neu laden zur Laufzeit?

Nein.
Änderungen an der Datei wirken sich erst nach einem Neustart des Programms aus.
🧪 Validierungsmodus

Du kannst die Konfiguration prüfen, ohne das Gateway zu starten:

./modbus_gateway /etc/modbus_gateway.ini --dry-run


⚙️ Abschnitt 21: Systemstart & CLI-Aufruf (main()-Funktion)

Die main()-Funktion ist der zentrale Einstiegspunkt des Gateways. Hier wird der gesamte Initialisierungsablauf, der Start der Serverkomponenten und der Haupt-Thread gesteuert.
🔁 Übersicht des Ablaufs

int main(int argc, char* argv[]) {
    ...
    // 1. CLI-Parameter prüfen
    // 2. Signal-Handler registrieren
    // 3. Logdatei öffnen
    // 4. Konfiguration laden
    // 5. MQTT initialisieren
    // 6. Modbus RTU initialisieren
    // 7. TCP-Server starten
    // 8. Client-Akzeptanz starten
    // 9. Shutdown
}

🧪 Unterstützte Aufrufe

./modbus_gateway [pfad/zur/config.ini] [--dry-run]

Optionen:

    --version: Zeigt die aktuelle Softwareversion

    --help oder -h: Zeigt eine kurze Hilfe

    --dry-run: Führt nur Konfigurationsprüfung durch

🔐 Signal-Handling

Behandelt SIGINT und SIGTERM, um das Gateway sauber zu beenden:

signal(SIGINT, signal_handler);
signal(SIGTERM, signal_handler);

🧾 Logfile öffnen

logFile.open(config.logFilePath, std::ios::app);
log("Starte Gateway...");

📥 Initialisierungen

if (!init_config(...)) return 1;
if (!dry_run && !init_mqtt()) return 1;
if (!dry_run && !init_modbus()) return 1;
if (!dry_run && !start_tcp_server()) return 1;

📡 Client-Verbindung

if (!dry_run) accept_clients();

Dieser Aufruf blockiert (läuft dauerhaft). Der Haupt-Thread bleibt so aktiv.
🔚 Programmbeendigung

shutdown_gateway();
return 0;

Der Shutdown ist robust implementiert: Alle MQTT-Verbindungen werden getrennt, offene RTU-Handles und TCP-Sockets geschlossen und Threads sauber beendet.


🧹 Abschnitt 22: Shutdown-Prozedur und Ressourcenfreigabe (shutdown_gateway())

Die Funktion shutdown_gateway() sorgt dafür, dass beim Beenden des Programms alle Ressourcen sauber freigegeben werden. Dies ist essenziell für die Systemstabilität und für Wiederstarts ohne „hängende“ Prozesse oder offene Ports.
🔧 Funktionssignatur

void shutdown_gateway();

🪚 Ablauf im Detail

// 1. Alle Client-Threads beenden
for (auto& t : client_threads)
    if (t.joinable()) t.join();

    Wartet darauf, dass alle durch accept_clients() erzeugten Threads (handle_client()) fertig sind.

// 2. MQTT-Verbindung sauber trennen
if (mqttClient && mqttClient->is_connected()) {
    mqttClient->publish(config.mqttTopicPrefix + "status", config.mqttLwtPayload.c_str(), MQTT_QOS, true);
    mqttClient->disconnect()->wait();
}

    Informiert den MQTT-Broker, dass das Gateway offline ist (LWT → Last Will Topic).

    Trennt die Verbindung synchron.

// 3. Modbus RTU-Schnittstelle schließen
if (rtu_ctx) {
    modbus_close(rtu_ctx);
    modbus_free(rtu_ctx);
}

    Vermeidet Speicherlecks und blockierte serielle Schnittstellen beim Neustart.

// 4. TCP-Server schließen
if (server_sock != -1)
    close(server_sock);

    Gibt den TCP-Port (Standard 502) frei.

// 5. Logdatei sauber schließen
log("Gateway beendet");
logFile.close();

    Letzter Logeintrag zur sauberen Protokollierung des Shutdowns.

✅ Zusammenfassung
Schritt	Beschreibung
1.	Client-Threads sauber beenden
2.	MQTT-Verbindung ordentlich abmelden
3.	Modbus-RTU freigeben
4.	TCP-Port schließen
5.	Logdatei mit Status schließen


📦 Abschnitt 23: Zusammenfassung & Projektstruktur
✅ Projektziel

Dieses Gateway dient als zuverlässige und erweiterbare Schnittstelle zwischen:

    🧱 Modbus TCP (z. B. SCADA-Systeme, Node-RED)

    📟 Modbus RTU (seriell) (z. B. Wärmepumpe, Sensoren)

    ☁️ MQTT (z. B. Home Assistant, Domoticz, Node-RED, Grafana)

Es ermöglicht das Lesen und Schreiben aller relevanten Modbus-Registertypen, ist konfigurierbar über eine .ini-Datei und kann auch als MQTT-basiertes Monitoring- und Steuerungs-Backend dienen.
📂 Projektstruktur (empfohlene Deployment-Struktur)

modbus_gateway/
├── src/
│   └── modbus_gateway.cpp     # Hauptprogramm (bereits dokumentiert)
├── config/
│   └── modbus_gateway.ini     # Konfigurationsdatei (Pfad: /etc/modbus_gateway.ini)
├── logs/
│   └── modbus_gateway.log     # Laufendes Logfile
├── systemd/
│   └── modbus_gateway.service # Optional: Autostart über systemd
└── README.md                  # Detaillierte Projektdokumentation

📑 Unterstützte MQTT-Themen (für Node-RED, MQTT-Clients etc.)
Typ	MQTT-Topic (Beispiel)	Beschreibung
Holding Register (RW)	modbus/write/42, modbus/read/42	Einzelnes Register lesen/schreiben
Input Register (RO)	modbus/read_input/100	Sensordaten etc. lesen
Coils (RW)	modbus/write_coil/7, modbus/read_coil/7	Digitale Ausgänge (on/off)
Discrete Inputs (RO)	modbus/read_discrete/5	Digitale Eingänge lesen
Mehrfachlesen	modbus/read_multi/0:4 (→ Reg. 0–3)	Mehrere Holding Register
	modbus/read_input_multi/100:2	Mehrere Input Register
	modbus/read_coil_multi/10:4	Mehrere Coils
	modbus/read_discrete_multi/20:2	Mehrere Discrete Inputs
💡 Beispiel-Nutzung mit MQTT

# Setze Modbus-Holding-Register 42 auf den Wert 77
mosquitto_pub -t modbus/write/42 -m 77

# Lese Modbus-Register 42
mosquitto_pub -t modbus/read/42 -n

Die Antwort erscheint auf dem Topic modbus/read/42.
🛠️ Kommandozeilenoptionen

./modbus_gateway /etc/modbus_gateway.ini
./modbus_gateway --dry-run
./modbus_gateway --version
./modbus_gateway --help

🧯 Shutdown- und Fehlertoleranz

    MQTT-Reconnect bei Verbindungsverlust

    Logging in Datei und MQTT

    Konfigurierbarer LWT (Last Will and Testament)

    Signalverarbeitung (SIGINT, SIGTERM) für sauberes Beenden

📦 Projektstatus

✅ Stabil für produktiven Einsatz
🛠️ Modular erweiterbar (z. B. Web-GUI, OTA, TLS)


