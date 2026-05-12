# Getting Started

## Overview

This guide helps you install Mynic, write your first Mynic file, and run the interpreter with a simple example. Mynic is designed to describe packet layouts and parse raw byte streams into structured data.

## Prerequisites

- A modern C++ build environment if building from source
- A shell or terminal for running CLI commands
- A text editor for writing `.myn` files

## Installation

### Mynic CLI

The Mynic CLI is distributed as a single executable named `mynic`.

1. Place the `mynic` executable in a location on your system `PATH`.
2. On Linux or macOS, a common location is `/usr/local/bin`.
3. On Windows, add the folder containing `mynic.exe` to your `PATH`.

Example:
```bash
cp mynic /usr/local/bin/
chmod +x /usr/local/bin/mynic
```

### Mynic API

The Mynic API is provided as a shared library named `mynic_api`.

- On Linux/macOS: `libmynic_api.so` or `libmynic_api.dylib`
- On Windows: `mynic_api.dll`

Install the library in the appropriate system folder or configure your project to load it directly.

## Your First Mynic Program

Create a new file with the `.myn` extension.

Example: `hello.myn`

```mynic
packet HELLO_PACKET {
    uint8 version;
    uint8 type;
    bytes4 identifier;
    string message;
}
```

This packet definition describes a simple structure that contains:

- `version` — 8-bit unsigned integer
- `type` — 8-bit unsigned integer
- `identifier` — 4 raw bytes
- `message` — null-terminated string

## Running Mynic

Use the CLI to interpret a byte stream with your packet definition.

```bash
mynic --input data.bin --packet HELLO_PACKET hello.myn
```

If your CLI supports standard input, you can also pipe binary data:

```bash
cat data.bin | mynic --packet HELLO_PACKET hello.myn
```

## Verifying Output

After running, Mynic should produce parsed output describing the packet fields and values. The exact output format depends on the CLI implementation and configuration.

## Next Steps

Continue learning with the following guides:

- `installation.md` — detailed install instructions
- `hello-world.md` — step-by-step first program tutorial
- `running-code.md` — examples of executing Mynic code
- `basic-debugging.md` — debugging tips for Mynic files
- `faq.md` — frequently asked questions

