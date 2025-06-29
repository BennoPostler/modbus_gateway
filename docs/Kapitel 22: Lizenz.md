---
title: Modbus Gateway – MQTT, TCP & RTU Integration für Raspberry Pi
layout: default
---

🏷️ Kapitel 22: Lizenz

Dieses Projekt folgt dem Prinzip der Offenheit, Transparenz und freien Weiterverwendung, orientiert an etablierten Open-Source-Praktiken. In diesem Kapitel findest du alle Lizenzinformationen für das Gateway selbst sowie alle verwendeten Bibliotheken.
📘 22.1 Projektlizenz (modbus_gateway)

Das Projekt modbus_gateway wird unter der folgenden Lizenz veröffentlicht:

MIT License

Copyright (c) 2025 Benno + OpenAI

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

🧩 22.2 Drittabhängigkeiten und Lizenzen
Bibliothek	Lizenztyp	Kompatibilität	Bemerkung
libmodbus	LGPLv2.1	✅ Ja	Dynamisch oder als Systempaket linkbar
Paho MQTT C	EPL/EDL	✅ Ja	Apache-kompatibel, permissiv
Paho MQTT C++	EPL/EDL	✅ Ja	Für C++ Wrapper um C-Bibliothek
ini.h/ini.c	zlib/libpng	✅ Ja	Kompakt, MIT-kompatibel
pthread / POSIX	libc/GPL	✅ Ja	Systembibliotheken, keine Lizenzbindung
🔄 22.3 Lizenzhinweise im Code

Alle Module, die Drittcode enthalten, beinhalten eine kurze Lizenzangabe am Anfang der Datei, z. B.:

// ini.h - Simple INI Parser
// License: zlib/libpng

Für eingebettete Bibliotheken wird empfohlen, auch die zugehörige LICENSE-Datei im Projekt mitzuführen.
📄 22.4 Lizenzkompatibilität und Weitergabe
Du darfst:

    das Projekt frei weitergeben und anpassen

    es in kommerziellen oder privaten Szenarien nutzen

    Binary-Distributionen anbieten

Du solltest:

    Lizenz- und Copyright-Hinweis beibehalten

    bei Forks oder Erweiterungen ebenfalls MIT-kompatibel lizenzieren

🧠 22.5 Lizenzwahl – warum MIT?

Die MIT-Lizenz wurde gewählt, weil sie:

    extrem verbreitet ist (GitHub, Open Source Foundation)

    mit praktisch allen Open-Source-Lizenzen kompatibel ist

    maximale Freiheit für Entwickler und Integratoren bietet

