
// Modbus TCP -> RTU + MQTT Gateway
// Kompilieren mit:
// g++ -o modbus_gateway modbus-tcp-mqtt-nibe.cpp ini.c   -I/usr/local/include -L/usr/local/lib   -lmodbus -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread

#include <fstream>
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>
#include <modbus/modbus.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <cerrno>
#include <cstring>
#include <ini.h>
#include <mqtt/async_client.h>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <ctime> // für std::time_t



std::mutex logMutex;

std::ofstream logFile;
#define DEFAULT_TCP_PORT 502
#define DEFAULT_SERIAL_PORT "/dev/ttyAMA0"
#define DEFAULT_BAUDRATE 9600
#define DEFAULT_PARITY 'N'
#define DEFAULT_DATA_BITS 8
#define DEFAULT_STOP_BITS 1
#define TCP_BACKLOG 5
#define MODBUS_RTU_TIMEOUT_SEC 1
#define MODBUS_RTU_TIMEOUT_USEC 0
#define MQTT_QOS 1
#define MQTT_TIMEOUT 10000

struct Config
{
    int mqttKeepAlive = 30;
    std::string mqttLwtPayload = "offline";
    std::string mqttOnlinePayload = "online";
    int modbusSlaveId = 1;
    std::string logFilePath = "/var/log/modbus_gateway.log";
    std::string serialPort = DEFAULT_SERIAL_PORT;
    int baudrate = DEFAULT_BAUDRATE;
    char parity = DEFAULT_PARITY;
    int dataBits = DEFAULT_DATA_BITS;
    int stopBits = DEFAULT_STOP_BITS;
    int tcpPort = DEFAULT_TCP_PORT;
    std::string mqttBroker = "tcp://localhost:1883";
    std::string mqttClientId = "modbus_gateway";
    std::string mqttTopicPrefix = "modbus/";
    std::string mqttUsername;
    std::string mqttPassword;
    bool debugHexDump = false;
    bool mqttUseTLS = false;
    std::string mqttCAFile;
    std::string mqttCertFile;
    std::string mqttKeyFile;
    bool mqttInsecure = false;  // falls Zertifikat nicht validiert werden soll
    int mqttStatsInterval = 60; // Default: alle 60 Sekunden Statistik senden
} config;

std::shared_ptr<mqtt::async_client> mqttClient = nullptr;
modbus_t *rtu_ctx = nullptr;
std::atomic<bool> run(true);
int server_sock = -1;
std::vector<std::thread> client_threads;
void reconnect_mqtt(); // <- Wichtig für spätere Verwendung
std::atomic<bool> mqttConnecting = false;
std::atomic<uint64_t> successful_rtu_requests{0}; // für Diagnose
std::atomic<uint64_t> failed_rtu_requests{0};
std::atomic<uint64_t> total_tcp_requests{0};
std::atomic<uint64_t> total_rtu_success{0};
std::atomic<uint64_t> total_rtu_failures{0};
std::atomic<uint64_t> total_dropped{0};
std::atomic<uint64_t> total_echo{0};
std::thread stats_thread;
std::atomic<uint64_t> total_rtu_requests{0};
std::atomic<uint64_t> total_errors{0};
std::time_t last_success_timestamp = 0;

void publish_stats()
{
    while (run)
    {
        std::this_thread::sleep_for(std::chrono::seconds(config.mqttStatsInterval));
        if (mqttClient && mqttClient->is_connected())
        {
            std::ostringstream oss;
            oss << "{"
                << "\"total_tcp_requests\":" << total_tcp_requests.load() << ","
                << "\"total_rtu_requests\":" << total_rtu_requests.load() << ","
                << "\"total_errors\":" << total_errors.load() << ","
                << "\"rtu_success\":" << total_rtu_success.load() << ","
                << "\"rtu_failures\":" << total_rtu_failures.load() << ","
                << "\"dropped\":" << total_dropped.load() << ","
                << "\"echo_replies\":" << total_echo.load() << ","
                << "\"last_success_timestamp\":" << last_success_timestamp
                << "}";

            mqttClient->publish(config.mqttTopicPrefix + "stats", oss.str().c_str(), MQTT_QOS, false);
        }
    }
}

// Hilfsfunktion für hex-Werte als String
std::string int_to_hex(uint16_t val, int width = 4)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(width) << val;
    return ss.str();
}

