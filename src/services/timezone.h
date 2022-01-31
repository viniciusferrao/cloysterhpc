//
// Created by Vinícius Ferrão on 11/01/22.
//

#ifndef CLOYSTERHPC_TIMEZONE_H
#define CLOYSTERHPC_TIMEZONE_H

#include <string>

class Timezone {
private:
    std::string m_timezone;
    std::vector<std::string> m_availableTimezones;

public:
    Timezone();
    ~Timezone() = default;

    void setTimezone(std::string_view);
    std::string_view getTimezone() const;
    void setSystemTimezone();
    std::vector<std::string> fetchAvailableTimezones();
    std::vector<std::string> getAvailableTimezones() const;

};


#endif //CLOYSTERHPC_TIMEZONE_H
