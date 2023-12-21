# Simple Server for Message Passing in C

## Overview
This repository contains a server application written in C that handles message passing between multiple clients. It uses socket programming to manage client connections and a custom message queue for storing and forwarding messages.

## Features
- Support for multiple client connections using file descriptor sets.
- Custom message parsing and queueing system.
- Basic command handling for client operations like `list`, `message`, `getMessage`, `quit`, and `register`.
- Graceful handling of client disconnection and message delivery with basic error control.

## Prerequisites
- A Unix-like operating system with standard networking libraries.
- GCC (GNU Compiler Collection) or another C compiler.
- Basic understanding of network programming in C.

## Installation
1. Clone the repository to your local machine.
2. Navigate to the cloned directory.

## Compilation
To compile the server program, use the following command:

```bash
gcc -o server server.c libParseMessage.c libMessageQueue.c -I.