void log(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(logMutex);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    logFile << std::put_time(std::localtime(&now), "%F %T") << " - " << msg << std::endl;

    // Nur senden, wenn MQTT verbunden UND mqttClient gültig
    if (mqttClient && mqttClient->is_connected())
    {
        mqttClient->publish(config.mqttTopicPrefix + "log", msg.c_str(), MQTT_QOS, false);
    }
}

int config_handler(void *user, const char *section, const char *name, const char *value)
{
    Config *p = (Config *)user;
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("serial", "port"))
        p->serialPort = value;
    else if (MATCH("serial", "baudrate"))
        p->baudrate = atoi(value);
    else if (MATCH("serial", "parity"))
        p->parity = value[0];
    else if (MATCH("serial", "databits"))
        p->dataBits = atoi(value);
    else if (MATCH("serial", "stopbits"))
        p->stopBits = atoi(value);
    else if (MATCH("tcp", "port"))
        p->tcpPort = atoi(value);
    else if (MATCH("mqtt", "broker"))
        p->mqttBroker = value;
    else if (MATCH("mqtt", "client_id"))
        p->mqttClientId = value;
    else if (MATCH("mqtt", "topic_prefix"))
        p->mqttTopicPrefix = value;
    else if (MATCH("mqtt", "username"))
        p->mqttUsername = value;
    else if (MATCH("mqtt", "password"))
        p->mqttPassword = value;
    else if (MATCH("mqtt", "keepalive"))
        p->mqttKeepAlive = atoi(value);
    else if (MATCH("mqtt", "lwt"))
        p->mqttLwtPayload = value;
    else if (MATCH("mqtt", "online"))
        p->mqttOnlinePayload = value;
    else if (MATCH("log", "file"))
        p->logFilePath = value;
    else if (MATCH("modbus", "slave_id"))
        p->modbusSlaveId = atoi(value);
    else if (MATCH("log", "hex_dump"))
        p->debugHexDump = atoi(value); // 1 = aktiv, 0 = aus
    else if (MATCH("mqtt", "use_tls"))
        p->mqttUseTLS = atoi(value);
    else if (MATCH("mqtt", "ca_file"))
        p->mqttCAFile = value;
    else if (MATCH("mqtt", "cert_file"))
        p->mqttCertFile = value;
    else if (MATCH("mqtt", "key_file"))
        p->mqttKeyFile = value;
    else if (MATCH("mqtt", "insecure"))
        p->mqttInsecure = atoi(value);
    else if (MATCH("mqtt", "stats_interval"))
        p->mqttStatsInterval = atoi(value); // in Sekunden
    else
        return 0;
    return 1;
}

void dump_hex(const std::string &prefix, const uint8_t *data, size_t len)
{
    std::ostringstream oss;
    oss << prefix;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)data[i] << " ";
    log(oss.str());
}

void signal_handler(int)
{
    run = false;
    if (server_sock != -1)
        shutdown(server_sock, SHUT_RDWR);
    log("Beende Gateway durch Signal...");
}

// void connection_lost(const std::string& cause) override {
//     log("MQTT-Verbindung verloren: " + cause);
//     std::thread(reconnect_mqtt).detach();  // detached, nicht blockierend
// }

class MQTTCallback : public virtual mqtt::callback
{
public:
    void connection_lost(const std::string &cause) override
    {
        log("MQTT-Verbindung verloren: " + cause);
        static std::atomic<bool> reconnecting = false;
        if (!reconnecting.exchange(true))
        {
            std::thread([]
                        {
                reconnect_mqtt();
                reconnecting = false; })
                .detach();
        }
    }

