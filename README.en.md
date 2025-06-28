Foreword

🔗 [📄 Deutsch (README.md)](README.md)

﻿Foreword  
🔗 📄 English (README.en.md)  
This software is in a functioning state.  
I am still expanding its functionality.  
If there is interest, please feel free to provide comments and suggestions for improvement.  
Modbus Gateway – Documentation  
1. Introduction and Overview  
This project provides a robust and flexible gateway that bridges Modbus TCP, Modbus RTU, and MQTT. It is ideally suited for the integration of classic industrial systems (e.g., heat pumps, energy meters, controllers) into modern IoT or smart home environments.  
Key features:  
- Mediation between Modbus TCP ↔ RTU  
- Extension to include MQTT communication  
- Support for Coils, Discrete Inputs, Input Registers, Holding Registers  
- Multiple read operations via MQTT  
- TLS encryption for MQTT (optional)  
- Configurable via .ini file  
- Log file output and MQTT log  
- Full support for Modbus diagnostic functions (0x08)  
- Statistics and diagnostic data Software system for use on devices such as the Raspberry Pi 5 (aarch64), which bidirectionally bridges Modbus TCP and Modbus RTU, provides MQTT interfaces, and implements monitoring, logging, and diagnostic functions according to the Modbus standard.  
2.1. Main functions  
🔁 Modbus TCP ↔ RTU Gateway  
Forwarding TCP requests via a serial RTU interface to one or more Modbus slaves.  

☁️ MQTT connection  
Direct access to Modbus registers (read/write) via MQTT topics including multiple operations.  

📊 Statistics & diagnostic data  
Publication of operational data via MQTT and return through diagnostic sub-functions.  

📁 INI-based configuration  
All operating parameters are configured via an easily readable .ini file.  

🛡️ TLS encryption  
MQTT connections can be optionally secured via TLS.  

📡 Watchdog and keepalive mechanisms  
Support for MQTT keepalive, automatic reconnect, TCP timeouts.  
2.2. In (Modbus TCP Clients)  
handle_client()    Forwards TCP requests to RTU, processes responses, implements diagnostics  
MQTTCallback    Processes incoming MQTT messages (reading/writing registers & coils)  
publish_stats()    Periodically published metrics (e.g., error counters, uptime, throughput)  
shutdown_gateway()    Cleanly terminates all threads, MQTT and RTU connection  
dump_hex()    Optional logging of Modbus telegrams (debugging)  
2.4. Data Flow Overview  
Modbus TCP Request  
↓  
handle_client()  
↓  
modbus_send_raw_request() → RTU  
↓  
modbus_receive_confirmation()  
↓  
build_tcp_response()  
↓  
TCP response to client  
MQTT Topic:  
- write/123 → write_register(123)  
- read/123 → read_register(123)  
- read_multi/100:3 → reads 3 registers from address 100  
↓  
MQTTCallback::message_arrived()  
↓  
modbus_read_/write_() → RTU  
↓  
Response via MQTT publish()  
2.5. Multiple Interfaces  
Supports multiple parallel TCP clients  
Supports simultaneous MQTT operation  
All Parity: N, E, or O  
Data bits: usually 8  
Stop bits: 1 or 2  
[tcp]  
Parameter  Description  
port  TCP port on which the gateway receives Modbus TCP requests (Default: 502)  
[modbus]  
Parameter  Meaning  
slave_id  Default slave ID for MQTT requests  
[mqtt]  
Parameter  Meaning  
broker  MQTT broker URI, e.g. tcp://192.168.0.82:1883  
client_id  MQTT Client ID  
topic_prefix  Prefix for all topics, e.g. modbus/  
username  Optional: Username  
password  Optional: Password  
keepalive  Keepalive in seconds  
lwt  Payload for Last Will (offline)  
online  Payload for successful connect (online)  
use_tls  1 = enable TLS, 0 = unencrypted  
ca_file  Path to CA file  
cert_file  Path to client certificate  
key_file  Path to private key  
insecure  1 = disable certificate verification  
stats_interval  Seconds interval for MQTT publication of statistics data  
[log]  
Parameter  Meaning  
file  Log file path  
hex_dump  1 = log Modbus packets as hex dump ts	modbus/read_discrete_multi/<start>:<count>	Read multiple digital inputs
Reset statistics	modbus/stats/reset	Resets all statistic counters
Retrieve statistics	modbus/command/stats	Response appears in modbus/stats
4.2. MQTT Examples
Example 1: Read single register
Publish:
mosquitto_pub -t modbus/read/100 -n
Response (Subscribe to modbus/read/100):
431
Example 2: Write single register
Publish:
mosquitto_pub -t modbus/write/100 -m "123"
Log output:
Modbus register 100 set to 123
Example 3: Read multiple registers
Publish:
mosquitto_pub -t modbus/read_multi/100:4 -n
Response (Subscribe to modbus/read_multi/100:4):
431,0,65535,22
Example 4: Set coil
mosquitto_pub -t modbus/write_coil/25 -m "1"
Example 5: Read discrete input
mosquitto_pub -t modbus/read_discrete/12 -n
Response:
0
4.3. Example: Statistics query
Query:
mosquitto_pub -t modbus/command/stats -n
Response:
Statistics (on-demand): TCP=233, RTU=233, Errors=2, start_tcp_server() Opens TCP socket and waits for incoming connections  
accept_clients() Accepts new clients and starts threads for processing  
handle_client() Processes individual Modbus-TCP requests, forwarding to RTU, response  
publish_stats() Periodic publication of statistical data via MQTT  
reconnect_mqtt() Logic for restoring a broken MQTT connection  
dump_hex() Optional hex dump for debugging purposes  
5.2. Data flow overview  
    +----------------+  
    |    MQTT Client  |  
    +----------------+  
            |  
            v  
      [MQTTCallback]  
            |  
