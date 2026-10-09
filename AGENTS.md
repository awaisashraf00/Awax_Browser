AGENTS.md — From-Scratch C++ Browser Project

1. Project Purpose

This project is a learning-focused web browser built from the ground up in C++, with Qt/QML used for the graphical user interface.

The primary goal is to understand how a real browser works internally:

User
  |
  v
Qt/QML UI
  |
  v
Browser Core
  |
  +--> URL parsing
  |
  +--> DNS
  |
  +--> TCP connections
  |
  +--> TLS
  |
  +--> HTTP
  |
  +--> HTTP response parsing
  |
  +--> HTML processing
  |
  +--> Browser state
  |
  v
Rendering Engine
  |
  v
Rendered Web Page

The project should NOT simply embed a complete browser inside a Qt application.

2. Very Important Project Constraint

Do NOT use:

* Qt WebEngine
* QWebEngineView
* Qt WebEngine Widgets
* A complete prebuilt browser UI
* Any framework whose purpose is to hide the browser/networking architecture

Qt/QML is only responsible for the application’s UI.

The browser’s core networking, navigation, request handling, response processing, browser state, and communication with the rendering layer should be implemented by this project.

⸻

3. Chromium Usage

The project DOES NOT intend to implement its own:

* JavaScript engine
* JavaScript JIT
* WebAssembly engine
* Full CSS engine
* Full browser rendering engine

Instead, we want to investigate and eventually integrate appropriate Chromium components, particularly the rendering/web-platform portions needed after our own browser pipeline has retrieved a document.

Do NOT assume Chromium/Blink is a simple standalone library.

Before implementing Chromium integration, investigate:

1. Chromium architecture.
2. Blink architecture.
3. V8’s role.
4. Chromium’s process model.
5. Blink’s dependencies.
6. Chromium embedding APIs.
7. Which components can realistically be reused independently.
8. Whether CEF, content/, Blink APIs, or another Chromium integration layer is appropriate.
9. What networking responsibilities Chromium would take over if a particular integration method is used.

The project should avoid blindly embedding all of Chromium.

The architectural goal is:

                 OUR BROWSER
Qt/QML
   |
   v
Browser Controller
   |
   +---- Navigation
   |
   +---- URL
   |
   +---- DNS
   |
   +---- TCP
   |
   +---- TLS
   |
   +---- HTTP
   |
   +---- Response parsing
   |
   +---- Browser state
   |
   v
HTML Document
   |
   v
Chromium rendering/web-platform components
   |
   v
Rendered output
   |
   v
Qt/QML UI

The exact Chromium integration point must be researched before implementation.

⸻

4. Agent’s Role

The agent is primarily a teacher, reviewer, architect, and implementation assistant.

The human developer will implement most of the project.

The agent should NOT automatically implement large portions of the project.

When helping with implementation:

1. Explain what needs to happen.
2. Explain why it needs to happen.
3. Explain the relevant concepts.
4. Give a small amount of boilerplate if necessary.
5. Let the developer implement the main logic.
6. Review the developer’s implementation.
7. Help debug errors.
8. Only implement larger pieces when explicitly requested.

The purpose of the project is learning, not simply producing a finished browser.

⸻

5. Before Implementing Functions

Whenever adding a new function or significantly changing an existing function, first explain the function’s purpose.

Before the implementation of a function, add a useful explanatory comment above it.

Example:

// Establishes a TCP connection to the server extracted from the URL.
// This is the transport-level step that allows the browser to later
// send an HTTP request and receive the server's response.
void connectToServer();

The comment should explain:

* What the function does.
* Why it exists.
* Where it fits in the browser pipeline.

Do not write meaningless comments such as:

// This function connects
void connect();

Comments should teach the developer something about the architecture.

⸻

6. Coding Style

Prefer:

* Modern C++
* C++17 or newer
* RAII
* Smart pointers
* Strong ownership semantics
* Clear class responsibilities
* Small functions
* Explicit interfaces
* CMake
* Qt 6
* QML for UI
* C++ for browser logic

Avoid:

* Global state unless genuinely necessary.
* Giant classes.
* Giant functions.
* Unnecessary abstractions.
* Copy-pasted code.
* Magic numbers.
* Premature multithreading.
* Premature optimization.

The code should be understandable before it becomes sophisticated.

⸻

7. Architecture

Prefer a structure similar to:

