# Mynic Language Reference

This document provides a comprehensive reference for all primitive datatypes, keywords, and built-in functions available in the Mynic language.

---

## Primitive Datatypes

All primitive datatypes supported by Mynic, including their bit sizes:

### Basic Types

| Datatype                | Bit Size          | Description                                            |
| ----------------------- | ----------------- | ------------------------------------------------------ |
| `bit` / `bits`      | 1                 | Single bit                                             |
| `byte` / `bytes`    | 8                 | Single byte                                            |
| `bool`                | 8                 | Boolean value (note: 1 byte, not 1 bit)                |
| `char`                | 8                 | ASCII Character                                       |
| `short`               | 16                | Signed 16-bit integer                                  |
| `ushort`              | 16                | Unsigned 16-bit integer                                |
| `int`                 | 32                | Signed 32-bit integer                                  |
| `uint`                | 32                | Unsigned 32-bit integer                                |
| `float` / `float32` | 32                | 32-bit floating-point (IEEE 754)                       |
| `double`              | 64                | 64-bit floating-point (IEEE 754)                       |
| `long`                | 64                | Signed 64-bit integer                                  |
| `ulong`               | 64                | Unsigned 64-bit integer                                |
| `string`              | 8 (per character) | Variable-length null-terminated or fixed-length string |

### Variable-Sized Integer Types

Mynic supports arbitrary-sized integer types using the format `intX` and `uintX`, where `X` is the number of bits (1-64):

- `int1`, `int2`, ..., `int64` - Signed integers of specified bit width
- `uint1`, `uint2`, ..., `uint64` - Unsigned integers of specified bit width

Example:

```mynic
uint12 partialByte;  // 12-bit unsigned integer
int48 largeInt;      // 48-bit signed integer
```

### Variable-Sized Bit and Byte Types

Similar to integers, `bits` and `bytes` types support variable sizes:

- `bits1`, `bits2`, ..., `bits64` - Raw bits of specified width
- `bytes1`, `bytes2`, ..., `bytes8` - Raw bytes (bit size = number × 8)

Example:

```mynic
bits12 rawBits;      // 12 raw bits
bytes3 threeBytes;   // 24 bits (3 bytes)
```

### DateTime Types

| Datatype         | Bit Size | Description                    |
| ---------------- | -------- | ------------------------------ |
| `datetime32s`  | 32       | Unix timestamp in seconds      |
| `datetime64s`  | 64       | Unix timestamp in seconds      |
| `datetime64ms` | 64       | Unix timestamp in milliseconds |
| `datetime64us` | 64       | Unix timestamp in microseconds |
| `datetime64ns` | 64       | Unix timestamp in nanoseconds  |

### TimeSpan Types

| Datatype         | Bit Size | Description              |
| ---------------- | -------- | ------------------------ |
| `timespan32s`  | 32       | Duration in seconds      |
| `timespan64s`  | 64       | Duration in seconds      |
| `timespan64ms` | 64       | Duration in milliseconds |
| `timespan64us` | 64       | Duration in microseconds |
| `timespan64ns` | 64       | Duration in nanoseconds  |

### Network Address Types

| Datatype                           | Bit Size | Description                          |
| ---------------------------------- | -------- | ------------------------------------ |
| `ipv4Address` / `ipAddress32`  | 32       | IPv4 address (X.X.X.X format)        |
| `ipv6Address` / `ipAddress128` | 64       | IPv6 address (64-bit representation) |
| `macAddress` / `macAddress48`  | 48       | MAC address                          |

---

## Keywords & Variables

### Reserved Keywords

Standard keywords used in Mynic syntax:

- `packet` - Defines a packet structure (entry point for byte parsing)
- `segment` - Defines a reusable structure
- `enum` - Defines an enumeration
- `typedef` - Creates a type alias
- `define` - Defines a constant
- `default` - Sets default settings for fields
- `bitfield` - Groups bit-level fields
- `union` - Overlapping field interpretation
- `branch` - Conditional parsing based on current byte value
- `switch` - Conditional parsing based on previously parsed variable

### Built-In Variables

#### TO_END

**Type:** `uint` (read-only)
**Description:** Returns the number of remaining bytes in the bit stream. Useful for dynamic array sizing.

**Example:**

```mynic
packet EXAMPLE {
    uint8 header;
    byte payload[TO_END];  // Read remaining bytes
}
```

## Built-In Functions

### Math Functions

All math functions are called using the syntax: `Math.functionName(parameters)`

#### Math.max(param1, param2, ...)

**Parameters:** Numeric values (variable length)
**Returns:** Numeric value (maximum of all parameters)
**Description:** Returns the maximum value among all parameters.

**Example:**

```mynic
packet EXAMPLE {
    uint8 val1;
    uint8 val2;
    uint8 result {expr: Math.max(val1, val2)};
}
```

#### Math.min(param1, param2, ...)

**Parameters:** Numeric values (variable length)
**Returns:** Numeric value (minimum of all parameters)
**Description:** Returns the minimum value among all parameters.

**Example:**

```mynic
packet EXAMPLE {
    float temperature;
    float threshold {expr: Math.min(temperature, 100.0)};
}
```

#### Math.pow(base, exponent)

**Parameters:**

- `base` - Numeric value
- `exponent` - Numeric value
  **Returns:** Double (base raised to the power of exponent)
  **Description:** Computes base raised to the power of exponent.

