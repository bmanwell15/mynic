# Event System Example Tutorial

This tutorial walks through creating a complete event logging system for a drone using Mynic. We'll build a packet structure that can log various drone events like power cycles, image captures, and communication acknowledgments.

## Overview

The event system demonstrates several advanced Mynic features:
- Enums with metadata
- Conditional parsing using switch statements
- Variable-length arrays
- Bitfields for compact data storage
- Nested segments for complex data structures

## Step 1: Define Event Types with Enums

First, we'll define the different types of events our drone can log using an enum with metadata.

```mynic
enum uint8 EVENT_OP_CODE {
    POWER_ON,           {hasDetails: false, detailsSizeBytes: 0}
    POWER_OFF,          {hasDetails: true, detailsSizeBytes: 1}
    IMAGE_DOWNLINK,     {hasDetails: true, detailsSizeBytes: 1036}
    UPLINK_ACK,         {hasDetails: false, detailsSizeBytes: 0}
    UPLINK_NAK,         {hasDetails: true, detailsSizeBytes: 1}
}
```

**What this does:**
- Creates an 8-bit unsigned integer enum for event operation codes
- Each enum value has metadata indicating whether it has additional details and how many bytes those details occupy
- `POWER_ON` and `UPLINK_ACK` are simple events with no extra data
- `POWER_OFF`, `IMAGE_DOWNLINK`, and `UPLINK_NAK` have additional details

## Step 2: Define Error Codes (if needed)

For events that can fail, we need additional enums for error codes.

```mynic
enum uint8 NAK_ERROR_CODE {
    UNKNOWN,
    INVALID_PARAMS,
    INVALID_OPTION_SELECTED
}
```

**What this does:**
- Defines possible error codes for negative acknowledgments (NAKs)
- Used by the `UPLINK_NAK` event to specify why an uplink failed

## Step 3: Create Detail Segments for Complex Events

Now we'll create segments for events that have additional data. Each segment defines the structure of the extra information.

### Power Off Details

```mynic
segment EVENT_POWER_OFF {bool restarting;}
```

**What this does:**
- Creates a segment for power-off events
- Contains a boolean indicating whether the drone is restarting after power off

### Image Downlink Details

```mynic
segment EVENT_IMAGE_DOWNLINK {
    float lat;
    float lon;
    float alt;
    bytes imageData[1024];
}
```

**What this does:**
- Creates a segment for image downlink events
- Contains GPS coordinates (latitude, longitude, altitude) as 4-byte floats
- Contains 1024 bytes of raw image data

### Uplink NAK Details

```mynic
segment EVENT_UPLINK_NAK {NAK_ERROR_CODE error;}
```

**What this does:**
- Creates a segment for uplink negative acknowledgment events
- Contains an error code explaining why the uplink failed

## Step 4: Create the Main Event Segment with Conditional Parsing

The main event segment uses a switch statement to conditionally parse details based on the event type.

```mynic
segment EVENT {
    EVENT_OP_CODE eventOpcode;
    switch eventOpcode {
        EVENT_POWER_OFF if "POWER_OFF";
        EVENT_IMAGE_DOWNLINK if "IMAGE_DOWNLINK";
        EVENT_UPLINK_NAK if "UPLINK_NAK";
    }
}
```

**What this does:**
- Defines the structure of a single event
- Always contains an `eventOpcode` field
- Uses a switch statement to parse additional details only when needed:
  - If `eventOpcode` is `POWER_OFF`, parse `EVENT_POWER_OFF` details
  - If `eventOpcode` is `IMAGE_DOWNLINK`, parse `EVENT_IMAGE_DOWNLINK` details
  - If `eventOpcode` is `UPLINK_NAK`, parse `EVENT_UPLINK_NAK` details
- Events like `POWER_ON` and `UPLINK_ACK` have no additional parsing

## Step 5: Build the Complete Packet Structure

Finally, we'll create the main packet that contains all the drone's event information.

```mynic
packet DRONE_EVENT_PACKET {
    datetime32s timestamp;
    string craftId[5];

    bitfield BATTERY_INFO {
        uint batteryPercentage : 7 {units: "%"};
        bool onLowPowerMode : 1;
    }

    uint8 numberOfEvents;
    EVENT events[numberOfEvents];
}
```

**What this does:**
- `timestamp`: When this packet was created (32-bit signed datetime)
- `craftId`: Unique 5-character identifier for the drone (e.g., "LR203")
- `BATTERY_INFO`: Compact bitfield containing:
  - `batteryPercentage`: 7-bit unsigned integer (0-100%) with units
  - `onLowPowerMode`: 1-bit boolean flag
- `numberOfEvents`: How many events are logged in this packet
- `events`: Variable-length array of EVENT segments

## Alternative: Using TO_END for Dynamic Arrays

Instead of specifying a count, you can use `TO_END` to parse until the stream ends:

```mynic
// Instead of:
uint8 numberOfEvents;
EVENT events[numberOfEvents];

// You could use:
EVENT events[TO_END];
```

**When to use each approach:**
- Use a count field when you know exactly how many items to expect
- Use `TO_END` when the array fills the remainder of the packet

## Complete Example

Putting it all together, here's the complete event system:

```mynic
enum uint8 EVENT_OP_CODE {
    POWER_ON,           {hasDetails: false, detailsSizeBytes: 0}
    POWER_OFF,          {hasDetails: true, detailsSizeBytes: 1}
    IMAGE_DOWNLINK,     {hasDetails: true, detailsSizeBytes: 1036}
    UPLINK_ACK,         {hasDetails: false, detailsSizeBytes: 0}
    UPLINK_NAK,         {hasDetails: true, detailsSizeBytes: 1}
}

enum uint8 NAK_ERROR_CODE {
    UNKNOWN,
    INVALID_PARAMS,
    INVALID_OPTION_SELECTED
}

segment EVENT_POWER_OFF {bool restarting;}
segment EVENT_IMAGE_DOWNLINK {
    float lat;
    float lon;
    float alt;
    bytes imageData[1024];
}
segment EVENT_UPLINK_NAK {NAK_ERROR_CODE error;}

segment EVENT {
    EVENT_OP_CODE eventOpcode;
    switch eventOpcode {
        EVENT_POWER_OFF if "POWER_OFF";
        EVENT_IMAGE_DOWNLINK if "IMAGE_DOWNLINK";
        EVENT_UPLINK_NAK if "UPLINK_NAK";
    }
}

packet DRONE_EVENT_PACKET {
    datetime32s timestamp;
    string craftId[5];

    bitfield BATTERY_INFO {
        uint batteryPercentage : 7 {units: "%"};
        bool onLowPowerMode : 1;
    }

    uint8 numberOfEvents;
    EVENT events[numberOfEvents];
}
```

## Key Concepts Demonstrated

1. **Enums with Metadata**: Attach additional information to enum values for runtime decisions
2. **Conditional Parsing**: Use switch statements to parse different structures based on field values
3. **Variable-Length Arrays**: Parse arrays where the length is determined by another field
4. **Bitfields**: Pack multiple small values into compact bit representations
5. **Segment Composition**: Build complex structures by combining simpler segments
6. **Dynamic Parsing**: Parse different amounts of data based on the event type

This event system efficiently handles variable amounts of data while maintaining a compact binary format suitable for resource-constrained devices like drones.