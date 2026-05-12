# Mynic Language Guide

### Table of Contents

### About

The Mynic Programming Language is declarative and specifically designed to define packet structures. It is not a general-purpose programming language and supports only features necessary for byte parsing. This guide describes the language's key features.

### Basic Syntax

Semicolons are optional and can be replaced by newlines. Mynic supports two comment styles: single-line comments using `//` and multi-line comments using `/* */`. Mynic is case-sensitive.

Example syntax:
```mynic
// Single-line comment
packet EXAMPLE {
    uint8 field;  // Semicolon is optional
    uint16 field2 // Newline also works
}

/* Multi-line comment
   spanning multiple lines */
define CONSTANT 42;
```

### Packets

Creating a packet is typically the first step in building a Mynic file. Packets serve as entry points for parsing byte streams. A packet definition includes its name and the subfields it contains, as shown below:

```mynic
    packet PACKET_NAME {
        [... subfields ...]
    }
```

It is important to note that a packet cannot be a subfield of any other object.

### Primitive Variables

With a packet defined, the next step is to specify how the Mynic Interpreter parses each field. For example, suppose the first two bytes of a packet contain a version number, where the first byte represents the major version and the second byte represents the minor version. In Mynic, this is implemented using two primitive values:

```mynic
    packet EXAMPLE_PACKET {
        uint8 majorVersion;
        uint8 minorVersion;
    }
```

##### Primitive Datatypes

Each primitive value consists of a datatype and a variable name. For example, `uint8` specifies an 8-bit unsigned integer field. Mynic supports numerous built-in datatypes, including integers, floating-point numbers, strings, dates, timespans, IP/MAC addresses, and standard C types. Refer to [language-reference.md](language-reference.md) for a complete list of available datatypes.

```mynic
    packet PRIMITIVE_TYPES {
        char ch;              // 8-bit character
        bool boolean;         // Note: A boolean is one byte, not one bit (use bitfields for 1-bit booleans)
        byte hexVal;          // Byte value, printed as hexadecimal
        float decimalNum;     // 4-byte floating-point value (IEEE 754 compliant)
        ipv4address ipaddr;   // 32-bit IPv4 address, formatted as "X.X.X.X"
    }
```

##### String Datatype

Strings behave differently from other primitive types. When a variable is declared as a string, the Mynic Interpreter reads bytes sequentially until it encounters a null termination byte (0x00). For fixed-length strings without null termination, specify the number of characters after the variable name (similar to array syntax). The following example demonstrates both approaches:

```mynic
    packet STRING_PACKET {
        string variableLengthStr;   // Reads bytes until null terminator (0x00) or stream end
        string fixedLengthStr[4];   // Fixed-length string: exactly 4 bytes, no null terminator needed
    }
```

##### Variable Size Integers, Bits, & Bytes

The `uint`, `int`, `bits`, and `bytes` datatypes support variable sizes. Specify the bit width immediately after the datatype name. If not specified, `uint` and `int` default to 4 bytes, while `bits` and `bytes` default to 1 byte. Maximum bit width is 64 bits. Consider the following example:

```mynic
    packet VAR_LEN {
        uint16 num1;  // 16-bit unsigned integer
        uint64 long;  // 64-bit unsigned integer

        bits16;       // 16 raw bits
        bytes2;       // 2 bytes
    }
```

##### Primitive Datatype Settings

Each primitive datatype supports configurable settings that can be customized individually. For instance, to parse a field as little-endian, modify the datatype settings as shown below. Separate multiple settings with commas.

```mynic
    packet PRIM_SETTINGS {
        float bf;                                               // Big-endian (default)
        float littleEndian {endianness: "little"};              // Little-endian
        float velocity {endianness: "little", units: "m/s"};    // Little-endian with units
    }
```

##### Creating a Primitive Datatype Alias

Mynic supports the `typedef` operator, similar to C/C++, which creates an alias for a datatype. The following example demonstrates creating a `checksum_t` alias for a 4-byte value:

