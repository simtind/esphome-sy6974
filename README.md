# `ESPHome` components

[![ESPHome release][esphome-release-shield]][esphome-release]
[![Open in Visual Studio Code][open-in-vscode-shield]][open-in-vscode]

[license-shield]: https://img.shields.io/static/v1?label=License&message=MIT&color=orange&logo=license
[license]: https://opensource.org/licenses/MIT
[esphome-release-shield]: https://img.shields.io/static/v1?label=ESPHome&message=2026.5.3&color=green&logo=esphome
[esphome-release]: https://GitHub.com/esphome/esphome/releases/
[open-in-vscode-shield]: https://img.shields.io/static/v1?label=+&message=Open+in+VSCode&color=blue&logo=visualstudiocode
[open-in-vscode]: https://open.vscode.dev/simtind/esphome-sy6974

An ESPHome component that allows you to control the sy6974 battery management IC. Originally developed for use with the reTerminal e1001. The component is adapted from the built-in ESPHome support for sy6970. This module remaps registers and removes the analog sensor component to adapt the existing functionality.

To use this repository you should configure it inside your yaml-configuration:

```yaml
external_components:
  - source: github://simtind/esphome-sy6974
```

Then use it the same way you would use the built-in sy6970 module. [See ESPHome documentation](https://esphome.io/components/sensor/sy6970/) for usage information. Note that SY6974 doesn't support analog measurements, so only binary and text sensors are available.