# M032 RGB LED USB HID Protocol

## USB interface

- USB class: vendor-defined HID
- VID: `0x0416`
- PID: `0x0125`
- Interrupt OUT: EP3, 64 bytes
- Interrupt IN: EP2, 64 bytes (reserved; command responses are not implemented yet)
- Host libraries may prepend report ID `0x00`; it is not part of the 64-byte protocol frame.

## Report format

| Offset | Size | Description |
|---:|---:|---|
| 0 | 1 | Magic, fixed `0xA5` |
| 1 | 1 | Protocol version, fixed `0x01` |
| 2 | 1 | Command ID |
| 3 | 1 | Host sequence number, echoed only in UART logs today |
| 4 | 1 | Payload length, `0..58` |
| 5 | 58 | Payload followed by zero padding |
| 63 | 1 | CRC-8 over bytes 0..62 |

CRC uses polynomial `0x07`, initial value `0x00`, no reflection and no final XOR.
Multi-byte values are not currently used; all fields are unsigned bytes.

## Commands

| ID | Name | Length | Payload from offset 5 | Firmware status |
|---:|---|---:|---|---|
| `0x01` | `GET_INFO` | 0 | None | Reserved. UART logs receipt, but EP2 does not return an info report yet. |
| `0x02` | `SET_CONFIG` | 3 | `[count, order, brightness]` | Implemented |
| `0x03` | `SET_STATIC` | 3 | `[red, green, blue]` | Implemented |
| `0x04` | `SET_PIXELS` | `1 + count*3` | `[count, R0,G0,B0, ...]` | Implemented, maximum 10 LEDs |
| `0x05` | `SET_EFFECT` | 7 | `[effect, red, green, blue, speed, brightness, direction]` | Implemented |
| `0x06` | `STOP` | 0 | None | Implemented; turns all LEDs off |

### SET_CONFIG

- `count`: `1..10`; an out-of-range value is ignored.
- `order`: `0=GRB/WS2812`, `1=RGB/LTST-E563CEGBW`.
- `brightness`: `0..255`, applied globally.
- Applying configuration preserves the active effect or direct-pixel source.
  When direct-pixel count increases, newly added pixels are initialized black.
- When count decreases, firmware sends one frame at the previous length with
  removed trailing pixels black before continuing at the shorter length. This
  clears colors already latched by downstream LEDs.

### SET_STATIC and SET_PIXELS

- Each color component is `0..255`.
- `SET_STATIC` preserves the brightness configured by `SET_CONFIG`.
- `SET_PIXELS` contains logical RGB triples; the firmware applies configured RGB/GRB wire order during SPI encoding.

### SET_EFFECT

| Effect | Value | RGB used | Speed used | Direction used |
|---|---:|---:|---:|---:|
| Static | 0 | Yes | No | No |
| Breathing | 1 | Yes | Yes | No |
| Blink | 2 | Yes | Yes | No |
| Gradient | 3 | Yes | Yes | Yes |
| Flow/rainbow | 4 | No | Yes | Yes |

- `speed`: `1..255`; zero is treated as 1 by the firmware.
- `brightness`: `0..255`; this command updates global brightness.
- `direction`: `0=Forward`, nonzero=`Reverse`.
- Unknown effect values fall back to Static.

## UART debug log

UART0 uses PB.12 RXD and PB.13 TXD at 115200, 8-N-1. Logging occurs from
`HID_Process()` in the main loop, never from the USB interrupt handler.

Examples:

```text
USB CMD seq=4 SET_CONFIG count=8 order=RGB brightness=128
USB CMD seq=5 SET_EFFECT effect=Gradient(3) rgb=255,0,0 speed=200 brightness=128 direction=Forward
USB CMD seq=6 STOP
USB RX invalid frame
```

## Extension guidance

When adding a command:

1. Assign its ID in `rgb_protocol.h` and the host-side command enum.
2. Define an exact payload length and byte layout here.
3. Add validation and handling in `HID_Process()`.
4. Keep long work out of `USBD_IRQHandler()`.
5. Add a UART log containing sequence, command name and key parameters.
6. If a response is required, define the EP2 IN report before advertising the command as bidirectional.