```mynic
    typedef bytes4 checksum_t;

    packet TYPEDEF_EXAMPLE {
        checksum_t checksum;  // 4-byte checksum field
    }
```

## Default Fields

Settings can be applied at the variable level or globally at the packet or file scope. Use a `default` block to configure default settings for all fields within a packet. Packet-level defaults override global defaults. Refer to [language-reference.md](language-reference.md) for a complete list of available options. To apply settings to all packets in the file, place a `default` block at the file scope (outside any packet).

```mynic
    default {  // Global default block: applied to all packets
        units: "m/s"
    }

    packet PACKET_DEF {
        default {  // Packet-level default: overrides global default
            units: "km/h"
        }
        float f;  // All fields in this packet use units of "km/h"
    }
```

## Arrays

Primitive values can be grouped into arrays. Array syntax is similar to standard variable declarations. Consider the following example:

```mynic
    packet ARRAY_DEF {
        uint arrayOfData[3];  // Array of 3 unsigned integers (4 bytes × 3 = 12 bytes total)
    }
```

The preceding example is a static array, where the element count is determined at compile time. Mynic also supports dynamic arrays, where the size is determined by a previously parsed variable. This is particularly useful for packets with length fields (such as UDP packets).

```mynic
    packet DYNAMIC_ARR {
        uint8 arraySize;
        byte dynamicArray[arraySize];  // Array size determined by the arraySize field
    }
```

In this example, the `arraySize` field determines the number of bytes in `dynamicArray`.

An important distinction: arrays bypass endianness rules, whereas multi-byte fields respect them. For instance, an array of 2 bytes reads bytes sequentially without endianness conversion, while a single 2-byte field applies endianness settings. The following example illustrates this difference with 4-byte input:

```mynic
    packet ARR_ENDIANNESS {
        /*
            Little-endian configuration throughout the packet.
            Input: 0x00112233
        */
        default { endianness: "little" }

        byte arrOfBytes[4];           // No endianness conversion: [0x00, 0x11, 0x22, 0x33]
        bytes4 chunkOfBytes;          // Endianness applied: [0x33221100]
        bytes2 twoChunksOfBytes[2];   // Endianness applied per chunk: [0x1100, 0x3322]
    }
```

## Pre-Defined Variables

Mynic supports defining constants that do not consume bytes during parsing. Similar to `#define` in C/C++, the `define` keyword creates constants for use throughout the file, including in array sizes and conditional branches. These constants are untyped, so care should be taken to use them appropriately based on context.

```mynic
    define ARR_LEN 2;           // Integer constant
    define MAX_LEN 0xFF;        // Hexadecimal constant
    define OK_STATUS "OK";      // String constant
```

## Bitfields

Normally, primitive fields are byte-aligned. To define bit-level fields, use a `bitfield` block. Bitfields automatically align to byte boundaries; if a bitfield does not end on a byte boundary, a padding field is automatically generated. Specify the bit width using a colon after the field name.

```mynic
    packet BITFIELD_PACKET {
        bitfield BITFIELD_NAME {
            uint field1 : 3;  // 3-bit unsigned integer field
            bool field2 : 1;  // 1-bit boolean field
            // Bitfield is 4 bits total; a 4-bit _remainder field is auto-generated for byte alignment
        }
    }
```

## Union Fields

Union fields allow interpreting the same bytes in multiple ways. All subfields in a union field occupy the same byte range, so all must be identical in size. The following example demonstrates parsing 2 bytes as either a number or a byte stream:

```mynic
    packet UNION_PACKET {
        union UNION_FIELD {
            uint16 asNum;   // Interpret as 16-bit unsigned integer
            bytes2 raw;     // Interpret as 2 raw bytes
        }
        byte byte3;         // Third byte of the stream
    }
```

## Segments

