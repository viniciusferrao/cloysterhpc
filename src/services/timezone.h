//
// Created by Vinícius Ferrão on 11/01/22.
//

#ifndef CLOYSTERHPC_TIMEZONE_H
#define CLOYSTERHPC_TIMEZONE_H

#include <string>

class Timezone {
private:
    std::string m_timezone;

public:
    void setTimezone(std::string_view);
    std::string_view getTimezone();
    void setSystemTimezone();
    std::vector<std::string> getAvailableTimezones();
};


#endif //CLOYSTERHPC_TIMEZONE_H
