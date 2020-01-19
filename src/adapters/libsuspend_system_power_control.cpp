/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "libsuspend_system_power_control.h"
#include "libsuspend/libsuspend.h"

#include "src/core/log.h"

#include <stdexcept>

namespace
{
char const* const log_tag = "LibsuspendSystemPowerControl";
}

repowerd::LibsuspendSystemPowerControl::LibsuspendSystemPowerControl(
    std::shared_ptr<Log> const& log)
    : log{log}
{
    libsuspend_init(0);

    if (std::string{libsuspend_getname()} == "mocksuspend")
        throw std::runtime_error{"Failed to initialize libsuspend"};

    log->log(log_tag, "Initialized using backend %s", libsuspend_getname());
}

void repowerd::LibsuspendSystemPowerControl::start_processing()
{
}

repowerd::HandlerRegistration
repowerd::LibsuspendSystemPowerControl::register_system_resume_handler(
    SystemResumeHandler const&)
{
    return HandlerRegistration{};
}

repowerd::HandlerRegistration
repowerd::LibsuspendSystemPowerControl::register_system_allow_suspend_handler(
    SystemAllowSuspendHandler const&)
{
    return HandlerRegistration{};
}

repowerd::HandlerRegistration
repowerd::LibsuspendSystemPowerControl::register_system_disallow_suspend_handler(
    SystemDisallowSuspendHandler const&)
{
    return HandlerRegistration{};
}

void repowerd::LibsuspendSystemPowerControl::allow_automatic_suspend(
    std::string const& id)
{
    std::lock_guard<std::mutex> lock{suspend_mutex};

    log->log(log_tag, "allow_suspend(%s)", id.c_str());
    return;

    if (suspend_disallowances.erase(id) > 0 &&
        suspend_disallowances.empty())
    {
        log->log(log_tag, "Preparing for suspend");
        libsuspend_prepare_suspend();
        libsuspend_enter_suspend();
    }
}

void repowerd::LibsuspendSystemPowerControl::disallow_automatic_suspend(
    std::string const& id)
{
    std::lock_guard<std::mutex> lock{suspend_mutex};

    log->log(log_tag, "disallow_suspend(%s)", id.c_str());

    return;
    auto const could_be_suspended = suspend_disallowances.empty();

    suspend_disallowances.insert(id);

    if (could_be_suspended)
    {
        log->log(log_tag, "exiting suspend");
        libsuspend_exit_suspend();
    }
}

void repowerd::LibsuspendSystemPowerControl::power_off()
{
    log->log(log_tag, "power_off()");

    if (system("shutdown -P now")) {}
}

void repowerd::LibsuspendSystemPowerControl::suspend()
{
}

void repowerd::LibsuspendSystemPowerControl::allow_default_system_handlers()
{
}

void repowerd::LibsuspendSystemPowerControl::disallow_default_system_handlers()
{
}