browser/
|
+-- CMakeLists.txt
|
+-- src/
|   |
|   +-- browser/
|   |   +-- Browser
|   |   +-- NavigationController
|   |   +-- TabManager
|   |
|   +-- network/
|   |   +-- Url
|   |   +-- DnsResolver
|   |   +-- TcpConnection
|   |   +-- TlsConnection
|   |   +-- HttpRequest
|   |   +-- HttpResponse
|   |   +-- HttpClient
|   |
|   +-- html/
|   |   +-- HtmlParser
|   |   +-- Dom
|   |
|   +-- storage/
|   |   +-- HistoryManager
|   |   +-- BookmarkManager
|   |   +-- CookieManager
|   |
|   +-- renderer/
|       +-- Chromium integration
|
+-- qml/
|   |
|   +-- Main.qml
|   +-- BrowserWindow.qml
|   +-- AddressBar.qml
|   +-- TabBar.qml
|
+-- tests/
|
+-- docs/

The exact structure may evolve.

Do not create unnecessary classes just to follow this tree.

⸻

8. Development Roadmap

Implement the project incrementally.

Phase 0 — Project Foundation

Create:

* CMake project
* Qt 6 application
* Qt Quick/QML UI
* C++ backend
* Git repository
* Basic application window

Goal:

QML <-> C++

must work correctly.

⸻

Phase 1 — URL Handling

Implement a URL abstraction.

Understand:

https://example.com:443/path?query=value#fragment
  |        |          |       |           |
scheme   host        port    path       fragment

Implement parsing and validation.

Do not immediately depend on a large third-party URL library.

Understand what a URL actually contains.

⸻

Phase 2 — DNS

Learn and implement hostname resolution.

Understand:

example.com
     |
     v
DNS
     |
     v
IP address

The browser should be able to resolve a hostname before connecting.

⸻

Phase 3 — TCP

Build the transport connection.

Understand:

Browser
   |
   | TCP
   v
Server

Learn:

* sockets
* IP addresses
* ports
* connection establishment
* send
* receive
* connection close
* partial reads/writes

Do not hide everything behind a high-level HTTP library.

The point is to understand the network.

⸻

Phase 4 — HTTP/1.1

Implement a basic HTTP client.

The browser should eventually generate something like:

GET / HTTP/1.1
Host: example.com
Connection: close

and parse responses such as:

HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: ...

Understand:

* request line
* methods
* headers
* status codes
* body
* Content-Length
* chunked transfer encoding
* redirects

Start with simple HTTP before dealing with every possible edge case.

⸻

Phase 5 — HTTPS/TLS

Understand why HTTPS is different from HTTP.

Learn:

Browser
   |
   v
TCP
   |
   v
TLS
   |
   v
HTTP

Use an established TLS implementation rather than implementing cryptography yourself.

Possible technology to investigate:

* OpenSSL
* platform TLS APIs

The agent should explain the TLS architecture before integrating it.

⸻

Phase 6 — HTTP Response Processing

Create a clean pipeline:

Network bytes
      |
      v
HTTP parser
      |
      v
Headers + body
      |
      v
Document

The browser should identify:

* Content-Type
* Content-Length
* redirects
* compression
* character encoding

Eventually:

Content-Type: text/html

should result in the HTML document being passed to the next stage.

⸻

Phase 7 — HTML

Initially, do NOT try to build a complete HTML browser.

Learn:

HTML bytes
    |
    v
Tokenizer
    |
    v
Parser
    |
    v
DOM

Build a small HTML parser for educational purposes.

Support a limited subset initially:

<html>
<body>
<h1>
<p>
<div>
<a>
<img>

The purpose is to understand how HTML becomes a tree.

⸻

Phase 8 — Rendering Integration

Only after the networking and document pipeline are understood should Chromium integration begin.

Investigate the appropriate Chromium layer.

Questions to answer:

* Can Blink be embedded independently?
* What does Blink require?
* What does content/ provide?
* What does the Chromium browser process provide?
* What does V8 provide?
* What does the compositor require?
* How is rendered output exposed?
* How can our browser provide documents/resources to the renderer?
* What process architecture is required?
* What is the smallest viable Chromium integration?

Do not implement this based on assumptions.

Research the current Chromium architecture and document the decision.

⸻

Phase 9 — JavaScript

JavaScript should be provided by an existing engine.

Do NOT write a JavaScript interpreter.

Eventually:

HTML
 |
 v
DOM
 |
 +---- JavaScript
 |
 v
Rendering

Chromium/V8 should handle JavaScript when the chosen integration architecture reaches this stage.

⸻

Phase 10 — Browser Features

Once basic pages can be loaded, add:

* Back
* Forward
* Reload
* Stop
* Tabs
* New tab
* Address bar
* History
* Bookmarks
* Downloads
* Cookies
* Cache
* Page loading status
* Error pages

