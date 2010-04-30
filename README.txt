CoreSpark is the collection of core libraries used in several products from Pixelspark. It contains the following libraries:

- TJShared: cross-platform layer for file handling, threads and concurrency, memory management (reference counting) and much more
- TJScript: a small scripting engine (using JavaScript-like syntax) implemented on top of TJShared
- TJNP: cross-platform library for handling TCP/UDP connections, name resolving, HTTP client and server
- TJCrashReporter: client application for use with Breakpad crash reporter (Windows-only)
- TJSharedUI: user interface library (Windows only)
- TJScout: a library for service discovery (currently only through Bonjour aka. mDNS/DNS-SD)
- TJDMXEngine: a library for transmitting DMX data through a variety of DMX controllers, with many functions such as macros and submixing
- TJDB: database abstraction layer with support for direct SQL as well as a persistence layer (which is integrated with the reference counting mechanism of TJShared)
- TJUpdater: small program used for auto-updating
- TJZip: ZIP library

More information on the libraries can be found at http://developer.pixelspark.nl

The libraries can be built on OS X through the XCode project files. Under Linux, the scons build system is used. Note that you should either
modify these scripts to be independent or create a top-level SConstruct file to work. On Windows, you need to create a Visual Studio (2008, express
will also work) and add the .vcproj files as usual.

The Libraries/ folder contains some third-party libraries used in several places; please read their licenses before using. When building, make sure
it is in the include path (as well as the Core/ directory, by the way).
