# Contributing to Mynic

Thank you for your interest in contributing to Mynic! This document provides guidelines and information to help you contribute effectively to the project.

## Table of Contents

- [How to Build](#how-to-build)
- [How to Run Tests](#how-to-run-tests)
- [Coding Standards](#coding-standards)
- [PR Process](#pr-process)
- [Branch Naming](#branch-naming)
- [Commit Style](#commit-style)
- [Issue Labels](#issue-labels)
- [Documentation Standards](#documentation-standards)
- [Architecture Overview](#architecture-overview)
- [Beginner-Friendly Tasks](#beginner-friendly-tasks)

## How to Build

Mynic uses CMake for building. The project consists of multiple components: the core library, adapters, CLI, and API.

### Building the CLI

To build the CLI executable:

```bash
./compile_cli.sh
```

This script configures the build in `src/cli/build`, compiles the project, and moves the executable to the `dist` directory.

### Building the API

To build the shared library API:

```bash
./compile_api.sh
```

This script configures the build in `src/api/build`, compiles the project, and moves the library to the `dist` directory.

### Prerequisites

- CMake 3.5 or higher
- C++20 compatible compiler
- Standard C++ libraries

## How to Run Tests

Currently, the project does not have an automated test suite implemented. Manual testing can be performed by:

1. Building the CLI using the instructions above
2. Running the CLI with sample Mynic files from `MynicFiles/Examples/`
3. Verifying output against expected results

Future contributions should include comprehensive unit tests and integration tests.

## Coding Standards

Mynic follows modern C++ best practices:

- Use C++20 standard features
- Follow RAII principles
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) appropriately
- Prefer `const` correctness
- Use meaningful variable and function names
- Include inline comments for complex logic
- Keep functions small and focused on a single responsibility
- Use exceptions for error handling where appropriate
- Follow the existing code style (4-space indentation, consistent bracing)

## PR Process

All contributions should be made via Pull Requests (PRs). Each feature, bug fix, or optimization should have its own dedicated branch.

### PR Requirements

1. Create a feature branch from `main`
2. Implement your changes
3. Update documentation as needed
4. Include inline comments in code for new or modified complex sections
5. Test your changes thoroughly
6. Submit a PR with a clear description including:
   - What was changed
   - Why the change was made
   - Any breaking changes or dependencies

### Review Process

- PRs require at least one maintainer review
- All CI checks must pass
- Maintainers may request changes before merging
- Once approved, a maintainer will merge the PR

## Branch Naming

Use descriptive, lowercase branch names with hyphens:

- `feature-name` for new features
- `bugfix-name` for bug fixes
- `optimization-name` for performance improvements

Examples:

- `feature-add-json-export`
- `bugfix-fix-parser-crash`
- `optimization-improve-lexer-speed`

## Commit Style

Follow conventional commit format:

```
type(scope): description
```

Types:

- `feat`: New features
- `fix`: Bug fixes
- `docs`: Documentation changes
- `style`: Code style changes (formatting, etc.)
- `refactor`: Code refactoring
- `test`: Adding or modifying tests
- `chore`: Maintenance tasks

Examples:

- `feat(parser): add support for dynamic arrays`
- `fix(lexer): resolve memory leak in token parsing`
- `docs: update installation instructions`

## Issue Labels

Issues use a structured labeling system to organize the development workflow:

- `enhancement`: New features or improvements
- `bug`: Bug reports
- `documentation`: Documentation-related tasks
- `good first issue`: Suitable for new contributors
- `help wanted`: Community assistance needed
- `priority:high`: High priority items
- `priority:medium`: Medium priority items
- `priority:low`: Low priority items
- `status:in-progress`: Currently being worked on
- `status:blocked`: Blocked by other issues
- `type:feature`: Feature requests
- `type:maintenance`: Maintenance tasks

## Documentation Standards

- All public APIs must be documented with clear descriptions
- Include code examples where appropriate
- Update documentation when making changes to existing functionality
- Use Markdown for documentation files
- Keep the `docs/` directory organized and up-to-date
- Include inline comments for complex algorithms or non-obvious code

## Architecture Overview

Mynic follows a 3-layer architecture:

### Core Layer

The foundation of Mynic, containing:

- **AST (Abstract Syntax Tree)**: Represents parsed Mynic language constructs
- **Lexer**: Tokenizes Mynic source code
- **Interpreter**: Executes Mynic programs and parses binary data
- **Error Handler**: Manages error reporting and handling
- **MynicLib**: Core library functions and utilities

### Adapters Layer

Handles input/output and data transformation:

- **JSON Adapter**: Converts parsed data to JSON format
- **File Handler**: Manages file I/O operations
- **Objects Adapter**: Provides object-oriented interfaces

### Distributions Layer

Application-specific implementations:

- **CLI**: Command-line interface for direct usage
- **API**: Shared library for integration with other applications

This layered architecture ensures separation of concerns and allows for different distributions while maintaining a consistent core.

----