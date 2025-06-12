# Overview

Design:

Logging library "ulog.h", header-only with console output (active by default) and memory buffer (optional).

Output format constant:
```
<timestamp> [<log-level>] [<logger-name>] <message>
```

Log messages can have anonymous parameters (uses formatter's anonymous parameter formatting):
```
logger.info("Current user name is {?}, date of birth: {?}", userName, dob); // all parameters converted via ustr::to_string
```

Log messages can have positional parameters (uses formatter's positional parameter formatting):
```
logger.info("Current user name is {0}, date of birth: {1}, {0}", userName, dob); // all parameters converted via ustr::to_string
```

Library uses namespace "ulog".


# Usage examples

```
// creates (or just returns) global logger with console and optional memory buffer
// global logger variable is thread-safe
auto globalLogger = ulog::getLogger();

// creates (or just returns) global logger with help of factory, factory default logger name
auto globalLogger = ulog::getLogger(<factory functor or lambda>);

// creates (or just returns) logger with console and optional memory buffer named "Matrix"
// logger registry is thread-safe
auto logger = ulog::getLogger("Matrix");

// creates (or just returns) logger with help of factory, factory receives logger name
auto logger = ulog::getLogger("Matrix", <factory functor or lambda>);

// named logger, writes {timestamp} [{log-level}] [{logger-name}] <message>
// e.g. 2021-10-11 15:43:12.001 [DEBUG] [Matrix] Found user...
logger.debug("Found user...");

// global logger, writes {timestamp} [{log-level}] <message>
// e.g. 2021-10-11 15:43:12.002 [INFO] Current user name is Jack, date of birth: 2000-11-13
globalLogger.info("Current user name is {0}, date of birth: {1}", userName, dob);

// buffer handling, inactive by default
{
  logger.enable_buffer(200); // activate buffer with capacity 200 messages, capacity = 0: unlimited
  logger.info("Start...");
  logger.debug("End of day {0}", nth);
  logger.flush(); // for console - writes "endl"

  // scanning buffer entries
  // you can use on buffer: count_if, all_of, any_of, none_of, find_if, copy_if

  // iteration over buffer - cbegin, cend
  for (auto it = buffer.cbegin(); it != buffer.cend(); ++it) {
    cout << ustr::to_string(it->message()) << "\n";
  }
  logger.clear_buffer();
  logger.disable_buffer();
  // calling buffer.begin();
}

// observers
{
  logger.add_observer(observer); // uses thread-safe registry defined for this logger
  // observer will receive LogEntry (timestamp, log-level, formatted message) for each new message event
  // observer methods: handleRegistered, handleUnregistered, handleNewMessage, handleFlush
  logger.remove_observer(observer); // thread-safe
}

// RAII observer scope
{
  observer_scope scope(logger, observer); // calls logger.add_observer(observer);
  ...
} // calls logger.remove_observer(observer);

// console deactivation (console is active by default)
{
  logger.disable_console();
  ...
  logger.info("Test"); // will not write anything to console
  logger.enable_console();
}

```



