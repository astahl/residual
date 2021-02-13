#include "SDL.h"
#include <optional>
#include <string>
#include <vector>

namespace ReSDL {
    struct AudioDevice {
        SDL_AudioDeviceID id = -1;

        AudioDevice(SDL_AudioDeviceID id)
            : id{id}
        {}

        AudioDevice(const AudioDevice&) = delete;
        AudioDevice(AudioDevice&& other) noexcept {
            std::swap(this->id, other.id);
        }

        AudioDevice& operator=(const AudioDevice&) = delete;
        AudioDevice& operator=(AudioDevice&& other) noexcept {
            if (this != &other) {
                std::swap(this->id, other.id);
            }
            return *this;
        }

        ~AudioDevice() {
            if (id > 0) {
                SDL_CloseAudioDevice(id);
            }
        }

        static std::optional<AudioDevice> open(const std::string& name, bool isCapture, const SDL_AudioSpec& desired, SDL_AudioSpec& obtained, int allowedChange = SDL_AUDIO_ALLOW_ANY_CHANGE) {
            auto deviceId = SDL_OpenAudioDevice(name.empty() ? nullptr : name.c_str(), isCapture, &desired, &obtained, allowedChange);
            if (deviceId >= 2) return std::make_optional(deviceId);
            else return std::nullopt;
        }

        static std::vector<std::string> enumerate(bool isCapture) {
            int count = SDL_GetNumAudioDevices(isCapture);
            std::vector<std::string> names;
            for (int i = 0; i < count; ++i) {
                names.push_back(SDL_GetAudioDeviceName(i, isCapture));
            }
            return names;
        }

        void pause() {
            SDL_PauseAudioDevice(this->id, 1);
        }

        void unpause() {
            SDL_PauseAudioDevice(this->id, 0);
        }

        int getQueuedAudioSize() {
            return SDL_GetQueuedAudioSize(this->id);
        }

        void clearQueuedAudio() {
            SDL_ClearQueuedAudio(this->id);
        }

        SDL_AudioStatus status() {
            return SDL_GetAudioDeviceStatus(this->id);
        }

        void lock() {
            SDL_LockAudioDevice(this->id);
        }

        void unlock() {
            SDL_UnlockAudioDevice(this->id);
        }

        template<typename T>
        void queueAudio(const T& data) {
            SDL_QueueAudio(this->id, std::data(data), std::size(data));
        }
    };
}