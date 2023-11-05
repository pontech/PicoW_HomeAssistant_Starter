#ifndef SRC_HAINTEGRATION
#define SRC_HAINTEGRATION

#include <ArduinoHA.h>

class HAIntegration {
    public:
    void loop();
    void configure();
    static void switchHandler(bool state, HASwitch* sender);
    static void numberHandler(HANumeric number, HANumber* sender);
};

#endif // SRC_HAINTEGRATION
