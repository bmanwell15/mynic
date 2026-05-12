# Implementation Details and System Architecture

This document provides a comprehensive overview of the Mynic project's implementation, detailing the architecture, components, and the purpose of each source file. Mynic is implemented in C++20 and follows a 3-layer architectural pattern: Core, Adapters, and Distributions.

## Architecture Overview

Mynic's architecture is designed for modularity and extensibility:

- **Core Layer**: Contains the fundamental components for parsing Mynic language definitions and interpreting binary data
- **Adapters Layer**: Provides interfaces for data transformation and I/O operations
- **Distributions Layer**: Application-specific implementations (CLI, API library)

## Core Layer

The Core layer implements the core functionality of the Mynic language processor, including lexical analysis, syntax parsing, and binary data interpretation.

### AST.h / AST.cpp

**Purpose**: Defines the Abstract Syntax Tree (AST) structures and implements the parser that converts tokenized Mynic source code into an AST representation.

**Key Components**:
- `AST` class: Main parser class that processes tokens into AST nodes
- Node type definitions: Enums and structs for different AST node types (packets, segments, primitives, etc.)
- Expression handling: Support for mathematical expressions, conditions, and function calls
- Type system: Definitions for data types, enums, typedefs, and complex structures

**Implementation Details**:
- Uses recursive descent parsing to build the AST
- Supports advanced features like unions, bitfields, arrays, and conditional branches
- Maintains a symbol table for typedef aliases and enum definitions
- Handles include directives for modular protocol definitions

### BitQueue.h

**Purpose**: Header-only class that provides bit-level manipulation of binary data streams.

**Key Features**:
- Bit-precise reading from byte arrays
- Support for popping and peeking bits
- Rewind functionality for backtracking
- Position tracking for stream management

**Implementation Details**:
- Efficient bit manipulation using bitwise operations
- Handles endianness considerations
- Optimized for performance in high-throughput parsing scenarios

### ErrorHandler.h / ErrorHandler.cpp

**Purpose**: Centralized error and warning handling system for the Mynic interpreter.

**Key Components**:
- Error reporting with detailed context and line information
- Warning system with categorized warning codes
- Token-based error location reporting
- Block depth tracking for nested structure errors

**Implementation Details**:
- Static methods for throwing errors and warnings
- Integration with lexer for precise error location
- Support for different error severity levels

### Interpreter.h / Interpreter.cpp

**Purpose**: Executes the AST against binary data to produce decoded packet structures.

**Key Components**:
- `Interpreter` class: Main execution engine
- `DecodedPacket` structure: Output format for parsed data
- Expression evaluation: Runtime evaluation of mathematical and logical expressions
- Field interpretation: Conversion of AST nodes to runtime values

**Implementation Details**:
- BitQueue integration for binary data processing
- Support for complex data types (primitives, arrays, unions, bitfields)
- Runtime expression evaluation with variable resolution
- Warning collection during interpretation

### lexer.h / lexer.cpp

**Purpose**: Lexical analyzer that tokenizes Mynic source code into a stream of tokens.

**Key Components**:
- `Lexer` class: Tokenization engine
- Token definitions: Comprehensive token types for the Mynic language
- File handling: Support for multi-file includes
- Block depth tracking: Maintains nesting level information

**Implementation Details**:
- State machine-based tokenization
- Support for comments, literals, and identifiers
- Line number and file index tracking
- Whitespace and newline handling

### MynicLib.h / MynicLib.cpp

**Purpose**: Standard library functions available within Mynic programs.

**Key Functions**:
- Mathematical operations: `mathMax`, `mathMin`, `mathPow`, `mathSqrt`, etc.
- Control flow: `rewind`, `skip`, `seek`, `terminateIf`
- Constants: `mathPi`, `mathE`

**Implementation Details**:
- Static methods for library function implementation
- Integration with interpreter for runtime execution
- Type-safe parameter handling

## Adapters Layer

The Adapters layer provides modular interfaces for data transformation and external system integration.

### adapters.h

