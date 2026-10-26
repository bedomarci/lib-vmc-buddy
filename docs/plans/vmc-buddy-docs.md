---
plan name: vmc-buddy-docs
plan description: Full hardware+SDK documentation
plan status: done
---

## Idea
Maintain complete English-language technical documentation for the production VMC Buddy ESP32 board and Arduino SDK. The October 26, 2026 schematic set is the hardware source of truth. Keep overview, interface details, jumper settings, configuration entry points, and SDK API information linked without repeating technical explanations.

## Implementation
- Keep README.md as project entry point and link to docs/README.md
- Keep docs/README.md as documentation and schematic index
- Keep docs/hardware_interfaces.md focused on board overview, shared pin mapping, pulse I/O, and power
- Keep interface details and connector pinouts in MDB.md, Wiegand.md, RS232.md, and RS485.md
- Keep docs/configuration_guide.md as a setup index; document the 2x15 through-hole field and solder jumpers in docs/jumper_configuration.md
- Keep docs/configuration_guide.md as a short setup entry point; document SDK behavior in sdk_api_reference.md
- Keep docs/sdk_api_reference.md as the source of truth for implemented API, examples, and limitations
- Verify relative links, anchors, schematic references, and mappings against the production sheets

## Required Specs
<!-- SPECS_START -->
<!-- SPECS_END -->
