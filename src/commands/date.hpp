#pragma once

#include <base.hpp>
#include <standard.hpp>

const char *__week_days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

class DateCommand : public BaseCommand
{
public:
    DateCommand()
        : BaseCommand(
              "date",
              "Retrieves the current system date and time",
              "",
              "date",
              {}) {}

    DWORD run(const Context &context)
    {
        SYSTEMTIME time;

        GetSystemTime(&time);
        std::cout << "System time (UTC): "
                  << __week_days[time.wDayOfWeek] << " " << time.wDay << "/" << time.wMonth << "/" << time.wYear << " "
                  << time.wHour << ":" << time.wMinute << ":" << time.wSecond << std::endl;

        GetLocalTime(&time);
        std::cout << "Local time: "
                  << __week_days[time.wDayOfWeek] << " " << time.wDay << "/" << time.wMonth << "/" << time.wYear << " "
                  << time.wHour << ":" << time.wMinute << ":" << time.wSecond << std::endl;

        return 0;
    }
};
