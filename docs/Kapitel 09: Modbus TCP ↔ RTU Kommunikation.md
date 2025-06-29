---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

🔄 Kapitel 9: Modbus TCP ↔ RTU Kommunikation
📦 Protokollübersicht

Das Gateway verbindet zwei Protokollwelten:
Protokoll	Typ	Transport	Anwendung
Modbus TCP	Binär	TCP/IP (Port 502)	Von SCADA, SPS, Visualisierungen
Modbus RTU	Binär (ASCII optional)	RS485 / Serial	Kommunikation mit realen Slaves

Die zentrale Aufgabe: Weiterleitung von TCP-Anfragen an RTU-Geräte und Rückleitung der Antwort – vollständig protokollkonform.
📥 TCP-Eingang

Der TCP-Server horcht standardmäßig auf Port 502, kann aber per config.ini konfiguriert werden:

tcpPort = 502

Jeder neue Client wird in einem separaten Thread (std::thread) verarbeitet.
📤 RTU-Ausgang

Die RTU-Kommunikation basiert auf der libmodbus-Bibliothek und nutzt ein konfiguriertes Gerät, z. B.:

serialPort = /dev/ttyAMA0
baudrate = 19200
parity = N
databits = 8
stopbits = 1

Alle RTU-Zugriffe werden synchron (blocking) verarbeitet.
🔄 Ablauf der Anfrageverarbeitung

    Empfang der TCP-Anfrage

        Auslesen von MBAP-Header (Transaction-ID etc.)

    Extraktion der PDU (Protocol Data Unit)

        Funktionscode + Payload

    Zusammenbau der RTU-Anfrage

        Unit-ID wird vorangestellt

    Versand über Modbus RTU

        Mittels modbus_send_raw_request()

    Empfang der RTU-Antwort

        Über modbus_receive_confirmation()

    Rekonstruktion der MBAP-TCP-Antwort

        Wiederherstellung der Transaction-ID

    Antwort an TCP-Client

        Per send() an den verbundenen Socket

📋 Unterstützte Funktionscodes
FC	Beschreibung	Unterstützt
0x01	Read Coils	✅
0x02	Read Discrete Inputs	✅
0x03	Read Holding Registers	✅
0x04	Read Input Registers	✅
0x05	Write Single Coil	✅
0x06	Write Single Holding Register	✅
0x0F	Write Multiple Coils	✅
0x10	Write Multiple Holding Registers	✅
0x08	Diagnostics (lokal + transparent)	✅
🔎 Diagnosefunktionen

Das Gateway erkennt einige Modbus-Diagnosefunktionen (FC 0x08) lokal:
Subfunktion	Beschreibung	Gateway-Reaktion
0x0000	Echo Test (Loopback)	✅ Lokale Antwort
0x000A	Clear Counters	✅ Setzt Statistik zurück
0x000B	Return Bus Message Count	✅ Liefert TCP-Zähler
0x000C	Return Bus Communication Error Count	✅ Liefert Fehleranzahl
andere	Weiterleitung an RTU	🔁
🔧 Debug-Hex-Dump

Wenn debugHexDump = true in der .ini gesetzt ist, werden alle übertragenen Daten im Log als Hex-Dump ausgegeben:

debugHexDump = true

Beispielausgabe:

→ RTU TX:  01 03 00 64 00 01 85 DB
← RTU RX:  01 03 02 00 7B 79 84

🚨 Fehlerbehandlung

    Bei Fehlern von libmodbus erfolgt Logging + Fehlerzählerinkrement

    Verbindungsfehler am TCP werden protokolliert und führen zum Thread-Abbruch

⚙️ Multi-Threaded Verarbeitung

    Jeder TCP-Client hat einen eigenen Thread

    Gleichzeitige Verarbeitung mehrerer Anfragen möglich

    Zugriff auf modbus_t* für RTU erfolgt serialisiert


