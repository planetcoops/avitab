/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "NavigraphApp.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace avitab {

NavigraphApp::NavigraphApp(FuncsPtr appFuncs):
    App(appFuncs),
    window(std::make_shared<Window>(getUIContainer(), "Navigraph")),
    navigraphApi(std::make_shared<navigraph::NavigraphAPI>(appFuncs->getNavigraph()))
{
    window->setOnClose([this] () { exit(); });
    reset();
}

void NavigraphApp::reset() {
    label.reset();
    label = std::make_shared<Label>(window, "This app allows you to use your Navigraph account to access the chart cloud.\n"
                                            "For more information about Navigraph, see navigraph.com\n"
                                            "Clicking login will open your browser and prompt you to login to navigraph\n"
            );
    label->alignInTopLeft();

    button.reset();
    button = std::make_shared<Button>(window, "Login");
    button->alignBelow(label);
    button->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onLoginButton(); });
    });
}

void NavigraphApp::onLoginButton() {
    button.reset();

    auto navigraph = api().getNavigraph();
    if (navigraph->canRelogin()) {
        label->setText("Please wait...");
        relogin();
        return;
    }

    auto link = navigraph->startAuth([this] () {
        api().executeLater([this] () {
            onAuthSuccess();
        });
    });
    platform::openBrowser(link);
    logger::info("Navigraph login link: %s", link.c_str());
    label->setText("Follow the instructions in your browser.\n"
                   "If your browser didn't start, manually open the link in AviTab's log file");

    button = std::make_shared<Button>(window, "Cancel");
    button->alignBelow(label);
    button->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onCancelLoginButton(); });
    });
}

void NavigraphApp::onCancelLoginButton() {
    auto navigraph = api().getNavigraph();
    navigraph->cancelAuth();
    reset();
}

void NavigraphApp::relogin() {
    auto navigraph = api().getNavigraph();

    try {
        navigraph->relogin([this] () {
            api().executeLater([this] () {
                onAuthSuccess();
            });
        });
    } catch (const std::exception &e) {
        label->setText(std::string("Error: ") + e.what());
        button.reset();
        button = std::make_shared<Button>(window, "Login");
        button->alignBelow(label);
        button->setCallback([this] (const Button &btn) {
            api().executeLater([this] () { onLoginButton(); });
        });
    }
}

void NavigraphApp::onAuthSuccess() {
    auto navigraph = api().getNavigraph();

    button.reset();

    navigraphApi->load();
    if (navigraphApi->isInDemoMode()) {
        label->setText("You are logged in but don't have a charts subscription. Only LEAL and KONT are usable in demo mode.");
    } else {
        label->setText("You are now logged in with a valid chart subscription!");
    }
}

} /* namespace avitab */