    void message_arrived(mqtt::const_message_ptr msg) override
    {
        const std::string &topic = msg->get_topic();
        const std::string &payload = msg->to_string();

        // === Einzelne Register/Coils ===
        if (topic.rfind(config.mqttTopicPrefix + "write/", 0) == 0)
        {
            std::string address_str = topic.substr((config.mqttTopicPrefix + "write/").length());
            int address = std::stoi(address_str);
            int value = std::stoi(payload);
            if (modbus_write_register(rtu_ctx, address, value) == -1)
            {
                log("Fehler beim Schreiben in Modbus-Register " + address_str + ": " + modbus_strerror(errno));
            }
            else
            {
                log("Modbus-Register " + address_str + " gesetzt auf " + payload);
            }
        }
        else if (topic.rfind(config.mqttTopicPrefix + "read/", 0) == 0)
        {
            std::string address_str = topic.substr((config.mqttTopicPrefix + "read/").length());
            int address = std::stoi(address_str);
            uint16_t val = 0;
            if (modbus_read_registers(rtu_ctx, address, 1, &val) == -1)
            {
                log("Fehler beim Lesen von Modbus-Register " + address_str + ": " + modbus_strerror(errno));
            }
            else
            {
                mqttClient->publish(config.mqttTopicPrefix + "read/" + address_str, std::to_string(val).c_str(), MQTT_QOS, false);
            }
        }
        else if (topic.rfind(config.mqttTopicPrefix + "write_coil/", 0) == 0)
        {
            std::string address_str = topic.substr((config.mqttTopicPrefix + "write_coil/").length());
            int address = std::stoi(address_str);
            int value = std::stoi(payload);
            if (modbus_write_bit(rtu_ctx, address, value) == -1)
            {
                log("Fehler beim Schreiben Coil " + address_str + ": " + modbus_strerror(errno));
            }
            else
            {
                log("Coil " + address_str + " gesetzt auf " + payload);
            }
        }
        else if (topic.rfind(config.mqttTopicPrefix + "read_coil/", 0) == 0)
        {
            std::string address_str = topic.substr((config.mqttTopicPrefix + "read_coil/").length());
            int address = std::stoi(address_str);
            uint8_t val;
            if (modbus_read_bits(rtu_ctx, address, 1, &val) == -1)
            {
                log("Fehler beim Lesen Coil " + address_str + ": " + modbus_strerror(errno));
            }
            else
            {
                mqttClient->publish(config.mqttTopicPrefix + "read_coil/" + address_str, std::to_string(val).c_str(), MQTT_QOS, false);
            }
        }
        else if (topic.rfind(config.mqttTopicPrefix + "read_discrete/", 0) == 0)
        {
            std::string address_str = topic.substr((config.mqttTopicPrefix + "read_discrete/").length());
            int address = std::stoi(address_str);
            uint8_t val;
            if (modbus_read_input_bits(rtu_ctx, address, 1, &val) == -1)
            {
                log("Fehler beim Lesen Discrete Input " + address_str + ": " + modbus_strerror(errno));
            }
            else
            {
                mqttClient->publish(config.mqttTopicPrefix + "read_discrete/" + address_str, std::to_string(val).c_str(), MQTT_QOS, false);
            }
        }
        else if (topic.rfind(config.mqttTopicPrefix + "read_input/", 0) == 0)
        {
            std::string address_str = topic.substr((config.mqttTopicPrefix + "read_input/").length());
            int address = std::stoi(address_str);
            uint16_t val;
            if (modbus_read_input_registers(rtu_ctx, address, 1, &val) == -1)
            {
                log("Fehler beim Lesen Input Register " + address_str + ": " + modbus_strerror(errno));
            }
            else
            {
                mqttClient->publish(config.mqttTopicPrefix + "read_input/" + address_str, std::to_string(val).c_str(), MQTT_QOS, false);
            }
        }
        else if (topic == config.mqttTopicPrefix + "stats/reset")
        {
            total_tcp_requests = 0;
            successful_rtu_requests = 0;
            failed_rtu_requests = 0;
            log("Statistikzähler über MQTT zurückgesetzt");
        }
        else if (topic == config.mqttTopicPrefix + "command/stats")
        {
            std::ostringstream oss;
            oss << "Statistik (on-demand): "
                << "TCP=" << total_tcp_requests.load()
                << ", RTU=" << total_rtu_requests.load()
                << ", Fehler=" << total_errors.load()
                << ", Letzter RTU-Erfolg: " << std::put_time(std::localtime(&last_success_timestamp), "%F %T");

            mqttClient->publish(config.mqttTopicPrefix + "stats", oss.str(), MQTT_QOS, false);
            log("MQTT-Befehl empfangen: Statistik veröffentlicht");
        }

        // === Mehrfach-Lesen ===
        auto handle_read_multi_registers = [&](const std::string &prefix, int (*fn)(modbus_t *, int, int, uint16_t *))
        {
            if (topic.rfind(prefix, 0) == 0)
            {
                std::string addr_str = topic.substr(prefix.length());
                size_t colon = addr_str.find(':');
                if (colon == std::string::npos)
                {
                    log("Fehlerhaftes Topic (kein ':' gefunden) bei Mehrfachlese-Anfrage: " + topic);
                    return;
                }

                int addr = std::stoi(addr_str.substr(0, colon));
                int count = std::stoi(addr_str.substr(colon + 1));
                if (addr < 0 || count <= 0 || count > 125)
                {
                    log("Ungültige Adress-/Anzahlparameter: addr=" + std::to_string(addr) + ", count=" + std::to_string(count));
                    return;
                }

                std::vector<uint16_t> values(count);
                int rc = fn(rtu_ctx, addr, count, values.data());
                if (rc == -1)
                {
                    log("Fehler beim Lesen von " + topic + ": " + modbus_strerror(errno));
                }
                else
                {
                    std::ostringstream oss;
                    for (int i = 0; i < count; ++i)
                    {
                        if (i > 0)
                            oss << ",";
                        oss << values[i];
                    }
                    mqttClient->publish(topic, oss.str().c_str(), MQTT_QOS, false);
                }
            }
        };

        auto handle_read_multi_bits = [&](const std::string &prefix, int (*fn)(modbus_t *, int, int, uint8_t *))
        {
            if (topic.rfind(prefix, 0) == 0)
            {
                std::string addr_str = topic.substr(prefix.length());
                size_t colon = addr_str.find(':');
                if (colon == std::string::npos)
                {
                    log("Fehlerhaftes Topic (kein ':' gefunden) bei Mehrfachlese-Anfrage: " + topic);
                    return;
                }

                int addr = std::stoi(addr_str.substr(0, colon));
                int count = std::stoi(addr_str.substr(colon + 1));
                if (addr < 0 || count <= 0 || count > 2000)
                {
                    log("Ungültige Adress-/Anzahlparameter: addr=" + std::to_string(addr) + ", count=" + std::to_string(count));
                    return;
                }

                std::vector<uint8_t> values(count);
                int rc = fn(rtu_ctx, addr, count, values.data());
                if (rc == -1)
                {
                    log("Fehler beim Lesen von " + topic + ": " + modbus_strerror(errno));
                }
                else
                {
                    std::ostringstream oss;
                    for (int i = 0; i < count; ++i)
                    {
                        if (i > 0)
                            oss << ",";
                        oss << static_cast<int>(values[i]);
                    }
                    mqttClient->publish(topic, oss.str().c_str(), MQTT_QOS, false);
                }
            }
        };

        handle_read_multi_registers(config.mqttTopicPrefix + "read_multi/", modbus_read_registers);
        handle_read_multi_registers(config.mqttTopicPrefix + "read_input_multi/", modbus_read_input_registers);
        handle_read_multi_bits(config.mqttTopicPrefix + "read_coil_multi/", modbus_read_bits);
        handle_read_multi_bits(config.mqttTopicPrefix + "read_discrete_multi/", modbus_read_input_bits);
    }
};

