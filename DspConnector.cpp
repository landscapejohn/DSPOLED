#include "easywsclient.hpp"
#include "jsmn.hpp"
#include <memory>
#include <cassert>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <string>
#include <array>

using easywsclient::WebSocket;
using namespace std;

namespace DspConnector
{
    float Volume;
    string ConfigName;
    string CaptureRate;
    string State;
    float CaptureSignalPeak[2];
    float PlaybackSignalPeak[4];
    bool Muted;

    bool _await_response;

    WebSocket::pointer ws = NULL;

    static int jsoneq(string json, jsmntok_t *tok, const char *s) {
        if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
            strncmp(json.c_str() + tok->start, s, tok->end - tok->start) == 0) {
            return 0;
        }
        return -1;
    }

    std::array<float, 4> _parse_signal_levels(string message)
    {
        jsmn_parser p;
        jsmntok_t t[32];

        jsmn_init(&p);
        int r = jsmn_parse(&p, message.c_str(), message.length(), t, sizeof(t) / sizeof(t[0]));
        if (r < 0) {
            throw ios_base::failure("Failed to parse JSON: " + r);
        }

        std::array<float, 4> signal_levels;
        int indexer = 0;

        for (int i = 1; i < r; i++)
        {
            if (t[i].type == JSMN_ARRAY)
            {
                continue;
            }

            if (indexer < 4)
            {
                signal_levels[indexer] = stof(message.substr(t[i].start, t[i].end - t[i].start));
                indexer++;
            }
        }

        return signal_levels;
    }

    string _parse_capture_rate(string message)
    {
        int capture_rate = stoi(message);

        if (capture_rate > 700000 && capture_rate < 800000)
            return "768 kHz";
        else if (capture_rate > 360000 && capture_rate < 400000)
            return "384 kHz";
        else if (capture_rate > 340000 && capture_rate < 360000)
            return "352.8 kHz";
        else if (capture_rate > 185000 && capture_rate < 200000)
            return "192 kHz";
        else if (capture_rate > 160000 && capture_rate < 185000)
            return "176.4 kHz";
        else if (capture_rate > 92000 && capture_rate < 100000)
            return "96 kHz";
        else if (capture_rate > 80000 && capture_rate < 92000)
            return "88.2 kHz";
        else if (capture_rate > 46000 && capture_rate < 52000)
            return "48 kHz";
        else if (capture_rate > 40000 && capture_rate < 46000)
            return "44.1 kHz";
        else if (capture_rate > 20000 && capture_rate < 24000)
            return "22,050 Hz";
        else if (capture_rate > 14400 && capture_rate < 17600)
            return "16 kHz";
        else if (capture_rate > 9900 && capture_rate < 1200)
            return "11,025 Hz";
        else if (capture_rate > 7200 && capture_rate < 8800)
            return "8 kHz";
        else
            return message;
    }

    void _parse_handled_message(string command, string value)
    {
        if (command.compare("GetConfigName") == 0)
        {
            ConfigName = value;
        }
        else if (command.compare("GetCaptureRate") == 0)
        {
            CaptureRate = _parse_capture_rate(value);
        }
        else if (command.compare("GetVolume") == 0)
        {
            Volume = stof(value);
        }
        else if (command.compare("GetState") == 0)
        {
            State = value;
        }
        else if (command.compare("GetCaptureSignalPeak") == 0)
        {
            std::array<float, 4> parsed_levels = _parse_signal_levels(value);
            CaptureSignalPeak[0] = parsed_levels[0];
            CaptureSignalPeak[1] = parsed_levels[1];
        }
        else if (command.compare("GetPlaybackSignalPeak") == 0)
        {
            std::array<float, 4> parsed_levels = _parse_signal_levels(value);
            PlaybackSignalPeak[0] = parsed_levels[0];
            PlaybackSignalPeak[1] = parsed_levels[1];
            PlaybackSignalPeak[2] = parsed_levels[2];
            PlaybackSignalPeak[3] = parsed_levels[3];
        }
        else if (command.compare("GetMute") == 0)
        {
            Muted = value == "true";
        }
    }

    void _handle_message(const std::string &message)
    {
        _await_response = false; // mark that the response has been received

        jsmn_parser p;
        jsmntok_t t[32];

        int i;
        int r;

        jsmn_init(&p);
        r = jsmn_parse(&p, message.c_str(), strlen(message.c_str()), t, sizeof(t) / sizeof(t[0]));
        if (r < 0)
        {
            throw ios_base::failure("Failed to parse JSON: " + r);
        }

        string command = message.substr(t[1].start, t[1].end - t[1].start);
        string result, value;

        command = message.substr(t[1].start, t[1].end - t[1].start);

        for (i = 1; i < r; i++)
        {
            if (jsoneq(message, &t[i], "result") == 0)
            {
                result = message.substr(t[i + 1].start, t[i + 1].end - t[i + 1].start);
            }
            else if (jsoneq(message, &t[i], "value") == 0)
            {
                value = message.substr(t[i + 1].start, t[i + 1].end - t[i + 1].start);
            }
            i++;
        }

        _parse_handled_message(command, value);
    }

    void _query_dsp(string command)
    {
        if (!ws)
        {
            ws = (WebSocket::from_url("ws://127.0.0.1:1234"));
        }

        ws->send("\"" + command + "\"");
        _await_response = true;
        while (ws->getReadyState() != WebSocket::CLOSED && _await_response)
        {
            ws->poll();
            ws->dispatch(_handle_message);
        }
    }

    void UpdateConfigName()
    {
        _query_dsp("GetConfigName");
    }

    void UpdateCaptureRate()
    {
        _query_dsp("GetCaptureRate");
    }

    void UpdateVolume()
    {
        _query_dsp("GetVolume");
    }

    void UpdateState()
    {
        _query_dsp("GetState");
    }

    void UpdateCaptureSignalPeak()
    {
        _query_dsp("GetCaptureSignalPeak");
    }

    void UpdatePlaybackSignalPeak()
    {
        _query_dsp("GetPlaybackSignalPeak");
    }

    void UpdateMuted()
    {
        _query_dsp("GetMute");
    }
};

//int main() {
//    DspConnector::UpdateConfigName();
//    DspConnector::UpdateCaptureRate();
//    DspConnector::UpdateVolume();
//    DspConnector::UpdateState();
//    DspConnector::UpdateCaptureSignalPeak();
//    DspConnector::UpdatePlaybackSignalPeak();
//    DspConnector::UpdateMuted();
//
//    cout << "Volume: " << DspConnector::Volume  << "\n";
//    cout << "Config Name: " << DspConnector::ConfigName << "\n";
//    cout << "Capture Rate: " << DspConnector::CaptureRate << "\n";
//    cout << "State: " << DspConnector::State << "\n";
//    cout << "Capture Signal Peak: " << DspConnector::CaptureSignalPeak[0] << "\t" << DspConnector::CaptureSignalPeak[1] << "\n";
//    cout << "Playback Signal Peak: " << DspConnector::PlaybackSignalPeak[0] << "\t" << DspConnector::PlaybackSignalPeak[1] << "\t" << DspConnector::PlaybackSignalPeak[2] << "\t" << DspConnector::PlaybackSignalPeak[3]  << "\n";
//    cout << "Muted: " << DspConnector::Muted << "\n";
//
//    return 0;
//}