As noted previously, packets cannot be nested within other packets. Segments provide a workaround for this limitation. Unlike packets, segments cannot serve as entry points for parsing but can be nested within packets or other segments. Reusable segments (defined at file scope) can be used multiple times as datatypes. Nested segments (lambda segments) are defined inside packets and group subfields together for output formatting (such as JSON), but are used only within that packet.

```mynic
    segment SEGMENT1 {
        ipv4address ipAddr;  // 32-bit IPv4 address
        uint16 portNum;      // 16-bit port number
    }

    packet SEGMENTS_EXAMPLE {
        uint8 num;
        SEGMENT1 ipInfo;     // Reusable segment (48 bits total)

        segment SEGMENT2 {   // Lambda segment: scoped to this packet, groups subfields
            bool isTrue;
            double bigDecimal;
        }
    }
```

## Branches & Switches

Branch fields enable conditional parsing of segments or other packets based on the current byte value. The interpreter reads a value (default: `byte`), evaluates conditions in order, and jumps to the matching segment. Branch conditions do not advance the byte stream position; subsequent fields continue from the same position. If no conditions match, an optional default branch is taken. Processing stops after the first matching condition (short-circuit behavior).

```mynic
    segment SEGMENT1 { uint16 num; }
    segment SEGMENT2 { bool isTrue; }
    // ... additional segments ...

    packet BRANCH_PACKET {
        branch uint8 {
            SEGMENT1 if 0;            // Jump to SEGMENT1 if value equals 0
            SEGMENT2 if 1;            // Jump to SEGMENT2 if value equals 1 (equivalent to "== 1")
            SEGMENT3 if <= 3;         // Jump if less than or equal to 3 (but not 0 or 1, already checked)
            segment SEGMENT4 {
                // Lambda segment definition and inline parsing
            } if > 16;                // Jump to SEGMENT4 if value greater than 16
            SEGMENT_DEFAULT defaults; // Default branch if no conditions match
        }
        byte b;  // Continue parsing after branch completes
    }
```

Switch statements are similar to branches but evaluate a previously parsed variable instead of the current byte stream position.

```mynic
    packet SWITCH_PACKET {
        uint8 num;
        bytes filler[8];
        switch num {
            SEGMENT1 if 0;  // Jump to SEGMENT1 if num equals 0
        }
    }
```

## Enumerations

Enumerations (enums) are one of Mynic's most powerful features, enabling the interpretation of raw byte values as symbolic names. Enums must specify a name and an unsigned integer datatype of any size. Once declared, an enum becomes a usable datatype in primitive variable declarations.

```mynic
    enum uint8 EXAMPLE_ENUM {
        ZERO = 0,   // Explicit value
        ONE,        // Auto-assigned to 1
        TWO,        // Auto-assigned to 2
        THREE,      // Auto-assigned to 3
        SEVEN = 7,  // Explicit value
        EIGHT       // Auto-assigned to 8
    }

    packet ENUM_PACKET {
        EXAMPLE_ENUM primitiveEnum;  // 8-bit field with enum values
    }
```

The preceding example demonstrates basic enum usage. Enums can also store additional metadata. Each enum member is a data object that can hold key-value pairs for use within the program. The following example demonstrates an enum used in an event system with metadata:

```mynic
    enum uint8 EVENTS {
        EVENT_POWER_ON = 1 {detailSize = 1, isCritical = true},
        EVENT_RESTART {detailSize = 2, isCritical = true},
        EVENT_POWER_OFF {detailSize = 1, isCritical = true}
    }

    packet EVENT_PACKET {
        EVENTS event;
        bytes rawDetails[event.detailSize];  // Array size determined by the parsed event's detailSize metadata
    }
```

## Import Statements

To connect multiple Mynic files together, use the `import` statement. Note that if the file cannot be found, it will silently fail.

```mynic
    import "newFile.myn"
```

## Keywords & Functions

Mynic does not support user-defined functions. However, several built-in functions are available for common tasks. Refer to [language-reference.md](language-reference.md) for a complete list and description of all keywords and built-in functions.

---