MQTTCallback mqttCb; // außerhalb aller Funktionen definieren

bool init_mqtt()
{
    try
    {
        mqttClient = std::make_shared<mqtt::async_client>(config.mqttBroker, config.mqttClientId);
        mqttClient->set_callback(mqttCb);

        mqtt::connect_options connOpts;
        connOpts.set_keep_alive_interval(config.mqttKeepAlive);
        connOpts.set_clean_session(false);

        mqtt::will_options will(config.mqttTopicPrefix + "status", config.mqttLwtPayload, MQTT_QOS, true);
        connOpts.set_will(will);

        if (!config.mqttUsername.empty())
        {
            connOpts.set_user_name(config.mqttUsername);
            connOpts.set_password(config.mqttPassword);
        }

        // --- TLS aktivieren, falls gewünscht ---
        mqtt::ssl_options sslOpts;
        if (config.mqttUseTLS)
        {
            log("MQTT TLS aktiviert.");

            sslOpts.set_enable_server_cert_auth(!config.mqttInsecure);

            if (!config.mqttCAFile.empty())
            {
                sslOpts.set_trust_store(config.mqttCAFile);
                log("CA-Datei: " + config.mqttCAFile);
            }

            if (!config.mqttCertFile.empty())
            {
                sslOpts.set_key_store(config.mqttCertFile);
                log("Client-Zertifikat: " + config.mqttCertFile);
            }

            if (!config.mqttKeyFile.empty())
            {
                sslOpts.set_private_key(config.mqttKeyFile);
                log("Client-Key: " + config.mqttKeyFile);
            }

            if (config.mqttInsecure)
                log("⚠️ TLS: Zertifikatprüfung deaktiviert (insecure)");

            connOpts.set_ssl(sslOpts);
        }

        mqttClient->connect(connOpts)->wait();
        mqttClient->publish(config.mqttTopicPrefix + "status", config.mqttOnlinePayload.c_str(), MQTT_QOS, true);
        mqttClient->subscribe(config.mqttTopicPrefix + "write/#", MQTT_QOS)->wait();
        mqttClient->subscribe(config.mqttTopicPrefix + "read/#", MQTT_QOS)->wait();
        mqttClient->subscribe(config.mqttTopicPrefix + "stats/reset", MQTT_QOS)->wait();
        mqttClient->subscribe(config.mqttTopicPrefix + "command/#", MQTT_QOS)->wait();

        return true;
    }
    catch (const mqtt::exception &e)
    {
        log("Fehler bei MQTT-Verbindung: " + std::string(e.what()));
        return false;
    }
}