+-----------+------------+  
|                        |  
v                        v  
Modbus-Write          Modbus-Read  
(via RTU)              (via RTU)  
|                        |  
+-----------+------------+  
|  
v  
Optional: Response via MQTT  
  
    +--------------------+  
    |   TCP Client (Modbus) |  
    +--------------------+  
            |  
            v  
   [handle_client()] classical INI file, typically located at:
 /etc/modbus_gateway.ini
This file is parsed at program startup and affects all interfaces (Modbus, MQTT, Logging, etc.). Alternatively, the path can be passed:
./modbus_gateway /path/to/configuration.ini
6.1. Sections and Keys
[serial] – Modbus RTU interface
Key	Type	Description	Example
port	String	Serial interface	/dev/ttyAMA0
baudrate	int	Baud rate	9600
parity	char	Parity: N, E, O	N
databits	int	Data bits (usually 8)	8
stopbits	int	Stop bits (1 or 2)	1
[tcp] – Modbus TCP server
Key	Type	Description	Example
port	int	TCP port on which the server listens	502
[modbus] – Modbus-specific options
Key	Type	Description	Example
slave_id	int	Default slave ID (overridden for TCP requests)	1
[mqtt] – MQTT connection
Key	Type	Description	Example
broker	String	MQTT broker 0  
baudrate = 9600  
parity = N  
databits = 8  
stopbits = 1  
[tcp]  
port = 502  
[modbus]  
slave_id = 1  
[mqtt]  
broker = tcp://192.168.0.82:1883  
client_id = modbus_gateway  
username = mqttuser  
password = secret  
topic_prefix = modbus/  
keepalive = 30  
lwt = offline  
online = online  
use_tls = 0  
stats_interval = 60  
[log]  
file = /var/log/modbus_gateway.log  
hex_dump = 1  
7. MQTT Integration – Topics, Payloads, and Examples  
The gateway uses MQTT for bidirectional communication: control, status messages, statistics, and error reports. The MQTT functionality is completely optional and can be disabled through configuration.  
7.1. Topic Overview  
Topic	Type	Description  
modbus/status	LWT	Connection status of the gateway (online / offline)  
modbus/stats	Status	Statistics as JSON (optionally cyclic)  
modbus/log	Log	Log messages from the gateway  
modbus/read/<type>/<address>	Command	Read access to registers  
modbus/write/<type>/<address>	Command	Write access to ad:  
[1, 2, 3, 4]  
Response (e.g. in modbus/response/coil)  
{  
"1": 1,  
"2": 0,  
"3": 1,  
"4": 1  
}  
7.6. Request Statistics  
Topic:  
modbus/command/stats  
Payload: (arbitrary or empty)  
Response (immediate status message to modbus/stats):  
{  
"tcp_requests": 234,  
"rtu_success": 231,  
"rtu_failures": 3,  
"dropped": 0,  
"echo_replies": 7  
}  
7.7. Logging  
Topic:  
modbus/log  
Example Payload:  
[2025-06-26 19:33:00] → RTU TX: 01 03 00 00 00 02 65 CB  
Activation via:  
[log]  
hex_dump = 1  
8. Modbus TCP ↔ RTU – Protocol Flow and Implementation  
The gateway acts as a protocol translator: it receives Modbus TCP requests from clients, converts them to Modbus RTU, forwards them via RS485, and responds with correctly constructed Modbus TCP packets.  
8.1. Process of a TCP Request  
TCP request received  

    The gateway listens on the configured port (Default: 502).  

    modbus_receive() extracts MBAP header and PDU.  