**Example:**

```mynic
packet EXAMPLE {
    uint8 value;
    double result {expr: Math.pow(value, 2)};  // value squared
}
```

#### Math.sqrt(value)

**Parameters:** `value` - Numeric value
**Returns:** Double (square root)
**Description:** Computes the square root of the given value.

**Example:**

```mynic
packet EXAMPLE {
    uint16 squared;
    double root {expr: Math.sqrt(squared)};
}
```

#### Math.log(value)

**Parameters:** `value` - Numeric value
**Returns:** Double (natural logarithm)
**Description:** Computes the natural logarithm (base e) of the value.

#### Math.log10(value)

**Parameters:** `value` - Numeric value
**Returns:** Double (base-10 logarithm)
**Description:** Computes the base-10 logarithm of the value.

#### Math.log2(value)

**Parameters:** `value` - Numeric value
**Returns:** Double (base-2 logarithm)
**Description:** Computes the base-2 logarithm of the value.

**Example:**

```mynic
packet EXAMPLE {
    uint32 value;
    double log_val {expr: Math.log10(value)};
    double bits {expr: Math.log2(value)};
}
```

#### Math.round(value)

**Parameters:** `value` - Numeric value
**Returns:** Double (rounded value)
**Description:** Rounds the given value to the nearest integer.

**Example:**

```mynic
packet EXAMPLE {
    float measurement;
    double rounded {expr: Math.round(measurement)};
}
```

#### Math.sign(value)

**Parameters:** `value` - Numeric value
**Returns:** Integer (-1, 0, or 1)
**Description:** Returns the sign of a value: -1 for negative, 0 for zero, 1 for positive.

**Example:**

```mynic
packet EXAMPLE {
    int16 number;
    int result {expr: Math.sign(number)};
}
```

#### Math.abs(value)

**Parameters:** `value` - Numeric value
**Returns:** Numeric value (absolute value)
**Description:** Returns the absolute (non-negative) value.

**Example:**

```mynic
packet EXAMPLE {
    int16 offset;
    uint magnitude {expr: Math.abs(offset)};
}
```

#### Math.PI()

**Returns:** Double (π ≈ 3.14159...)
**Description:** Returns the mathematical constant Pi.

**Example:**

```mynic
packet EXAMPLE {
    float radius;
    double area {expr: radius * radius * Math.PI()};
}
```

#### Math.E()

**Returns:** Double (e ≈ 2.71828...)
**Description:** Returns the mathematical constant e (Euler's number).

**Example:**

```mynic
packet EXAMPLE {
    double result {expr: Math.E()};
}
```

---

### Stream Control Functions

#### REWIND(numOfBits)

**Parameters:** `numOfBits` - Unsigned integer
**Description:** Moves the bit stream position backward by the specified number of bits. Useful for re-reading previously parsed data.

**Example:**

```mynic
packet EXAMPLE {
    uint16 header;
    REWIND(16);  // Move back 16 bits to re-read header
    uint8 byte1;
    uint8 byte2;
}
```

#### SKIP(numOfBits)

**Parameters:** `numOfBits` - Unsigned integer
**Description:** Skips forward by the specified number of bits without parsing or storing data.

**Example:**

```mynic
packet EXAMPLE {
    uint8 type;
    SKIP(8);  // Skip 1 byte (8 bits)
    uint16 payload;
}
```

#### SEEK(bitPosition)

**Parameters:** `bitPosition` - Unsigned integer
**Description:** Sets the absolute position in the bit stream. The position is 0-indexed.

**Example:**

```mynic
packet EXAMPLE {
    uint8 header;
    SEEK(32);  // Jump to bit 32 (byte 4)
    uint32 data;
}
```

#### TERMINATE_IF(condition)

**Parameters:** `condition` - Boolean condition
**Description:** Stops parsing the current packet if the condition evaluates to true. Useful for early termination based on specific criteria.

**Example:**

```mynic
packet EXAMPLE {
    uint8 flags;
    TERMINATE_IF(flags == 0xFF);  // Stop parsing if flags is 0xFF
    uint32 payload;  // Not parsed if condition was true
}
```

#### RETURN_IF(condition) [Reserved for Future Use]

**Parameters:** `condition` - Boolean condition
**Description:** Reserved for future implementation. Would return from the current segment/packet if the condition is true.

---

## Operator Support

### Arithmetic Operators

- `+` - Addition
- `-` - Subtraction
- `*` - Multiplication
- `/` - Division
- `%` - Modulo

### Comparison Operators

- `==` - Equality
- `!=` - Inequality
- `<` - Less than
- `>` - Greater than
- `<=` - Less than or equal
- `>=` - Greater than or equal

### Logical Operators

- `&&` - Logical AND
- `||` - Logical OR

---

## Best Practices

1. **Use meaningful datatype sizes** - Choose the appropriate bit width for fields to match protocol specifications
2. **Leverage enums** - Use enumerations for fields with well-defined values
3. **Use typedef for reusability** - Create type aliases for commonly used datatypes
4. **Apply default settings** - Use `default` blocks to avoid repetition
5. **Document with comments** - Use `//` and `/* */` for clarity
6. **Test edge cases** - Verify behavior with minimum and maximum values
7. **Use stream control carefully** - REWIND, SKIP, and SEEK can be complex; use judiciously

---