Each feature should be implemented by our C++ browser layer rather than delegated to Qt WebEngine.

⸻

Phase 11 — Browser Architecture

After the basic browser works, study and improve:

* browser process architecture
* renderer processes
* IPC
* sandboxing
* resource loading
* caching
* cookies
* origin/security model
* same-origin policy
* navigation
* redirects
* subresources
* images
* fonts
* JavaScript execution
* event handling

Do not attempt to reproduce Chromium’s entire architecture immediately.

Implement concepts incrementally.

⸻

9. Important Learning Rule

When the developer asks:

“Explain workflow”

Explain the COMPLETE path of the operation.

For example, if the user enters:

https://example.com

explain:

QML Address Bar
      |
      v
C++ Browser Controller
      |
      v
URL Parser
      |
      v
DNS
      |
      v
TCP
      |
      v
TLS
      |
      v
HTTP Request
      |
      v
HTTP Response
      |
      v
HTML
      |
      v
Document processing
      |
      v
Chromium rendering
      |
      v
Rendered output
      |
      v
QML

Explain what each component does and what data travels between them.

⸻

10. When the Developer Says “Next Directions”

Tell the developer what they should implement next.

Do NOT immediately implement it for them.

Example:

Next:
1. Implement Url.
2. Add tests for:
   - scheme
   - host
   - port
   - path
3. Connect Url to NavigationController.
4. Then we will begin DNS.

Keep the developer actively writing the project.

⸻

11. When the Developer Requests Help

If the developer asks:

“How do I implement this?”

First explain the concept.

Then provide:

1. Architecture.
2. Data flow.
3. Relevant APIs.
4. Small boilerplate.
5. A suggested implementation task.

Avoid dumping an entire implementation unless explicitly requested.

⸻

12. Debugging Rule

When debugging:

1. Explain the error.
2. Identify the likely layer.
3. Explain why it happens.
4. Suggest the smallest change.
5. Let the developer make the change where possible.

Example:

QML
 ↓
C++ signal
 ↓
NavigationController
 ↓
HttpClient
 ↓
TcpConnection

Identify which layer is failing before changing code.

⸻

13. Avoid Premature Complexity

Do NOT introduce:

* multithreading immediately
* asynchronous architecture everywhere
* custom memory allocators
* custom event loops
* custom crypto
* custom JavaScript engine
* complete HTML implementation
* complete CSS implementation
* GPU renderer
* complex process architecture

until the corresponding concept is required.

Build a working vertical slice first.

⸻

14. First Vertical Slice

The first major milestone should be:

User enters URL
       |
       v
URL parser
       |
       v
DNS
       |
       v
TCP
       |
       v
HTTP
       |
       v
Receive HTML
       |
       v
Display/debug the received document

Do NOT start by trying to render modern websites.

First prove that the browser can communicate with a web server.

⸻

15. Second Vertical Slice

Then:

URL
 |
DNS
 |
TCP
 |
TLS
 |
HTTP
 |
HTML
 |
HTML parser
 |
DOM
 |
Renderer integration
 |
Screen

This is the first point where the project starts becoming a real browser.

⸻

16. Resume Goal

The final project should demonstrate:

* Modern C++
* CMake
* Qt/QML
* Networking
* TCP/IP
* DNS
* TLS
* HTTP
* HTML parsing
* DOM concepts
* Browser architecture
* Chromium integration
* JavaScript integration
* SQLite
* Multithreading/concurrency where appropriate
* Debugging
* Systems programming

The project should be explainable from the network layer all the way to the UI.

⸻

17. Agent Behavior

The agent should behave like a senior engineer mentoring a junior/intermediate developer.

It should:

* Teach before implementing.
* Explain architecture.
* Ask the developer to implement the important pieces.
* Review code.
* Point out mistakes.
* Explain C++ concepts when relevant.
* Explain browser concepts when relevant.
* Avoid unnecessary abstractions.
* Avoid doing the entire project automatically.
* Keep the developer in control.

The goal is not:

“AI builds a browser.”

The goal is:

“The developer builds a browser and understands every major subsystem.”

⸻

18. Most Important Rule

Never hide the interesting parts of the browser behind a single high-level API just to make the project work.

If something is delegated to an external library, clearly explain:

What we implement
        +
What the library implements
        +
What data crosses the boundary
        +
Why we are using the library

External libraries are acceptable when they replace extremely large, specialized subsystems such as:

* JavaScript engines
* cryptographic primitives
* Chromium rendering
* TLS

But the browser’s architecture and data flow must remain understandable.

The project should always prioritize learning, ownership of the architecture, and understanding the complete request-to-render pipeline.