Analyze MBAP header  

    Transaction ID, receive
0x0000 Loopback test (Echo) Response generated locally
0x000A Reset statistics counter Set counter to 0
0x000B Return TCP request counter Response generated locally
0x000C Return error counter Response generated locally
other Not recognized Request forwarded to RTU
8.3 Hex Debug Log
The function dump_hex() generates a log in hexadecimal form:
dump_hex("→ RTU TX: ", data, len);
Example output (in modbus/log):
→ RTU TX: 01 03 00 00 00 02 65 CB
← RTU RX: 01 03 04 00 0A 00 0B 75 D3
8.4 Connection Parameters
Configurable in the INI file:
[serial]
port = /dev/ttyAMA0
baudrate = 9600
parity = N
databits = 8
stopbits = 1
[modbus]
slave_id = 1
8.5 RTU Timing Behavior
modbus_set_response_timeout(ctx, 1, 0);  // 1 second timeout
Reliable response times heavily depend on the RTU slave. Typical roundtrip times: 5 – 50 ms.
9. System Integration: systemd, Autostart & Logging
So that the gateway runs automatically at system startup and cleanly status modbus_gateway  
Log display (Live):  
journalctl -u modbus_gateway -f  
9.4. Logrotate Support  
Path: /etc/logrotate.d/modbus_gateway  
/var/log/modbus_gateway.log {  
rotate 7  
daily  
compress  
delaycompress  
missingok  
notifempty  
copytruncate  
}  
Activation (manual testing):  
sudo logrotate -f /etc/logrotate.d/modbus_gateway  
9.5. Binary & Paths  
By default, the application is compiled to:  
/usr/local/bin/modbus_gateway  
Configuration:  
/etc/modbus_gateway.ini  
10. Compilation, Deployment, and Makefile  
The application is compiled with g++ under Linux. For use on a Raspberry Pi (e.g., Pi 5 with Raspberry Pi OS 64 Bit) it can be built natively or via cross-compile.  
10.1. Makefile for native compilation (e.g., on the Raspberry Pi)  
Path: Makefile  
TARGET = modbus_gateway  
SRC = modbus_gateway.cpp ini.c  
CXXFLAGS = -Wall -O2 -std=c++17 -I/usr/local/include  
LDFLAGS = -L/usr/local/lib -lmodbus -lpaho-mqttpp3 -lpaho-mqtt3as -lpthread  
.PHONY: all clean _gateway.ini.example  
├── README.md  
10.4. Release Archive (optional)  
Creating an archive:  
tar czf modbus_gateway.tar.gz modbus_gateway/  
Deploying on GitHub or copying locally via scp to the target Raspberry Pi:  
scp modbus_gateway.tar.gz pi@raspberrypi:/home/pi/  
11. Examples of MQTT and Modbus TCP Integration  
This chapter presents practical examples of using the gateway with MQTT and Modbus TCP. The target audience is integrators who use the gateway, e.g., with Node-RED, Home Assistant, or their own automations.  
11.1. MQTT Examples  
📥 Writing a Holding Register  
mosquitto_pub -t modbus/write/100 -m 42  
➡️ Sets Modbus Holding Register 100 to the value 42.  
📤 Reading a Holding Register  
mosquitto_pub -t modbus/read/100 -n  
📬 The gateway responds with:  
Topic: modbus/read/100  
Payload: 42  
📥 Writing a Coil (Switch Command)  
mosquitto_pub -t modbus/write_coil/12 -m 1  
➡️ Turns Coil 12 ON.  
📤 Reading a Coil  
mosquitto_pub -t se 100

-c 2: 2 Read register

127.0.0.1: Gateway address  
Response (if successful):  
[1]: 42  
[2]: 81  
11.3. Node-RED Integration (Example)  
Subscribe Node:  
Topic: modbus/read/100  

Receives: Register value  
Inject Node:  
Topic: modbus/write/100  

