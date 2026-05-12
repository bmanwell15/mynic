# Mynic C++ API Reference

The Mynic C++ API provides programmatic access to packet parsing and decoding functionality. This reference documents all public methods available in the `Mynic` class.

## Class Overview

```cpp
#include "Mynic.h"

Mynic decoder;
```

## Constructor & Destructor

### Mynic()

Creates a new Mynic instance with default settings.

```cpp
Mynic::Mynic()
```

**Description:**
- Initializes the AST (Abstract Syntax Tree) parser
- Sets up the interpreter for packet decoding
- Creates cross-references between AST and interpreter components

### ~Mynic()

Destroys the Mynic instance and cleans up resources.

```cpp
Mynic::~Mynic()
```

## File Management

### loadFile()

Loads and parses a Mynic definition file.

```cpp
bool loadFile(std::string& filename)
```

**Parameters:**
- `filename`: Path to the `.myn` file to load

**Returns:**
- `true` if the file was loaded successfully
- `false` if the file could not be read or parsed

**Description:**
- Files are only loaded once; subsequent calls with the same filename are ignored
- Adds the file to the global file tracking list
- Parses the file content into AST nodes
- Merges imported definitions with existing ones

**Example:**
```cpp
Mynic decoder;
std::string filename = "packet_definitions.myn";
if (decoder.loadFile(filename)) {
    std::cout << "File loaded successfully" << std::endl;
} else {
    std::cout << "Failed to load file" << std::endl;
}
```

## Packet Decoding

### decodePacket() - Hex String

Decodes a packet from a hexadecimal string.

```cpp
DecodedPacket decodePacket(const std::string& strBytes, const std::string& packetName, bool asBits = false)
```

**Parameters:**
- `strBytes`: Hexadecimal string or bit string representing the packet data
- `packetName`: Name of the packet definition to use for decoding
- `asBits`: If `true`, treats `strBytes` as a bit string instead of hex

**Returns:**
- `DecodedPacket` containing the parsed packet data

**Description:**
- Converts hex string to byte array (unless `asBits` is true)
- Uses the specified packet definition to parse the binary data
- Returns structured data with all parsed fields

**Example:**
```cpp
std::string hexData = "4500003c0000000040060000c0a80001c0a80002";
DecodedPacket packet = decoder.decodePacket(hexData, "TCP_PACKET");
```

### decodePacket() - Byte Array

Decodes a packet from a byte array.

```cpp
DecodedPacket decodePacket(const std::vector<uint8_t>& dataBytes, const std::string& packetName)
```

**Parameters:**
- `dataBytes`: Vector of bytes containing the packet data
- `packetName`: Name of the packet definition to use for decoding

**Returns:**
- `DecodedPacket` containing the parsed packet data

**Example:**
```cpp
std::vector<uint8_t> rawData = {0x45, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00};
DecodedPacket packet = decoder.decodePacket(rawData, "TCP_PACKET");
```

### decodeFile()

Decodes multiple packets from a file.

```cpp
std::vector<DecodedPacket> decodeFile(std::string filepath, char sep, std::string& packetName)
```

**Parameters:**
- `filepath`: Path to the file containing packet data
- `sep`: Character separator between packets in the file
- `packetName`: Name of the packet definition to use for decoding

**Returns:**
- Vector of `DecodedPacket` objects, one for each packet found in the file

**Description:**
- Reads the entire file content
- Splits the content by the separator character
- Decodes each segment as a separate packet
- Useful for processing packet capture files

**Example:**
```cpp
std::string packetName = "TCP_PACKET";
auto packets = decoder.decodeFile("packets.txt", '\n', packetName);
for (const auto& packet : packets) {
    // Process each packet
}
```

## Schema Inspection

### printSchema()

Prints the complete schema of all loaded packet definitions.

```cpp
void printSchema()
```

**Description:**
- Displays all loaded packets, segments, enums, and other definitions
- Shows the structure and relationships between components
- Useful for debugging and understanding loaded definitions

**Example:**
```cpp
decoder.loadFile("definitions.myn");
decoder.printSchema();
```

### printPacket()

Prints the structure of a specific packet.

```cpp
void printPacket(const std::string& packetName)
```

**Parameters:**
- `packetName`: Name of the packet to display

