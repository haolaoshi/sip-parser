# SIP Parser

A SIP message parser and INVITE transaction state machine implemented in C++17.

## Features

- Parse SIP request and response messages from raw strings
- Header value trimming (leading/trailing whitespace)
- Body extraction using `Content-Length` byte count
- INVITE transaction state machine (UAC side, RFC 3261)

## Design Decisions

**`std::variant<Request, Response>` instead of union or inheritance**  
Enforces type-safe access at compile time. No undefined behavior from wrong-type access.

**`map<string, vector<string>>` for headers**  
SIP allows multiple values for the same header (e.g., multiple `Via` headers). A single-value map would silently drop duplicates.

**Body extracted by `Content-Length` bytes, not by reading to end of string**  
In a TCP stream, multiple SIP messages are concatenated. Reading to end-of-string would absorb the next message's content. `Content-Length` is the authoritative byte boundary per RFC 3261.

**`Calling` and `Proceeding` as separate states**  
`Calling`: no response received yet — UAC retransmits INVITE (Timer B running).  
`Proceeding`: provisional response received — peer is reachable, retransmission stops, waiting for final response.  
These represent two different levels of call establishment certainty and require different behavior.

## State Machine

INVITE transaction UAC-side states and transitions:

```
Idle ──SendInvite──► Calling
                        │
           Recv 1xx ────┤
                        ▼
                    Proceeding
                        │
           Recv 200 ────┤──── Recv 200 (from Calling)
                        ▼
                    Connected
                        │
        RecvBYE/SendBYE─┘
                        ▼
                    Terminated

Calling/Proceeding ──Recv 4xx/5xx/6xx──► Terminated
Calling/Proceeding ──SendCancel────────► Terminated
Calling/Proceeding ──TimerBTimeout─────► Terminated
```

## Build

Requires C++17. Tested on Linux (WSL2).

```bash
g++ -std=c++17 sip_parser.cpp sip_transaction.cpp -o main
./main
```

## Project Structure

```
sip_message.h        # SipMessage data structure (variant, headers, body)
sip_parser.cpp       # parse() function — raw string → SipMessage
sip_transaction.h    # State and event enums
sip_transaction.cpp  # transition() function — state machine
```