void reconnect_mqtt()
{
    if (mqttConnecting.exchange(true))
        return; // Verhindert parallele Reconnects

    while (run)
    {
        try
        {
            mqtt::connect_options connOpts;
            connOpts.set_keep_alive_interval(config.mqttKeepAlive);
            connOpts.set_clean_session(false); // wichtig: bestehende Subscriptions erhalten

            mqtt::will_options will(
                config.mqttTopicPrefix + "status",
                config.mqttLwtPayload,
                MQTT_QOS,
                true);
            connOpts.set_will(will);

            if (!config.mqttUsername.empty())
            {
                connOpts.set_user_name(config.mqttUsername);
                connOpts.set_password(config.mqttPassword);
            }

            // --- TLS aktivieren, falls konfiguriert ---
            mqtt::ssl_options sslOpts;
            if (config.mqttUseTLS)
            {
                log("MQTT TLS (Reconnect) aktiviert.");

                sslOpts.set_enable_server_cert_auth(!config.mqttInsecure);

                if (!config.mqttCAFile.empty())
                {
                    sslOpts.set_trust_store(config.mqttCAFile);
                    log("CA-Datei: " + config.mqttCAFile);
                }

                if (!config.mqttCertFile.empty())
                {
                    sslOpts.set_key_store(config.mqttCertFile);
                    log("Client-Zertifikat: " + config.mqttCertFile);
                }

                if (!config.mqttKeyFile.empty())
                {
                    sslOpts.set_private_key(config.mqttKeyFile);
                    log("Client-Key: " + config.mqttKeyFile);
                }

                if (config.mqttInsecure)
                    log("⚠️ TLS: Zertifikatprüfung deaktiviert (insecure)");

                connOpts.set_ssl(sslOpts);
            }

            mqttClient->connect(connOpts)->wait();
            log("MQTT-Reconnect erfolgreich");

            mqttClient->set_callback(mqttCb);
            mqttClient->publish(config.mqttTopicPrefix + "status",
                                config.mqttOnlinePayload.c_str(),
                                MQTT_QOS, true);
            mqttClient->subscribe(config.mqttTopicPrefix + "write/#", MQTT_QOS)->wait();
            mqttClient->subscribe(config.mqttTopicPrefix + "read/#", MQTT_QOS)->wait();
            mqttClient->subscribe(config.mqttTopicPrefix + "stats/reset", MQTT_QOS)->wait();

            break; // Erfolgreich verbunden, Schleife verlassen
        }
        catch (const mqtt::exception &e)
        {
            log("MQTT-Reconnect fehlgeschlagen: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    mqttConnecting = false;
}

bool init_modbus()
{
    rtu_ctx = modbus_new_rtu(config.serialPort.c_str(), config.baudrate, config.parity, config.dataBits, config.stopBits);
    if (!rtu_ctx)
    {
        log("Fehler beim Öffnen der seriellen Schnittstelle: " + std::string(modbus_strerror(errno)));
        return false;
    }
    modbus_set_slave(rtu_ctx, config.modbusSlaveId);
    modbus_set_response_timeout(rtu_ctx, MODBUS_RTU_TIMEOUT_SEC, MODBUS_RTU_TIMEOUT_USEC);
    if (modbus_connect(rtu_ctx) == -1)
    {
        log("Fehler beim Verbinden über RTU: " + std::string(modbus_strerror(errno)));
        modbus_free(rtu_ctx);
        return false;
    }
    log("Modbus RTU initialisiert");
    return true;
}

/**
 * Baut eine vollständige Modbus-TCP-Antwort aus MBAP-Header + RTU-Response.
 *
 * @param request_mbap  Pointer auf Original-TCP-Anfrage (mind. 7 Bytes)
 * @param rtu_response  RTU-Antwort, beginnend mit Slave-ID (also response[0])
 * @param rtu_len       Länge der RTU-Antwort
 * @return              Vektor mit vollständigem TCP-Antwortpaket
 */
std::vector<uint8_t> build_tcp_response(const uint8_t *request_mbap, const uint8_t *rtu_response, int rtu_len)
{
    std::vector<uint8_t> tcp_response;

    // MBAP Header (Transaction ID + Protocol ID + Length + Unit ID)
    tcp_response.insert(tcp_response.end(), request_mbap, request_mbap + 4); // TID + PID

    uint16_t pdu_len = rtu_len - 1;                        // ohne Slave-ID
    tcp_response.push_back(0x00);                          // High-Byte Länge
    tcp_response.push_back(static_cast<uint8_t>(pdu_len)); // Low-Byte Länge

    tcp_response.push_back(rtu_response[0]); // Unit ID (Slave ID)

    // PDU (ohne Slave-ID)
    tcp_response.insert(tcp_response.end(), rtu_response + 1, rtu_response + rtu_len);

    return tcp_response;
}

void handle_client(int client_sock, modbus_t *rtu_ctx)
{
    log("Neuer Client verbunden");

    modbus_t *tcp_ctx = modbus_new_tcp(NULL, 0);
    modbus_set_socket(tcp_ctx, client_sock);

    constexpr size_t MAX_ADU_LENGTH = MODBUS_TCP_MAX_ADU_LENGTH;
    uint8_t query[MAX_ADU_LENGTH];

    while (run)
    {
        int query_len = modbus_receive(tcp_ctx, query);
        if (query_len > 7)
        {
            total_tcp_requests++;

            // --- MBAP-Header extrahieren ---
            uint16_t transaction_id = (query[0] << 8) | query[1];
            uint16_t protocol_id = (query[2] << 8) | query[3];
            uint8_t unit_id = query[6];

            uint8_t *pdu = query + 7;
            int pdu_len = query_len - 7;

            modbus_set_slave(rtu_ctx, unit_id);
            uint8_t function_code = pdu[0];

            // --- Diagnose lokal behandeln ---
            if (function_code == 0x08 && pdu_len >= 5)
            {
                uint16_t subfunction = (pdu[1] << 8) | pdu[2];
                log("→ Modbus Diagnose-Funktion erkannt (Subfunction: 0x" + int_to_hex(subfunction, 4) + ")");

                switch (subfunction)
                {
                case 0x0000: // Echo-Test
                {
                    log("→ Lokale Antwort auf Echo-Test");
                    std::vector<uint8_t> resp(query, query + 7);
                    resp.insert(resp.end(), pdu, pdu + 5);
                    send(client_sock, resp.data(), resp.size(), 0);
                    continue;
                }
                case 0x000A: // Zähler zurücksetzen
                {
                    log("→ Zähler werden zurückgesetzt");
                    successful_rtu_requests = 0;
                    failed_rtu_requests = 0;
                    total_tcp_requests = 0;
                    total_errors = 0;

                    std::vector<uint8_t> resp(query, query + 7);
                    resp.insert(resp.end(), pdu, pdu + 5);
                    send(client_sock, resp.data(), resp.size(), 0);
                    continue;
                }
                case 0x000B: // TCP-Requests zurückgeben
                {
                    uint16_t count = static_cast<uint16_t>(total_tcp_requests.load());
                    std::vector<uint8_t> resp(query, query + 7);
                    resp.push_back(0x08);
                    resp.push_back(0x00);
                    resp.push_back(0x0B);
                    resp.push_back(count >> 8);
                    resp.push_back(count & 0xFF);
                    send(client_sock, resp.data(), resp.size(), 0);
                    continue;
                }
                case 0x000C: // Fehler zurückgeben
                {
                    uint16_t count = static_cast<uint16_t>(total_errors.load());
                    std::vector<uint8_t> resp(query, query + 7);
                    resp.push_back(0x08);
                    resp.push_back(0x00);
                    resp.push_back(0x0C);
                    resp.push_back(count >> 8);
                    resp.push_back(count & 0xFF);
                    send(client_sock, resp.data(), resp.size(), 0);
                    continue;
                }
                default:
                    log("→ Subfunktion wird nicht lokal unterstützt → Weiterleitung an RTU");
                    break;
                }
            }

            // --- RTU-Anfrage vorbereiten ---
            uint8_t full_request[MODBUS_RTU_MAX_ADU_LENGTH];
            full_request[0] = unit_id;
            memcpy(&full_request[1], pdu, pdu_len);
            int full_len = pdu_len + 1;

            if (config.debugHexDump)
                dump_hex("→ RTU TX: ", full_request, full_len);

            int sent = modbus_send_raw_request(rtu_ctx, full_request, full_len);
            if (sent == -1)
            {
                total_errors++;
                failed_rtu_requests++;
                log("Fehler beim Senden an RTU: " + std::string(modbus_strerror(errno)));
                break;
            }

            // --- RTU-Antwort lesen ---
            uint8_t response[MODBUS_RTU_MAX_ADU_LENGTH];
            int res_len = modbus_receive_confirmation(rtu_ctx, response);
            if (res_len == -1)
            {
                failed_rtu_requests++;
                total_errors++;
                log("Fehler beim Empfangen von RTU: " + std::string(modbus_strerror(errno)));
                break;
            }

            successful_rtu_requests++;

            if (config.debugHexDump)
                dump_hex("← RTU RX: ", response, res_len);

            // --- MBAP-Antwort konstruieren ---
            uint16_t tcp_len = res_len + 1;
            std::vector<uint8_t> tcp_response;
            tcp_response.reserve(tcp_len + 7);

            tcp_response.push_back(transaction_id >> 8);
            tcp_response.push_back(transaction_id & 0xFF);
            tcp_response.push_back(protocol_id >> 8);
            tcp_response.push_back(protocol_id & 0xFF);
            tcp_response.push_back(tcp_len >> 8);
            tcp_response.push_back(tcp_len & 0xFF);
            tcp_response.push_back(unit_id);
            tcp_response.insert(tcp_response.end(), response, response + res_len);

            ssize_t sent_tcp = send(client_sock, tcp_response.data(), tcp_response.size(), 0);
            if (sent_tcp < 0)
            {
                total_errors++;
                log("Fehler beim Senden an TCP-Client: " + std::string(strerror(errno)));
                break;
            }
        }
        else if (query_len == -1 && errno != ECONNRESET)
        {
            total_errors++;
            log("Fehler bei modbus_receive: " + std::string(modbus_strerror(errno)));
            break;
        }
        else
        {
            break;
        }
    }

    modbus_close(tcp_ctx);
    modbus_free(tcp_ctx);
    close(client_sock);
    log("Client getrennt");
}

bool start_tcp_server()
{
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        log("Socket-Fehler: " + std::string(strerror(errno)));
        return false;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config.tcpPort);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        log("Fehler beim Binden an Port " + std::to_string(config.tcpPort) + ": " + std::string(strerror(errno)));
        close(server_sock);
        return false;
    }

    if (listen(server_sock, TCP_BACKLOG) < 0)
    {
        log("Listen-Fehler: " + std::string(strerror(errno)));
        close(server_sock);
        return false;
    }

    log("Gateway hört auf Port " + std::to_string(config.tcpPort));
    return true;
}

void accept_clients()
{
    while (run)
    {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr *)&client_addr, &client_len);
        if (client_sock < 0)
        {
            if (run)
                log("Fehler beim Akzeptieren: " + std::string(strerror(errno)));
            continue;
        }

        // ⏱ Timeout für Empfang und Senden setzen
        struct timeval timeout;
        timeout.tv_sec = 60; // 60 Sekunden
        timeout.tv_usec = 0;
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        client_threads.emplace_back(handle_client, client_sock, rtu_ctx);
    }
}