**Description:**
- Shows the detailed structure of the specified packet
- Includes all fields, their types, and sizes
- Only displays packets that match the exact name

**Example:**
```cpp
decoder.printPacket("TCP_PACKET");
```

### printSegment()

Prints the structure of a specific segment.

```cpp
void printSegment(const std::string& segmentName)
```

**Parameters:**
- `segmentName`: Name of the segment to display

**Description:**
- Shows the detailed structure of the specified segment
- Includes all fields, their types, and sizes
- Only displays segments that match the exact name

**Example:**
```cpp
decoder.printSegment("TCP_HEADER");
```

## Export Functions

### exportToFile()

Exports a decoded packet to a file in JSON format.

```cpp
void exportToFile(DecodedPacket& decodedPacket, std::string& filepath, bool appendMode = false)
```

**Parameters:**
- `decodedPacket`: The decoded packet to export
- `filepath`: Path where the JSON file should be written
- `appendMode`: If `true`, appends to existing file; if `false`, overwrites

**Description:**
- Converts the decoded packet to JSON format
- Writes to the specified file path
- Can either overwrite or append to existing files

**Example:**
```cpp
DecodedPacket packet = decoder.decodePacket(hexData, "TCP_PACKET");
std::string outputPath = "decoded_packet.json";
decoder.exportToFile(packet, outputPath);
```

## Utility Functions

### version()

Returns the version string of the Mynic library.

```cpp
std::string version()
```

**Returns:**
- Version string in the format "Mynic version X.X.X"

**Example:**
```cpp
std::cout << "Using " << decoder.version() << std::endl;
```

### split()

Splits a string by a delimiter character.

```cpp
std::vector<std::string> split(const std::string &txt, char ch)
```

**Parameters:**
- `txt`: String to split
- `ch`: Delimiter character

**Returns:**
- Vector of substrings split by the delimiter

**Description:**
- Internal utility function for string processing
- Exposed publicly for convenience

## Data Structures

### DecodedPacket

The result of packet decoding operations.

```cpp
struct DecodedPacket {
    std::string packetName;              // Name of the packet definition used
    std::vector<uint8_t> rawBytes;       // Original binary data
    std::shared_ptr<InterpretedPacket> rootField;  // Parsed packet structure
    std::string timestamp;               // Timestamp of decoding
    std::vector<InterpreterWarning> warnings;      // Any warnings generated during parsing
};
```

**Fields:**
- `packetName`: The packet definition name used for decoding
- `rawBytes`: Copy of the original input bytes
- `rootField`: Hierarchical structure of parsed fields and values
- `timestamp`: When the packet was decoded
- `warnings`: List of any warnings or issues encountered during parsing

## Error Handling

- File loading failures return `false` from `loadFile()`
- Packet decoding may generate warnings stored in the `DecodedPacket.warnings` vector
- Invalid packet names or malformed data may cause exceptions
- All methods are synchronous and block until completion

## Thread Safety

The Mynic class is not thread-safe. Create separate instances for concurrent use.

## Memory Management

- Packet definitions are stored in shared pointers
- Decoded packets maintain references to their source data
- Large binary files should be processed in chunks to manage memory usage

## Example Usage

```cpp
#include "Mynic.h"
#include <iostream>

int main() {
    // Create decoder instance
    Mynic decoder;

    // Load packet definitions
    std::string defFile = "packet_defs.myn";
    if (!decoder.loadFile(defFile)) {
        std::cerr << "Failed to load definitions" << std::endl;
        return 1;
    }

    // Decode hex data
    std::string hexData = "4500003c0000000040060000c0a80001c0a80002";
    DecodedPacket packet = decoder.decodePacket(hexData, "TCP_PACKET");

    // Export to JSON
    std::string outputFile = "decoded.json";
    decoder.exportToFile(packet, outputFile);

    // Check for warnings
    if (!packet.warnings.empty()) {
        std::cout << "Warnings during parsing:" << std::endl;
        for (const auto& warning : packet.warnings) {
            std::cout << warning.message << std::endl;
        }
    }

    return 0;
}
```</content>
<parameter name="filePath">c:\Users\benja\OneDrive - purdue.edu\Programming\C++\mynic\docs\user-guide\cpp-api.md