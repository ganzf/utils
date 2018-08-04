//
// Created by arroganz on 1/1/18.
//

#pragma once

# include <vector>
# include <string>
# include <sstream>
# include <unordered_map>
# include "math.hpp"

namespace futils {
    enum class EventType
    {
        Closed, Resized,
        LostFocus, GainedFocus,
        TextEntered,
        KeyPressed, KeyReleased,
        MouseWheelMoved,
        MouseWheelScrolled, MouseButtonPressed, MouseButtonReleased,
        MouseMoved, MouseEntered, MouseLeft,
        JoystickButtonPressed, JoystickButtonReleased,
        JoystickMoved, JoystickConnected, JoystickDisconnected, TouchBegan,
        TouchMoved, TouchEnded,
        SensorChanged,
        Count
    };

    enum class InputState : int {
        Undefined,
        Up,
        Down,
        GoingUp,
        GoingDown,
        Mouse,
        Joystick,
        Wheel
    };

    struct MouseClicked
    {
        futils::Vec2<int> pos;
    };

    struct MouseReleased
    {
        futils::Vec2<int> pos;
    };

    struct MouseMoved
    {
        futils::Vec2<int> current;
    };

    enum class Keys : int {
        Undefined = 0,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        ArrowUp, ArrowDown, ArrowLeft, ArrowRight,
        Return, Backspace, Space, Escape, Delete, Tab,
        LCtrl, RCtrl, LShift, RShift, Alt,
        Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
        Capslock, PageUp, PageDown,
        Ampersand, Hashtag, Quote, DoubleQuote, Dash, Underscore,
        LParenthesis, RParenthesis, LBracket, RBracket, LSquareBracket, RSquareBracket,
        Colon, SemiColon, QuestionMark, ExclamationMark, Comma, Dot, Percent, Asterisk,
        Slash, BackSlash,
        LButton, RButton, MouseWheelUp, MouseWheelDown, MouseWheelButton,
        JoystickA, JoystickB, JoystickX, JoystickY,
        NBR_SUPPORTED_KEYS
    };

    static const std::unordered_map<futils::Keys, char> keyToChar = {
            std::pair<Keys, char>(Keys::A, 'a'),
            std::pair<Keys, char>(Keys::B, 'b'),
            std::pair<Keys, char>(Keys::C, 'c'),
            std::pair<Keys, char>(Keys::D, 'd'),
            std::pair<Keys, char>(Keys::E, 'e'),
            std::pair<Keys, char>(Keys::F, 'f'),
            std::pair<Keys, char>(Keys::G, 'g'),
            std::pair<Keys, char>(Keys::H, 'h'),
            std::pair<Keys, char>(Keys::I, 'i'),
            std::pair<Keys, char>(Keys::J, 'j'),
            std::pair<Keys, char>(Keys::K, 'k'),
            std::pair<Keys, char>(Keys::L, 'l'),
            std::pair<Keys, char>(Keys::M, 'm'),
            std::pair<Keys, char>(Keys::N, 'n'),
            std::pair<Keys, char>(Keys::O, 'o'),
            std::pair<Keys, char>(Keys::P, 'p'),
            std::pair<Keys, char>(Keys::Q, 'q'),
            std::pair<Keys, char>(Keys::R, 'r'),
            std::pair<Keys, char>(Keys::S, 's'),
            std::pair<Keys, char>(Keys::T, 't'),
            std::pair<Keys, char>(Keys::U, 'u'),
            std::pair<Keys, char>(Keys::V, 'v'),
            std::pair<Keys, char>(Keys::W, 'w'),
            std::pair<Keys, char>(Keys::X, 'x'),
            std::pair<Keys, char>(Keys::Y, 'y'),
            std::pair<Keys, char>(Keys::Z, 'z'),
            std::pair<Keys, char>(Keys::Space , ' '),
            std::pair<Keys, char>(Keys::Num0, '0'),
            std::pair<Keys, char>(Keys::Num1, '1'),
            std::pair<Keys, char>(Keys::Num2, '2'),
            std::pair<Keys, char>(Keys::Num3, '3'),
            std::pair<Keys, char>(Keys::Num4, '4'),
            std::pair<Keys, char>(Keys::Num5, '5'),
            std::pair<Keys, char>(Keys::Num6, '6'),
            std::pair<Keys, char>(Keys::Num7, '7'),
            std::pair<Keys, char>(Keys::Num8, '8'),
            std::pair<Keys, char>(Keys::Num9, '9'),
            std::pair<Keys, char>(Keys::Dot, '.'),
    };

    struct InputAction
    {
        Keys key;
        InputState state;
        InputAction() = default;
        InputAction(Keys key, InputState state): key(key), state(state) {}
        bool operator==(const InputAction &other) const
        {
            return key == other.key && state == other.state;
        }
    };

    struct InputSequence
    {
        InputSequence() = default;
        InputSequence(futils::Keys key) {
            actions.push_back(InputAction(key, futils::InputState::GoingDown));
        }
        InputSequence(futils::Keys key, futils::InputState state) {
            actions.push_back(InputAction(key, state));
        }
        std::vector<InputAction> actions;

        bool operator==(const InputSequence &other) const
        {
            if (actions.size() != other.actions.size())
                return true;
            std::size_t index{0};
            for (;index < actions.size();index++)
            {
                auto &k1 = actions[index];
                auto &k2 = other.actions[index];
                if (k1.key != k2.key || k1.state != k2.state)
                    return false;
            }
            return true;
        }
    };
}

namespace std {
    template <>
    struct hash<futils::InputSequence>
    {
        using Key = futils::InputSequence;
        std::size_t operator()(const Key& k) const
        {
            std::string tmp;
            for (auto &a : k.actions)
            {
                tmp += std::to_string(futils::pairingFunction((int)a.key, (int)a.state));
            }
            std::stringstream sstream(tmp);
            std::size_t result;
            sstream >> result;
            return result;
        }
    };

    template <>
    struct hash<futils::InputAction>
    {
        using Key = futils::InputAction;
        std::size_t operator()(const Key& k) const {
            return (size_t)futils::pairingFunction((int)k.key, (int)k.state);
        }
    };
}
