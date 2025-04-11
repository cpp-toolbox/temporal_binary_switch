#ifndef TEMPORAL_BINARY_SWITCH_HPP
#define TEMPORAL_BINARY_SWITCH_HPP

class TemporalBinarySwitch {
  private:
    bool state = false;
    bool m_just_switched_on = false;
    bool m_just_switched_off = false;

  public:
    TemporalBinarySwitch() {}

    void set_true() {
        if (!state) {
            m_just_switched_on = true;
            m_just_switched_off = false;
        } else {
            m_just_switched_on = false;
        }
        state = true;
    }

    void set_false() {
        if (state) {
            m_just_switched_off = true;
            m_just_switched_on = false;
        } else {
            m_just_switched_off = false;
        }
        state = false;
    }

    bool just_switched_on() const { return m_just_switched_on; }
    bool just_switched_off() const { return m_just_switched_off; }

    bool just_switched_on_temporal() {
        if (m_just_switched_on) {
            m_just_switched_on = false;
            return true;
        }
        return false;
    }

    bool just_switched_off_temporal() {
        if (m_just_switched_off) {
            m_just_switched_off = false;
            return true;
        }
        return false;
    }
};

#endif // TEMPORAL_BINARY_SWITCH_HPP
