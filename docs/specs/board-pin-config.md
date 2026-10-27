# Board Pin Configuration Spec

Scope: repo

## Requirements

- Keep each production GPIO and shift-register bit value in one source: `src/macros.h`.
- Provide a `BoardConfig` structured view (`boardCfg()`) initialized from those definitions; do not maintain a second set of pin literals.
- Keep pulse input/output, LED, UART, RS-232, RS-485, and Wiegand mappings together.
- `validateBoardConfig()` checks physical GPIO uniqueness and shift-register bit ranges at runtime.
- `UARTBus` prevents simultaneous use of shared UART2/GPIO16/GPIO17 routes by MDB, RS-485, and RS-232 channel 2.
- Add another board definition only when a second board revision exists; avoid speculative selectors.

## Scope

- **REPO** — source pin mapping used by SDK peripherals.
