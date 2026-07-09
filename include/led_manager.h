#ifndef LED_MANAGER_H
#define LED_MANAGER_H

class LedManager {
public:
    enum State {
        STATE_OFF,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_DISCONNECTED,
        STATE_PULSE_YELLOW,
        STATE_PULSE_BLUE,
        STATE_PULSE_WHITE,
        STATE_ALERT_RED
    };

    LedManager(int redPin, int greenPin, int bluePin);
    void begin();
    void update(unsigned long currentMillis);
    void setState(State state);
    State getState() const;
    void setEnabled(bool enabled);
    bool isEnabled() const;

private:
    void writePins(bool r, bool g, bool b);
    void turnOffAll();

    int _redPin;
    int _greenPin;
    int _bluePin;
    State _state;
    bool _enabled;

    unsigned long _lastToggleTime;
    bool _blinkState;
    unsigned long _stateStartTime;
    bool _stateInitialized;
};

#endif // LED_MANAGER_H
