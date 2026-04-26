# Multi-Client TCP/UDP Chat Server

### Windows Version | C++ | Winsock2 | Windows Threads

---

## File Structure

```
tcp_server_windows/
├── server.cpp          ← Main server (TCP + UDP)
├── client.cpp          ← TCP client
├── udp_client.cpp      ← UDP client
├── README.md           ← Ye file
└── .vscode/
    ├── tasks.json               ← VS Code build tasks
    └── c_cpp_properties.json    ← IntelliSense config
```

---

## Step 1 — MinGW Install Karo (Compiler)

Windows pe C++ compile karne ke liye **MinGW** chahiye.

1. Ye link pe jao:
   **https://www.msys2.org/**

2. Installer download karo aur install karo
   (default path: `C:\msys64`)

3. Install hone ke baad **MSYS2 terminal** khulega
   Ye command run karo:

   ```
   pacman -S mingw-w64-x86_64-gcc
   ```

4. Windows PATH mein add karo:
   - Search: "Environment Variables"
   - `Path` → Edit → New
   - Ye dalo: `C:\msys64\mingw64\bin`
   - OK → OK

5. Verify karo — **Command Prompt** mein:
   ```
   g++ --version
   ```
   Version number aaye toh install sahi hua!

---

## Step 2 — VS Code Setup

1. VS Code install karo (agar nahi hai):
   **https://code.visualstudio.com/**

2. Ye extension install karo:
   - `C/C++` by Microsoft
   - `Code Runner` (optional — handy hai)

3. Folder kholo:
   ```
   File → Open Folder → tcp_server_windows
   ```

---

## Step 3 — Build Karo

### Option A — VS Code se (easy):

```
Ctrl + Shift + B
```

"Build All Files" select karo → Enter

### Option B — Terminal se:

VS Code mein terminal kholo (`Ctrl + `` ` ```) aur ye run karo:

```bash
g++ -o server.exe server.cpp -lws2_32 -std=c++17
g++ -o client.exe client.cpp -lws2_32 -std=c++17
g++ -o udp_client.exe udp_client.cpp -lws2_32 -std=c++17
```

Agar koi error nahi aaya → build successful!

---

## Step 4 — Run Karo

**3 alag terminals kholo** VS Code mein:
`Ctrl + `` ` `` ` → upar `+` button se naya terminal

### Terminal 1 — Server:

```bash
.\server.exe
```

Output:

```
=========================================
  Multi-Client TCP/UDP Chat Server
  (Windows Version)
=========================================
[TCP] Port 8080  |  [UDP] Port 8081
Waiting for connections...
```

### Terminal 2 — Pehla Client:

```bash
.\client.exe
```

Naam type karo jab pooche → Enter

### Terminal 3 — Doosra Client:

```bash
.\client.exe
```

Alag naam do → ab dono ke beech chat ho sakti hai!

### Terminal 4 (optional) — UDP Client:

```bash
.\udp_client.exe
```

Koi bhi message type karo → server echo karega

---

## Kya Hoga Run Karne Pe

```
Server Terminal:              Client 1:              Client 2:
─────────────────             ─────────────          ─────────────
Waiting...                    Enter name: Prashant   Enter name: Rahul
[+] Prashant joined           Connected!
[+] Rahul joined                                     >> Prashant joined!
[Prashant]: Hello!            Hello!                 [Prashant]: Hello!
[Rahul]: Hi bro!              [Rahul]: Hi bro!       Hi bro!
```

---

## Linux vs Windows — Kya Badla

| Linux (POSIX)             | Windows (Winsock)       |
| ------------------------- | ----------------------- |
| `#include <sys/socket.h>` | `#include <winsock2.h>` |
| `pthread_create()`        | `CreateThread()`        |
| `std::mutex`              | `CRITICAL_SECTION`      |
| `close(fd)`               | `closesocket(sock)`     |
| `SOCKET` = `int`          | `SOCKET` = special type |
| Kuch setup nahi           | `WSAStartup()` zaruri   |

---

## Common Errors aur Fix

**"g++ not found"**

```
MinGW install nahi hua ya PATH mein nahi hai
→ Step 1 dobara karo
```

**"WSAStartup failed"**

```
Winsock initialize nahi hua
→ Normally nahi hoga — agar ho toh PC restart karo
```

**"Connect failed"**

```
Server pehle start karo, phir client
```

**"Port already in use"**

```
Command Prompt mein:
netstat -ano | findstr :8080
taskkill /PID <number> /F
```

**Firewall Warning aaye toh:**

```
"Allow Access" click karo — server ko network access chahiye
```

---

## Concepts Jo Interview Mein Kaam Aayenge

| Concept             | Code mein kahan                                     |
| ------------------- | --------------------------------------------------- |
| Socket lifecycle    | `socket()` → `bind()` → `listen()` → `accept()`     |
| TCP vs UDP          | `SOCK_STREAM` vs `SOCK_DGRAM`                       |
| Multi-threading     | `CreateThread()` per client                         |
| Synchronization     | `EnterCriticalSection()` / `LeaveCriticalSection()` |
| Memory management   | `new SOCKET(s)` → thread mein `delete`              |
| Broadcast           | Sabko message forward karna                         |
| Graceful disconnect | `recv() <= 0` check                                 |

# Tcp_server_windows