void shutdown_gateway()
{
    for (auto &t : client_threads)
        if (t.joinable())
            t.join();

    if (stats_thread.joinable())
    {
        stats_thread.join();
    }

    if (mqttClient && mqttClient->is_connected())
    {
        mqttClient->publish(config.mqttTopicPrefix + "status", config.mqttLwtPayload.c_str(), MQTT_QOS, true);
        mqttClient->disconnect()->wait();
    }

    if (rtu_ctx)
    {
        modbus_close(rtu_ctx);
        modbus_free(rtu_ctx);
    }

    if (server_sock != -1)
        close(server_sock);

    log("Gateway beendet");
    logFile.close();
}

bool init_config(int argc, char *argv[], bool &dry_run)
{
    const char *config_path = "/etc/modbus_gateway.ini";

    if (argc > 1)
    {
        std::string arg1(argv[1]);

        if (arg1 == "--dry-run" || arg1 == "--validate-only")
        {
            dry_run = true;
        }
        else
        {
            config_path = argv[1];
        }
    }

    if (ini_parse(config_path, config_handler, &config) < 0)
    {
        log("Konnte Konfiguration nicht laden, nutze Defaults.");
        return false;
    }

    // TLS-Konfigurationsprüfung
    if (config.mqttUseTLS)
    {
        log("TLS ist aktiviert → MQTT-Verbindung erfolgt verschlüsselt.");

        if (config.mqttCAFile.empty())
            log("⚠️  Warnung: Kein CA-Zertifikat angegeben (cafile)");

        if ((config.mqttCertFile.empty() || config.mqttKeyFile.empty()) && !config.mqttInsecure)
            log("⚠️  Warnung: Client-Zertifikat und Key fehlen – Verbindung könnte scheitern.");

        if (!config.mqttCAFile.empty() && access(config.mqttCAFile.c_str(), R_OK) != 0)
            log("⚠️  Fehler: CA-Datei nicht lesbar: " + config.mqttCAFile);

        if (!config.mqttCertFile.empty() && access(config.mqttCertFile.c_str(), R_OK) != 0)
            log("⚠️  Fehler: Client-Zertifikat nicht lesbar: " + config.mqttCertFile);

        if (!config.mqttKeyFile.empty() && access(config.mqttKeyFile.c_str(), R_OK) != 0)
            log("⚠️  Fehler: Client-Key nicht lesbar: " + config.mqttKeyFile);

        if (config.mqttInsecure)
            log("⚠️  TLS: Zertifikatprüfung wurde explizit deaktiviert (insecure = true)");
    }

    return true;
}

