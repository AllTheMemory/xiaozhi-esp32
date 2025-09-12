#include "application.h"
#include "assets/lang_config.h"
#include "button.h"
#include "codecs/no_audio_codec.h"
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
  Button boot_button_;
  Button volume_up_button_;
  Button volume_down_button_;

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
    InitializeButtons();
  }

  virtual Led *GetLed() override {
    static SingleLed led(BUILTIN_LED_GPIO);
    return &led;
  }

  virtual AudioCodec* GetAudioCodec() override {
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
        return &audio_codec;
    }
};

DECLARE_BOARD(BidAudioBoard);
