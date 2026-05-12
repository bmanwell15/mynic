# Mynic CLI Reference

The Mynic Command Line Interface (CLI) provides a text-based interface for loading packet definitions, decoding binary data, and managing the Mynic environment.

## Getting Started

To start the Mynic CLI, run the compiled binary:

```bash
./mynic_cli
```

The CLI will present an interactive prompt where you can enter commands.

## Commands

### load

Loads one or more Mynic definition files into memory.

**Syntax:**
```
load <filename1> [filename2] [filename3] ...
```

**Parameters:**
- `filename`: Path to a `.myn` file containing packet definitions

**Example:**
```
load examples/tcp-packet.myn examples/event-system.myn
```

**Description:**
- Loads the specified files and parses their packet definitions
- Displays the loaded schema after successful loading
- If a file fails to load, displays an error message but continues with other files
- Files are only loaded once; subsequent loads of the same file are ignored

### interpret

Decodes binary data using a specified packet definition.

**Syntax:**
```
interpret <hex_bytes> as <packet_name>
```

**Parameters:**
- `hex_bytes`: Hexadecimal string representing the binary data to decode
- `packet_name`: Name of the packet definition to use for decoding

**Example:**
```
interpret 4500003c0000000040060000c0a80001c0a8000200a00050000000000000000050022000c0a80001c0a80002 as TCP_PACKET
```

**Description:**
- Decodes the provided hexadecimal bytes using the specified packet definition
- Outputs the decoded packet as JSON
- Displays the time taken for interpretation in milliseconds
- Requires the packet definition to be loaded first using the `load` command

### version

Displays version information for the Mynic CLI and core library.

**Syntax:**
```
version
```

**Example:**
```
version
```

**Output:**
```
Mynic CLI version 1.0.0
Mynic version 1.0.0

Created by Benjamin Manwell.
https://github.com/bmanwell15
```

### refresh

Reloads all currently loaded files and refreshes the schema display.

**Syntax:**
```
refresh
```

**Description:**
- Clears the current Mynic instance
- Reloads all previously loaded files
- Displays the updated schema
- Useful for picking up changes to definition files without restarting the CLI

### reset

Clears all loaded files and resets the Mynic environment.

**Syntax:**
```
reset
```

**Description:**
- Prompts for confirmation before proceeding
- Clears all loaded files and packet definitions
- Creates a fresh Mynic instance
- Requires typing "yes" to confirm the reset operation

## Command Structure

All commands follow a simple structure:
- Commands are case-sensitive
- Parameters are separated by spaces
- File paths should be properly escaped if they contain spaces
- Commands execute immediately and provide feedback

## Error Handling

- Invalid commands display an error message indicating the unrecognized command
- Missing parameters show appropriate error messages
- File loading failures are reported individually without stopping the batch load
- Interpretation errors are displayed with details about what went wrong

## Examples

### Complete Workflow Example

```
load examples/tcp-packet.myn
interpret 4500003c0000000040060000c0a80001c0a8000200a00050000000000000000050022000c0a80001c0a80002 as TCP_PACKET
version
refresh
```

This sequence loads a TCP packet definition, decodes some sample TCP packet data, shows version information, and refreshes the loaded files.

## Tips

- Use the `load` command to load your packet definitions before attempting to decode data
- Hex bytes should be provided as continuous strings without spaces or 0x prefixes
- The `refresh` command is useful during development when you're iteratively updating packet definitions
- Use `reset` with caution as it clears all loaded definitions</content>
<parameter name="filePath">c:\Users\benja\OneDrive - purdue.edu\Programming\C++\mynic\docs\user-guide\cli-reference.md