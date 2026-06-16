#include "sy6974.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::sy6974 {

static const char *const TAG = "sy6974";

bool SY6974Component::read_all_registers_() {
  // Read all registers from 0x00 to 0x0A in one transaction (11 bytes)
  if (!this->read_bytes(SY6974_REG_INPUT_CURRENT_LIMIT, this->data_.registers, SY6974_NUM_REGS)) {
    ESP_LOGW(TAG, "Failed to read registers 0x00-0x0A");
    return false;
  }

  return true;
}

bool SY6974Component::write_register_(uint8_t reg, uint8_t value) {
  if (!this->write_byte(reg, value)) {
    ESP_LOGW(TAG, "Failed to write register 0x%02X", reg);
    return false;
  }
  return true;
}

bool SY6974Component::update_register_(uint8_t reg, uint8_t mask, uint8_t value) {
  uint8_t reg_value;
  if (!this->read_byte(reg, &reg_value)) {
    ESP_LOGW(TAG, "Failed to read register 0x%02X for update", reg);
    return false;
  }
  reg_value = (reg_value & ~mask) | (value & mask);
  return this->write_register_(reg, reg_value);
}

void SY6974Component::setup() {
  ESP_LOGV(TAG, "Setting up SY6974...");

  // Try to read chip ID
  uint8_t reg_value;
  if (!this->read_byte(SY6974_REG_STATUS1, &reg_value)) {
    ESP_LOGE(TAG, "Failed to communicate with SY6974");
    this->mark_failed();
    return;
  }

  uint8_t dev_rev = reg_value & 0x03;
  if (dev_rev != 0x00) {
    ESP_LOGW(TAG, "Unexpected chip revision: 0x%02X (expected 0x00)", dev_rev);
  }
  uint8_t dev_pn = (reg_value >> 3)  & 0x0F;
  if (dev_pn != 0x05) {
    ESP_LOGW(TAG, "Unexpected chip revision: 0x%02X (expected 0x05)", dev_pn);
  }

  // Apply configuration options (all have defaults now)
  ESP_LOGV(TAG, "Setting LED enabled to %s", ONOFF(this->led_enabled_));
  this->set_led_enabled(this->led_enabled_);

  ESP_LOGV(TAG, "Setting input current limit to %u mA", this->input_current_limit_);
  this->set_input_current_limit(this->input_current_limit_);

  ESP_LOGV(TAG, "Setting charge voltage to %u mV", this->charge_voltage_);
  this->set_charge_target_voltage(this->charge_voltage_);

  ESP_LOGV(TAG, "Setting charge current to %u mA", this->charge_current_);
  this->set_charge_current(this->charge_current_);

  ESP_LOGV(TAG, "Setting precharge current to %u mA", this->precharge_current_);
  this->set_precharge_current(this->precharge_current_);

  ESP_LOGV(TAG, "Setting charge enabled to %s", ONOFF(this->charge_enabled_));
  this->set_charge_enabled(this->charge_enabled_);

  ESP_LOGV(TAG, "Setting ADC measurements to %s", ONOFF(this->enable_adc_));
  this->set_enable_adc_measure(this->enable_adc_);

  ESP_LOGV(TAG, "SY6974 initialized successfully");
}

void SY6974Component::dump_config() {
  ESP_LOGCONFIG(TAG,
                "SY6974:\n"
                "  LED Enabled: %s\n"
                "  Input Current Limit: %u mA\n"
                "  Charge Voltage: %u mV\n"
                "  Charge Current: %u mA\n"
                "  Precharge Current: %u mA\n"
                "  Charge Enabled: %s\n"
                "  ADC Enabled: %s",
                ONOFF(this->led_enabled_), this->input_current_limit_, this->charge_voltage_, this->charge_current_,
                this->precharge_current_, ONOFF(this->charge_enabled_), ONOFF(this->enable_adc_));
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with SY6974 failed!");
  }
}

void SY6974Component::update() {
  if (this->is_failed()) {
    return;
  }

  // Read all registers in one transaction
  if (!this->read_all_registers_()) {
    ESP_LOGW(TAG, "Failed to read registers during update");
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();

  // Notify all listeners with the new data
  for (auto *listener : this->listeners_) {
    listener->on_data(this->data_);
  }
}

void SY6974Component::set_input_current_limit(uint16_t milliamps) {
  if (this->is_failed())
    return;

  if (milliamps < INPUT_CURRENT_MIN) {
    milliamps = INPUT_CURRENT_MIN;
  }

  uint8_t val = (milliamps - INPUT_CURRENT_MIN) / INPUT_CURRENT_STEP;
  if (val > 0x1F) {
    val = 0x1F;
  }

  this->update_register_(SY6974_REG_CONTROL0, 0x1F, val);
}

void SY6974Component::set_charge_target_voltage(uint16_t millivolts) {
  if (this->is_failed())
    return;

  if (millivolts < CHG_VOLTAGE_BASE) {
    millivolts = CHG_VOLTAGE_BASE;
  }

  uint8_t val = (millivolts - CHG_VOLTAGE_BASE) / CHG_VOLTAGE_STEP;
  if (val > 0x1F) {
    val = 0x1F;
  }

  this->update_register_(SY6974_REG_CONTROL4, 0xF8, val << 3);
}

void SY6974Component::set_precharge_current(uint16_t milliamps) {
  if (this->is_failed())
    return;

  if (milliamps < PRE_CHG_BASE_MA) {
    milliamps = PRE_CHG_BASE_MA;
  }

  uint8_t val = (milliamps - PRE_CHG_BASE_MA) / PRE_CHG_STEP_MA;
  if (val > 0x0F) {
    val = 0x0F;
  }

  this->update_register_(SY6974_REG_CONTROL3, 0xF0, val << 4);
}

void SY6974Component::set_charge_current(uint16_t milliamps) {
  if (this->is_failed())
    return;

  if (milliamps < FAST_CHG_BASE_MA) {
    milliamps = FAST_CHG_BASE_MA;
  }

  uint8_t val = (milliamps - FAST_CHG_BASE_MA) / FAST_CHG_STEP_MA;
  if (val > 0x3F) {
    val = 0x3F;
  }

  this->update_register_(SY6974_REG_CONTROL2, 0x3F, val);
}

void SY6974Component::set_charge_enabled(bool enabled) {
  if (this->is_failed())
    return;

  this->update_register_(SY6974_REG_CONTROL1, 0x10, enabled ? 0x10 : 0x00);
}

void SY6974Component::set_led_enabled(bool enabled) {
  if (this->is_failed())
    return;

  this->update_register_(SY6974_REG_CONTROL0, 0x60, enabled ? 0x00 : 0x60);
}

}  // namespace esphome::sy6974
