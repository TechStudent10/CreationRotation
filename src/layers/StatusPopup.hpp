#include <Geode/Geode.hpp>

using namespace geode::prelude;

class StatusPopup : geode::Popup {
protected:
    CCLabelBMFont* statusLabel = nullptr;
    bool init(ZStringView title);

public:
    void setStatus(ZStringView status);

    static StatusPopup* create(ZStringView title);
};
