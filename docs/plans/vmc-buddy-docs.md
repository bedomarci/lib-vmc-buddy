---
plan name: vmc-buddy-docs
plan description: Full hardware+SDK documentation
plan status: done
---

## Idea
Create complete Hungarian-language technical documentation for the VMC Buddy ESP32-based vending machine gateway SDK. Documentation covers: (1) README.md with overview, block diagram, features matrix, project structure, quickstart; (2) docs/hardware_interfaces.md with detailed per-interface pinouts, electrical operation, and schematic references; (3) docs/configuration_guide.md with jumper settings, master/slave modes, example configs; (4) docs/sdk_api_reference.md with exact API signatures, class descriptions, usage examples. All content based on existing source code, macro definitions, and user-provided hardware descriptions.

## Implementation
- Create README.md with project overview, Mermaid block diagram, features matrix, project structure, and quickstart guide
- Create docs/hardware_interfaces.md with per-interface chapters: MDB, Wiegand, RS485, RS232, Pulse I/O with pinout tables and operation principles
- Create docs/configuration_guide.md with U32/U35 jumper tables, header jumpers, and practical master/slave config examples
- Create docs/sdk_api_reference.md with exact API signatures, class hierarchy, event system, and usage examples
- Verify all file references, cross-links between docs, and consistency with source code macros/APIs

## Required Specs
<!-- SPECS_START -->
<!-- SPECS_END -->