int main(int argc, char *argv[])
{
    bool dry_run = false;
    if (argc > 1 && std::string(argv[1]) == "--version")
    {
        std::cout << "modbus_gateway Version 1.0.0" << std::endl;
        return 0;
    }

    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))
    {
        std::cout << "Modbus TCP ↔ RTU + MQTT Gateway\n\n";
        std::cout << "Aufruf:\n";
        std::cout << "  ./modbus_gateway [config.ini] [Optionen]\n\n";
        std::cout << "Optionen:\n";
        std::cout << "  --help, -h            Diese Hilfe anzeigen\n";
        std::cout << "  --version             Versionsnummer anzeigen\n";
        std::cout << "  --dry-run             Nur Konfiguration prüfen, keine Verbindungen aufbauen\n";
        std::cout << "  --validate-only       Alias für --dry-run\n\n";
        std::cout << "Beispiel:\n";
        std::cout << "  ./modbus_gateway /etc/modbus_gateway.ini\n";
        std::cout << "  ./modbus_gateway /etc/modbus_gateway.ini --dry-run\n";
        return 0;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    logFile.open(config.logFilePath, std::ios::app);
    log("Starte Gateway...");

    if (!init_config(argc, argv, dry_run))
        return 1;
    if (!dry_run && !init_mqtt())
        return 1;
    if (!dry_run && !init_modbus())
        return 1;
    if (!dry_run && !start_tcp_server())
        return 1;

    // ⏱ Statistik-Thread starten
    std::thread stats_thread;
    if (config.mqttStatsInterval > 0 && !dry_run)
    {
        stats_thread = std::thread(publish_stats);
    }

    // 🧩 Hauptverarbeitung starten
    if (dry_run)
    {
        log("Testlauf (--dry-run / --validate-only): Konfiguration geprüft, keine Verbindung hergestellt.");
        std::cout << "Validierung erfolgreich: Konfiguration wurde geladen.\n";
        std::cout << "MQTT-Server: " << config.mqttBroker << "\n";
        std::cout << "Modbus-Gerät: " << config.serialPort << " @ " << config.baudrate << "\n";
        std::cout << "TCP-Port: " << config.tcpPort << "\n";
        return 0;
    }

    accept_clients(); // Wird nur im echten Betrieb aufgerufen

    // 🧹 Beenden & Cleanup
    shutdown_gateway();

    return 0;
}
