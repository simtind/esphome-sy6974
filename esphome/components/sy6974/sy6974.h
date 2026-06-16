#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"
#include <vector>

namespace esphome::sy6974 {

// SY6974 Register addresses.
static const uint8_t SY6974_REG_CONTROL0 = 0x00;
static const uint8_t SY6974_REG_CONTROL1 = 0x01;
static const uint8_t SY6974_REG_CONTROL2 = 0x02;
static const uint8_t SY6974_REG_CONTROL3 = 0x03;
static const uint8_t SY6974_REG_CONTROL4 = 0x04;
static const uint8_t SY6974_REG_CONTROL5 = 0x05;
static const uint8_t SY6974_REG_CONTROL6 = 0x06;
static const uint8_t SY6974_REG_CONTROL7 = 0x07;
static const uint8_t SY6974_REG_STATUS0  = 0x08;
static const uint8_t SY6974_REG_FAULT    = 0x09;
static const uint8_t SY6974_REG_STATUS1  = 0x0A;
static const uint8_t SY6974_NUM_REGS     = 0x0B;

// Constants for voltage and current calculations
static const uint16_t VBUS_BASE_MV = 2600;       // mV
static const uint16_t VBUS_STEP_MV = 100;        // mV
static const uint16_t VBAT_BASE_MV = 2304;       // mV
static const uint16_t VBAT_STEP_MV = 20;         // mV
static const uint16_t VSYS_BASE_MV = 2304;       // mV
static const uint16_t VSYS_STEP_MV = 20;         // mV
static const uint16_t CHG_CURRENT_STEP_MA = 50;  // mA
static const uint16_t PRE_CHG_BASE_MA = 60;      // mA
static const uint16_t PRE_CHG_STEP_MA = 60;      // mA
static const uint16_t FAST_CHG_BASE_MA = 60;      // mA
static const uint16_t FAST_CHG_STEP_MA = 60;      // mA
static const uint16_t CHG_VOLTAGE_BASE = 3856;   // mV
static const uint16_t CHG_VOLTAGE_STEP = 32;     // mV
static const uint16_t INPUT_CURRENT_MIN = 100;   // mA
static const uint16_t INPUT_CURRENT_STEP = 100;  // mA

// Bus Status values (REG_08[7:5])
enum BusStatus {
  BUS_STATUS_NO_INPUT = 0,
  BUS_STATUS_USB_SDP = 1,
  BUS_STATUS_ADAPTER = 5,
  BUS_STATUS_OTG = 7,
};

// Charge Status values (REG_08[1:0])
enum ChargeStatus {
  CHARGE_STATUS_NOT_CHARGING = 0,
  CHARGE_STATUS_PRE_CHARGE = 1,
  CHARGE_STATUS_FAST_CHARGE = 2,
  CHARGE_STATUS_CHARGE_DONE = 3,
};

// Structure to hold all register data read in one transaction
struct SY6974Data {
  uint8_t registers[SY6974_NUM_REGS];
};

// Listener interface for components that want to receive SY6974 data updates
class SY6974Listener {
 public:
  virtual void on_data(const SY6974Data &data) = 0;
};

class SY6974Component : public PollingComponent, public i2c::I2CDevice {
 public:
  SY6974Component(bool led_enabled, uint16_t input_current_limit, uint16_t charge_voltage, uint16_t charge_current,
                  uint16_t precharge_current, bool charge_enabled)
      : led_enabled_(led_enabled),
        input_current_limit_(input_current_limit),
        charge_voltage_(charge_voltage),
        charge_current_(charge_current),
        precharge_current_(precharge_current),
        charge_enabled_(charge_enabled) {}
  void setup() override;
  void dump_config() override;
  void update() override;

  // Listener registration
  void add_listener(SY6974Listener *listener) { this->listeners_.push_back(listener); }

  // Configuration methods to be called from lambdas
  void set_input_current_limit(uint16_t milliamps);
  void set_charge_target_voltage(uint16_t millivolts);
  void set_precharge_current(uint16_t milliamps);
  void set_charge_current(uint16_t milliamps);
  void set_charge_enabled(bool enabled);
  void set_led_enabled(bool enabled);

 protected:
  bool read_all_registers_();
  bool write_register_(uint8_t reg, uint8_t value);
  bool update_register_(uint8_t reg, uint8_t mask, uint8_t value);

  SY6974Data data_{};
  std::vector<SY6974Listener *> listeners_;

  // Configuration values to set during setup()
  bool led_enabled_;
  uint16_t input_current_limit_;
  uint16_t charge_voltage_;
  uint16_t charge_current_;
  uint16_t precharge_current_;
  bool charge_enabled_;
};

}  // namespace esphome::sy6974