Payload: 123  
➡️ This allows Node-RED to set and read Modbus values – ideal for automation.  
12. Error handling and debugging  
This section describes how errors in the gateway can be detected, logged, and ideally resolved. It helps both developers and integrators to analyze problems more quickly.  
12.1. Overview of Error Sources  
Error source     Typical cause            Suggested solution  
Modbus RTU Timeout  No device responds to the request  Check baud rate, parity, slave ID  
MQTT Error         Broker unreachable or certificate error   Check MQTT configuration  
TCP Error          TCP client disconnects the connection     Normal at connection end, log if necessary  
INI Error          Syntax error or path not present   Check path & format set to 0  
0x000B Return number of TCP requests 2-byte response with current count  
0x000C Return number of errors 2-byte response with error count  
These are ideal for external monitoring software.  
12.5. MQTT Statistics and Diagnostic Data  
All counter values are included in the topic modbus/stats, e.g.:  
{  
"total_tcp_requests": 156,  
"total_rtu_requests": 154,  
"total_errors": 2,  
"rtu_success": 154,  
"rtu_failures": 0,  
"dropped": 0,  
"echo_replies": 1,  
"last_success_timestamp": 1719414809  
}  
Allows for graphical evaluations (e.g., in Grafana or Node-RED Dashboard).  
12.6. Increased Fault Tolerance  
The gateway does not immediately drop TCP connections on every error, but allows for, e.g.:  
multiple faulty RTU communications in succession  

empty received TCP packets → are ignored  

connection drops → clean cleanup in shutdown_gateway()  
13. Deployment and System Integration  
This chapter describes how the Modbus gateway is deployed on a target system. StandardError=journal
[Install]
WantedBy=multi-user.target
13.4. Activating the service
sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable modbus_gateway
sudo systemctl start modbus_gateway
13.5. Checking status
systemctl status modbus_gateway
journalctl -u modbus_gateway -f
13.6. Deployment folder structure
Recommended:
/home/benno/modbus_gateway/
├── modbus_gateway            # Executable
├── modbus_gateway.cpp        # Source code
├── ini.c / ini.h             # INI parser
├── Makefile
├── README.md
├── systemd/
│   └── modbus_gateway.service
└── config/
└── modbus_gateway.ini
13.7. Tips
Logs default to /var/log/modbus_gateway.log

The log file can be rotated via logrotate

