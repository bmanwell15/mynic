# Interpreter Warnings Guide

This guide describes all warning messages that can be produced during the interpretation of Mynic bytecode. Warnings indicate potential issues or edge cases encountered during packet parsing, but allow interpretation to continue (unlike errors, which halt execution).

## Warning List

### UNKNOWN (Code: 0)
**Status:** Reserved
**Message:** Undefined warning type.

**Description:** This is the default or undefined warning code. It should not be encountered in normal use and typically indicates an internal error in the interpreter.

---

### VALIDATION_FAIL (Code: 1)
**Status:** Reserved for Future Use
**Expected Usage:** Occurs when AST validation fails before interpretation begins.

**Description:** This warning is reserved for cases where the abstract syntax tree (AST) fails validation checks before the interpretation process begins. This would catch structural or semantic issues in the parsed Mynic code.

---

### PACKET_NOT_FOUND (Code: 2)
**Status:** Implemented
**Message:** `Packet definition for [packet_name] not found.`

**Description:** The specified packet name cannot be found in the abstract syntax tree. This warning occurs when attempting to interpret bytes using a packet definition that does not exist in the loaded Mynic file.

**Example:**
```mynic
// If no packet named "MY_PACKET" is defined
decodedPacket = interpreter.interpretBytes(data, "MY_PACKET", ast);
// Warning: Packet definition for MY_PACKET not found.
```

---

### EXPR_VAR_NOT_PRIMITIVE (Code: 3)
**Status:** Implemented (Multiple Uses)
**Messages:** 
- `Cannot do expr on non primitive variable '[variable_name]'.`
- `Value [value] does not map to a definition in enum '[enum_name]'`

**Description:** Occurs when attempting to use a non-primitive field in an expression context, or when an enumeration value doesn't match any defined enum member.

**Example:**
```mynic
packet EXAMPLE {
    segment SEGMENT1 { uint8 num; }
    
    bytes expr[SEGMENT1];  // Warning: Cannot do expr on non primitive variable
}
```

---

### ENUM_VAL_NOT_FOUND (Code: 4)
**Status:** Reserved for Future Use
**Expected Usage:** Occurs when attempting to access an enumeration value that doesn't exist.

**Description:** This warning would be triggered when a parsed value does not correspond to any member of an enumeration. Currently, this is reported under EXPR_VAR_NOT_PRIMITIVE.

---

### ENUM_NOT_FOUND (Code: 5)
**Status:** Reserved for Future Use
**Expected Usage:** Occurs when referencing an undefined enum type.

**Description:** This warning would be triggered when a field references an enum type that has not been defined in the Mynic file.

---

### ENUM_TYPE_NOT_INT (Code: 6)
**Status:** Implemented
**Message:** `Enum '[enum_name]' must have uint type.`

**Description:** Enumerations must use an unsigned integer datatype (uint8, uint16, uint32, uint64, etc.). This warning occurs when an enum is defined with a non-unsigned integer type.

**Example:**
```mynic
enum int8 BAD_ENUM {  // Error: int8 is signed
    VALUE1 = 0
}
```

---

### UNKNOWN_DATATYPE (Code: 7)
**Status:** Implemented
**Message:** `Unknown datatype: [datatype_name]`

**Description:** The specified datatype is not recognized by the interpreter. This occurs when a field uses a datatype that is neither a primitive type, an enum, nor a defined segment.

**Example:**
```mynic
packet EXAMPLE {
    mysteryType field;  // Warning: Unknown datatype: mysteryType
}
```

---

### INVALID_STRING_OPERATION (Code: 8)
**Status:** Implemented
**Message:** `Invalid operator '[operator]'. Strings '[string1]' and '[string2]' can only be added or compared for equality.`

**Description:** Occurs when an invalid binary operator is applied to string operands. Strings support only concatenation (`+`), equality (`==`), and inequality (`!=`) operations.

**Example:**
```mynic
// In an expression context with strings
string1 * string2  // Warning: Invalid operator '*'. Strings cannot be multiplied.
```

---

### INVALID_BINARY_OPERATOR_TYPE (Code: 9)
**Status:** Implemented
**Message:** `Invalid types for binary operator: [operator]`

**Description:** The binary operator cannot be applied to the given types. This occurs when attempting to use incompatible types with operators (e.g., multiplying incompatible types).

---

### FUNCTION_NOT_FOUND (Code: 10)
**Status:** Implemented
**Message:** `Function call '[class_name].[function_name]' not found.`

**Description:** A function call references a function that does not exist. This warning occurs when using a built-in function that is not implemented or when the function name is misspelled.

**Example:**
```mynic
packet EXAMPLE {
    float value;
    float result {expr: Math.nonexistent(value)};  // Warning: Function not found
}
```

---

### VARIABLE_NOT_FOUND_IN_EXPR (Code: 11)
**Status:** Implemented
**Message:** `Variable '[variable_name]' not found in expr.`

**Description:** A variable reference in an expression cannot be resolved. This occurs when an expression references a field that hasn't been parsed yet or doesn't exist.

