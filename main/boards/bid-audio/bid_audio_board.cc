#include "application.h"
#include "assets/lang_config.h"
#include "button.h"
#include "codecs/es8311_audio_codec.h"
#include "config.h"
#include "display/display.h"
#include "led/single_led.h"
#include "wifi_board.h"

#include <driver/i2c_master.h>
#include <esp_log.h>
#include <wifi_station.h>

#define TAG "BidAudioBoard"

class BidAudioBoard : public WifiBoard {
private:
  i2c_master_bus_handle_t codec_i2c_bus_;
  Button boot_button_;
  Button volume_up_button_;
  Button volume_down_button_;

  void InitializeCodecI2c() {
    i2c_master_bus_config_t i2c_bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
        .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags =
            {
                .enable_internal_pullup = 1,
            },
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    if (i2c_master_probe(codec_i2c_bus_, AUDIO_CODEC_ES8311_ADDR, 1000) !=
        ESP_OK) {
      while (true) {
        ESP_LOGE(TAG, "Failed to probe ES8311, check wiring");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }
    }
  }

  void InitializeButtons() {
    boot_button_.OnClick([this]() {
      auto &app = Application::GetInstance();
      if (app.GetDeviceState() == kDeviceStateStarting &&
          !WifiStation::GetInstance().IsConnected()) {
        ResetWifiConfiguration();
      }
      app.ToggleChatState();
    });
    boot_button_.OnPressDown(
        []() { Application::GetInstance().StartListening(); });
    boot_button_.OnPressUp(
        []() { Application::GetInstance().StopListening(); });

    volume_up_button_.OnClick([this]() {
      auto codec = GetAudioCodec();
      int volume = codec->output_volume() + 10;
      if (volume > 100)
        volume = 100;
      codec->SetOutputVolume(volume);
      GetDisplay()->ShowNotification(Lang::Strings::VOLUME +
                                     std::to_string(volume));
    });
    volume_up_button_.OnLongPress([this]() {
      GetAudioCodec()->SetOutputVolume(100);
      GetDisplay()->ShowNotification(Lang::Strings::MAX_VOLUME);
    });

    volume_down_button_.OnClick([this]() {
      auto codec = GetAudioCodec();
      int volume = codec->output_volume() - 10;
      if (volume < 0)
        volume = 0;
      codec->SetOutputVolume(volume);
      GetDisplay()->ShowNotification(Lang::Strings::VOLUME +
                                     std::to_string(volume));
    });
    volume_down_button_.OnLongPress([this]() {
      GetAudioCodec()->SetOutputVolume(0);
      GetDisplay()->ShowNotification(Lang::Strings::MUTED);
    });
  }

public:
  BidAudioBoard()
      : boot_button_(BOOT_BUTTON_GPIO),
        volume_up_button_(VOLUME_UP_BUTTON_GPIO),
        volume_down_button_(VOLUME_DOWN_BUTTON_GPIO) {
    InitializeCodecI2c();
    InitializeButtons();
  }

  virtual Led *GetLed() override {
    static SingleLed led(BUILTIN_LED_GPIO);
    return &led;
  }

  virtual AudioCodec *GetAudioCodec() override {
    static Es8311AudioCodec audio_codec(
        codec_i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE,
        AUDIO_OUTPUT_SAMPLE_RATE, AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK,
        AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
        AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
    return &audio_codec;
  }
};

DECLARE_BOARD(BidAudioBoard);