MQTT errors and reconnections are handled automatically
14. INI configuration format and parameter reference
The central configuration of the gateway is done via an INI file. This allows you to set all relevant parameters r	Type	Description  
port	Number	TCP port for Modbus-TCP server  
[modbus]  
Parameter	Type	Description  
slave_id	Number	Default Slave ID (only for MQTT access)  
[mqtt]  
Parameter	Type	Description  
broker	String	MQTT broker URI (e.g. tcp://localhost:1883)  
client_id	String	Client ID of the MQTT client  
topic_prefix	String	Prefix for all MQTT topics (e.g. modbus/)  
username	String	MQTT username  
password	String	MQTT password  
keepalive	Number	Keepalive in seconds  
lwt	String	Last Will Payload (e.g. "offline")  
online	String	Online payload at startup (e.g. "online")  
use_tls	0/1	Enable TLS (1 = yes)  
ca_file	String	Path to CA file  
cert_file	String	Client certificate file  
key_file	String	Client key file  
insecure	0/1	Disable TLS certificate validation (unsafe!)  
stats_interval	Number	Interval in seconds for publishing  
[log]  
Parameter	Type	Description  
file	String	Path to log file  
hex_dump	0/1	Enable hexdump for Modbus raw data (debug)  
15. Examples for : Read Holding Register 0 (40001) from Slave 3  
Topic (Request):  
modbus/read/holding/3/0  
Response:  
modbus/response/holding/3/0  
Payload:  
[1234]  
Example 4: Read Input Register 30010 to 30014 from Slave 2  
Topic (Request):  
modbus/read/input/2/9,5  
Response:  
modbus/response/input/2/9  
Payload:  
[318, 319, 320, 321, 322]  
15.3. Receive diagnostic data via MQTT  
If mqttStatsInterval > 0 is set in the INI file, the gateway sends statistics periodically:  
Topic:  
modbus/stats  
Payload:  
{  
"tcp_requests": 123,  
"rtu_success": 120,  
"rtu_failures": 3,  
"dropped": 0,  
"echo_replies": 5  
}  
15.4. Manually request statistics via command  
Topic:  
modbus/command/stats  
Payload: (optional, e.g., "now")  
Response will again be on modbus/stats.  
16. Examples for Modbus TCP communication  
This chapter shows how an external Modbus TCP client can connect to the gateway to access the underlying RTU devices via the TCP interface. The gateway acts as a proxy. 1. 
16.4. Additional Clients
Other clients such as:
QModMaster (Windows/Qt)

Node-RED with Modbus Nodes

Python (pymodbus)

CODESYS or TIA Portal 
can work directly with the gateway without special adjustments, as long as they support Modbus TCP. 
17. Logging and Diagnostics
17.1. File Logging
All events, errors, status changes are logged to the file:
 /var/log/modbus_gateway.log
Configurable via:
[log]
file = /var/log/modbus_gateway.log
17.2. MQTT Logging
Each log line is additionally published on the topic:
modbus/log 
provided that MQTT is connected. 
17.3. Enable Hex Dump
For analyzing RTU communication, a hex dump can be enabled:
[log]
hex_dump = 1
Example output:
→ RTU TX: 01 03 00 6B 00 03 76 87
← RTU RX: 01 03 06 02 2B 00 00 00 64 39 80
18. systemd Integration, Auto Start, and User Management
To load the gateway automatically at system startup and run it as a service in the background, a your modbus_gateway  
18.5. Restart/Stop  
sudo systemctl restart modbus_gateway  
sudo systemctl stop modbus_gateway  
19. Security and TLS  
19.1. Enable TLS for MQTT  
The connection to the MQTT broker can be encrypted with TLS. Set the following parameters in the INI file:  
[mqtt]  
use_tls = 1  
ca_file = /etc/ssl/certs/ca.crt  
cert_file = /etc/ssl/certs/client.crt  
key_file = /etc/ssl/private/client.key  
insecure = 0  
use_tls = 1 enables TLS  

insecure = 1 disables server certificate verification (not recommended!)  
19.2. Test MQTT over TLS  
If your broker is reachable at mqtts://broker.local:8883, for example:  
broker = ssl://broker.local:8883  
Make sure that the port and protocol (ssl:// instead of tcp://) are correct.  
20. Statistics and Monitoring  
20.1. Automatic Statistics (periodically)  
Every mqttStatsInterval seconds, the following topic is published:  
modbus/stats  
Example content:  
{  
"total_tcp_requests": 1432,  
"total_rtu_requests": no further reconnect thread started.

Successful reconnect leads to the rebuilding of all subscriptions and publication of the online status.  
21.3. Signal Handling  
The following signals are handled:  
SIGINT (e.g., Ctrl+C)

SIGTERM (e.g., via kill)  
These lead to a clean shutdown:  
All client threads are terminated.

MQTT is unsubscribed and the offline status is sent.

Modbus is closed cleanly.

Log file is terminated.  
21.4. TCP Timeout  
Each connected client receives a timeout:  
timeout.tv_sec = 60;  
setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  
setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));  
Connections without activity will thus be automatically disconnected after 60 seconds.  
22. Logging and Diagnosis  
22.1. Log File  
Default: /var/log/modbus_gateway.log  
Path configurable via INI:  
[log]  
file = /var/log/modbus_gateway.log  
Format:  
YYYY-MM-DD HH:MM:SS - ice  
23.6. MQTT Topics Overview (compact)  
Purpose	Topic	Example  
Status	modbus/status	online / offline  
Logging	modbus/log	Log entries  
Statistics	modbus/stats	JSON with counter values  
Retrieve statistics	modbus/command/stats	empty  
Reset statistics	modbus/stats/reset	empty  
Read Holding Register	modbus/read/1234	Response: 42  
Write Holding Register	modbus/write/1234	Message: 42  
Read Coil	modbus/read_coil/1234	Response: 1  
Write Coil	modbus/write_coil/1234	Message: 0  
Read Input Register	modbus/read_input/1234	Response: 1042  
Read Discrete Input	modbus/read_discrete/1234	Response: 0  
Read Multiple Registers	modbus/read_multi/100:4	Response: 123,456,789,1011  
Read Multiple Coils	modbus/read_coil_multi/0:8	Response: 1,1,0,0,1,0,1,1  
23.7. Licenses / Dependencies  
libmodbus – https://libmodbus.org/  

Eclipse Paho MQTT C++ – https://www.eclipse.org/paho/  

inih (INI-Parser) – https://github.com/benhoyt/inih  
All third-party components are under liberal open-source licenses