**Purpose**: Header file that aggregates all adapter modules for convenient inclusion.

**Contents**: Includes `json.h`, `FileHandler.h`, and `objects.h`

### json.h / json.cpp

**Purpose**: JSON serialization adapter for converting decoded packets to JSON format.

**Key Features**:
- Complete JSON encoding of `DecodedPacket` structures
- Support for nested objects and arrays
- Proper handling of different data types
- Timestamp and metadata inclusion

**Implementation Details**:
- Recursive JSON object construction
- Type-aware value serialization
- Integration with interpreter output structures

### FileHandler.h / FileHandler.cpp

**Purpose**: File I/O utilities for reading and writing data files.

**Key Functions**:
- `read`: Read entire file contents
- `write`: Write content to file (overwrite)
- `append`: Append content to existing file

**Implementation Details**:
- Standard C++ file stream operations
- Error handling for file operations
- UTF-8 text file support

### objects.h / objects.cpp

**Purpose**: Object-oriented interfaces for schema inspection and debugging.

**Key Functions**:
- `printSchema`: Display AST structure
- `printPacket`: Display packet definitions
- `printSegment`: Display segment definitions

**Implementation Details**:
- Tree traversal for AST visualization
- Formatted output for debugging
- Integration with AST node structures

## API Layer

The API layer provides the main programmatic interface for using Mynic functionality.

### Mynic.h / Mynic.cpp

**Purpose**: Main API class that orchestrates the entire Mynic processing pipeline.

**Key Methods**:
- `loadFile`: Load and parse Mynic definition files
- `decodePacket`: Decode binary data using loaded definitions
- `decodeFile`: Process multiple packets from a file
- `exportToFile`: Save decoded results to file
- `printSchema` / `printPacket` / `printSegment`: Schema inspection

**Implementation Details**:
- Coordinates lexer, AST, and interpreter components
- Supports both string and byte array input
- File-based batch processing capabilities
- Integration with all adapter modules

## CLI Layer

The CLI layer provides a command-line interface for interactive use of Mynic.

### CommandHandler.h / CommandHandler.cpp

**Purpose**: Command processing and execution for the CLI application.

**Key Features**:
- Command parsing and dispatch
- Interactive session management
- File loading and interpretation commands
- Export functionality

**Implementation Details**:
- Static command handler methods
- Integration with Mynic API
- Command-line argument processing

### main.cpp

**Purpose**: Entry point for the CLI application.

**Key Components**:
- Command-line argument parsing
- Interactive REPL loop
- Signal handling
- Application lifecycle management

**Implementation Details**:
- Main function with argument processing
- Integration with CommandHandler
- Graceful shutdown handling

## Build System

Mynic uses CMake for cross-platform builds:

- **Core Library**: Built as object files included in distributions
- **API Library**: Shared library (`mynic_api.so`) for external integration
- **CLI Executable**: Standalone binary with embedded core functionality

### Dependencies

- C++20 compatible compiler
- Standard C++ libraries
- CMake 3.5+

### Compilation Flags

- Optimization: `-O3` for performance
- Debugging: `-g` for development builds
- Sanitization: Address sanitizer for memory safety

## Data Flow

1. **Source Loading**: Mynic files are read and tokenized by the Lexer
2. **Parsing**: Tokens are parsed into AST by the AST class
3. **Interpretation**: AST is executed against binary data by the Interpreter
4. **Output**: Results are formatted via Adapters (JSON, file output, etc.)
5. **Distribution**: CLI or API provides the final interface

## Key Design Decisions

- **Modular Architecture**: Clear separation of concerns across layers
- **Header-Only Components**: BitQueue for performance-critical code
- **Static Methods**: Library functions and error handlers for thread safety
- **Shared Ownership**: Extensive use of `std::shared_ptr` for AST management
- **Type Safety**: Strong typing with `std::variant` for value representation
- **Performance Focus**: Bit-level operations and optimized data structures

## Future Considerations

- Unit test framework integration
- Performance profiling and optimization
- Extended adapter ecosystem
- Plugin architecture for custom data types