**Example:**
```mynic
packet EXAMPLE {
    bytes data[undefinedSize];  // Warning: Variable 'undefinedSize' not found
}
```

---

### VARIABLE_NOT_FOUND_IN_SWITCH (Code: 12)
**Status:** Implemented
**Message:** `Variable '[variable_name]' in switch not found.`

**Description:** The variable referenced in a switch statement cannot be found in previously parsed fields. Switch statements must reference fields that have already been interpreted.

**Example:**
```mynic
packet EXAMPLE {
    switch undefinedVar {  // Warning: Variable 'undefinedVar' not found
        SEGMENT1 if 0;
    }
}
```

---

### ARRAY_LENGTH_NOT_INT (Code: 13)
**Status:** Implemented
**Message:** `Dynamic array length must be a numeric value, not [type].`

**Description:** A dynamic array length expression evaluates to a non-numeric type (typically a string). Array lengths must resolve to numeric values.

**Example:**
```mynic
packet EXAMPLE {
    define SIZE "ten";  // String constant
    bytes data[SIZE];   // Warning: Dynamic array length must be numeric
}
```

---

### WRONG_NUMBER_OF_PARAMETERS (Code: 14)
**Status:** Reserved for Future Use
**Expected Usage:** Occurs when a function is called with an incorrect number of arguments.

**Description:** This warning would be triggered when a built-in function is called with more or fewer parameters than it expects.

---

### VARIABLE_NOT_ARITHMETIC (Code: 15)
**Status:** Reserved for Future Use
**Expected Usage:** Occurs when a non-numeric variable is used in an arithmetic operation.

**Description:** This warning would be triggered when attempting to perform arithmetic operations on string or non-numeric variables in expression contexts.

---

### BIT_QUEUE_INDEX_OUT_OF_BOUNDS (Code: 16)
**Status:** Reserved for Future Use
**Expected Usage:** Occurs when attempting to access bit stream data beyond available bytes.

**Description:** This warning would be triggered when the interpreter attempts to pop more bits from the bit queue than are available in the input stream. Currently, a related warning BIT_QUEUE_EMPTY handles this scenario.

---

### BIT_QUEUE_EMPTY (Code: 17)
**Status:** Implemented
**Message:** `Attepted to pop bits when the bit stream is empty.`

**Description:** The interpreter attempted to read from the byte stream when no more data is available. This occurs when the input data is shorter than expected based on the packet definition, or when a dynamic array or variable-length field extends beyond the available data.

**Example:**
```
Input: 0x01 (1 byte)
Packet Definition: Expects 2 bytes
Warning: Attempted to pop bits when the bit stream is empty
```

---

## Summary Table

| Code | Warning Code | Status | Message |
|---|---|---|---|
| 0 | UNKNOWN | Reserved | Undefined warning type |
| 1 | VALIDATION_FAIL | Reserved | AST validation failure |
| 2 | PACKET_NOT_FOUND | ✓ Implemented | Packet definition not found |
| 3 | EXPR_VAR_NOT_PRIMITIVE | ✓ Implemented | Non-primitive in expression / enum value mismatch |
| 4 | ENUM_VAL_NOT_FOUND | Reserved | Enum value doesn't exist |
| 5 | ENUM_NOT_FOUND | Reserved | Enum type not defined |
| 6 | ENUM_TYPE_NOT_INT | ✓ Implemented | Enum must use uint type |
| 7 | UNKNOWN_DATATYPE | ✓ Implemented | Unknown datatype |
| 8 | INVALID_STRING_OPERATION | ✓ Implemented | Unsupported string operation |
| 9 | INVALID_BINARY_OPERATOR_TYPE | ✓ Implemented | Invalid binary operator for types |
| 10 | FUNCTION_NOT_FOUND | ✓ Implemented | Function not found |
| 11 | VARIABLE_NOT_FOUND_IN_EXPR | ✓ Implemented | Variable not found in expression |
| 12 | VARIABLE_NOT_FOUND_IN_SWITCH | ✓ Implemented | Variable not found in switch |
| 13 | ARRAY_LENGTH_NOT_INT | ✓ Implemented | Array length not numeric |
| 14 | WRONG_NUMBER_OF_PARAMETERS | Reserved | Function parameter count mismatch |
| 15 | VARIABLE_NOT_ARITHMETIC | Reserved | Non-numeric in arithmetic operation |
| 16 | BIT_QUEUE_INDEX_OUT_OF_BOUNDS | Reserved | Bit index exceeds available data |
| 17 | BIT_QUEUE_EMPTY | ✓ Implemented | Bit stream exhausted |

---

## Best Practices

When encountering warnings during interpretation:

1. **Review the warning message carefully** – It typically indicates the specific field, variable, or operation causing the issue.
2. **Check packet definitions** – Ensure all referenced packets, segments, and enums are properly defined.
3. **Validate variable references** – Variables used in expressions must be defined before the expression is evaluated.
4. **Verify input data** – If you encounter `BIT_QUEUE_EMPTY`, verify that your input data matches the packet definition's expected length.
5. **Check enum definitions** – Ensure enum types use unsigned integer datatypes and that parsed values match defined enum members.
