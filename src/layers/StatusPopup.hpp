#include <Geode/Geode.hpp>

using namespace geode::prelude;

class StatusPopup : geode::Popup<std::string> {
protected:
    CCLabelBMFont* statusLabel;
    bool setup(std::string title) override;
public:
    void setStatus(std::string status);

    static StatusPopup* create(std::string title);
};
